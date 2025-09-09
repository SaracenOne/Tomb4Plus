#pragma once
#include "../global/types.h"

extern THREAD MainThread;

void GameClose();
int32_t GameMain(void* ptr);
uint16_t GetRandom(WATERTAB* wt, int32_t lp);
void init_water_table();
bool GameInitialise();
int32_t S_SaveGame(int32_t slot_num);
int32_t S_LoadGame(int32_t slot_num);

#define WATER_TABLE_COUNT 22
#define WATER_TABLE_SIZE 64
#define WIBBLE_TABLE_SIZE 32
#define UNUSED_WIBBLE_TABLE_SIZE 256

extern WATERTAB WaterTable[WATER_TABLE_COUNT][WATER_TABLE_SIZE];
extern THREAD MainThread;
extern int16_t* clipflags;
extern float vert_wibble_table[WIBBLE_TABLE_SIZE];
extern int32_t SaveCounter;
