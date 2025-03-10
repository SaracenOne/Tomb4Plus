#pragma once
#include "../global/types.h"

const size_t X32_SAVEGAME_MESH_SIZE = 38;
const size_t INTERNAL_SAVEGAME_MESH_SIZE = sizeof(MESH_DATA);

#define MAX_HUB_LEVELS 10

int32_t CheckSumValid(char* buffer);
void sgInitialiseHub(int32_t dont_save_lara);
void SaveLaraData();
void WriteSG(void* pointer, int32_t size);
void ReadSG(void* pointer, int32_t size);
void SaveHubData(int32_t index);
void RestoreLaraData(bool full_save);
void sgRestoreLevel();
void CreateCheckSum();
void sgSaveLevel();
void sgSaveGame();
void sgRestoreGame();
int32_t OpenSaveGame(uint8_t current_level, int32_t saving);
void SaveLevelData(bool full_save, bool use_full_flipmask);
void RestoreLevelData(bool full_save, bool use_full_flipmask);

extern LEGACY_SAVEGAME_INFO savegame;
