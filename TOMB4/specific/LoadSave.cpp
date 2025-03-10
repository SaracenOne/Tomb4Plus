#include "../tomb4/pch.h"
#include "LoadSave.h"
#include "../game/text.h"
#include "../game/sound.h"
#include "audio.h"
#include "dxsound.h"
#include "input.h"
#include "function_table.h"
#include "drawroom.h"
#include "polyinsert.h"
#include "winmain.h"
#include "output.h"
#include "../game/gameflow.h"
#include "../game/savegame.h"
#include "gamemain.h"
#include "specificfx.h"
#include "time.h"
#include "dxshell.h"
#include "function_stubs.h"
#include "texture.h"
#include "../game/newinv.h"
#include "../game/camera.h"
#include "3dmath.h"
#include "../game/control.h"
#include "../game/lara.h"
#include "../tomb4/tomb4.h"
#include "../tomb4/troyestuff.h"
#include "drawbars.h"

#include "../specific/input.h"
#include "../tomb4/mod_config.h"
#include "file.h"
#include "platform.h"

int32_t sfx_frequencies[3] = { 11025, 22050, 44100 };
int32_t SoundQuality = 1;
int32_t MusicVolume = 40;
int32_t SFXVolume = 80;
int32_t ControlMethod;
bool MonoScreenOn;

static MONOSCREEN_STRUCT MonoScreen;
static LEGACY_SAVEFILE_INFO SaveGames[MAX_SAVEGAMES] = {};

void DoOptions() {
	char** keyboard_buttons;
	char* txt;
	static int32_t menu;
	static uint32_t sel = 1;	//selection
	static uint32_t sel2;		//selection for when mapping keys
	static int32_t mSliderCol = 0xFF3F3F3F;
	static int32_t sSliderCol = 0xFF3F3F3F;
	static int32_t sfx_bak;
	static int32_t sfx_quality_bak;
	static int32_t sfx_breath_db = -1;
	uint32_t nMask;
	int32_t f, y, i, lp;
	static int8_t sfx_backup_flag;	//have we backed sfx stuff up?
	static bool waiting_for_key = 0;

	if (!(sfx_backup_flag & 1)) {
		sfx_backup_flag |= 1;
		sfx_bak = SFXVolume;
	}

	if (!(sfx_backup_flag & 2)) {
		sfx_backup_flag |= 2;
		sfx_quality_bak = SoundQuality;
	}

	f = font_height - 4;

	if (menu) { //controls menu
		if (menu == 200) {
			TroyeMenu(f, menu, sel);
			return;
		}

		if (Gameflow->Language == GERMAN)
			keyboard_buttons = (char**)GermanKeyboard;
		else
			keyboard_buttons = (char**)KeyboardButtons;

		nMask = 17;
		small_font = 1;
		PrintString(phd_centerx >> 2, f, sel & 1 ? 1 : 2, GetFixedStringForTextID(TXT_Control_Method), 0);

		y = 1;
		i = 1;

		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, "\x18", 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, "\x1A", 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, "\x19", 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, "\x1B", 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Duck), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Dash), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Walk), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Jump), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Action), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Draw_Weapon), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Use_Flare), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Look), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Roll), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Inventory), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Step_Left), 0);
		PrintString(phd_centerx >> 2, f + y++ * font_height, sel & (1 << i++) ? 1 : 2, GetFixedStringForTextID(TXT_Step_Right), 0);

		if (!ControlMethod)
			PrintString(phd_centerx + (phd_centerx >> 2), f, sel2 & 1 ? 1 : 6, GetFixedStringForTextID(TXT_Keyboard), 0);
		else if (ControlMethod == 1)
			PrintString(phd_centerx + (phd_centerx >> 2), f, sel2 & 1 ? 1 : 6, GetFixedStringForTextID(TXT_Joystick), 0);
		else if (ControlMethod == 2)
			PrintString(phd_centerx + (phd_centerx >> 2), f, sel2 & 1 ? 1 : 6, GetFixedStringForTextID(TXT_Reset), 0);

		y = 1;
		i = 1;

		for (lp = 0; lp < 16; lp++) {
			int dik = keyboard_layout[1][lp];

			txt = (waiting_for_key && sel2 & (1 << i)) ? GetFixedStringForTextID(TXT_Waiting) : keyboard_buttons[dik];
			if (txt)
				PrintString(phd_centerx + (phd_centerx >> 2), f + y++ * font_height, sel2 & (1 << i++) ? 1 : 6, txt, 0);
			else
				PrintString(phd_centerx + (phd_centerx >> 2), f + y++ * font_height, sel2 & (1 << i++) ? 1 : 6, "???", 0);
		}

		small_font = 0;

		if (ControlMethod < 2 && !waiting_for_key) {
			if (dbinput & IN_FORWARD) {
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				sel >>= 1;
			}

			if (dbinput & IN_BACK) {
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				sel <<= 1;
			}
		}

		if (waiting_for_key) {
			i = 0;

			if (keymap[SDL_SCANCODE_ESCAPE]) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				sel2 = 0;
				dbinput = 0;
				waiting_for_key = 0;
				return;
			}

			for (lp = 0; lp < keymap_count; lp++) {
				int16_t tomb4_scancode = (int16_t)convert_sdl_scancode_to_tomb_keycode(lp);
				if (keymap[lp] && keyboard_buttons[tomb4_scancode]) {
					if (tomb4_scancode != T4P_KEY_RETURN && tomb4_scancode != T4P_KEY_LEFT && tomb4_scancode != T4P_KEY_RIGHT && tomb4_scancode != T4P_KEY_UP && tomb4_scancode != T4P_KEY_DOWN) {
						waiting_for_key = 0;

						sel2 >>= 2;

						while (sel2) {
							i++;
							sel2 >>= 1;
						}

						sel2 = 0;

						keyboard_layout[1][i] = tomb4_scancode;
					}
				}
			}
			if (ControlMethod == 1) {
			}

			CheckKeyConflicts();
			dbinput = 0;
		}

		if (dbinput & IN_SELECT && sel > 1 && ControlMethod < 2) {
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			sel2 = sel;
			waiting_for_key = 1;
		}

		if (dbinput & IN_SELECT && ControlMethod == 2) {
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			ControlMethod = 0;
			memcpy(keyboard_layout[1], keyboard_layout, 72);
		}

		if (sel & 1) {
			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				ControlMethod--;
			}

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				ControlMethod++;
			}

			if (ControlMethod > 2)
				ControlMethod = 2;

			if (ControlMethod < 0)
				ControlMethod = 0;

			if (ControlMethod == 1) {
#if 0
				joy.dwSize = sizeof(JOYINFOEX);

				if (joyGetPosEx(0, &joy) == JOYERR_UNPLUGGED) {
					if (dbinput & IN_LEFT)
						ControlMethod = 0;

					if (dbinput & IN_RIGHT)
						ControlMethod = 2;
				}
#endif
			}
		}

		if (!sel)
			sel = 1;

		if (sel > uint32_t(1 << (nMask - 1)))
			sel = 1 << (nMask - 1);

		if (dbinput & IN_DESELECT) {
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

			if (ControlMethod < 2)
				menu = 0;

			dbinput = 0;
			sel = 1;
		}
	} else { //'main' menu
		nMask = 6;
		f = 3 * font_height;
		PrintString(phd_centerx, f, 6, GetFixedStringForTextID(TXT_Options), FF_CENTER);
		PrintString(phd_centerx, f + font_height + (font_height >> 1), sel & 1 ? 1 : 2, GetFixedStringForTextID(TXT_Control_Configuration), FF_CENTER);
		PrintString(phd_centerx >> 2, f + 3 * font_height, sel & 0x2 ? 1 : 2, GetFixedStringForTextID(TXT_Music_Volume), 0);
		PrintString(phd_centerx >> 2, f + 4 * font_height, sel & 0x4 ? 1 : 2, GetFixedStringForTextID(TXT_SFX_Volume), 0);
		PrintString(phd_centerx >> 2, f + 5 * font_height, sel & 0x8 ? 1 : 2, GetFixedStringForTextID(TXT_Sound_Quality), 0);
		PrintString(phd_centerx >> 2, f + 6 * font_height, sel & 0x10 ? 1 : 2, GetFixedStringForTextID(TXT_Targeting), 0);
		DoSlider(400, 3 * font_height - (font_height >> 1) + f + 4, 200, 16, MusicVolume, 0xFF1F1F1F, 0xFF3F3FFF, mSliderCol);
		DoSlider(400, f + 4 * font_height + 4 - (font_height >> 1), 200, 16, SFXVolume, 0xFF1F1F1F, 0xFF3F3FFF, sSliderCol);

		if (!SoundQuality)
			PrintString(phd_centerx + (phd_centerx >> 2), f + 5 * font_height, sel & 8 ? 1 : 6, GetFixedStringForTextID(TXT_Low), 0);
		else if (SoundQuality == 1)
			PrintString(phd_centerx + (phd_centerx >> 2), f + 5 * font_height, sel & 8 ? 1 : 6, GetFixedStringForTextID(TXT_Medium), 0);
		else if (SoundQuality == 2)
			PrintString(phd_centerx + (phd_centerx >> 2), f + 5 * font_height, sel & 8 ? 1 : 6, GetFixedStringForTextID(TXT_High), 0);

		if (App.AutoTarget)
			PrintString(phd_centerx + (phd_centerx >> 2), f + 6 * font_height, sel & 0x10 ? 1 : 6, GetFixedStringForTextID(TXT_Automatic), 0);
		else
			PrintString(phd_centerx + (phd_centerx >> 2), f + 6 * font_height, sel & 0x10 ? 1 : 6, GetFixedStringForTextID(TXT_Manual), 0);

		PrintString(phd_centerx, (font_height >> 1) + f + 7 * font_height, sel & 0x20 ? 1 : 2, "tomb4 options", FF_CENTER);

		if (dbinput & IN_FORWARD) {
			SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			sel >>= 1;
		}

		if (dbinput & IN_BACK) {
			SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			sel <<= 1;
		}

		if (dbinput & IN_SELECT && sel & 1) {
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			menu = 1;
		}

		if (dbinput & IN_SELECT && sel & 0x20) {
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			sel = 1;
			menu = 200;
		}

		if (!sel)
			sel = 1;

		if (sel > uint32_t(1 << (nMask - 1)))
			sel = 1 << (nMask - 1);

		mSliderCol = 0xFF3F3F3F;
		sSliderCol = 0xFF3F3F3F;

		if (sel & 2) {
			sfx_bak = SFXVolume;

			if (linput & IN_LEFT)
				MusicVolume--;

			if (linput & IN_RIGHT)
				MusicVolume++;

			if (MusicVolume > 100)
				MusicVolume = 100;
			else if (MusicVolume < 0)
				MusicVolume = 0;

			sSliderCol = 0xFF3F3F3F;
			mSliderCol = 0xFF7F7F7F;
			ACMSetVolume();
		} else if (sel & 4) {
			if (linput & IN_LEFT)
				SFXVolume--;

			if (linput & IN_RIGHT)
				SFXVolume++;

			if (SFXVolume > 100)
				SFXVolume = 100;
			else if (SFXVolume < 0)
				SFXVolume = 0;

			if (SFXVolume != sfx_bak) {
				if (sfx_breath_db == -1 || !DSIsChannelPlaying(0)) {
					S_SoundStopAllSamples();
					sfx_bak = SFXVolume;
					sfx_breath_db = SoundEffect(SFX_LARA_BREATH, 0, SFX_ALWAYS);
					DSChangeVolume(0, -100 * ((100 - SFXVolume) >> 1));
				} else if (sfx_breath_db != -1 && DSIsChannelPlaying(0))
					DSChangeVolume(0, -100 * ((100 - SFXVolume) >> 1));
			}

			mSliderCol = 0xFF3F3F3F;
			sSliderCol = 0xFF7F7F7F;
		} else if (sel & 8) {
			sfx_bak = SFXVolume;

			if (dbinput & IN_LEFT)
				SoundQuality--;

			if (dbinput & IN_RIGHT)
				SoundQuality++;

			if (SoundQuality > 2)
				SoundQuality = 2;
			else if (SoundQuality < 0)
				SoundQuality = 0;

			if (SoundQuality != sfx_quality_bak) {
				S_SoundStopAllSamples();
				DXChangeOutputFormat(sfx_frequencies[SoundQuality], 0);
				sfx_quality_bak = SoundQuality;
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}
		} else if (sel & 16) {
			if (dbinput & IN_LEFT) {
				if (App.AutoTarget)
					App.AutoTarget = 0;

				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_RIGHT) {
				if (!App.AutoTarget)
					App.AutoTarget = 1;

				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}

			savegame.AutoTarget = App.AutoTarget;
		}
	}
}

void DisplayGameStats() {
	int32_t sec, days, hours, min, y;
	char buf[40];

	y = phd_centery - (font_height << 2);
	PrintString(phd_centerx, y, 6, GetFixedStringForTextID(TXT_Statistics), FF_CENTER);
	PrintString(phd_centerx, y + 2 * font_height, 2, GetCustomStringForTextID(gfLevelNames[gfCurrentLevel]), FF_CENTER);
	PrintString(phd_centerx >> 2, y + 3 * font_height, 2, GetFixedStringForTextID(TXT_Time_Taken), 0);
	PrintString(phd_centerx >> 2, y + 4 * font_height, 2, GetFixedStringForTextID(TXT_Distance_Travelled), 0);
	PrintString(phd_centerx >> 2, y + 5 * font_height, 2, GetFixedStringForTextID(TXT_Ammo_Used), 0);
	PrintString(phd_centerx >> 2, y + 6 * font_height, 2, GetFixedStringForTextID(TXT_Health_Packs_Used), 0);
	PrintString(phd_centerx >> 2, y + 7 * font_height, 2, GetFixedStringForTextID(TXT_Secrets_Found), 0);

	sec = GameTimer / 30;
	days = sec / 86400;
	hours = (sec % 86400) / 3600;
	min = (sec / 60) % 60;
	sec = (sec % 60);

	sprintf(buf, "%02d:%02d:%02d", (days * 24) + hours, min, sec);
	PrintString(phd_centerx + (phd_centerx >> 2), y + 3 * font_height, 6, buf, 0);

	sprintf(buf, "%dm", savegame.Game.Distance / 419);
	PrintString(phd_centerx + (phd_centerx >> 2), y + 4 * font_height, 6, buf, 0);

	sprintf(buf, "%d", savegame.Game.AmmoUsed);
	PrintString(phd_centerx + (phd_centerx >> 2), y + 5 * font_height, 6, buf, 0);

	sprintf(buf, "%d", savegame.Game.HealthUsed);
	PrintString(phd_centerx + (phd_centerx >> 2), y + 6 * font_height, 6, buf, 0);


	sprintf(buf, "%d / %d", savegame.Game.Secrets, get_game_mod_level_stat_info(gfCurrentLevel)->secret_count);
	PrintString(phd_centerx + (phd_centerx >> 2), y + 7 * font_height, 6, buf, 0);
}

int32_t S_DisplayPauseMenu(int32_t reset_selection, int32_t reset_menu) {
	static int32_t menu, selection = 1;
	int32_t y;

	if (!menu) {
		if (reset_selection) {
			selection = reset_selection;
			menu = 0;
			if (reset_menu >= 0) {
				menu = reset_menu;
			}
		} else {
			y = phd_centery - font_height;
			PrintString(phd_centerx, y - ((3 * font_height) >> 1), 6, GetFixedStringForTextID(TXT_Paused), FF_CENTER);
			PrintString(phd_centerx, y, selection & 1 ? 1 : 2, GetFixedStringForTextID(TXT_Statistics), FF_CENTER);
			PrintString(phd_centerx, y + font_height, selection & 2 ? 1 : 2, GetFixedStringForTextID(TXT_Options), FF_CENTER);
			PrintString(phd_centerx, y + 2 * font_height, selection & 4 ? 1 : 2, GetFixedStringForTextID(TXT_Exit_to_Title), FF_CENTER);

			if (dbinput & IN_FORWARD) {
				if (selection > 1)
					selection >>= 1;

				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_BACK) {
				if (selection < 4)
					selection <<= 1;

				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_DESELECT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				return 1;
			}

			if (dbinput & IN_SELECT && !keymap[SDL_SCANCODE_LALT]) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

				if (selection & 1)
					menu = 2;
				else if (selection & 2)
					menu = 1;
				else if (selection & 4)
					return 8;
			}
		}
	} else if (menu == 1) {
		DoOptions();

		if (dbinput & IN_DESELECT) {
			menu = 0;
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
		}
	} else if (menu == 2) {
		DisplayGameStats();

		if (dbinput & IN_DESELECT) {
			menu = 0;
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

			if (get_game_mod_level_misc_info(gfCurrentLevel)->always_exit_from_statistics_screen)
				return 1;
		}
	}

	return 0;
}

int32_t DoLoadSave(int32_t LoadSave) {
	// Tomb4Plus: handling for increased savegame count.

	LEGACY_SAVEFILE_INFO* pSave;
	static int32_t selection;
	int32_t txt;
	size_t l;
	uint8_t color;
	char string[80];
	char name[41];

	if (LoadSave & IN_SAVE)
		txt = TXT_Save_Game;
	else
		txt = TXT_Load_Game;

	float font_scale = 15.0f / (float)MAX_SAVEGAMES * 1.06f;
	int32_t scaled_font_height = (int32_t)(savegame_font_height * font_scale);

	PrintString(phd_centerx, savegame_font_height, 6, GetFixedStringForTextID(txt), FF_CENTER);

	for (int i = 0; i < MAX_SAVEGAMES; i++) {
		pSave = &SaveGames[i];
		color = 2;

		if (i == selection)
			color = 1;

		memset(name, ' ', 40);
		l = strlen(pSave->name);

		if (l > 40)
			l = 40;

		strncpy(name, pSave->name, l);
		name[40] = 0;
		small_font = 1;

		if (pSave->valid) {
			sprintf(string, "%03d", pSave->num);
			PrintStringScaled(phd_centerx - int32_t((float)phd_winwidth / 640.0F * 310.0), savegame_font_height + scaled_font_height * (i + 2), color, string, 0, 1.0F, font_scale);
			PrintStringScaled(phd_centerx - int32_t((float)phd_winwidth / 640.0F * 270.0), savegame_font_height + scaled_font_height * (i + 2), color, name, 0, 1.0F, font_scale);
			sprintf(string, "%d %s %02d:%02d:%02d", pSave->days, GetFixedStringForTextID(TXT_days), pSave->hours, pSave->minutes, pSave->seconds);
			PrintStringScaled(phd_centerx - int32_t((float)phd_winwidth / 640.0F * -135.0), savegame_font_height + scaled_font_height * (i + 2), color, string, 0, 1.0F, font_scale);
		} else {
			sprintf(string, "%s", pSave->name);
			PrintStringScaled(phd_centerx, savegame_font_height + scaled_font_height * (i + 2), color, string, FF_CENTER, 1.0F, font_scale);
		}

		small_font = 0;
	}

	if (dbinput & IN_FORWARD) {
		selection--;
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
	}

	if (dbinput & IN_BACK) {
		selection++;
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
	}

	if (selection < 0)
		selection = 0;
	else if (selection >= MAX_SAVEGAMES)
		selection = MAX_SAVEGAMES - 1;

	if (dbinput & IN_SELECT) {
		if (SaveGames[selection].valid || LoadSave == IN_SAVE)
			return selection;

		SoundEffect(SFX_LARA_NO, 0, SFX_ALWAYS);
	}

	return -1;
}

int32_t S_LoadSave(int32_t load_or_save, int32_t mono, int32_t inv_active) {
	int32_t fade, ret;

	fade = 0;

	if (!mono)
		CreateMonoScreen();

	GetSaveLoadFiles();

	if (!inv_active)
		InventoryActive = 1;

	while (1) {
		S_InitialisePolyList();

		if (fade)
			dbinput = 0;
		else
			S_UpdateInput();

		SetDebounce = 1;
		S_DisplayMonoScreen();
		ret = DoLoadSave(load_or_save);
		UpdatePulseColour();
		S_OutputPolyList();
		S_DumpScreen();

		if (ret >= 0) {
			if (load_or_save & IN_SAVE) {
				sgSaveGame();
				S_SaveGame(ret);
				GetSaveLoadFiles();
				break;
			}

			fade = ret + 1;
			S_LoadGame(ret);

			if (!DeathMenuActive)
				SetFade(0, 255);

			ret = -1;
		}

		if (fade && DoFade == 2) {
			ret = fade - 1;
			break;
		}

		if (input & IN_OPTION) {
			ret = -1;
			break;
		}

		if (MainThread.ended)
			break;
	}

	TIME_Init();

	if (!mono)
		FreeMonoScreen();

	if (!inv_active)
		InventoryActive = 0;

	return ret;
}

void S_DisplayMonoScreen() {
}

void CreateMonoScreen() {
	MonoScreenOn = true;
}

void FreeMonoScreen() {
	MonoScreenOn = false;
}

void RGBM_Mono(uint8_t * r, uint8_t * g, uint8_t * b) {
	uint8_t c;

	if (tomb4.inv_bg_mode != INV_BG_MODE_CLEAR) {
		c = (*r + *b) >> 1;
		*r = c;
		*g = c;
		*b = c;
	}
}

static void BitMaskGetNumberOfBits(uint32_t bitMask, uint32_t& bitDepth, uint32_t& bitOffset) {
	int32_t i;

	if (!bitMask) {
		bitOffset = 0;
		bitDepth = 0;
		return;
	}

	for (i = 0; !(bitMask & 1); i++)
		bitMask >>= 1;

	bitOffset = i;

	for (i = 0; bitMask != 0; i++)
		bitMask >>= 1;

	bitDepth = i;
}

void CheckKeyConflicts() {
	int16_t key;

	for (int i = 0; i < 18; i++) {
		key = keyboard_layout[0][i];
		conflict[i] = 0;

		for (int j = 0; j < 18; j++) {
			if (key == keyboard_layout[1][j]) {
				conflict[i] = 1;
				break;
			}
		}
	}
}

int32_t S_PauseMenu(int32_t force_menu) {
	int32_t fade, ret;

	fade = 0;
	CreateMonoScreen();
	S_DisplayPauseMenu(1, force_menu);
	InventoryActive = 1;
	S_SetReverbType(1);

	do {
		S_InitialisePolyList();

		if (fade)
			dbinput = 0;
		else
			S_UpdateInput();

		SetDebounce = 1;
		S_DisplayMonoScreen();
		ret = S_DisplayPauseMenu(0);
		UpdatePulseColour();
		S_OutputPolyList();
		S_DumpScreen();

		if (ret == 1)
			break;

		if (ret == 8) {
			fade = 8;
			ret = 0;
			SetFade(0, 255);
		}

		if (fade && DoFade == 2) {
			ret = fade;
			break;
		}

	} while (!MainThread.ended);

	TIME_Init();
	FreeMonoScreen();
	InventoryActive = 0;
	return ret;
}

int32_t GetSaveLoadFiles() {
	FILE* file;
	LEGACY_SAVEFILE_INFO *pSave;
	LEGACY_SAVEGAME_INFO save_info;
	static int32_t nSaves;
	char name[75];

	SaveCounter = 0;

	for (int i = 0; i < MAX_SAVEGAMES; i++) {
		pSave = &SaveGames[i];
		sprintf(name, "savegame.%d", i);

		std::string full_path = savegame_dir_path + name;

		file = platform_fopen(full_path.c_str(), "rb");

		if (!file) {
			pSave->valid = 0;
			strcpy(pSave->name, GetFixedStringForTextID(TXT_Empty_Slot));
			continue;
		}

		fread(&pSave->name, sizeof(char), 75, file);
		fread(&pSave->num, sizeof(int32_t), 1, file);
		fread(&pSave->days, sizeof(int16_t), 1, file);
		fread(&pSave->hours, sizeof(int16_t), 1, file);
		fread(&pSave->minutes, sizeof(int16_t), 1, file);
		fread(&pSave->seconds, sizeof(int16_t), 1, file);
		fread(&save_info, 1, sizeof(LEGACY_SAVEGAME_INFO), file);

		if (!CheckSumValid((char*)&save_info)) {
			pSave->valid = 0;
			strcpy(pSave->name, GetFixedStringForTextID(TXT_Empty_Slot));
			continue;
		}

		if (pSave->num > SaveCounter)
			SaveCounter = pSave->num;

		pSave->valid = 1;
		fclose(file);
		nSaves++;
	}

	SaveCounter++;
	return nSaves;
}
