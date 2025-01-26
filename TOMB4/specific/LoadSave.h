#pragma once
#include "../global/types.h"
#include <string>

#define MAX_SAVEGAMES 32

void DoOptions();
void DisplayGameStats();
long S_DisplayPauseMenu(long reset_selection, long reset_menu = -1);
long DoLoadSave(long LoadSave);
long S_LoadSave(long load_or_save, long mono, long inv_active);
void S_DisplayMonoScreen();
void CreateMonoScreen();
void FreeMonoScreen();
void RGBM_Mono(uchar* r, uchar* g, uchar* b);
#ifndef USE_BGFX
void ConvertSurfaceToTextures(LPDIRECTDRAWSURFACEX surface);
#endif
void CheckKeyConflicts();
long S_PauseMenu(long force_menu = 0);
long GetSaveLoadFiles();

extern long sfx_frequencies[3];
extern long SoundQuality;
extern long MusicVolume;
extern long SFXVolume;
extern long ControlMethod;
extern bool MonoScreenOn;
