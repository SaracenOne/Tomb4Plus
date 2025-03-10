#pragma once
#include "../global/types.h"
#include <string>

#define MAX_SAVEGAMES 32

void DoOptions();
void DisplayGameStats();
int32_t S_DisplayPauseMenu(int32_t reset_selection, int32_t reset_menu = -1);
int32_t DoLoadSave(int32_t LoadSave);
int32_t S_LoadSave(int32_t load_or_save, int32_t mono, int32_t inv_active);
void S_DisplayMonoScreen();
void CreateMonoScreen();
void FreeMonoScreen();
void RGBM_Mono(uint8_t* r, uint8_t* g, uint8_t* b);
void CheckKeyConflicts();
int32_t S_PauseMenu(int32_t force_menu = 0);
int32_t GetSaveLoadFiles();

extern int32_t sfx_frequencies[3];
extern int32_t SoundQuality;
extern int32_t MusicVolume;
extern int32_t SFXVolume;
extern int32_t ControlMethod;
extern bool MonoScreenOn;
