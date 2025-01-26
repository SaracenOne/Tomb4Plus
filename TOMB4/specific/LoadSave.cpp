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

long sfx_frequencies[3] = { 11025, 22050, 44100 };
long SoundQuality = 1;
long MusicVolume = 40;
long SFXVolume = 80;
long ControlMethod;
bool MonoScreenOn;

static MONOSCREEN_STRUCT MonoScreen;
static LEGACY_SAVEFILE_INFO SaveGames[MAX_SAVEGAMES] = {};

void DoOptions()
{
	char** keyboard_buttons;
	char* txt;
	static long menu;
	static ulong sel = 1;	//selection
	static ulong sel2;		//selection for when mapping keys
	static long mSliderCol = 0xFF3F3F3F;
	static long sSliderCol = 0xFF3F3F3F;
	static long sfx_bak;
	static long sfx_quality_bak;
	static long sfx_breath_db = -1;
	ulong nMask;
	long f, y, i, lp;
#ifndef USE_SDL
	long jread, jx, jy;
#endif
	static char sfx_backup_flag;	//have we backed sfx stuff up?
	static bool waiting_for_key = 0;

	if (!(sfx_backup_flag & 1))
	{
		sfx_backup_flag |= 1;
		sfx_bak = SFXVolume;
	}

	if (!(sfx_backup_flag & 2))
	{
		sfx_backup_flag |= 2;
		sfx_quality_bak = SoundQuality;
	}

	f = font_height - 4;

	if (menu)	//controls menu
	{
		if (menu == 200)
		{
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

		for (lp = 0; lp < 16; lp++)
		{
			int dik = keyboard_layout[1][lp];

			txt = (waiting_for_key && sel2 & (1 << i)) ? GetFixedStringForTextID(TXT_Waiting) : keyboard_buttons[dik];
			if (txt)
				PrintString(phd_centerx + (phd_centerx >> 2), f + y++ * font_height, sel2 & (1 << i++) ? 1 : 6, txt, 0);
			else
				PrintString(phd_centerx + (phd_centerx >> 2), f + y++ * font_height, sel2 & (1 << i++) ? 1 : 6, "???", 0);
		}

		small_font = 0;

		if (ControlMethod < 2 && !waiting_for_key)
		{
			if (dbinput & IN_FORWARD)
			{
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				sel >>= 1;
			}

			if (dbinput & IN_BACK)
			{
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				sel <<= 1;
			}
		}

		if (waiting_for_key)
		{
			i = 0;

#ifdef USE_SDL
			if (keymap[SDL_SCANCODE_ESCAPE])
#else
			if (keymap[DIK_ESCAPE])
#endif
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				sel2 = 0;
				dbinput = 0;
				waiting_for_key = 0;
				return;
			}

#ifdef USE_SDL
			for (lp = 0; lp < keymap_count; lp++)
			{
				short tomb4_scancode = (short)convert_sdl_scancode_to_tomb_keycode(lp);
				if (keymap[lp] && keyboard_buttons[tomb4_scancode])
				{
					if (tomb4_scancode != T4P_KEY_RETURN && tomb4_scancode != T4P_KEY_LEFT && tomb4_scancode != T4P_KEY_RIGHT && tomb4_scancode != T4P_KEY_UP && tomb4_scancode != T4P_KEY_DOWN)
					{
						waiting_for_key = 0;

						sel2 >>= 2;

						while (sel2)
						{
							i++;
							sel2 >>= 1;
						}

						sel2 = 0;

						keyboard_layout[1][i] = tomb4_scancode;
					}
				}
			}
#else
			for (lp = 0; lp < 255; lp++)
			{
				if (keymap[lp] && keyboard_buttons[lp])
				{
					if (lp != DIK_RETURN && lp != DIK_LEFT && lp != DIK_RIGHT && lp != DIK_UP && lp != DIK_DOWN)
					{
						waiting_for_key = 0;

						sel2 >>= 2;

						while (sel2)
						{
							i++;
							sel2 >>= 1;
						}

						sel2 = 0;
						layout[1][i] = (short)convert_sdl_scancode_to_tomb_keycode(lp);
					}
				}
			}
#endif
			if (ControlMethod == 1)
			{
#ifndef USE_SDL
				jread = ReadJoystick(jx, jy);

				if (jread)
				{
					i = 0;
					sel2 >>= 2;

					while (sel2)
					{
						i++;
						sel2 >>= 1;
					}

					sel2 = 0;
					lp = 0;

					while (jread)
					{
						jread >>= 1;
						lp++;
					}

					layout[1][i] = short(lp + 255);
					waiting_for_key = 0;
				}
#endif
			}

			CheckKeyConflicts();
			dbinput = 0;
		}

		if (dbinput & IN_SELECT && sel > 1 && ControlMethod < 2)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			sel2 = sel;
			waiting_for_key = 1;
#ifndef USE_SDL
			memset(keymap, 0, sizeof(keymap));
#endif
		}

		if (dbinput & IN_SELECT && ControlMethod == 2)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			ControlMethod = 0;
			memcpy(keyboard_layout[1], keyboard_layout, 72);
		}

		if (sel & 1)
		{
			if (dbinput & IN_LEFT)
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				ControlMethod--;
			}

			if (dbinput & IN_RIGHT)
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				ControlMethod++;
			}

			if (ControlMethod > 2)
				ControlMethod = 2;

			if (ControlMethod < 0)
				ControlMethod = 0;

			if (ControlMethod == 1)
			{
#if 0
				joy.dwSize = sizeof(JOYINFOEX);

				if (joyGetPosEx(0, &joy) == JOYERR_UNPLUGGED)
				{
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

		if (sel > ulong(1 << (nMask - 1)))
			sel = 1 << (nMask - 1);

		if (dbinput & IN_DESELECT)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

			if (ControlMethod < 2)
				menu = 0;

			dbinput = 0;
			sel = 1;
		}
	}
	else	//'main' menu
	{
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

		if (dbinput & IN_FORWARD)
		{
			SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			sel >>= 1;
		}

		if (dbinput & IN_BACK)
		{
			SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			sel <<= 1;
		}

		if (dbinput & IN_SELECT && sel & 1)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			menu = 1;
		}

		if (dbinput & IN_SELECT && sel & 0x20)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			sel = 1;
			menu = 200;
		}

		if (!sel)
			sel = 1;

		if (sel > ulong(1 << (nMask - 1)))
			sel = 1 << (nMask - 1);

		mSliderCol = 0xFF3F3F3F;
		sSliderCol = 0xFF3F3F3F;

		if (sel & 2)
		{
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
		}
		else if (sel & 4)
		{
			if (linput & IN_LEFT)
				SFXVolume--;

			if (linput & IN_RIGHT)
				SFXVolume++;

			if (SFXVolume > 100)
				SFXVolume = 100;
			else if (SFXVolume < 0)
				SFXVolume = 0;

			if (SFXVolume != sfx_bak)
			{
				if (sfx_breath_db == -1 || !DSIsChannelPlaying(0))
				{
					S_SoundStopAllSamples();
					sfx_bak = SFXVolume;
					sfx_breath_db = SoundEffect(SFX_LARA_BREATH, 0, SFX_ALWAYS);
					DSChangeVolume(0, -100 * ((100 - SFXVolume) >> 1));
				}
				else if (sfx_breath_db != -1 && DSIsChannelPlaying(0))
					DSChangeVolume(0, -100 * ((100 - SFXVolume) >> 1));
			}

			mSliderCol = 0xFF3F3F3F;
			sSliderCol = 0xFF7F7F7F;
		}
		else if (sel & 8)
		{
			sfx_bak = SFXVolume;

			if (dbinput & IN_LEFT)
				SoundQuality--;

			if (dbinput & IN_RIGHT)
				SoundQuality++;

			if (SoundQuality > 2)
				SoundQuality = 2;
			else if (SoundQuality < 0)
				SoundQuality = 0;

			if (SoundQuality != sfx_quality_bak)
			{
				S_SoundStopAllSamples();
				DXChangeOutputFormat(sfx_frequencies[SoundQuality], 0);
				sfx_quality_bak = SoundQuality;
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}
		}
		else if (sel & 16)
		{
			if (dbinput & IN_LEFT)
			{
				if (App.AutoTarget)
					App.AutoTarget = 0;

				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_RIGHT)
			{
				if (!App.AutoTarget)
					App.AutoTarget = 1;

				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}

			savegame.AutoTarget = App.AutoTarget;
		}
	}
}

void DisplayGameStats()
{
	long sec, days, hours, min, y;
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

long S_DisplayPauseMenu(long reset_selection, long reset_menu)
{
	static long menu, selection = 1;
	long y;

	if (!menu)
	{
		if (reset_selection)
		{
			selection = reset_selection;
			menu = 0;
			if (reset_menu >= 0) {
				menu = reset_menu;
			}
		}
		else
		{
			y = phd_centery - font_height;
			PrintString(phd_centerx, y - ((3 * font_height) >> 1), 6, GetFixedStringForTextID(TXT_Paused), FF_CENTER);
			PrintString(phd_centerx, y, selection & 1 ? 1 : 2, GetFixedStringForTextID(TXT_Statistics), FF_CENTER);
			PrintString(phd_centerx, y + font_height, selection & 2 ? 1 : 2, GetFixedStringForTextID(TXT_Options), FF_CENTER);
			PrintString(phd_centerx, y + 2 * font_height, selection & 4 ? 1 : 2, GetFixedStringForTextID(TXT_Exit_to_Title), FF_CENTER);

			if (dbinput & IN_FORWARD)
			{
				if (selection > 1)
					selection >>= 1;

				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_BACK)
			{
				if (selection < 4)
					selection <<= 1;

				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_DESELECT)
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				return 1;
			}

#ifdef USE_SDL
			if (dbinput & IN_SELECT && !keymap[SDL_SCANCODE_LALT])
#else
			if (dbinput & IN_SELECT && !keymap[DIK_LALT])
#endif
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

				if (selection & 1)
					menu = 2;
				else if (selection & 2)
					menu = 1;
				else if (selection & 4)
					return 8;
			}
		}
	}
	else if (menu == 1)
	{
		DoOptions();

		if (dbinput & IN_DESELECT)
		{
			menu = 0;
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
		}
	}
	else if (menu == 2)
	{
		DisplayGameStats();

		if (dbinput & IN_DESELECT)
		{
			menu = 0;
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

			if (get_game_mod_level_misc_info(gfCurrentLevel)->always_exit_from_statistics_screen)
				return 1;
		}
	}

	return 0;
}

long DoLoadSave(long LoadSave)
{
	// Tomb4Plus: handling for increased savegame count.

	LEGACY_SAVEFILE_INFO* pSave;
	static long selection;
	long txt;
	size_t l;
	uchar color;
	char string[80];
	char name[41];

	if (LoadSave & IN_SAVE)
		txt = TXT_Save_Game;
	else
		txt = TXT_Load_Game;

	float font_scale = 15.0f / (float)MAX_SAVEGAMES * 1.06f;
	long scaled_font_height = (long)(savegame_font_height * font_scale);

	PrintString(phd_centerx, savegame_font_height, 6, GetFixedStringForTextID(txt), FF_CENTER);

	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
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

		if (pSave->valid)
		{
			sprintf(string, "%03d", pSave->num);
			PrintStringScaled(phd_centerx - long((float)phd_winwidth / 640.0F * 310.0), savegame_font_height + scaled_font_height * (i + 2), color, string, 0, 1.0F, font_scale);
			PrintStringScaled(phd_centerx - long((float)phd_winwidth / 640.0F * 270.0), savegame_font_height + scaled_font_height * (i + 2), color, name, 0, 1.0F, font_scale);
			sprintf(string, "%d %s %02d:%02d:%02d", pSave->days, GetFixedStringForTextID(TXT_days), pSave->hours, pSave->minutes, pSave->seconds);
			PrintStringScaled(phd_centerx - long((float)phd_winwidth / 640.0F * -135.0), savegame_font_height + scaled_font_height * (i + 2), color, string, 0, 1.0F, font_scale);
		}
		else
		{
			sprintf(string, "%s", pSave->name);
			PrintStringScaled(phd_centerx, savegame_font_height + scaled_font_height * (i + 2), color, string, FF_CENTER, 1.0F, font_scale);
		}

		small_font = 0;
	}

	if (dbinput & IN_FORWARD)
	{
		selection--;
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
	}

	if (dbinput & IN_BACK)
	{
		selection++;
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
	}

	if (selection < 0)
		selection = 0;
	else if (selection >= MAX_SAVEGAMES)
		selection = MAX_SAVEGAMES - 1;

	if (dbinput & IN_SELECT)
	{
		if (SaveGames[selection].valid || LoadSave == IN_SAVE)
			return selection;

		SoundEffect(SFX_LARA_NO, 0, SFX_ALWAYS);
	}

	return -1;
}

long S_LoadSave(long load_or_save, long mono, long inv_active)
{
	long fade, ret;

	fade = 0;

	if (!mono)
		CreateMonoScreen();

	GetSaveLoadFiles();

	if (!inv_active)
		InventoryActive = 1;

	while (1)
	{
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

		if (ret >= 0)
		{
			if (load_or_save & IN_SAVE)
			{
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

		if (fade && DoFade == 2)
		{
			ret = fade - 1;
			break;
		}

		if (input & IN_OPTION)
		{
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

#ifndef USE_BGFX
static void S_DrawTile(long x, long y, long w, long h, LPDIRECT3DTEXTUREX t, long c0, long c1, long c2, long c3)
{
	GFXTLBUMPVERTEX v[4] = {};
	float u1, v1, u2, v2;

	u1 = 0;
	v1 = 0;
	u2 = 1.0F;
	v2 = 1.0F;

	v[0].sx = (float)x;
	v[0].sy = (float)y;
	v[0].sz = 0.995F;
	v[0].tu = u1;
	v[0].tv = v1;
	v[0].rhw = 1;
	v[0].color = c0;
	v[0].specular = 0xFF000000;

	v[1].sx = float(w + x);
	v[1].sy = (float)y;
	v[1].sz = 0.995F;
	v[1].tu = u2;
	v[1].tv = v1;
	v[1].rhw = 1;
	v[1].color = c1;
	v[1].specular = 0xFF000000;

	v[2].sx = float(w + x);
	v[2].sy = float(h + y);
	v[2].sz = 0.995F;
	v[2].tu = u2;
	v[2].tv = v2;
	v[2].rhw = 1;
	v[2].color = c3;
	v[2].specular = 0xFF000000;

	v[3].sx = (float)x;
	v[3].sy = float(h + y);
	v[3].sz = 0.995F;
	v[3].tu = u1;
	v[3].tv = v2;
	v[3].rhw = 1;
	v[3].color = c2;
	v[3].specular = 0xFF000000;

	App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
	App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0);
	DXAttempt(App.dx.lpD3DDevice->SetTexture(0, t));
	App.dx.lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF, v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 1);

	if (App.Filtering)
	{
		App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	}
}
#endif

void S_DisplayMonoScreen()
{
#ifndef USE_BGFX
	ulong col;

	if (tomb4.inv_bg_mode == INV_BG_MODE_ORIGINAL || tomb4.inv_bg_mode == INV_BG_MODE_CLEAR)
		col = 0xFFFFFFFF;
	else
		col = 0xFFFFFF80;

	S_DrawTile(0, 0, phd_winwidth, phd_winheight, MonoScreen.tex, col, col, col, col);
#endif
}

void CreateMonoScreen()
{
	MonoScreenOn = true;

#ifndef USE_BGFX
	if (App.dx.Flags & DXF_WINDOWED)
		ConvertSurfaceToTextures(App.dx.lpBackBuffer);
	else
		ConvertSurfaceToTextures(App.dx.lpPrimaryBuffer);
#endif
}

void FreeMonoScreen()
{
#ifndef USE_BGFX
	if (MonoScreen.surface)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Surface", MonoScreen.surface, MonoScreen.surface->Release());
		MonoScreen.surface = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Surface");

	if (MonoScreen.tex)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Texture", MonoScreen.tex, MonoScreen.tex->Release());
		MonoScreen.tex = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Texture");

#endif
	MonoScreenOn = false;
}

void RGBM_Mono(uchar * r, uchar * g, uchar * b)
{
	uchar c;

	if (tomb4.inv_bg_mode != INV_BG_MODE_CLEAR)
	{
		c = (*r + *b) >> 1;
		*r = c;
		*g = c;
		*b = c;
	}
}

static void BitMaskGetNumberOfBits(ulong bitMask, ulong& bitDepth, ulong& bitOffset)
{
	long i;

	if (!bitMask) 
	{
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

#ifndef USE_BGFX
static void WinVidGetColorBitMasks(COLOR_BIT_MASKS* bm, LPDDPIXELFORMAT pixelFormat)
{
	bm->dwRBitMask = pixelFormat->dwRBitMask;
	bm->dwGBitMask = pixelFormat->dwGBitMask;
	bm->dwBBitMask = pixelFormat->dwBBitMask;
	bm->dwRGBAlphaBitMask = pixelFormat->dwRGBAlphaBitMask;

	BitMaskGetNumberOfBits(bm->dwRBitMask, bm->dwRBitDepth, bm->dwRBitOffset);
	BitMaskGetNumberOfBits(bm->dwGBitMask, bm->dwGBitDepth, bm->dwGBitOffset);
	BitMaskGetNumberOfBits(bm->dwBBitMask, bm->dwBBitDepth, bm->dwBBitOffset);
	BitMaskGetNumberOfBits(bm->dwRGBAlphaBitMask, bm->dwRGBAlphaBitDepth, bm->dwRGBAlphaBitOffset);
}

static void CustomBlt(LPDDSURFACEDESCX dst, ulong dstX, ulong dstY, LPDDSURFACEDESCX src, LPRECT srcRect)
{
	COLOR_BIT_MASKS srcMask, dstMask;
	uchar* srcLine;
	uchar* dstLine;
	uchar* srcPtr;
	uchar* dstPtr;
	ulong srcX, srcY, width, height, srcBpp, dstBpp, color, high, low, r, g, b;

	srcX = srcRect->left;
	srcY = srcRect->top;
	width = srcRect->right - srcRect->left;
	height = srcRect->bottom - srcRect->top;
	srcBpp = src->ddpfPixelFormat.dwRGBBitCount / 8;
	dstBpp = dst->ddpfPixelFormat.dwRGBBitCount / 8;
	WinVidGetColorBitMasks(&srcMask, &src->ddpfPixelFormat);
	WinVidGetColorBitMasks(&dstMask, &dst->ddpfPixelFormat);
	srcLine = (uchar*)src->lpSurface + srcY * src->lPitch + srcX * srcBpp;
	dstLine = (uchar*)dst->lpSurface + dstY * dst->lPitch + dstX * dstBpp;

	for (ulong j = 0; j < height; j++) 
	{
		srcPtr = srcLine;
		dstPtr = dstLine;

		for (ulong i = 0; i < width; i++)
		{
			color = 0;
			memcpy(&color, srcPtr, srcBpp);
			r = (color & srcMask.dwRBitMask) >> srcMask.dwRBitOffset;
			g = (color & srcMask.dwGBitMask) >> srcMask.dwGBitOffset;
			b = (color & srcMask.dwBBitMask) >> srcMask.dwBBitOffset;

			if (srcMask.dwRBitDepth < dstMask.dwRBitDepth) 
			{
				high = dstMask.dwRBitDepth - srcMask.dwRBitDepth;
				low = (srcMask.dwRBitDepth > high) ? srcMask.dwRBitDepth - high : 0;
				r = (r << high) | (r >> low);
			}
			else if (srcMask.dwRBitDepth > dstMask.dwRBitDepth)
				r >>= srcMask.dwRBitDepth - dstMask.dwRBitDepth;

			if (srcMask.dwGBitDepth < dstMask.dwGBitDepth)
			{
				high = dstMask.dwGBitDepth - srcMask.dwGBitDepth;
				low = (srcMask.dwGBitDepth > high) ? srcMask.dwGBitDepth - high : 0;
				g = (g << high) | (g >> low);
			}
			else if (srcMask.dwGBitDepth > dstMask.dwGBitDepth)
				g >>= srcMask.dwGBitDepth - dstMask.dwGBitDepth;

			if (srcMask.dwBBitDepth < dstMask.dwBBitDepth) 
			{
				high = dstMask.dwBBitDepth - srcMask.dwBBitDepth;
				low = (srcMask.dwBBitDepth > high) ? srcMask.dwBBitDepth - high : 0;
				b = (b << high) | (b >> low);
			}
			else if (srcMask.dwBBitDepth > dstMask.dwBBitDepth)
				b >>= srcMask.dwBBitDepth - dstMask.dwBBitDepth;

			RGBM_Mono((uchar*)&r, (uchar*)&g, (uchar*)&b);
			color = dst->ddpfPixelFormat.dwRGBAlphaBitMask; // destination is opaque
			color |= r << dstMask.dwRBitOffset;
			color |= g << dstMask.dwGBitOffset;
			color |= b << dstMask.dwBBitOffset;
			memcpy(dstPtr, &color, dstBpp);
			srcPtr += srcBpp;
			dstPtr += dstBpp;
		}

		srcLine += src->lPitch;
		dstLine += dst->lPitch;
	}
}
#endif

#ifndef USE_BGFX
void ConvertSurfaceToTextures(LPDIRECTDRAWSURFACEX surface)
{
	DDSURFACEDESCX tSurf;
	DDSURFACEDESCX uSurf;
	RECT r = {};
	ushort* pTexture;
	ushort* pSrc;

	memset(&tSurf, 0, sizeof(tSurf));
	tSurf.dwSize = sizeof(DDSURFACEDESCX);
	surface->Lock(0, &tSurf, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
	pSrc = (ushort*)tSurf.lpSurface;
	MonoScreen.surface = CreateTexturePage(tSurf.dwWidth, tSurf.dwHeight, 0, 0, RGBM_Mono, -1);

	memset(&uSurf, 0, sizeof(uSurf));
	uSurf.dwSize = sizeof(DDSURFACEDESCX);
	MonoScreen.surface->Lock(0, &uSurf, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
	pTexture = (ushort*)uSurf.lpSurface;

	r.left = 0;
	r.top = 0;
	r.right = tSurf.dwWidth;
	r.bottom = tSurf.dwHeight;
	CustomBlt(&uSurf, 0, 0, &tSurf, &r);

	MonoScreen.surface->Unlock(0);
	DXAttempt(MonoScreen.surface->QueryInterface(TEXGUID, (void**)&MonoScreen.tex));
	surface->Unlock(0);
}
#endif

void CheckKeyConflicts()
{
	short key;

	for (int i = 0; i < 18; i++)
	{
		key = keyboard_layout[0][i];
		conflict[i] = 0;

		for (int j = 0; j < 18; j++)
		{
			if (key == keyboard_layout[1][j])
			{
				conflict[i] = 1;
				break;
			}
		}
	}
}

long S_PauseMenu(long force_menu)
{
	long fade, ret;

	fade = 0;
	CreateMonoScreen();
	S_DisplayPauseMenu(1, force_menu);
	InventoryActive = 1;
	S_SetReverbType(1);

	do
	{
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

		if (ret == 8)
		{
			fade = 8;
			ret = 0;
			SetFade(0, 255);
		}

		if (fade && DoFade == 2)
		{
			ret = fade;
			break;
		}

	} while (!MainThread.ended);

	TIME_Init();
	FreeMonoScreen();
	InventoryActive = 0;
	return ret;
}

long GetSaveLoadFiles()
{
	FILE* file;
	LEGACY_SAVEFILE_INFO *pSave;
	LEGACY_SAVEGAME_INFO save_info;
	static long nSaves;
	char name[75];

	SaveCounter = 0;

	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
		pSave = &SaveGames[i];
		sprintf(name, "savegame.%d", i);

		std::string full_path = savegame_dir_path + name;

		file = platform_fopen(full_path.c_str(), "rb");

		if (!file)
		{
			pSave->valid = 0;
			strcpy(pSave->name, GetFixedStringForTextID(TXT_Empty_Slot));
			continue;
		}

		fread(&pSave->name, sizeof(char), 75, file);
		fread(&pSave->num, sizeof(long), 1, file);
		fread(&pSave->days, sizeof(short), 1, file);
		fread(&pSave->hours, sizeof(short), 1, file);
		fread(&pSave->minutes, sizeof(short), 1, file);
		fread(&pSave->seconds, sizeof(short), 1, file);
		fread(&save_info, 1, sizeof(LEGACY_SAVEGAME_INFO), file);

		if (!CheckSumValid((char*)&save_info))
		{
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
