#pragma once

#include <bgfx/bgfx.h>

#define MAX_SORT_BUFFERS 64
#define SORT_BUFFER_VERT_COUNT 4096 * 3
#define MAX_SORT_DRAW_COMMANDS 32768

#define MAX_DRAW_COMMANDS 8

extern uint32_t bgfx_clear_col;

extern float bgfx_fog_color[4];
extern float bgfx_volumetric_fog_color[4];
extern float bgfx_fog_parameters[4];

extern uint32_t total_sort_verts_in_current_buffer;
extern uint32_t last_sort_command_idx;
extern uint32_t last_sort_vertex_buffer_idx;
extern uint32_t last_sort_vertex_buffer_offset;

extern bgfx::ProgramHandle m_outputVTLTexProgram;
extern bgfx::ProgramHandle m_outputVTLTexAlphaClippedProgram;
extern bgfx::ProgramHandle m_outputVTLTexAlphaBlendedProgram;
extern bgfx::ProgramHandle m_outputVTLAlphaProgram;

extern bgfx::UniformHandle s_texColor;
extern bgfx::VertexLayout ms_outputBucketVertexLayout;

extern GFXTLBUMPVERTEX* sort_buffer_vertex_buffer;
extern bgfx::DynamicVertexBufferHandle sort_buffer_vertex_handle[MAX_SORT_BUFFERS];

struct BGFXSortDrawCommand {
	uint32_t draw_type = 0;
	uint32_t buffer_offset = 0;
	uint32_t buffer_id = 0;
	uint32_t count = 0;
	bgfx::TextureHandle texture;
};

struct BGFXDrawCommand {
	bool is_sorted_command = false;
	bool clear_depth_buffer = false;
	size_t last_idx = 0;
};

extern BGFXSortDrawCommand *sort_draw_commands;

extern void SetupOutputBucketVertexLayout();

extern void InitializeBGFX();
extern void ShutdownBGFX();

extern void SetupBGFXOutputPolyList();
extern void RenderBGFXDrawLists();

extern void StartBGFXFrame();
extern void EndBGFXFrame();

extern void AddBGFXDrawCommand(bool is_sorted_command, bool clear_depth_buffer);
extern void AddBGFXDrawSortCommand(GFXTLBUMPVERTEX* info, int16_t num_verts, int16_t texture, int16_t type);
extern void ClearBGFXDrawCommand();

extern void FindBGFXBucket(int32_t tpage, GFXTLBUMPVERTEX** Vpp, int32_t** nVtxpp);
extern void AddBGFXSortList(bool clear_depth_buffer);