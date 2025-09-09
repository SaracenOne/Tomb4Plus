#include "../tomb4/pch.h"
#include "troyestuff.h"
#include "../game/sound.h"
#include "../game/text.h"
#include "tomb4.h"
#include "../specific/input.h"
#include "../specific/3dmath.h"

#define PAGE0_NUM	14
#define PAGE1_NUM	12
#define YPOS	textY + y++ * font_height
#define CHECK_SEL(c)	selection & (1 << s++) ? 1 : c

void TroyeMenu(int32_t textY, int32_t& menu, uint32_t& selection) {
	int32_t num;
	static int32_t page = 0;
	bool changed;

	num = 0;
	changed = 0;

	if (dbinput & IN_DESELECT) {
		SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
		menu = 0;
		dbinput &= ~IN_DESELECT;
		page = 0;
		return;
	}

	font_height = smol_font_height;

	switch (page) {
		case 0:
			changed = Page0(num, textY, selection);
			break;

		case 1:
			changed = Page1(num, textY, selection);
			break;
	}

	PrintString(phd_centerx - (phd_centerx >> 3), textY + (num + 2) * font_height, selection & (1 << num) ? 1 : 6, "\x19", 0);
	PrintString(phd_centerx + (phd_centerx >> 3), textY + (num + 2) * font_height, selection & (1 << num) ? 1 : 6, "\x1B", 0);

	font_height = stash_font_height;

	if (dbinput & IN_FORWARD) {
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
		selection >>= 1;
	}

	if (dbinput & IN_BACK) {
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
		selection <<= 1;
	}

	if (!selection)
		selection = 1;

	if (selection > uint32_t(1 << num))
		selection = 1 << num;

	if (selection & (1 << num)) {
		if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

			if (page) {
				page = 0;
				num = PAGE0_NUM;
			} else {
				page = 1;
				num = PAGE1_NUM;
			}

			selection = 1 << num;
		}
	}

	if (changed)
		save_new_tomb4_settings();
}

bool Page0(int32_t& num, int32_t textY, uint32_t selection) {
	char buffer[80];
	int32_t y, s;
	bool changed;

	changed = 0;
	num = PAGE0_NUM;
	y = 2;
	s = 0;

	PrintString(phd_centerx, 2 * font_height, 6, "New Tomb4 Options", FF_CENTER);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Footprints", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Shadow mode", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Crawl Tilting", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Flexible Crawling", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Fix Climb Up Delay", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Gameover Menu", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Bar Mode", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Bar Positions", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Enemy Bars", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Cutscene Skipper", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Cheats", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Loading text", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Mono Screen Style", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Loadbar Style", 0);

	y = 2;
	s = 0;

	strcpy(buffer, tomb4.footprints ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.shadow_mode == SHADOW_MODE_ORIGINAL ?
	       "original" : tomb4.shadow_mode == SHADOW_MODE_CIRCLE ?
	       "circle" : tomb4.shadow_mode == SHADOW_MODE_PSX_CIRCLE ?
	       "faded circle" : "PSX");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.crawltilt ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.flexible_crawling ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.fix_climb_up_delay ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.gameover ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.bar_mode == BAR_MODE_ORIGINAL ?
	       "original" : tomb4.bar_mode == BAR_MODE_IMPROVED ?
	       "TR5" : tomb4.bar_mode == BAR_MODE_PSX ? "PSX" : "Custom");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.bars_pos == BARS_POS_ORIGINAL ?
	       "original" : tomb4.bars_pos == BARS_POS_IMPROVED ?
	       "improved" : tomb4.bars_pos == BARS_POS_PSX ? "PSX" : "Custom");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.enemy_bars ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.cutseq_skipper ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.cheats ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.loadingtxt ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.inv_bg_mode == INV_BG_MODE_ORIGINAL ?
	       "original" : tomb4.inv_bg_mode == INV_BG_MODE_TR5 ? "TR5" : "clear");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.tr5_loadbar ? "TR5" : "TR4");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	switch (selection) {
		case 1 << 0:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.footprints = !tomb4.footprints;
				changed = 1;
			}

			break;

		case 1 << 1:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.shadow_mode = (shadow_mode_enum)((int32_t)tomb4.shadow_mode + 1);

				if (tomb4.shadow_mode >= SHADOW_MODE_ENUM_SIZE)
					tomb4.shadow_mode = (shadow_mode_enum)((int32_t)SHADOW_MODE_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.shadow_mode = (shadow_mode_enum)((int32_t)tomb4.shadow_mode - 1);
				;
				if (tomb4.shadow_mode == SHADOW_MODE_NULL)
					tomb4.shadow_mode = (shadow_mode_enum)((int32_t)SHADOW_MODE_ENUM_SIZE - 1);


				changed = 1;
			}

			break;

		case 1 << 2:

			if (dbinput & IN_RIGHT || dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.crawltilt = !tomb4.crawltilt;
				changed = 1;
			}

			break;

		case 1 << 3:

			if (dbinput & IN_RIGHT || dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.flexible_crawling = !tomb4.flexible_crawling;
				changed = 1;
			}

			break;

		case 1 << 4:

			if (dbinput & IN_RIGHT || dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.fix_climb_up_delay = !tomb4.fix_climb_up_delay;
				changed = 1;
			}

			break;

		case 1 << 5:

			if (dbinput & IN_RIGHT || dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.gameover = !tomb4.gameover;
				changed = 1;
			}

			break;

		case 1 << 6:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.bar_mode = (bar_mode_enum)((int32_t)tomb4.bar_mode + 1);

				if (tomb4.bar_mode >= BAR_MODE_ENUM_SIZE)
					tomb4.bar_mode = (bar_mode_enum)((int32_t)BAR_MODE_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.bar_mode = (bar_mode_enum)((int32_t)tomb4.bar_mode - 1);
				;
				if (tomb4.bar_mode == BAR_MODE_NULL)
					tomb4.bar_mode = (bar_mode_enum)((int32_t)BAR_MODE_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

		case 1 << 7:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.bars_pos = (bars_pos_enum)((int32_t)tomb4.bars_pos + 1);

				if (tomb4.bars_pos >= BARS_POS_ENUM_SIZE)
					tomb4.bars_pos = (bars_pos_enum)((int32_t)BARS_POS_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.bars_pos = (bars_pos_enum)((int32_t)tomb4.bars_pos - 1);
				;
				if (tomb4.bars_pos == BARS_POS_NULL)
					tomb4.bars_pos = (bars_pos_enum)((int32_t)BARS_POS_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

		case 1 << 8:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.enemy_bars = !tomb4.enemy_bars;
				changed = 1;
			}

			break;

		case 1 << 9:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.cutseq_skipper = !tomb4.cutseq_skipper;
				changed = 1;
			}

			break;

		case 1 << 10:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.cheats = !tomb4.cheats;
				changed = 1;
			}

			break;

		case 1 << 11:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.loadingtxt = !tomb4.loadingtxt;
				changed = 1;
			}

			break;

		case 1 << 12:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.inv_bg_mode = (inv_bg_mode_enum)((int32_t)tomb4.inv_bg_mode + 1);

				if (tomb4.inv_bg_mode >= INV_BG_MODE_ENUM_SIZE)
					tomb4.inv_bg_mode = (inv_bg_mode_enum)((int32_t)INV_BG_MODE_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.inv_bg_mode = (inv_bg_mode_enum)((int32_t)tomb4.inv_bg_mode - 1);
				;
				if (tomb4.inv_bg_mode == INV_BG_MODE_NULL)
					tomb4.inv_bg_mode = (inv_bg_mode_enum)((int32_t)INV_BG_MODE_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

		case 1 << 13:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.tr5_loadbar = !tomb4.tr5_loadbar;
				changed = 1;
			}

			break;
	}

	return changed;
}

bool Page1(int32_t& num, int32_t textY, uint32_t selection) {
	char buffer[80];
	int32_t y, s;
	bool changed;

	changed = 0;
	num = PAGE1_NUM;
	y = 2;
	s = 0;

	PrintString(phd_centerx, 2 * font_height, 6, "New Tomb4 Options", FF_CENTER);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Look Transparency", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Ammo Counter", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Ammotype Hotkeys", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Combat Cam Tilt", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Show Healthbar in Inventory", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Light Static Objects", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Reverb", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Minimum Clip Range", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Bars Scale", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Freeze When Game Unfocused", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Shade Inventory Items", 0);
	PrintString(phd_centerx >> 2, YPOS, CHECK_SEL(2), "Volumetric Flash Grenades", 0);

	y = 2;
	s = 0;

	switch (tomb4.look_transparency) {
		case LOOK_TRANSPARENCY_DEFAULT:
			strcpy(buffer, "default");
			break;
		case LOOK_TRANSPARENCY_ON:
			strcpy(buffer, "on");
			break;
		case LOOK_TRANSPARENCY_OFF:
			strcpy(buffer, "off");
			break;
		default:
			strcpy(buffer, "???");
			break;
	}
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.ammo_counter ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.ammotype_hotkeys ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.combat_cam_tilt ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.hpbar_inv ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.static_lighting ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.reverb == REVERB_OFF ?
	       "off" : tomb4.reverb == REVERB_LARA_ROOM ? "Lara room" : "Camera room");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	sprintf(buffer, "%i", tomb4.minimum_clip_range);
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	sprintf(buffer, "%.1f", tomb4.GUI_Scale);
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	strcpy(buffer, tomb4.hang_game_thread ? "on" : "off");
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	switch (tomb4.pickup_lighting) {
		case PICKUP_LIGHTING_DEFAULT:
			strcpy(buffer, "default");
			break;
		case PICKUP_LIGHTING_OFF:
			strcpy(buffer, "off");
			break;
		case PICKUP_LIGHTING_ON:
			strcpy(buffer, "on");
			break;
		default:
			strcpy(buffer, "???");
			break;
	}
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	switch (tomb4.volumetric_flash_grenades) {
		case VOLUMETRIC_FLASH_GRENADES_DEFAULT:
			strcpy(buffer, "default");
			break;
		case VOLUMETRIC_FLASH_GRENADES_FLASH_ONLY:
			strcpy(buffer, "flash");
			break;
		case VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_ONLY:
			strcpy(buffer, "volumetric");
			break;
		case VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_AND_FLASH:
			strcpy(buffer, "flash+volumetric");
			break;
		default:
			strcpy(buffer, "???");
			break;
	}
	PrintString(phd_centerx + (phd_centerx >> 1), YPOS, CHECK_SEL(6), buffer, 0);

	switch (selection) {
		case 1 << 0:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.look_transparency = (look_transparency_enum)((int32_t)tomb4.look_transparency + 1);

				if (tomb4.look_transparency >= LOOK_TRANSPARENCY_ENUM_SIZE)
					tomb4.look_transparency = (look_transparency_enum)((int32_t)LOOK_TRANSPARENCY_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.look_transparency = (look_transparency_enum)((int32_t)tomb4.look_transparency - 1);
				;
				if (tomb4.look_transparency == LOOK_TRANSPARENCY_NULL)
					tomb4.look_transparency = (look_transparency_enum)((int32_t)LOOK_TRANSPARENCY_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

		case 1 << 1:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.ammo_counter = !tomb4.ammo_counter;
				changed = 1;
			}

			break;

		case 1 << 2:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.ammotype_hotkeys = !tomb4.ammotype_hotkeys;
				changed = 1;
			}

			break;

		case 1 << 3:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.combat_cam_tilt = !tomb4.combat_cam_tilt;
				changed = 1;
			}

			break;

		case 1 << 4:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.hpbar_inv = !tomb4.hpbar_inv;
				changed = 1;
			}

			break;

		case 1 << 5:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.static_lighting = !tomb4.static_lighting;
				changed = 1;
			}

			break;

		case 1 << 6:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.reverb = (reverb_enum)((int32_t)tomb4.reverb + 1);

				if (tomb4.reverb >= REVERB_ENUM_SIZE)
					tomb4.reverb = (reverb_enum)((int32_t)REVERB_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.reverb = (reverb_enum)((int32_t)tomb4.reverb - 1);
				;
				if (tomb4.reverb == REVERB_NULL)
					tomb4.reverb = (reverb_enum)((int32_t)REVERB_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

		case 1 << 7:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.minimum_clip_range++;

				if (tomb4.minimum_clip_range > 64)
					tomb4.minimum_clip_range = 64;

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				if (tomb4.minimum_clip_range > 0)
					tomb4.minimum_clip_range--;

				changed = 1;
			}

			break;

		case 1 << 8:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.GUI_Scale += 0.1F;

				if (tomb4.GUI_Scale > 1.9F)
					tomb4.GUI_Scale = 1.9F;

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.GUI_Scale -= 0.1F;

				if (tomb4.GUI_Scale < 1.0F)
					tomb4.GUI_Scale = 1.0F;

				changed = 1;
			}

			break;
		case 1 << 9:

			if (dbinput & IN_LEFT || dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.hang_game_thread = !tomb4.hang_game_thread;
				changed = 1;
			}

			break;
		case 1 << 10:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.pickup_lighting = (pickup_lighting_enum)((int32_t)tomb4.pickup_lighting + 1);

				if (tomb4.pickup_lighting >= PICKUP_LIGHTING_ENUM_SIZE)
					tomb4.pickup_lighting = (pickup_lighting_enum)((int32_t)PICKUP_LIGHTING_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.pickup_lighting = (pickup_lighting_enum)((int32_t)tomb4.pickup_lighting - 1);

				if (tomb4.pickup_lighting == PICKUP_LIGHTING_NULL)
					tomb4.pickup_lighting = (pickup_lighting_enum)((int32_t)PICKUP_LIGHTING_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

		case 1 << 11:

			if (dbinput & IN_RIGHT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.volumetric_flash_grenades = (volumetric_flash_grenades_enum)((int32_t)tomb4.volumetric_flash_grenades + 1);

				if (tomb4.volumetric_flash_grenades >= VOLUMETRIC_FLASH_GRENADES_ENUM_SIZE)
					tomb4.volumetric_flash_grenades = (volumetric_flash_grenades_enum)((int32_t)VOLUMETRIC_FLASH_GRENADES_NULL + 1);

				changed = 1;
			}

			if (dbinput & IN_LEFT) {
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				tomb4.volumetric_flash_grenades = (volumetric_flash_grenades_enum)((int32_t)tomb4.volumetric_flash_grenades - 1);

				if (tomb4.volumetric_flash_grenades == VOLUMETRIC_FLASH_GRENADES_NULL)
					tomb4.volumetric_flash_grenades = (volumetric_flash_grenades_enum)((int32_t)VOLUMETRIC_FLASH_GRENADES_ENUM_SIZE - 1);

				changed = 1;
			}

			break;

	}

	return changed;
}

#undef PAGE0_NUM
#undef PAGE1_NUM
#undef YPOS
#undef CHECK_SEL
