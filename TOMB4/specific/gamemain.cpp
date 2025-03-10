#include "../tomb4/pch.h"
#include "gamemain.h"
#include "audio.h"
#include "file.h"
#include "function_stubs.h"
#include "function_table.h"
#include "3dmath.h"
#include "../game/text.h"
#include "time.h"
#include "winmain.h"
#include "../game/sound.h"
#include "../game/gameflow.h"
#include "dxshell.h"
#include "../game/savegame.h"
#include "../tomb4/tomb4.h"
#include "platform.h"

#include "specificfx.h"

#include "../game/trep/trepsave.h"

#include "../tomb4/tomb4plus/t4plus_weather.h"
#include "../tomb4/mod_config.h"

#include "../game/trng/trng_savegame.h"

#include "bgfx.h"

WATERTAB WaterTable[WATER_TABLE_COUNT][WATER_TABLE_SIZE];
THREAD MainThread;
int16_t* clipflags;
float vert_wibble_table[WIBBLE_TABLE_SIZE];
int32_t SaveCounter;

static float unused_vert_wibble_table[UNUSED_WIBBLE_TABLE_SIZE];
static uint8_t water_abs[4] = { 4, 8, 12, 16 };
static int16_t water_shimmer[4] = { 31, 63, 95, 127 };
static int16_t water_choppy[4] = { 16, 53, 90, 127 };

void GameClose() {
	Log(2, "GameClose");
	ACMClose();
	FreeLevel();

	T4PlusCleanup();

	SYSTEM_FREE(clipflags);

	if (wav_file_buffer)
		SYSTEM_FREE(wav_file_buffer);

	if (ADPCMBuffer)
		SYSTEM_FREE(ADPCMBuffer);

	if (logF)
		fclose(logF);

	SYSTEM_FREE(malloc_buffer);
	SYSTEM_FREE(gfScriptFile);
	SYSTEM_FREE(gfLanguageFile);

	SYSTEM_FREE(mesh_mapping_table);

	ShutdownBGFX();
}

int GameMain(void* ptr) {
	Log(2, "GameMain");

	if (GameInitialise()) {
		InitialiseFunctionTable();
		HWInitialise();
		InitWindow(0, 0, App.dx.dwRenderWidth, App.dx.dwRenderHeight, 20, (BLOCK_SIZE * 20), DEFAULT_FOV, App.dx.dwRenderWidth, App.dx.dwRenderHeight);
		// T4Plus - Moved init fonts to a per-level basis
		TIME_Init();
		App.SetupComplete = 1;
		S_CDStop();
		ClearSurfaces();

		if (!App.SoundDisabled)
			SOUND_Init();

		RPC_Init();
		init_tomb4_stuff();
		DoGameflow();
		GameClose();
		S_CDStop();

		RPC_close();
#ifdef _WIN32
		PostMessage(App.hWnd, WM_CLOSE, 0, 0);
#endif
		MainThread.active = 0;

#ifdef _WIN32
		_endthreadex(1);
#else
		SDL_DetachThread(MainThread.handle);
#endif
	}

	return 1;
}

uint16_t GetRandom(WATERTAB* wt, int32_t lp) {
	int32_t loop;
	uint16_t ret;

	do {
		ret = rand() & 0xFC;

		for (loop = 0; loop < lp; loop++)
			if (wt[loop].random == ret)
				break;

	} while (loop != lp);

	return ret;
}

void init_water_table() {
	float fSin;
	int32_t lSin;
	int16_t sSin, angle;

	srand(121197);

	for (int i = 0; i < WATER_TABLE_SIZE; i++) {
		sSin = rcossin_tbl[i << 7];
		WaterTable[0][i].shimmer = (63 * sSin) >> 15;
		WaterTable[0][i].choppy = (16 * sSin) >> 12;
		WaterTable[0][i].random = (uint8_t)GetRandom(&WaterTable[0][0], i);
		WaterTable[0][i].abs = 0;

		WaterTable[1][i].shimmer = (32 * sSin) >> 15;
		WaterTable[1][i].choppy = 0;
		WaterTable[1][i].random = (uint8_t)GetRandom(&WaterTable[1][0], i);
		WaterTable[1][i].abs = -3;

		WaterTable[2][i].shimmer = (64 * sSin) >> 15;
		WaterTable[2][i].choppy = 0;
		WaterTable[2][i].random = (uint8_t)GetRandom(&WaterTable[2][0], i);
		WaterTable[2][i].abs = 0;

		WaterTable[3][i].shimmer = (96 * sSin) >> 15;
		WaterTable[3][i].choppy = 0;
		WaterTable[3][i].random = (uint8_t)GetRandom(&WaterTable[3][0], i);
		WaterTable[3][i].abs = 4;

		WaterTable[4][i].shimmer = (127 * sSin) >> 15;
		WaterTable[4][i].choppy = 0;
		WaterTable[4][i].random = (uint8_t)GetRandom(&WaterTable[4][0], i);
		WaterTable[4][i].abs = 8;

		for (int j = 0, k = 5; j < 4; j++, k += 4) {
			for (int m = 0; m < 4; m++) {
				WaterTable[k + m][i].shimmer = -((sSin * water_shimmer[m]) >> 15);
				WaterTable[k + m][i].choppy = sSin * water_choppy[j] >> 12;
				WaterTable[k + m][i].random = (uint8_t)GetRandom(&WaterTable[k + m][0], i);
				WaterTable[k + m][i].abs = water_abs[m];
			}
		}
	}

	for (int i = 0; i < WIBBLE_TABLE_SIZE; i++) {
		fSin = sinf(float(i * (M_PI / 16.0F)));
		vert_wibble_table[i] = fSin + fSin;
	}

	for (int i = 0; i < UNUSED_WIBBLE_TABLE_SIZE; i++) {
		angle = 0x10000 * i / 256;
		lSin = phd_sin(angle);
		unused_vert_wibble_table[i] = float(lSin >> (W2V_SHIFT - 5));
	}
}

bool GameInitialise() {
	init_game_malloc();
	reset_virtual_game_malloc_offset();
	clipflags = (int16_t*)SYSTEM_MALLOC(0x4000);
	init_water_table();
	InitWeatherFX(); // TRLE
	return 1;
}

int32_t S_SaveGame(int32_t slot_num) {
	size_t bytes;
	int32_t days, hours, minutes, seconds;
	char buffer[80], counter[16];

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "savegame.%d", slot_num);

	std::string full_path = savegame_dir_path + buffer;

	FILE* file = fopen(full_path.c_str(), "wb");

	if (file) {
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s", GetCustomStringForTextID(gfLevelNames[gfCurrentLevel]));

		bytes = fwrite(buffer, sizeof(char), 75, file);
		bytes = fwrite(&SaveCounter, sizeof(int32_t), 1, file);
		days = savegame.Game.Timer / 30 / 86400;
		hours = savegame.Game.Timer / 30 % 86400 / 3600;
		minutes = savegame.Game.Timer / 30 / 60 % 60;
		seconds = savegame.Game.Timer / 30 % 60;

		bytes = fwrite(&days, sizeof(int16_t), 1, file);
		bytes = fwrite(&hours, sizeof(int16_t), 1, file);
		bytes = fwrite(&minutes, sizeof(int16_t), 1, file);
		bytes = fwrite(&seconds, sizeof(int16_t), 1, file);
		bytes = fwrite(&savegame, sizeof(LEGACY_SAVEGAME_INFO), 1, file);

		NGWriteNGSavegameBuffer(file);

		fclose(file);
		sprintf(counter, "%d", SaveCounter);
		SaveCounter++;

		MOD_GLOBAL_INFO* mod_global_info = get_game_mod_global_info();
		if (mod_global_info->trep_using_extended_saves) {
			S_TREPSavegame(slot_num);
		}

		return 1;
	}

	return 0;
}

int32_t S_LoadGame(int32_t slot_num) {
	int32_t value;
	char buffer[80];

	sprintf(buffer, "savegame.%d", slot_num);

	std::string full_path = savegame_dir_path + buffer;

	FILE* file = platform_fopen(full_path.c_str(), "rb");

	T4PlusLevelReset();

	if (file) {
		if (fread(buffer, sizeof(char), 75, file) == 0) {
			fclose(file);
			return 0;
		}
		if (fread(&value, sizeof(int32_t), 1, file) == 0) {
			fclose(file);
			return 0;
		}
		if (fread(&value, sizeof(int32_t), 1, file) == 0) {
			fclose(file);
			return 0;
		}
		if (fread(&value, sizeof(int32_t), 1, file) == 0) {
			fclose(file);
			return 0;
		}
		if (fread(&savegame, sizeof(LEGACY_SAVEGAME_INFO), 1, file) == 0) {
			fclose(file);
			return 0;
		}

		// NGLE
		NGReadNGSavegameBuffer(file);

		fclose(file);

		MOD_GLOBAL_INFO *mod_global_info = get_game_mod_global_info();
		if (mod_global_info->trep_using_extended_saves) {
			S_TREPLoadgame(slot_num);
		}

		return 1;
	}

	return 0;
}
