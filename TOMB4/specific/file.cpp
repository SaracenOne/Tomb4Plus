#include "../tomb4/pch.h"

#include <vector>

#include <zlib.h>

#include "../specific/gamemain.h"
#include "file.h"
#include "function_stubs.h"
#include "texture.h"
#include "lighting.h"
#include "dxsound.h"
#include "drawbars.h"
#include "dxshell.h"
#include "drawroom.h"
#include "../game/setup.h"
#include "../game/objects.h"
#include "../game/laraskin.h"
#include "../game/items.h"
#include "specificfx.h"
#include "../game/tomb4fx.h"
#include "audio.h"
#include "../game/spotcam.h"
#include "../game/control.h"
#include "../game/camera.h"
#include "polyinsert.h"
#include "../game/box.h"
#include "../game/draw.h"
#include "../game/effect2.h"
#include "../game/effects.h"
#include "../game/sound.h"
#include "winmain.h"
#include "../game/lara.h"
#include "output.h"
#include "../game/gameflow.h"

#include "../game/trng/trng.h"
#include "../tomb4/tomb4plus/t4plus_weather.h"
#include "../tomb4/mod_config.h"
#include "cmdline.h"
#include "platform.h"

#include "dirent.h"

TEXTURESTRUCT* textinfo;
SPRITESTRUCT* spriteinfo;
THREAD LevelLoadingThread;

TEXTURESTRUCT* AnimatingWaterfalls[3];
int32_t AnimatingWaterfallsV[3];

int32_t num_meshes = 0;
int32_t num_anims = 0;

CHANGE_STRUCT* changes;
RANGE_STRUCT* ranges;
AIOBJECT* AIObjects;
int16_t* aranges;
int16_t* frames;
int16_t* commands;
int16_t* floor_data;
int16_t* mesh_base;
int32_t nAnimUVRanges;
int32_t number_cameras;
int16_t nAIObjects;

size_t mesh_mapping_table_count = 0;
MESH_MAP_TABLE_ENTRY* mesh_mapping_table = nullptr;

static FILE* level_fp = nullptr;
static char* FileData = nullptr;
static char* UncompressedData = nullptr;
static char* CompressedData = nullptr;
static int32_t num_items = 0;
static bool file_loading_failed = false;

void RGB_Swizzle(uint8_t* r, uint8_t* g, uint8_t* b) {
	uint8_t r2 = *r;
	uint8_t g2 = *g;
	uint8_t b2 = *b;

	*r = b2;
	*g = g2;
	*b = r2;
}

int LoadLevel(void* name) {
	OBJECT_INFO* obj;
	TEXTURESTRUCT* tex;
	char* pData;
	int32_t version, size, compressedSize;
	int16_t RTPages, OTPages, BTPages;

	Log(2, "LoadLevel");
	FreeLevel();
	memset(malloc_ptr, 0, MALLOC_SIZE);
	memset(&lara, 0, sizeof(LARA_INFO));

	Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
	nTextures = 1;
	Textures[0].tex = BGFX_INVALID_HANDLE;
	Textures[0].width = 0;
	Textures[0].height = 0;
	Textures[0].bump = 0;

	S_InitLoadBar(20);
	S_LoadBar();

	CompressedData = nullptr;
	UncompressedData = nullptr;
	FileData = nullptr;

	level_fp = T4PFileOpen((const char*)name);

	if (level_fp) {
		NGLoadLevelInfo(level_fp);

		fseek(level_fp, 0, SEEK_SET);

		fread(&version, 1, 4, level_fp);
		if (version != 0x345254) {
			if (version == 0x63345254) {
				platform_fatal_error("Level %s is encrypted. Encrypted levels are not currently supported. Please decrypt the level and try again.", name);
				return -1;
			} else {
				platform_fatal_error("Level %s has an invalid ident and failed to load", name);
				return -1;
			}
		}

		fread(&RTPages, 1, 2, level_fp);
		fread(&OTPages, 1, 2, level_fp);
		fread(&BTPages, 1, 2, level_fp);

		Log(7, "Process Level Data");
		LoadTextures(RTPages, OTPages, BTPages);
		fread(&size, 1, 4, level_fp);
		fread(&compressedSize, 1, 4, level_fp);

		if (compressedSize <= 0 || size <= 0) {
			platform_fatal_error("Level %s format is invalid.", name);
			return -1;
		}

		CompressedData = (char*)SYSTEM_MALLOC(compressedSize);
		UncompressedData = (char*)SYSTEM_MALLOC(size);

		if (!CompressedData || !UncompressedData) {
			platform_fatal_error("Failed to allocate memory for level %s.", name);
			return -1;
		}

		FileData = UncompressedData;

		fread(CompressedData, compressedSize, 1u, level_fp);
		Decompress(FileData, CompressedData, compressedSize, size);
		SYSTEM_FREE(CompressedData);

		pData = FileData;
		S_LoadBar();

		if (!LoadRooms()) {
			platform_fatal_error("Failed to load rooms for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadObjects()) {
			platform_fatal_error("Failed to load objects for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadSprites()) {
			platform_fatal_error("Failed to load sprites for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadCameras()) {
			platform_fatal_error("Failed to load cameras for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadSoundEffects()) {
			platform_fatal_error("Failed to load sound effects for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadBoxes()) {
			platform_fatal_error("Failed to load boxes for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadAnimatedTextures()) {
			platform_fatal_error("Failed to load animated textures for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadTextureInfos()) {
			platform_fatal_error("Failed to load texture infos for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadItems()) {
			platform_fatal_error("Failed to load items for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadAIInfo()) {
			platform_fatal_error("Failed to load AI info for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (!LoadCinematic()) {
			platform_fatal_error("Failed to load cinematic for level %s.", name);
			return -1;
		}
		S_LoadBar();

		if (acm_ready && !App.SoundDisabled)
			LoadSamples();
		SYSTEM_FREE(UncompressedData);
		S_LoadBar();

		for (int i = 0; i < 3; i++) {
			obj = &objects[WATERFALL1 + i];

			if (obj->loaded) {
				tex = &textinfo[mesh_vtxbuf[obj->mesh_index]->gt4[4] & 0x7FFF];
				AnimatingWaterfalls[i] = tex;
				AnimatingWaterfallsV[i] = (int32_t)tex->v1;
			}
		}

		S_LoadBar();
		S_GetUVRotateTextures();

		InitTarget_2();
		S_LoadBar();

		SetupGame();
		S_LoadBar();

		SetFadeClip(0, 1);
		reset_cutseq_vars();
		FileClose(level_fp);
	} else {
		file_loading_failed = true;
	}

	LevelLoadingThread.active = 0;

#ifdef _WIN32
	_endthreadex(1);
#else
	SDL_DetachThread(MainThread.handle);
#endif

	return 1;
}

int32_t S_LoadLevelFile(int32_t num) {
	char name[80];

	Log(2, "S_LoadLevelFile");
	strcpy(name, &gfFilenameWad[gfFilenameOffset[num]]);
	strcat(name, ".TR4");

	for (int i = 0; i < strlen(name); i++) {
		if (name[i] == '\\') {
			name[i] = '/';
		} else {
			name[i] = tolower((uint8_t)name[i]);
		}
	}

	LevelLoadingThread.active = 1;
	LevelLoadingThread.ended = 0;
	LevelLoadingThread.handle = SDL_CreateThread(LoadLevel, "LoadLevel", name);
	if (LevelLoadingThread.handle == NULL) {
		printf("SDL_CreateThread failed: %s\n", SDL_GetError());
		return -1;
	}

	while (LevelLoadingThread.active) {};

	if (file_loading_failed) {
		platform_fatal_error("Level %s could not be loaded.", name);
	}

	return 1;
}

void FreeLevel() {
	MESH_DATA** vbuf;
	MESH_DATA* mesh;
	ROOM_INFO* r;

	Log(2, "FreeLevel");

	for (int i = 0; i < num_level_meshes; i++) {
		vbuf = &mesh_vtxbuf[i];
		mesh = *vbuf;

		if (mesh->Buffer) {
			Log(4, "Released %s @ %x", "Mesh VB", mesh->Buffer);
			SYSTEM_FREE(mesh->Buffer);
			mesh->Buffer = nullptr;
		}
	}

	if (room) {
		for (int i = 0; i < number_rooms; i++) {
			r = &room[i];

			if (r->Buffer) {
				Log(4, "Released %s @ %x", "Source VB", r->Buffer);
				SYSTEM_FREE(r->Buffer);
				r->Buffer = nullptr;
			} else
				Log(1, "%s Attempt To Release NULL Ptr", "Source VB");
		}
	}

	Log(5, "Free Textures");
	FreeTextures();
	Log(5, "Free Lights");
	DXFreeSounds();
	if (OutsideRoomTable)
		SYSTEM_FREE(OutsideRoomTable);
	if (OutsideRoomOffsets)
		SYSTEM_FREE(OutsideRoomOffsets);
	malloc_ptr = malloc_buffer;
	malloc_free = malloc_size;
	reset_virtual_game_malloc_offset();
}

void to_lowercase(char *str) {
	for (int i = 0; str[i]; i++) {
		str[i] = tolower((uint8_t)str[i]);
	}
}

FILE* T4PFileOpen(const char* name) {
#ifdef _WIN32
	std::string full_path = working_dir_path;
	full_path += name;

	// Convert forward slashes to backslashes on Windows
	for (char& c : full_path) {
		if (c == '/') {
			c = '\\';
		}
	}

	Log(5, "FileOpen - %s", full_path.c_str());
	FILE *file = platform_fopen(full_path.c_str(), "rb");

	if (!file)
		Log(1, "Unable To Open %s", full_path.c_str());

	return file;
#else
	// Split path into components
	std::vector<std::string> components;
	const char* start = name;
	while (*start != '\0') {
		// Skip leading slashes
		while (*start == '/') ++start;
		if (*start == '\0') break;

		const char* end = start;
		while (*end != '\0' && *end != '/') ++end;

		components.emplace_back(start, end - start);
		start = end;
	}

	if (components.empty()) {
		Log(1, "Empty path provided: %s", name);
		return nullptr;
	}

	std::string current_path = working_dir_path;
	// Ensure working directory ends with slash
	if (!current_path.empty() && current_path.back() != '/') {
		current_path += '/';
	}

	// Process each path component
	for (size_t i = 0; i < components.size(); ++i) {
		const std::string& target = components[i];
		bool is_last = (i == components.size() - 1);

		// Convert target component to lowercase
		std::string target_lower = target;
		for (char& c : target_lower) c = tolower(c);

		DIR* dir = opendir(current_path.c_str());
		if (!dir) {
			Log(1, "Cannot open directory: %s", current_path.c_str());
			return nullptr;
		}

		std::string found_name;
		bool found = false;

		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			// Convert directory entry name to lowercase
			std::string entry_name = entry->d_name;
			std::string entry_lower = entry_name;
			for (char& c : entry_lower) c = tolower(c);

			if (entry_lower == target_lower) {
				// For non-last components, verify it's a directory
				if (!is_last) {
					std::string test_path = current_path + entry_name;
					struct stat st;
					if (stat(test_path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
						continue;
					}
				}

				found = true;
				found_name = entry_name;
				break;
			}
		}
		closedir(dir);

		if (!found) {
			Log(1, "Component not found: %s in %s", target.c_str(), current_path.c_str());
			return nullptr;
		}

		current_path += found_name;
		if (!is_last) {
			current_path += '/';
		}
	}

	Log(5, "T4PFileOpen - %s", current_path.c_str());
	FILE* file = platform_fopen(current_path.c_str(), "rb");
	if (!file) {
		Log(1, "Unable to open %s", current_path.c_str());
	}
	return file;
#endif
}

void FileClose(FILE* file) {
	Log(2, "FileClose");
	fclose(file);
}

int32_t FileSize(FILE* file) {
	int32_t size;

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

size_t T4PLoadFileAtRelativePath(const char* name, char** dest) {
	FILE* file;
	size_t size, count;

	Log(2, "T4PLoadFileAtRelativePath");
	Log(5, "File - %s", name);
	file = T4PFileOpen(name);

	if (!file)
		return 0;

	size = FileSize(file);
	if (!*dest)
		*dest = (char*)SYSTEM_MALLOC(size);

	count = fread(*dest, 1, size, file);
	Log(5, "Read - %d FileSize - %d", count, size);

	if (count != size) {
		Log(1, "Error Reading File");
		FileClose(file);
		SYSTEM_FREE(*dest);
		return 0;
	}

	FileClose(file);

	return size;
}

bool LoadTextures(int32_t RTPages, int32_t OTPages, int32_t BTPages) {
	bgfx::TextureHandle tSurf;
	uint8_t* TextureData;
	int32_t* d;
	char* pData;
	char* pComp;
	char* s;
	int32_t format, skip, nTex, c;
	size_t compressedSize = 0;
	size_t size = 0;
	uint8_t r, g, b, a;

	Log(2, "LoadTextures");
	nTextures = 1;
	format = 0;
	skip = 4;
	format = 1;

	if (format <= 1) {
		compressedSize = size = 0;
		fread(&size, sizeof(uint32_t), 1, level_fp);
		fread(&compressedSize, sizeof(uint32_t), 1, level_fp);

		CompressedData = (char*)SYSTEM_MALLOC(compressedSize);
		if (!CompressedData) {
			platform_fatal_error("Failed to allocate memory!");
			return false;
		}
		FileData = (char*)SYSTEM_MALLOC(size);
		if (!FileData) {
			platform_fatal_error("Failed to allocate memory!");
			return false;
		}

		fread(CompressedData, compressedSize, 1, level_fp);
		Decompress(FileData, CompressedData, uint32_t(compressedSize), uint32_t(size));

		compressedSize = size = 0;
		fread(&size, sizeof(uint32_t), 1, level_fp);
		fread(&compressedSize, sizeof(uint32_t), 1, level_fp);
		fseek(level_fp, uint32_t(compressedSize), SEEK_CUR);
		SYSTEM_FREE(CompressedData);
	} else {
		compressedSize = size = 0;
		fread(&size, sizeof(uint32_t), 1, level_fp);
		fread(&compressedSize, sizeof(uint32_t), 1, level_fp);
		fseek(level_fp, uint32_t(compressedSize), SEEK_CUR);

		compressedSize = size = 0;
		fread(&size, sizeof(uint32_t), 1, level_fp);
		fread(&compressedSize, sizeof(uint32_t), 1, level_fp);

		CompressedData = (char*)SYSTEM_MALLOC(compressedSize);
		FileData = (char*)SYSTEM_MALLOC(size);
		fread(CompressedData, compressedSize, 1, level_fp);
		Decompress(FileData, CompressedData, uint32_t(compressedSize), uint32_t(size));
		SYSTEM_FREE(CompressedData);
	}

	pData = FileData;

	Log(5, "RTPages %d", RTPages);
	size = RTPages * skip * 0x10000;
	TextureData = (uint8_t*)SYSTEM_MALLOC(size);
	memcpy(TextureData, FileData, size);
	FileData += size;
	S_LoadBar();

	for (int i = 0; i < RTPages; i++) {
		Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
		nTex = nTextures;
		nTextures++;
		tSurf = CreateTexturePage(App.TextureSize, App.TextureSize, 0, (int32_t*)(TextureData + (i * skip * 0x10000)), 0, format);
		Textures[nTex].tex = tSurf;
		Textures[nTex].width = App.TextureSize;
		Textures[nTex].height = App.TextureSize;
		Textures[nTex].bump = 0;
	}

	SYSTEM_FREE(TextureData);

	Log(5, "OTPages %d", OTPages);
	size = OTPages * skip * 0x10000;
	TextureData = (uint8_t*)SYSTEM_MALLOC(size);
	memcpy(TextureData, FileData, size);
	FileData += size;
	S_LoadBar();

	for (int i = 0; i < OTPages; i++) {
		Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
		nTex = nTextures;
		nTextures++;
		tSurf = CreateTexturePage(App.TextureSize, App.TextureSize, 0, (int32_t*)(TextureData + (i * skip * 0x10000)), 0, format);
		Textures[nTex].tex = tSurf;
		Textures[nTex].width = App.TextureSize;
		Textures[nTex].height = App.TextureSize;
		Textures[nTex].bump = 0;
	}

	SYSTEM_FREE(TextureData);
	S_LoadBar();

	Log(5, "BTPages %d", BTPages);

	if (BTPages) {
		size = BTPages * skip * 0x10000;
		TextureData = (uint8_t*)SYSTEM_MALLOC(size);
		memcpy(TextureData, FileData, size);
		FileData += size;

		for (int i = 0; i < BTPages; i++) {
			if (i < (BTPages >> 1))
				tSurf = CreateTexturePage(App.TextureSize, App.TextureSize, 0, (int32_t*)(TextureData + (i * skip * 0x10000)), 0, format);
			else {
				if (!App.BumpMapping)
					break;

				tSurf = CreateTexturePage(App.BumpMapSize, App.BumpMapSize, 0, (int32_t*)(TextureData + (i * skip * 0x10000)), 0, format);
			}

			Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
			nTex = nTextures;
			nTextures++;

			Textures[nTex].tex = tSurf;

			if (i < (BTPages >> 1)) {
				Textures[nTex].width = App.TextureSize;
				Textures[nTex].height = App.TextureSize;
			} else {
				Textures[nTex].width = App.BumpMapSize;
				Textures[nTex].height = App.BumpMapSize;
			}

			Textures[nTex].bump = 1;
			Textures[nTex].bumptpage = nTex + (BTPages >> 1);
		}

		SYSTEM_FREE(TextureData);
	}

	SYSTEM_FREE(pData);

	compressedSize = size = 0;
	fread(&size, sizeof(uint32_t), 1, level_fp);
	fread(&compressedSize, sizeof(uint32_t), 1, level_fp);

	CompressedData = (char*)SYSTEM_MALLOC(compressedSize);
	FileData = (char*)SYSTEM_MALLOC(size);
	fread(CompressedData, compressedSize, 1, level_fp);
	Decompress(FileData, CompressedData, uint32_t(compressedSize), uint32_t(size));
	SYSTEM_FREE(CompressedData);

	pData = FileData;
	TextureData = (uint8_t*)SYSTEM_MALLOC(0x40000);

	if (!gfCurrentLevel) {	//main menu logo
		const char *logo_pak_path = nullptr;

		pComp = 0;
		CompressedData = 0;

		if (Gameflow->Language == US)
			logo_pak_path = "data/uslogo.pak";
		else if (Gameflow->Language == GERMAN)
			logo_pak_path = "data/grlogo.pak";
		else if (Gameflow->Language == FRENCH)
			logo_pak_path = "data/frlogo.pak";
		else
			logo_pak_path = "data/uklogo.pak";

		size = T4PLoadFileAtRelativePath(logo_pak_path, &CompressedData);
		if (size == 0 || size >= UINT32_MAX) {
			logo_pak_path = "data/uklogo.pak";
			// Fallback to uklogo.pak if the corresponding logo failed to load.
			size = T4PLoadFileAtRelativePath(logo_pak_path, &CompressedData);
			if (size == 0 || size >= UINT32_MAX) {
				platform_fatal_error("Failed to load %s", logo_pak_path);
				return false;
			}
		}

		pComp = (char*)SYSTEM_MALLOC(*(int32_t*)CompressedData);
		Decompress(pComp, CompressedData + 4, uint32_t((size - 4) & 0xffffffff), *(int32_t*)CompressedData);
		SYSTEM_FREE(CompressedData);

		for (int i = 0; i < 2; i++) {
			s = pComp + (i * 768);
			d = (int32_t*)TextureData;

			for (int y = 0; y < 256; y++) {
				for (int x = 0; x < 256; x++) {
					b = *(s + (x * 3) + (y * 0x600));
					g = *(s + (x * 3) + (y * 0x600) + 1);
					r = *(s + (x * 3) + (y * 0x600) + 2);
					a = -1;

					if (!r && !b && !g)
						a = 0;

					c = RGBA(r, g, b, a);
					*d++ = c;
				}
			}

			Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
			nTex = nTextures;
			nTextures++;
			tSurf = CreateTexturePage(256, 256, 0, (int32_t*)TextureData, 0, 0);
			Textures[nTex].tex = tSurf;
			Textures[nTex].width = 256;
			Textures[nTex].height = 256;
			Textures[nTex].bump = 0;
		}

		SYSTEM_FREE(pComp);
	}

	//font
	memcpy(TextureData, FileData, 0x40000);
	FileData += 0x40000;


	Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
	nTex = nTextures;
	nTextures++;
	tSurf = CreateTexturePage(256, 256, 0, (int32_t*)TextureData, RGB_Swizzle, 0);

	Textures[nTex].tex = tSurf;
	Textures[nTex].width = 256;
	Textures[nTex].height = 256;
	Textures[nTex].bump = 0;

	//sky
	memcpy(TextureData, FileData, 0x40000);
	FileData += 0x40000;

	Textures = (TEXTURE*)AddStruct(Textures, nTextures, sizeof(TEXTURE));
	nTex = nTextures;
	nTextures++;

	tSurf = CreateTexturePage(256, 256, 0, (int32_t*)TextureData, RGB_Swizzle, 0);

	Textures[nTex].tex = tSurf;
	Textures[nTex].width = 256;
	Textures[nTex].height = 256;
	Textures[nTex].bump = 0;

	SYSTEM_FREE(TextureData);
	SYSTEM_FREE(pData);
	return 1;
}

bool LoadRooms() {
	ROOM_INFO* r;
	int32_t size, nDoors;

	Log(2, "LoadRooms");
	wibble = 0;
	NumLevelFogBulbs = 0;
	FileData += sizeof(int32_t);
	number_rooms = *(int16_t*)FileData;
	FileData += sizeof(int16_t);
	Log(7, "Number Of Rooms %d", number_rooms);

	if (number_rooms < 0 || number_rooms > 1024) {
		Log(1, "Incorrect Number Of Rooms");
		return false;
	}

	room = (ROOM_INFO*)game_malloc(number_rooms * sizeof(ROOM_INFO));
	increment_virtual_game_malloc_offset(number_rooms * 148);

	if (!room)
		return false;

	for (int i = 0; i < number_rooms; i++) {
		r = &room[i];

		r->x = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		r->y = 0;

		r->z = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		r->minfloor = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		r->maxceiling = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		size = *(int32_t*)FileData;
		FileData += sizeof(int32_t);
		r->data = (int16_t*)game_malloc(size * sizeof(int16_t));
		increment_virtual_game_malloc_offset(size * sizeof(int16_t));

		memcpy(r->data, FileData, size * sizeof(int16_t));
		FileData += size * sizeof(int16_t);

		nDoors = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		if (nDoors) {
			r->door = (int16_t*)game_malloc((16 * nDoors + 1) * sizeof(int16_t));
			increment_virtual_game_malloc_offset((16 * nDoors + 1) * sizeof(int16_t));

			r->door[0] = (int16_t)nDoors;
			memcpy(r->door + 1, FileData, 16 * nDoors * sizeof(int16_t));
			FileData += 16 * nDoors * sizeof(int16_t);
		} else
			r->door = 0;

		r->x_size = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		r->y_size = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		size = r->x_size * r->y_size * sizeof(FLOOR_INFO);
		r->floor = (FLOOR_INFO*)game_malloc(size);
		increment_virtual_game_malloc_offset(r->x_size * r->y_size * 8);

		memcpy(r->floor, FileData, size);
		FileData += size;

		r->ambient = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		r->num_lights = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		if (r->num_lights) {
			size = sizeof(LIGHTINFO) * r->num_lights;
			r->light = (LIGHTINFO*)game_malloc(size);
			increment_virtual_game_malloc_offset(46 * r->num_lights);

			memcpy(r->light, FileData, size);
			FileData += size;
		} else
			r->light = 0;

		r->num_meshes = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		if (r->num_meshes) {
			size = sizeof(MESH_INFO) * r->num_meshes;
			r->mesh = (MESH_INFO*)game_malloc(size);
			increment_virtual_game_malloc_offset(22 * r->num_meshes);
			memcpy(r->mesh, FileData, size);
			FileData += size;

			if (ng_level_info[gfCurrentLevel].is_ngle_level) {
				for (int j = 0; j < r->num_meshes; j++)
					r->mesh[j].Flags |= 1;
			} else {
				for (int j = 0; j < r->num_meshes; j++)
					r->mesh[j].Flags = 1;
			}
		} else
			r->mesh = 0;

		r->flipped_room = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		r->flags = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		r->MeshEffect = *(int8_t*)FileData;
		FileData += sizeof(int8_t);

		r->ReverbType = *(int8_t*)FileData;
		FileData += sizeof(int8_t);

		r->FlipNumber = *(int8_t*)FileData;
		FileData += sizeof(int8_t);

		r->left = 0x7FFF;
		r->top = 0x7FFF;
		r->bound_active = 0;
		r->right = 0;
		r->bottom = 0;
		r->item_number = NO_ITEM;
		r->fx_number = NO_ITEM;
		ProcessRoomData(r, using_multi_color_fog_bulbs);
	}

	BuildOutsideTable();
	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	floor_data = (int16_t*)game_malloc(2 * size);
	increment_virtual_game_malloc_offset(2 * size);
	memcpy(floor_data, FileData, 2 * size);
	FileData += sizeof(int16_t) * size;
	Log(0, "Floor Data Size %d @ %x", size, floor_data);
	return 1;
}

bool LoadObjects() {
	OBJECT_INFO* obj;
	STATIC_INFO* stat;
	int16_t** mesh;
	int16_t** mesh_size;
	int32_t size, num, slot;

	Log(2, "LoadObjects");
	memset(objects, 0, sizeof(objects));
	memset(static_objects, 0, sizeof(STATIC_INFO) * NUMBER_STATIC_OBJECTS);

	size = *(int32_t*)FileData;
	if (size <= 0) {
		return false;
	}

	FileData += sizeof(int32_t);
	if (size <= 0) {
		return false;
	}

	mesh_base = (int16_t*)game_malloc(size * sizeof(int16_t));
	increment_virtual_game_malloc_offset(size * sizeof(int16_t));

	memcpy(mesh_base, FileData, size * sizeof(int16_t));
	FileData += size * sizeof(int16_t);

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	meshes = (int16_t**)game_malloc(2 * size * sizeof(int16_t*));
	increment_virtual_game_malloc_offset(2 * size * 4);

#if INTPTR_MAX == INT64_MAX
	{
		int read_amount = 0;
		for (int i = 0; i < size; i++) {
			meshes[i] = (int16_t *)(*(uint32_t*)FileData);
			FileData += sizeof(uint32_t);

			read_amount += sizeof(uint32_t);
		}
	}
#elif INTPTR_MAX == INT32_MAX
	{
		int read_amount = size * sizeof(int16_t*);
		memcpy(meshes, FileData, read_amount);
		FileData += read_amount;
	}
#else
#error Unknown pointer size or missing size macros!
#endif

	for (int i=0; i<size; i++)
		meshes[i] = mesh_base + (size_t)meshes[i] / 2;

	num_meshes = size;

	num_anims = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	anims = (ANIM_STRUCT*)game_malloc(sizeof(ANIM_STRUCT) * num_anims);
	increment_virtual_game_malloc_offset(40 * num_anims);
#if INTPTR_MAX == INT64_MAX
	size_t remaining_struct_size = sizeof(ANIM_STRUCT) - sizeof(size_t);

	for (int i = 0; i < num_anims; i++) {
		anims[i].frame_ptr = (int16_t *)size_t(*(uint32_t*)FileData);
		FileData += sizeof(uint32_t);

		void *offset_ptr = &anims[i].interpolation;

		memcpy(offset_ptr, FileData, remaining_struct_size);
		FileData += remaining_struct_size;
	}
#elif INTPTR_MAX == INT32_MAX
	memcpy(anims, FileData, sizeof(ANIM_STRUCT) * num_anims);
	FileData += sizeof(ANIM_STRUCT) * num_anims;
#else
#error Unknown pointer size or missing size macros!
#endif

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	changes = (CHANGE_STRUCT*)game_malloc(sizeof(CHANGE_STRUCT) * size);
	increment_virtual_game_malloc_offset(6 * size);

	memcpy(changes, FileData, sizeof(CHANGE_STRUCT) * size);
	FileData += sizeof(CHANGE_STRUCT) * size;

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	ranges = (RANGE_STRUCT*)game_malloc(sizeof(RANGE_STRUCT) * size);
	increment_virtual_game_malloc_offset(8 * size);

	memcpy(ranges, FileData, sizeof(RANGE_STRUCT) * size);
	FileData += sizeof(RANGE_STRUCT) * size;

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	commands = (int16_t*)game_malloc(sizeof(int16_t) * size);
	increment_virtual_game_malloc_offset(sizeof(int16_t) * size);

	memcpy(commands, FileData, sizeof(int16_t) * size);
	FileData += sizeof(int16_t) * size;

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	bones = (int32_t*)game_malloc(sizeof(int32_t) * size);
	increment_virtual_game_malloc_offset(sizeof(int32_t) * size);

	memcpy(bones, FileData, sizeof(int32_t) * size);
	FileData += sizeof(int32_t) * size;

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	frames = (int16_t*)game_malloc(sizeof(int16_t) * size);
	increment_virtual_game_malloc_offset(sizeof(int16_t) * size);

	memcpy(frames, FileData, sizeof(int16_t) * size);
	FileData += sizeof(int16_t) * size;

	for (int i = 0; i < num_anims; i++)
		anims[i].frame_ptr = (int16_t*)((size_t)anims[i].frame_ptr + (size_t)frames);

	num = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	for (int i = 0; i < num; i++) {
		slot = *(int32_t*)FileData;
		FileData += sizeof(int32_t);
		obj = &objects[slot];

		obj->nmeshes = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		obj->mesh_index = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		obj->bone_index = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		obj->frame_base = (int16_t*)(*(size_t*)FileData);
		FileData += sizeof(uint32_t);

		obj->anim_index = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		obj->loaded = 1;
	}

	CreateSkinningData();

	for (int i = 0; i < NUMBER_OBJECTS; i++) {
		obj = &objects[i];
		obj->mesh_index *= 2;
	}

	mesh = meshes;
	mesh_size = &meshes[num_meshes];
	memcpy(mesh_size, mesh, num_meshes * sizeof(int16_t *));

	for (int i = 0; i < num_meshes; i++) {
		*mesh++ = *mesh_size;
		*mesh++ = *mesh_size;
		mesh_size++;
	}

	InitialiseObjects();

	num = *(int32_t*)FileData;	//statics
	FileData += sizeof(int32_t);

	for (int i = 0; i < num; i++) {
		slot = *(int32_t*)FileData;
		FileData += sizeof(int32_t);
		stat = &static_objects[slot];

		stat->mesh_number = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		memcpy(&stat->x_minp, FileData, 6 * sizeof(int16_t));
		FileData += 6 * sizeof(int16_t);

		memcpy(&stat->x_minc, FileData, 6 * sizeof(int16_t));
		FileData += 6 * sizeof(int16_t);

		stat->flags = *(int16_t*)FileData;
		FileData += sizeof(int16_t);
	}

	for (int i = 0; i < NUMBER_STATIC_OBJECTS; i++) {
		stat = &static_objects[i];
		stat->mesh_number *= 2;
	}

	ProcessMeshData(num_meshes * 2);
	return 1;
}

bool LoadSprites() {
	STATIC_INFO* stat;
	OBJECT_INFO* obj;
	SPRITESTRUCT* sptr;
	PHDSPRITESTRUCT sprite;
	int32_t num_sprites, num_slots, slot;

	Log(2, "LoadSprites");
	FileData += 3;
	num_sprites = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	spriteinfo = (SPRITESTRUCT*)game_malloc(sizeof(SPRITESTRUCT) * num_sprites);
	increment_virtual_game_malloc_offset(24 * num_sprites);

	for (int i = 0; i < num_sprites; i++) {
		sptr = &spriteinfo[i];
		memcpy(&sprite, FileData, sizeof(PHDSPRITESTRUCT));
		FileData += sizeof(PHDSPRITESTRUCT);
		sptr->height = sprite.height;
		sptr->offset = sprite.offset;
		sptr->tpage = sprite.tpage;
		sptr->width = sprite.width;
		sptr->x1 = float((sprite.x1) * (1.0F / 256.0F));
		sptr->y1 = float((sprite.y1) * (1.0F / 256.0F));
		sptr->x2 = float((sprite.x2) * (1.0F / 256.0F));
		sptr->y2 = float((sprite.y2) * (1.0F / 256.0F));
		sptr->x1 += (1.0F / 256.0F);
		sptr->y1 += (1.0F / 256.0F);
		sptr->x2 -= (1.0F / 256.0F);
		sptr->y2 -= (1.0F / 256.0F);
		sptr->tpage++;
	}

	num_slots = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	if (num_slots <= 0)
		return 1;

	for (int i = 0; i < num_slots; i++) {
		slot = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		if (slot >= NUMBER_OBJECTS) {
			slot -= NUMBER_OBJECTS;
			stat = &static_objects[slot];
			stat->mesh_number = *(int16_t*)FileData;
			FileData += sizeof(int16_t);
			stat->mesh_number = *(int16_t*)FileData;
			FileData += sizeof(int16_t);
		} else {
			obj = &objects[slot];
			obj->nmeshes = *(int16_t*)FileData;
			FileData += sizeof(int16_t);
			obj->mesh_index = *(int16_t*)FileData;
			FileData += sizeof(int16_t);
			obj->loaded = 1;
		}
	}

	return 1;
}

bool LoadCameras() {
	Log(2, "LoadCameras");
	number_cameras = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	if (number_cameras) {
		camera.fixed = (OBJECT_VECTOR*)game_malloc(number_cameras * sizeof(OBJECT_VECTOR));
		increment_virtual_game_malloc_offset(16 * number_cameras);

		memcpy(camera.fixed, FileData, number_cameras * sizeof(OBJECT_VECTOR));
		FileData += number_cameras * sizeof(OBJECT_VECTOR);
	}

	number_spotcams = *(int16_t*)FileData;
	FileData += sizeof(int32_t);				//<<---- look at me

	if (number_spotcams) {
		memcpy(SpotCam, FileData, number_spotcams * sizeof(SPOTCAM));
		FileData += number_spotcams * sizeof(SPOTCAM);
	}

	return 1;
}

bool LoadSoundEffects() {
	Log(2, "LoadSoundEffects");
	number_sound_effects = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	Log(8, "Number of SFX %d", number_sound_effects);

	if (number_sound_effects) {
		sound_effects = (OBJECT_VECTOR*)game_malloc(number_sound_effects * sizeof(OBJECT_VECTOR));
		increment_virtual_game_malloc_offset(16 * number_sound_effects);
		memcpy(sound_effects, FileData, number_sound_effects * sizeof(OBJECT_VECTOR));
		FileData += number_sound_effects * sizeof(OBJECT_VECTOR);
	}

	return 1;
}

bool LoadBoxes() {
	BOX_INFO* box;
	int32_t size;

	Log(2, "LoadBoxes");
	num_boxes = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	boxes = (BOX_INFO*)game_malloc(sizeof(BOX_INFO) * num_boxes);
	increment_virtual_game_malloc_offset(8 * num_boxes);

	memcpy(boxes, FileData, sizeof(BOX_INFO) * num_boxes);
	FileData += sizeof(BOX_INFO) * num_boxes;

	size = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	overlap = (uint16_t*)game_malloc(sizeof(uint16_t) * size);
	increment_virtual_game_malloc_offset(sizeof(uint16_t) * size);

	memcpy(overlap, FileData, sizeof(uint16_t) * size);
	FileData += sizeof(uint16_t) * size;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			ground_zone[j][i] = (int16_t*)game_malloc(sizeof(int16_t) * num_boxes);
			increment_virtual_game_malloc_offset(sizeof(int16_t) * num_boxes);

			memcpy(ground_zone[j][i], FileData, sizeof(int16_t) * num_boxes);
			FileData += sizeof(int16_t) * num_boxes;
		}

		ground_zone[4][i] = (int16_t*)game_malloc(sizeof(int16_t) * num_boxes);
		increment_virtual_game_malloc_offset(sizeof(int16_t) * num_boxes);

		memcpy(ground_zone[4][i], FileData, sizeof(int16_t) * num_boxes);
		FileData += sizeof(int16_t) * num_boxes;
	}

	for (int i = 0; i < num_boxes; i++) {
		box = &boxes[i];

		if (box->overlap_index & 0x8000)
			box->overlap_index |= 0x4000;
		else if (gfLevelFlags & GF_TRAIN && box->height > -256)
			box->overlap_index |= 0xC000;
	}

	return 1;
}

bool LoadAnimatedTextures() {
	int32_t num_anim_ranges;

	num_anim_ranges = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	aranges = (int16_t*)game_malloc(num_anim_ranges * 2);
	increment_virtual_game_malloc_offset(num_anim_ranges * 2);

	memcpy(aranges, FileData, num_anim_ranges * 2);
	FileData += num_anim_ranges * sizeof(int16_t);
	nAnimUVRanges = *(int8_t*)FileData;
	FileData += sizeof(int8_t);
	return 1;
}

bool LoadTextureInfos() {
	TEXTURESTRUCT* t;
	PHDTEXTURESTRUCT tex;
	int32_t val;

	Log(2, "LoadTextureInfos");
	FileData += 3;

	val = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	Log(5, "Texture Infos : %d", val);
	textinfo = (TEXTURESTRUCT*)game_malloc(val * sizeof(TEXTURESTRUCT));
	increment_virtual_game_malloc_offset(val * 38);

	for (int i = 0; i < val; i++) {
		t = &textinfo[i];
		memcpy(&tex, FileData, sizeof(PHDTEXTURESTRUCT));
		FileData += sizeof(PHDTEXTURESTRUCT);
		t->drawtype = tex.drawtype;
		t->tpage = tex.tpage & 0x7FFF;
		t->flag = tex.tpage ^ (tex.tpage ^ tex.flag) & 0x7FFF;
		t->u1 = float(tex.u1) * (1.0F / 65535.0F);
		t->v1 = float(tex.v1) * (1.0f / 65535.0F);
		t->u2 = float(tex.u2) * (1.0f / 65535.0F);
		t->v2 = float(tex.v2) * (1.0f / 65535.0F);
		t->u3 = float(tex.u3) * (1.0f / 65535.0F);
		t->v3 = float(tex.v3) * (1.0f / 65535.0F);
		t->u4 = float(tex.u4) * (1.0f / 65535.0F);
		t->v4 = float(tex.v4) * (1.0f / 65535.0F);
	}

	AdjustUV(val);
	Log(5, "Created %d Texture Pages", nTextures - 1);
	return 1;
}

bool LoadItems() {
	ITEM_INFO* item;
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	STATIC_INFO* stat;
	int32_t x, y, z;

	Log(2, "LoadItems");
	num_items = *(int32_t*)FileData;
	FileData += 4;

	if (!num_items)
		return 1;

	// TRLE: increased item limit
	vanilla_item_malloc_offset = get_virtual_game_malloc_offset();
	items = (ITEM_INFO*)game_malloc(ITEM_COUNT * sizeof(ITEM_INFO));
	increment_virtual_game_malloc_offset(VANILLA_ITEM_COUNT * TR4_VANILLA_ITEM_STRUCT_SIZE);

	level_items = num_items;
	InitialiseItemArray(ITEM_COUNT); // TRLE

	for (int i = 0; i < num_items; i++) {
		item = &items[i];

		item->object_number = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		item->room_number = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		item->pos.x_pos = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		item->pos.y_pos = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		item->pos.z_pos = *(int32_t*)FileData;
		FileData += sizeof(int32_t);

		item->pos.y_rot = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		item->shade = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		item->trigger_flags = *(int16_t*)FileData;
		FileData += sizeof(int16_t);

		item->flags = *(int16_t*)FileData;
		FileData += sizeof(int16_t);
	}

	for (int i = 0; i < num_items; i++)
		InitialiseItem(i);

	for (int i = 0; i < number_rooms; i++) {
		r = &room[i];

		for (int j = 0; j < r->num_meshes; j++) {
			x = (r->mesh[j].x - r->x) >> 10;
			z = (r->mesh[j].z - r->z) >> 10;

			floor = &(r->floor[x * r->x_size + z]);

			if (!(boxes[floor->box].overlap_index & 0x4000) && (gfCurrentLevel != 4 || i != 19 && i != 23 && i != 16)) {
				stat = &static_objects[r->mesh[j].static_number];
				y = floor->floor << 8;

				if (y <= (r->mesh[j].y - stat->y_maxc + HALF_BLOCK_SIZE) && y < r->mesh[j].y - stat->y_minc) {
					if (!stat->x_maxc || !stat->x_minc || !stat->z_maxc || !stat->z_minc ||
					        (stat->x_maxc ^ stat->x_minc) & 0x8000 && (stat->z_maxc ^ stat->z_minc) & 0x8000) {
						x = (r->mesh[j].x - r->x) >> 10;
						z = (r->mesh[j].z - r->z) >> 10;
						r->floor[x * r->x_size + z].stopper = 1;
					}
				}
			}
		}
	}

	return 1;
}

bool LoadCinematic() {
	FileData += sizeof(int16_t);
	return 1;
}

bool LoadAIInfo() {
	int32_t num_ai;

	num_ai = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	if (num_ai) {
		nAIObjects = (int16_t)num_ai;
		AIObjects = (AIOBJECT*)game_malloc(sizeof(AIOBJECT) * num_ai);
		increment_virtual_game_malloc_offset(24 * num_ai);

		memcpy(AIObjects, FileData, sizeof(AIOBJECT) * num_ai);
		FileData += sizeof(AIOBJECT) * num_ai;
	}

	return 1;
}

struct  T4PLUS_RIFF_HEADER {
	int8_t			riff_ident[4];
	uint32_t	chunk_size;
	int8_t			wave_ident[4];
};

struct  T4PLUS_WAV_FORMAT {
	uint16_t	audio_format;
	uint16_t	channels;
	uint32_t	samples_per_second;
	uint32_t	bytes_per_second;
	uint16_t	block_align;
	uint16_t	bits_per_sample;
};

bool LoadSamples() {
	int32_t num_samples, uncomp_size, comp_size;
	static int32_t num_sample_infos;

	int max_samples = MAX_SAMPLES;
	if (ng_level_info[gfCurrentLevel].is_using_global_sound_map) {
		max_samples = MAX_NGLE_SAMPLES;
	}

	Log(2, "LoadSamples");
	sample_lut = (int16_t*)game_malloc(max_samples * sizeof(int16_t));
	increment_virtual_game_malloc_offset(MAX_SAMPLES * sizeof(int16_t));

	memcpy(sample_lut, FileData, max_samples * sizeof(int16_t));
	FileData += max_samples * sizeof(int16_t);
	num_sample_infos = *(int32_t*)FileData;
	FileData += sizeof(int32_t);
	Log(8, "Number Of Sample Infos %d", num_sample_infos);

	if (!num_sample_infos) {
		Log(1, "No Sample Infos");
		return 0;
	}

	sample_infos = (SAMPLE_INFO*)game_malloc(sizeof(SAMPLE_INFO) * num_sample_infos);
	increment_virtual_game_malloc_offset(8 * num_sample_infos);

	memcpy(sample_infos, FileData, sizeof(SAMPLE_INFO) * num_sample_infos);
	FileData += sizeof(SAMPLE_INFO) * num_sample_infos;
	num_samples = *(int32_t*)FileData;
	FileData += sizeof(int32_t);

	if (!num_samples) {
		Log(1, "No Samples");
		return 0;
	}

	Log(8, "Number Of Samples %d", num_samples);
	fread(&num_samples, sizeof(uint32_t), 1, level_fp);
	InitSampleDecompress();

	if (num_samples <= 0) {
		FreeSampleDecompress();
		return 1;
	}

	if (num_samples > MAX_SAMPLE_BUFFERS) {
		num_samples = MAX_SAMPLE_BUFFERS;
		Log(1, "Maximum sample buffers overrun!");
	}

	for (int i = 0; i < num_samples; i++) {
		fread(&uncomp_size, sizeof(uint32_t), 1, level_fp);
		fread(&comp_size, sizeof(uint32_t), 1, level_fp);

		if (get_game_mod_global_info()->tr_use_adpcm_audio) {
			if (comp_size < (DECOMPRESS_BUFFER_LEN + 0x5A)) {
				fread(samples_buffer, comp_size, 1, level_fp);

				if (!DXCreateSampleADPCM(samples_buffer, comp_size, uncomp_size, i)) {
					FreeSampleDecompress();
					return 0;
				}
			} else {
				platform_fatal_error("Sample buffer overrun!");
				return 0;
			}
		} else {
			T4PLUS_RIFF_HEADER header;
			int8_t chunk_1_format[4];
			int chunk_1_size;

			int read_bytes = 0;

			fread(&header, sizeof(T4PLUS_RIFF_HEADER), 1, level_fp);
			read_bytes += sizeof(T4PLUS_RIFF_HEADER);

			fread(&chunk_1_format, sizeof(chunk_1_format), 1, level_fp);
			read_bytes += sizeof(chunk_1_format);
			fread(&chunk_1_size, sizeof(chunk_1_size), 1, level_fp);
			read_bytes += sizeof(chunk_1_size);

			T4PLUS_WAV_FORMAT wav_format;
			int chunk_1_remaining_size = chunk_1_size - sizeof(T4PLUS_WAV_FORMAT);
			fread(&wav_format, sizeof(T4PLUS_WAV_FORMAT), 1, level_fp);
			read_bytes += sizeof(T4PLUS_WAV_FORMAT);

			if (chunk_1_remaining_size > 0) {
				fseek(level_fp, chunk_1_remaining_size, SEEK_CUR);
				read_bytes += chunk_1_remaining_size;
			}

			int8_t chunk_2_format[4];
			int chunk_2_size;
			fread(&chunk_2_format, sizeof(chunk_2_format), 1, level_fp);
			read_bytes += sizeof(chunk_2_format);
			fread(&chunk_2_size, sizeof(chunk_2_size), 1, level_fp);
			read_bytes += sizeof(chunk_2_size);

			if (chunk_2_size > 0 && chunk_2_size < DECOMPRESS_BUFFER_LEN) {
				memset(samples_buffer, 0x00, DECOMPRESS_BUFFER_LEN);

				int remainder = comp_size - (read_bytes + chunk_2_size);

				if (remainder < 0) {
					fread(samples_buffer, chunk_2_size + remainder, 1, level_fp);
				} else {
					fread(samples_buffer, chunk_2_size, 1, level_fp);
					fseek(level_fp, remainder, SEEK_CUR);
				}
				read_bytes += chunk_2_size;

				if (!DXCreateSample(samples_buffer, chunk_2_size, wav_format.samples_per_second, i)) {
					FreeSampleDecompress();
					return 0;
				}
			} else {
				Log(1, "Sample buffer overrun!");
			}
		}
	}

	FreeSampleDecompress();
	return 1;
}

void S_GetUVRotateTextures() {
	TEXTURESTRUCT* tex;
	int16_t* pRange;

	pRange = aranges + 1;

	for (int i = 0; i < nAnimUVRanges; i++, pRange++) {
		for (int j = (int)*(pRange++); j >= 0; j--, pRange++) {
			tex = &textinfo[*pRange];
			AnimatingTexturesV[i][j][0] = tex->v1;
		}

		pRange--;
	}
}

void AdjustUV(int32_t num) {
	TEXTURESTRUCT* tex;
	float u, v;
	uint16_t type;

	Log(2, "AdjustUV");

	for (int i = 0; i < num; i++) {
		tex = &textinfo[i];
		Textures[tex->tpage].tpage++;
		tex->tpage++;
		u = 1.0F / float(Textures[tex->tpage].width << 1);
		v = 1.0F / float(Textures[tex->tpage].height << 1);
		type = tex->flag & 7;

		if (tex->flag & 0x8000) {
			switch (type) {
				case 0:
					tex->u1 += u;
					tex->v1 += v;
					tex->u2 -= u;
					tex->v2 += v;
					tex->u3 += u;
					tex->v3 -= v;
					break;

				case 1:
					tex->u1 -= u;
					tex->v1 += v;
					tex->u2 -= u;
					tex->v2 -= v;
					tex->u3 += u;
					tex->v3 += v;
					break;

				case 2:
					tex->u1 -= u;
					tex->v1 -= v;
					tex->u2 += u;
					tex->v2 -= v;
					tex->u3 -= u;
					tex->v3 += v;
					break;

				case 3:
					tex->u1 += u;
					tex->v1 -= v;
					tex->u2 += u;
					tex->v2 += v;
					tex->u3 -= u;
					tex->v3 -= v;
					break;

				case 4:
					tex->u1 -= u;
					tex->v1 += v;
					tex->u2 += u;
					tex->v2 += v;
					tex->u3 -= u;
					tex->v3 -= v;
					break;

				case 5:
					tex->u1 += u;
					tex->v1 += v;
					tex->u2 += u;
					tex->v2 -= v;
					tex->u3 -= u;
					tex->v3 += v;
					break;

				case 6:
					tex->u1 += u;
					tex->v1 -= v;
					tex->u2 -= u;
					tex->v2 -= v;
					tex->u3 += u;
					tex->v3 += v;
					break;

				case 7:
					tex->u1 -= u;
					tex->v1 -= v;
					tex->u2 -= u;
					tex->v2 += v;
					tex->u3 += u;
					tex->v3 -= v;
					break;

				default:
					Log(1, "TextureInfo Type %d Not Found", type);
					break;
			}
		} else {
			switch (type) {
				case 0:
					tex->u1 += u;
					tex->v1 += v;
					tex->u2 -= u;
					tex->v2 += v;
					tex->u3 -= u;
					tex->v3 -= v;
					tex->u4 += u;
					tex->v4 -= v;
					break;

				case 1:
					tex->u1 -= u;
					tex->v1 += v;
					tex->u2 += u;
					tex->v2 += v;
					tex->u3 += u;
					tex->v3 -= v;
					tex->u4 -= u;
					tex->v4 -= v;
					break;

				default:
					Log(1, "TextureInfo Type %d Not Found", type);
					break;
			}
		}
	}
}

bool Decompress(char* pDest, char* pCompressed, int32_t compressedSize, int32_t size) {
	z_stream stream;

	Log(2, "Decompress");
	memset(&stream, 0, sizeof(z_stream));
	stream.avail_in = compressedSize;
	stream.avail_out = size;
	stream.next_out = (Bytef*)pDest;
	stream.next_in = (Bytef*)pCompressed;
	inflateInit(&stream);
	inflate(&stream, Z_FINISH);

	if (stream.total_out != size) {
		Log(1, "Error Decompressing Data");
		return 0;
	}

	inflateEnd(&stream);
	Log(5, "Decompression OK");
	return 1;
}