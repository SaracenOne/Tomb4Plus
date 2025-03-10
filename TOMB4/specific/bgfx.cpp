#include "../tomb4/pch.h"
#include <bx/readerwriter.h>
#include "platform.h"
#include "winmain.h"
#include "drawroom.h"
#include "bgfx.h"
#include "function_stubs.h"
#include "texture.h"
#include "polyinsert.h"
#include "../game/gameflow.h"
#include "../tomb4/tomb4.h"
#include "../tomb4/mod_config.h"
#include "3dmath.h"
#include "file.h"
#include "../game/trng/trng_extra_state.h"

uint32_t bgfx_clear_col = 0x00000000;

enum BGFXFogParameters {
	BGFX_FOG_START_PARAMETER = 0,
	BGFX_FOG_END_PARAMETER,
	BGFX_FOG_PLACEHOLDER_1_PARAMETER,
	BGFX_FOG_PLACEHOLDER_2_PARAMETER,
	BGFX_FOG_PARAMETER_COUNT
};

float bgfx_fog_color[4];
float bgfx_volumetric_fog_color[4];
float bgfx_fog_parameters[BGFX_FOG_PARAMETER_COUNT];

bgfx::ProgramHandle m_outputVTLTexProgram = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle m_outputVTLTexAlphaClippedProgram = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle m_outputVTLTexAlphaBlendedProgram = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle m_outputVTLAlphaProgram = BGFX_INVALID_HANDLE;

bgfx::UniformHandle s_texColor;
bgfx::VertexLayout ms_outputBucketVertexLayout;

bgfx::UniformHandle u_fogColor;
bgfx::UniformHandle u_volumetricFogColor;
bgfx::UniformHandle u_fogParameters;

uint32_t total_sort_verts_in_current_buffer = 0;
uint32_t first_bucket_command_idx = 0;
uint32_t last_bucket_command_idx = 0;
uint32_t last_sort_command_idx = 0;
uint32_t last_sort_vertex_buffer_idx = 0;
uint32_t last_sort_vertex_buffer_offset = 0;

GFXTLBUMPVERTEX *sort_buffer_vertex_buffer = nullptr;

const bgfx::Memory *sort_buffer_vertex_buffers_ref[MAX_SORT_BUFFERS];
bgfx::DynamicVertexBufferHandle sort_buffer_vertex_handle[MAX_SORT_BUFFERS];

BGFXSortDrawCommand *sort_draw_commands = nullptr;
BGFXDrawCommand draw_commands[MAX_DRAW_COMMANDS];

size_t current_draw_commands = 0;

void SetupOutputBucketVertexLayout() {
	ms_outputBucketVertexLayout
	.begin()
	.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, true)
	.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
	.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
	.end();
}

static const bgfx::Memory* loadMem(const char* _filePath) {
	FILE* fin = platform_fopen(_filePath, "rb");

	if (fin == nullptr) {
		printf("%s not found\n", _filePath);
		fflush(stdout);
		return NULL;
	}

	if (fseek(fin, 0, SEEK_END) != 0) {
	}

	int32_t file_size = ftell(fin);
	if (file_size == -1) {
	}

	uint32_t size = file_size;
	const bgfx::Memory* mem = bgfx::alloc(size + 1);

	rewind(fin);

	size_t read_amount = fread(mem->data, 1, size, fin);
	if (read_amount != size) {
		Log(0, "Failed to read BGFX data.");
	}

	fclose(fin);

	mem->data[mem->size - 1] = '\0';

	return mem;
}

bgfx::ShaderHandle loadShader(const char* _name) {
	char filePath[512];

	const char* shaderPath = "???";

	switch (bgfx::getRendererType()) {
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			shaderPath = "shaders/dx11/";
			break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:
			shaderPath = "shaders/pssl/";
			break;
		case bgfx::RendererType::Metal:
			shaderPath = "shaders/metal/";
			break;
		case bgfx::RendererType::Nvn:
			shaderPath = "shaders/nvn/";
			break;
		case bgfx::RendererType::OpenGL:
			shaderPath = "shaders/glsl/";
			break;
		case bgfx::RendererType::OpenGLES:
			shaderPath = "shaders/essl/";
			break;
		case bgfx::RendererType::Vulkan:
			shaderPath = "shaders/spirv/";
			break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
	}

	bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
	bx::strCat(filePath, BX_COUNTOF(filePath), _name);
	bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

	const bgfx::Memory *shader_memory = loadMem(filePath);
	if (shader_memory) {
		bgfx::ShaderHandle handle = bgfx::createShader(shader_memory);
		bgfx::setName(handle, _name);

		return handle;
	} else {
		platform_fatal_error("Failed to load shader at path: '%s'", filePath);
	}

	return BGFX_INVALID_HANDLE;
}

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName) {
	bgfx::ShaderHandle vsh = loadShader(_vsName);
	bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
	if (NULL != _fsName) {
		fsh = loadShader(_fsName);
	}

	return bgfx::createProgram(vsh, fsh, true);
}

// The thread this gets called from becomes the API thread.
void InitializeBGFX() {
	bgfx::renderFrame();

	bgfx::Init init;
	init.type = bgfx::RendererType::OpenGL;
	init.vendorId = BGFX_PCI_ID_NONE;
	init.platformData.nwh = SDLGetNativeWindowHandle(sdl_window);
	init.platformData.ndt = SDLGetNativeDisplayHandle(sdl_window);
	init.resolution.width = App.dx.dwRenderWidth;
	init.resolution.height = App.dx.dwRenderHeight;
	init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X16;
	if (!bgfx::init(init)) {
		platform_fatal_error("Could not create BGFX API context!");
	}

	//bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, bgfx_clear_col, 1.0f, 0);
	bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

	SetupOutputBucketVertexLayout();
	for (int i = 0; i < MAX_BUCKETS; i++) {
		TEXTUREBUCKET *bucket = &Bucket[i];
		bucket->handle = bgfx::createDynamicVertexBuffer(BUCKET_VERT_COUNT, ms_outputBucketVertexLayout);
	}

	sort_draw_commands = (BGFXSortDrawCommand *)SYSTEM_MALLOC(MAX_SORT_DRAW_COMMANDS * sizeof(BGFXSortDrawCommand));
	sort_buffer_vertex_buffer = (GFXTLBUMPVERTEX*) SYSTEM_MALLOC(SORT_BUFFER_VERT_COUNT * MAX_SORT_BUFFERS * sizeof(GFXTLBUMPVERTEX));

	for (int i = 0; i < MAX_SORT_BUFFERS; i++) {
		sort_buffer_vertex_handle[i] = bgfx::createDynamicVertexBuffer(SORT_BUFFER_VERT_COUNT, ms_outputBucketVertexLayout);
	}

	u_fogColor = bgfx::createUniform("u_fogColor", bgfx::UniformType::Vec4);
	u_volumetricFogColor = bgfx::createUniform("u_volumetricFogColor", bgfx::UniformType::Vec4);
	u_fogParameters = bgfx::createUniform("u_fogParameters", bgfx::UniformType::Vec4);

	m_outputVTLTexProgram = loadProgram("vs_vtl_tex", "fs_vtl_tex");
	if (App.Filtering) {
		m_outputVTLTexAlphaClippedProgram = loadProgram("vs_vtl_tex_alpha_clipped_filter", "fs_vtl_tex_alpha_clipped_filter");
	} else {
		m_outputVTLTexAlphaClippedProgram = loadProgram("vs_vtl_tex_alpha_clipped_point", "fs_vtl_tex_alpha_clipped_point");
	}
	m_outputVTLTexAlphaBlendedProgram = loadProgram("vs_vtl_tex_alpha_blended", "fs_vtl_tex_alpha_blended");
	m_outputVTLAlphaProgram = loadProgram("vs_vtl_alpha", "fs_vtl_alpha");
}

void ShutdownBGFX() {
	bgfx::destroy(m_outputVTLTexProgram);
	bgfx::destroy(m_outputVTLTexAlphaClippedProgram);
	bgfx::destroy(m_outputVTLTexAlphaBlendedProgram);
	bgfx::destroy(m_outputVTLAlphaProgram);

	bgfx::destroy(u_fogColor);
	bgfx::destroy(u_volumetricFogColor);
	bgfx::destroy(u_fogParameters);

	bgfx::shutdown();

	SYSTEM_FREE(sort_draw_commands);
	SYSTEM_FREE(sort_buffer_vertex_buffer);
}

void SetupBGFXOutputPolyList() {
	float view[16];
	float ortho[16];

	const bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
	const bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };

	bx::mtxLookAt(view, eye, at);

	bx::mtxOrtho(ortho, 0.0f, (float)App.dx.dwRenderWidth, (float)App.dx.dwRenderHeight, 0.0f, 0.0f, 2.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

	bgfx::setViewTransform(0, view, ortho);

	bgfx::setViewRect(0, 0, 0, uint16_t(App.dx.dwRenderWidth), uint16_t(App.dx.dwRenderHeight));

}

void RenderBGFXDrawLists() {
	// Hack to prevent pickup display flickering.
	bool multipass_frame = false;

	if (NGGetDrawState() != NG_DRAW_STATE_FROZEN) {
		size_t current_bucket_idx = 0;
		size_t current_sort_idx = 0;

		MOD_LEVEL_ENVIRONMENT_INFO* environment_info = get_game_mod_level_environment_info(gfCurrentLevel);
		if (gfLevelFlags & GF_TRAIN || environment_info->force_train_fog) {
			bgfx_fog_parameters[BGFX_FOG_START_PARAMETER] = float(DEFAULT_FOG_START_BLOCKS);
			bgfx_fog_parameters[BGFX_FOG_END_PARAMETER] = float(DEFAULT_FOG_END_BLOCKS);
		} else {
			bgfx_fog_parameters[BGFX_FOG_START_PARAMETER] = LevelFogStart / float(BLOCK_SIZE);
			bgfx_fog_parameters[BGFX_FOG_END_PARAMETER] = LevelFogEnd / float(BLOCK_SIZE);
		};

		for (size_t i = 0; i < MAX_SORT_BUFFERS; i++) {
			bgfx::update(sort_buffer_vertex_handle[i], 0, sort_buffer_vertex_buffers_ref[i]);
		}

		for (size_t i = 0; i < current_draw_commands; i++) {
			if (draw_commands[i].clear_depth_buffer) {
				bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, bgfx_clear_col, 1.0f, 0);
				bgfx::frame();
				multipass_frame = true;
			}

			if (draw_commands[i].is_sorted_command) {
				for (; current_sort_idx < draw_commands[i].last_idx; current_sort_idx++) {
					uint64_t state = UINT64_C(0);
					bool is_blended = false;

					switch (sort_draw_commands[current_sort_idx].draw_type) {
						case 0: {
							is_blended = false;
							state = 0
							        | BGFX_STATE_WRITE_MASK
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | UINT64_C(0);
							break;
						}
						case 1: {
							is_blended = false;
							state = 0
							        | BGFX_STATE_WRITE_MASK
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | BGFX_STATE_BLEND_ALPHA
							        | UINT64_C(0);
							break;
						}
						case 2: {
							is_blended = true;
							state = 0
							        | BGFX_STATE_WRITE_RGB
							        | BGFX_STATE_DEPTH_TEST_LEQUAL
							        | BGFX_STATE_BLEND_ADD
							        | UINT64_C(0);
							break;
						}
						case 3: {
							is_blended = true;
							state = 0
							        | BGFX_STATE_WRITE_RGB
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | BGFX_STATE_BLEND_ALPHA
							        | UINT64_C(0);
							break;
						}
						case 4: {
							is_blended = false;
							state = 0
							        | BGFX_STATE_WRITE_RGB
							        | BGFX_STATE_BLEND_ALPHA
							        | UINT64_C(0);
							break;
						}
						case 5: {
							is_blended = true;
							state = 0
							        | BGFX_STATE_WRITE_RGB
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_INV_SRC_COLOR)
							        | UINT64_C(0);
							break;
						}
						case 6: {
							is_blended = true;
							state = 0
							        | BGFX_STATE_WRITE_RGB
							        | BGFX_STATE_WRITE_A
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | BGFX_STATE_BLEND_ADD
							        | BGFX_STATE_PT_LINES
							        | UINT64_C(0);
							break;
						}
						case 7: {
							is_blended = true;
							state = 0
							        | BGFX_STATE_WRITE_MASK
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							        | UINT64_C(0);
							break;
						}
						default: {
							state = 0
							        | BGFX_STATE_WRITE_RGB
							        | BGFX_STATE_DEPTH_TEST_LESS
							        | BGFX_STATE_BLEND_ALPHA
							        | UINT64_C(0);
							break;
						}
					}

					bgfx::setState(state);
					bgfx::setVertexBuffer(
					    0,
					    sort_buffer_vertex_handle[sort_draw_commands[current_sort_idx].buffer_id],
					    sort_draw_commands[current_sort_idx].buffer_offset,
					    sort_draw_commands[current_sort_idx].count);

					if (sort_draw_commands[current_sort_idx].texture.idx != 0xffff) {
						bgfx::setTexture(0, s_texColor, sort_draw_commands[current_sort_idx].texture);
						bgfx::setUniform(u_fogColor, bgfx_fog_color);
						bgfx::setUniform(u_volumetricFogColor, bgfx_volumetric_fog_color);
						bgfx::setUniform(u_fogParameters, bgfx_fog_parameters);
						if (is_blended) {
							bgfx::submit(0, m_outputVTLTexAlphaBlendedProgram);
						} else {
							bgfx::submit(0, m_outputVTLTexAlphaClippedProgram);
						}
					} else {
						bgfx::submit(0, m_outputVTLAlphaProgram);
					}
				}
			} else {
				for (; current_bucket_idx < draw_commands[i].last_idx; current_bucket_idx++) {
					TEXTUREBUCKET* bucket = &Bucket[current_bucket_idx];

					if (bucket->tpage == 1)
						bucket->tpage = 1;

					if (!bucket->nVtx)
						continue;

					uint64_t state = 0
					                 | BGFX_STATE_WRITE_RGB
					                 | BGFX_STATE_WRITE_Z
					                 | BGFX_STATE_DEPTH_TEST_LESS
					                 | UINT64_C(0);

					bgfx::update(bucket->handle, 0, bgfx::makeRef(bucket->vtx, BUCKET_VERT_COUNT * sizeof(GFXTLBUMPVERTEX)));

					bgfx::setVertexBuffer(0, bucket->handle, 0, bucket->nVtx);
					bgfx::setTexture(0, s_texColor, Textures[bucket->tpage].tex);
					bgfx::setState(state);
					bgfx::setUniform(u_fogColor, bgfx_fog_color);
					bgfx::setUniform(u_volumetricFogColor, bgfx_volumetric_fog_color);
					bgfx::setUniform(u_fogParameters, bgfx_fog_parameters);

					bgfx::submit(0, m_outputVTLTexProgram);

					bucket->nVtx = 0;
					bucket->tpage = -1;
					DrawPrimitiveCnt++;
				}
			}
		}
	} else {
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, bgfx_clear_col, 1.0f, 0);
		bgfx::frame();
		multipass_frame = true;
	}

	if (multipass_frame) {
		bgfx::setViewClear(0, BGFX_CLEAR_DEPTH, bgfx_clear_col, 1.0f, 0);
		bgfx::frame();
		bgfx::frame();
	} else {
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, bgfx_clear_col, 1.0f, 0);
		bgfx::frame();
	}
}

void StartBGFXFrame() {
	total_sort_verts_in_current_buffer = 0;
	last_sort_command_idx = 0;
	last_sort_vertex_buffer_idx = 0;
	last_sort_vertex_buffer_offset = 0;

	last_bucket_command_idx = 0;
	first_bucket_command_idx = 0;

	ClearBGFXDrawCommand();

	for (int i = 0; i < MAX_SORT_BUFFERS; i++) {
		sort_buffer_vertex_buffers_ref[i] = bgfx::makeRef((i * SORT_BUFFER_VERT_COUNT) + sort_buffer_vertex_buffer, SORT_BUFFER_VERT_COUNT * sizeof(GFXTLBUMPVERTEX));
	}
}

void EndBGFXFrame() {
}

void AddBGFXDrawCommand(bool is_sorted_command, bool clear_depth_buffer) {
	if (current_draw_commands >= MAX_DRAW_COMMANDS) {
		platform_fatal_error("Exceeded maximum draw commands.");
		return;
	}

	draw_commands[current_draw_commands].is_sorted_command = is_sorted_command;
	draw_commands[current_draw_commands].clear_depth_buffer = clear_depth_buffer;

	if (is_sorted_command) {
		draw_commands[current_draw_commands].last_idx = last_sort_command_idx;
	} else {
		draw_commands[current_draw_commands].last_idx = last_bucket_command_idx;
		first_bucket_command_idx = last_bucket_command_idx;
	}

	current_draw_commands++;
}

void AddBGFXDrawSortCommand(GFXTLBUMPVERTEX* info, int16_t num_verts, int16_t texture, int16_t type) {
	if (last_sort_command_idx >= MAX_SORT_DRAW_COMMANDS) {
		platform_fatal_error("Overrun max sort commands.");
		return;
	}

	sort_draw_commands[last_sort_command_idx].count = num_verts;
	sort_draw_commands[last_sort_command_idx].buffer_offset = last_sort_vertex_buffer_offset;
	sort_draw_commands[last_sort_command_idx].buffer_id = last_sort_vertex_buffer_idx;
	sort_draw_commands[last_sort_command_idx].texture = Textures[texture].tex;
	sort_draw_commands[last_sort_command_idx].draw_type = type;

	last_sort_vertex_buffer_offset += num_verts;
	last_sort_command_idx++;

	DrawPrimitiveCnt++;
}

void ClearBGFXDrawCommand() {
	current_draw_commands = 0;
}

void FindBGFXBucket(int32_t tpage, GFXTLBUMPVERTEX** Vpp, int32_t** nVtxpp) {
	TEXTUREBUCKET* bucket;

	for (size_t i = first_bucket_command_idx; i < last_bucket_command_idx; i++) {
		bucket = &Bucket[i];

		if (bucket->tpage == tpage && bucket->nVtx < BUCKET_VERT_COUNT - 32) {
			*Vpp = &bucket->vtx[bucket->nVtx];
			*nVtxpp = &bucket->nVtx;

			return;
		}
	}

	for (uint32_t i = last_bucket_command_idx; i < MAX_BUCKETS; i++) {
		bucket = &Bucket[i];

		if (bucket->tpage == -1) {
			bucket->tpage = tpage;
			*Vpp = bucket->vtx;
			*nVtxpp = &bucket->nVtx;

			if (i >= last_bucket_command_idx) {
				last_bucket_command_idx = ++i;
			}

			return;
		}
	}

	platform_fatal_error("Max texture bucket count exceeded.");
}

void AddBGFXSortList(bool clear_depth_buffer) {
	SORTLIST* pSort;
	GFXTLBUMPVERTEX* vtx;
	GFXTLBUMPVERTEX* bVtx;
	int32_t num;
	int16_t nVtx = 0, tpage, drawtype;

	if (!SortCount)
		return;

	bgfx::setState(BGFX_STATE_BLEND_ALPHA);
	bgfx::setState(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
	{
		pSort = SortList[0];

		for (num = 0; num < SortCount; num++) {
			pSort = SortList[num];

			if (pSort->drawtype == 0 || pSort->drawtype == 1 || pSort->drawtype == 4)
				break;
		}

		bVtx = &sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset];

		tpage = pSort->tpage;
		drawtype = pSort->drawtype;

		for (; num < SortCount; num++) {
			pSort = SortList[num];

			if (pSort->drawtype == 0 || pSort->drawtype == 1 || pSort->drawtype == 4) {
				if (pSort->drawtype == drawtype && pSort->tpage == tpage) {
					vtx = (GFXTLBUMPVERTEX*)(pSort + 1);
					total_sort_verts_in_current_buffer += pSort->nVtx;

					if (total_sort_verts_in_current_buffer >= SORT_BUFFER_VERT_COUNT) {
						AddBGFXDrawSortCommand(&sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset], nVtx, tpage, drawtype);
						nVtx = 0;

						last_sort_vertex_buffer_offset = 0;
						last_sort_vertex_buffer_idx++;

						bVtx = &sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset];
						total_sort_verts_in_current_buffer = 0;
					}

					for (int i = 0; i < pSort->nVtx; i++, vtx++, bVtx++) {
						bVtx->sx = vtx->sx;
						bVtx->sy = vtx->sy;
						bVtx->sz = vtx->sz;
						bVtx->rhw = vtx->rhw;
						bVtx->color = vtx->color;
						bVtx->specular = vtx->specular;
						bVtx->tu = vtx->tu;
						bVtx->tv = vtx->tv;
						bVtx->tx = vtx->tx;
						bVtx->ty = vtx->ty;
						nVtx++;
					}
				} else {
					AddBGFXDrawSortCommand(&sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset], nVtx, tpage, drawtype);	//inlined
					drawtype = pSort->drawtype;
					tpage = pSort->tpage;

					while (last_sort_vertex_buffer_offset + pSort->nVtx >= SORT_BUFFER_VERT_COUNT - 4) {
						last_sort_vertex_buffer_offset = 0;
						last_sort_vertex_buffer_idx++;

						if (last_sort_vertex_buffer_idx >= MAX_SORT_BUFFERS) {
							platform_fatal_error("Overlapped max vertex sort buffers.");
							return;
						}
					}

					bVtx = &sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset];
					nVtx = 0;
					num--;
				}
			}
		}


		if (nVtx)
			AddBGFXDrawSortCommand(&sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset], nVtx, tpage, drawtype);	//inlined

		for (num = SortCount - 1; num >= 0; num--) {
			pSort = SortList[num];

			if (pSort->drawtype == 2 || pSort->drawtype == 3 || pSort->drawtype == 5 || pSort->drawtype == 6 || pSort->drawtype == 7)
				break;
		}

		tpage = pSort->tpage;
		drawtype = pSort->drawtype;
		bVtx = &sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset];
		nVtx = 0;

		for (; num >= 0; num--) {
			pSort = SortList[num];

			if (pSort->drawtype == 2 || pSort->drawtype == 3 || pSort->drawtype == 5 || pSort->drawtype == 6 || pSort->drawtype == 7) {
				if (pSort->tpage == tpage && pSort->drawtype == drawtype) {
					vtx = (GFXTLBUMPVERTEX*)(pSort + 1);
					total_sort_verts_in_current_buffer += pSort->nVtx;

					if (total_sort_verts_in_current_buffer >= SORT_BUFFER_VERT_COUNT) {
						AddBGFXDrawSortCommand(&sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset], nVtx, tpage, drawtype);
						nVtx = 0;

						last_sort_vertex_buffer_offset = 0;
						last_sort_vertex_buffer_idx++;

						bVtx = &sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset];
						total_sort_verts_in_current_buffer = 0;
					}

					for (int i = 0; i < pSort->nVtx; i++, vtx++, bVtx++) {
						bVtx->sx = vtx->sx;
						bVtx->sy = vtx->sy;
						bVtx->sz = vtx->sz;
						bVtx->rhw = vtx->rhw;
						bVtx->color = vtx->color;
						bVtx->specular = vtx->specular;
						bVtx->tu = vtx->tu;
						bVtx->tv = vtx->tv;
						bVtx->tx = vtx->tx;
						bVtx->ty = vtx->ty;
						nVtx++;
					}
				} else {
					AddBGFXDrawSortCommand(&sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset], nVtx, tpage, drawtype);	//inlined
					tpage = pSort->tpage;
					nVtx = 0;
					drawtype = pSort->drawtype;

					bVtx = &sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset];
					num++;
				}
			}
		}

		if (nVtx)
			AddBGFXDrawSortCommand(&sort_buffer_vertex_buffer[(SORT_BUFFER_VERT_COUNT * last_sort_vertex_buffer_idx) + last_sort_vertex_buffer_offset], nVtx, tpage, drawtype);	//inlined
	}

	AddBGFXDrawCommand(true, clear_depth_buffer);
}