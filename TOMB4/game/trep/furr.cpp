#include "../../tomb4/pch.h"

#include "furr.h"
#include "../../tomb4/tomb4plus/t4plus_weather.h"
#include "../../tomb4/tomb4plus/t4plus_inventory.h"
#include "../control.h"
#include "../../specific/function_stubs.h"
#include "../../specific/input.h"
#include "../lara.h"
#include "../objects.h"
#include "../effects.h"
#include "../gameflow.h"
#include "../../specific/LoadSave.h"
#include "../tomb4fx.h"
#include "../items.h"
#include "../traps.h"
#include "../sound.h"
#include "../../specific/audio.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"
#include "../../tomb4/tomb4plus/t4plus_objects.h"

int8_t furr_oneshot_buffer[LAST_FURR_FLIPEFFECT];
FURRFlipeffectTable furr_flipeffect_table[LAST_FURR_FLIPEFFECT - FIRST_FURR_FLIPEFFECT];

int furr_get_state_field(ITEM_INFO *item, int item_state_address_offset) {
	switch(item_state_address_offset) {
		case 14: {
			return item->current_anim_state;
			break;
		}
		case 16: {
			return item->goal_anim_state;
			break;
		}
		case 18: {
			return item->required_anim_state;
			break;
		}
		case 20: {
			return item->anim_number;
			break;
		}
		case 22: {
			return item->frame_number;
			break;
		}
		case 24: {
			return item->room_number;
			break;
		}
		case 34: {
			return item->hit_points;
			break;
		}
		case 64: {
			return item->pos.x_pos;
			break;
		}
		case 68: {
			return item->pos.y_pos;
			break;
		}
		case 72: {
			return item->pos.z_pos;
			break;
		}
		case 76: {
			return item->pos.x_rot;
			break;
		}
		case 78: {
			return item->pos.y_rot;
			break;
		}
		case 80: {
			return item->pos.z_rot;
			break;
		}
		default: {
			Log(0, "item_state_address_offset %s has not yet been implemented.\n", item_state_address_offset);
			break;
		}
	}

	return 0;
}

// Params:
// ID_1
// ID_2
FURRResult furr_cmd_oneshot(FURRParameters params) {
	if (params.first_parameter == params.second_parameter) {
		if (params.first_parameter < LAST_FURR_FLIPEFFECT) {
			furr_oneshot_buffer[params.first_parameter] = true;
		} else {
			Log(1, "ONESHOT FURR command wrote outside of valid memory range!");
		}
	} else {
		Log(1, "Unknown ONESHOT FURR command behaviour!");
		return FURR_RESULT_ERROR;
	}

	return FURR_RESULT_OK;
}

// Params:
// TO_EQUIP
FURRResult furr_cmd_equip(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_holster_weapons(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// AMOUNT
// LIMIT
FURRResult furr_cmd_inc_hp(FURRParameters params) {
	items[lara.item_number].hit_points += params.first_parameter;
	if (items[lara.item_number].hit_points > params.second_parameter) {
		items[lara.item_number].hit_points = params.second_parameter;
	}

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
FURRResult furr_cmd_dec_hp(FURRParameters params) {
	items[lara.item_number].hit_points -= params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
// LIMIT
FURRResult furr_cmd_inc_air(FURRParameters params) {
	lara.air += params.first_parameter;
	if (lara.air > params.second_parameter) {
		lara.air = params.second_parameter;
	}

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
FURRResult furr_cmd_dec_air(FURRParameters params) {
	lara.air -= params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
// LIMIT
FURRResult furr_cmd_inc_sprint(FURRParameters params) {
	DashTimer += params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
FURRResult furr_cmd_dec_sprint(FURRParameters params) {
	DashTimer -= params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
FURRResult furr_cmd_set_poison(FURRParameters params) {
	lara.poisoned = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_kill_lara(FURRParameters params) {
	if (items[lara.item_number].hit_points >= 0 && lara.water_status != LW_FLYCHEAT) {
		items[lara.item_number].hit_status = 1;
		items[lara.item_number].hit_points = -1;
	}

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_remove_pistols(FURRParameters params) {
	lara.pistols_type_carried = 0;

	return FURR_RESULT_OK;
}

// Params:
// ONOFF
FURRResult furr_cmd_set_binoculars(FURRParameters params) {
	if (params.first_parameter == 0) {
		lara.binoculars = 0;
	} else {
		lara.binoculars = 1;
	}

	return FURR_RESULT_OK;
}

// Params:
// ONOFF
FURRResult furr_cmd_set_crowbar(FURRParameters params) {
	if (params.first_parameter == 0) {
		lara.crowbar = 0;
	} else {
		lara.crowbar = 1;
	}

	return FURR_RESULT_OK;
}

// Params:
// ONOFF
FURRResult furr_cmd_set_lasersight(FURRParameters params) {
	if (params.first_parameter == 0) {
		lara.lasersight = 0;
	} else {
		lara.lasersight = 1;
	}

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_deactive_weapons(FURRParameters params) {
	lara.last_gun_type = WEAPON_NONE;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_activate_pistols(FURRParameters params) {
	lara.last_gun_type = WEAPON_PISTOLS;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_activate_uzis(FURRParameters params) {
	lara.last_gun_type = WEAPON_UZI;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_activate_revolver(FURRParameters params) {
	lara.last_gun_type = WEAPON_REVOLVER;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_activate_shotgun(FURRParameters params) {
	lara.last_gun_type = WEAPON_SHOTGUN;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_activate_grenadegun(FURRParameters params) {
	lara.last_gun_type = WEAPON_GRENADE;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_activate_crossbow(FURRParameters params) {
	lara.last_gun_type = WEAPON_CROSSBOW;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_empty_holsters(FURRParameters params) {
	lara.holster = T4PlusGetLaraHolstersSlotID();

	return FURR_RESULT_OK;
}

// Params:
// WEAPON
FURRResult furr_cmd_fill_holsters(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_empty_backdraw(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// WEAPON
FURRResult furr_cmd_fill_backdraw(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_remove_all_guns(FURRParameters params) {
	T4PlusSetInventoryCount(PISTOLS_ITEM, 0, false);
	T4PlusSetInventoryCount(SHOTGUN_ITEM, 0, false);
	T4PlusSetInventoryCount(UZI_ITEM, 0, false);
	T4PlusSetInventoryCount(SIXSHOOTER_ITEM, 0, false);
	T4PlusSetInventoryCount(CROSSBOW_ITEM, 0, false);
	T4PlusSetInventoryCount(GRENADE_GUN_ITEM, 0, false);

	T4PlusSetValidLaraGunType();

	return FURR_RESULT_OK;
}

// Params:
// Flip Number
FURRResult furr_cmd_call_flip(FURRParameters params) {
	effect_routines[params.first_parameter](0);

	return FURR_RESULT_OK;
}

// Params:
// VALUE
FURRResult furr_cmd_move_byte(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// VALUE
FURRResult furr_cmd_move_word(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// VALUE
FURRResult furr_cmd_move_dword(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// KEYTYPE
FURRResult furr_cmd_add_key(FURRParameters params) {
	lara.keyitems |= params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// KEYTYPE
FURRResult furr_cmd_remove_key(FURRParameters params) {
	lara.keyitems &= ~params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// QUEST_ITEM_ID
FURRResult furr_cmd_add_questitem(FURRParameters params) {
	if (params.first_parameter >= QUEST_ITEM1 && params.first_parameter <= QUEST_ITEM6) {
		T4PlusSetInventoryCount(params.first_parameter, params.second_parameter, false);
		return FURR_RESULT_OK;
	} else {
		return FURR_RESULT_ERROR;
	}

	return FURR_RESULT_OK;
}

// Params:
// QUEST_ITEM_ID
FURRResult furr_cmd_remove_questitem(FURRParameters params) {
	T4PlusSetInventoryCount(params.first_parameter, 0, false);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_add_examine1(FURRParameters params) {
	T4PlusSetInventoryCount(EXAMINE1, 1, false);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_remove_examine1(FURRParameters params) {
	T4PlusSetInventoryCount(EXAMINE1, 0, false);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_add_examine2(FURRParameters params) {
	T4PlusSetInventoryCount(EXAMINE2, 1, false);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_remove_examine2(FURRParameters params) {
	T4PlusSetInventoryCount(EXAMINE2, 0, false);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_add_examine3(FURRParameters params) {
	T4PlusSetInventoryCount(EXAMINE3, 1, false);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_remove_examine3(FURRParameters params) {
	T4PlusSetInventoryCount(EXAMINE3, 0, false);

	return FURR_RESULT_OK;
}

// Params:
// WATERSKIN_ID
FURRResult furr_cmd_set_waterskin1(FURRParameters params) {
	lara.small_water_skin = (uint8_t)params.first_parameter & 0xff;

	return FURR_RESULT_OK;
}

// Params:
// WATERSKIN_ID
FURRResult furr_cmd_set_waterskin2(FURRParameters params) {
	lara.big_water_skin = (uint8_t)params.first_parameter & 0xff;

	return FURR_RESULT_OK;
}

// Params:
// PUZZLEITEM_ID, AMOUNT
FURRResult furr_cmd_set_puzzleitem(FURRParameters params) {
	if (params.first_parameter >= PUZZLE_ITEM1 && params.first_parameter <= PUZZLE_ITEM12) {
		T4PlusSetInventoryCount(params.first_parameter, params.second_parameter, false);
		return FURR_RESULT_OK;
	} else {
		return FURR_RESULT_ERROR;
	}
}

// Params:
// PUZZLEITEM_ID, AMOUNT
FURRResult furr_cmd_add_puzzleitem(FURRParameters params) {
	if (params.first_parameter >= PUZZLE_ITEM1 && params.first_parameter <= PUZZLE_ITEM12) {
		lara.puzzleitems[params.first_parameter - 175] += (int8_t)params.second_parameter & 0xff;
		return FURR_RESULT_OK;
	} else {
		return FURR_RESULT_ERROR;
	}
}

// Params:
// PUZZLECOMBO_ID
FURRResult furr_cmd_add_puzzlecombo(FURRParameters params) {
	if (params.first_parameter >= 0 && params.first_parameter <= 15) {
		lara.puzzleitemscombo |= (1 << (params.first_parameter));
		return FURR_RESULT_OK;
	} else {
		return FURR_RESULT_ERROR;
	}
}

// Params:
// SUPPLY_ID
FURRResult furr_cmd_set_supply(FURRParameters params) {
	switch (params.first_parameter) {
		case 0x80DFFF: // (Pistol Ammo)
			lara.num_pistols_ammo = params.second_parameter;
			break;
		case 0x80E001: // (Uzi Ammo)
			lara.num_uzi_ammo = params.second_parameter;
			break;
		case 0x80E005: // (Shotgun Normal Ammo)
			lara.num_shotgun_ammo1 = params.second_parameter;
			break;
		case 0x80E007: // (Shotgun Wideshot Ammo)
			lara.num_shotgun_ammo1 = params.second_parameter;
			break;
		case 0x80E003: // (Revolver Ammo)
			lara.num_revolver_ammo = params.second_parameter;
			break;
		case 0x80E009: // (Grenade Normal Ammo)
			lara.num_grenade_ammo1 = params.second_parameter;
			break;
		case 0x80E00B: // (Grenade Super Ammo)
			lara.num_grenade_ammo2 = params.second_parameter;
			break;
		case 0x80E00D: // (Grenade Flash Ammo)
			lara.num_grenade_ammo3 = params.second_parameter;
			break;
		case 0x80E00F: // (Crossbow Normal Ammo)
			lara.num_crossbow_ammo1 = params.second_parameter;
			break;
		case 0x80E011: // (Crossbow Poison Ammo)
			lara.num_crossbow_ammo2 = params.second_parameter;
			break;
		case 0x80E013: // (Crossbow Explosive Ammo)
			lara.num_crossbow_ammo3 = params.second_parameter;
			break;
		case 0x80DFF9: // (Small Medkit)
			lara.num_small_medipack = params.second_parameter;
			break;
		case 0x80DFFB: // (Large Medkit)
			lara.num_large_medipack = params.second_parameter;
			break;
		case 0x80DFFD: // (Flares)
			lara.num_flares = params.second_parameter;
			break;
		default:
			return FURR_RESULT_ERROR;
	}

	return FURR_RESULT_OK;
}

// Params:
// SUPPLY_ID
FURRResult furr_cmd_add_supply(FURRParameters params) {
	switch (params.first_parameter) {
		case 0x80DFFF: // (Pistol Ammo)
			lara.num_pistols_ammo += params.second_parameter;
			break;
		case 0x80E001: // (Uzi Ammo)
			lara.num_uzi_ammo += params.second_parameter;
			break;
		case 0x80E005: // (Shotgun Normal Ammo)
			lara.num_shotgun_ammo1 += params.second_parameter;
			break;
		case 0x80E007: // (Shotgun Wideshot Ammo)
			lara.num_shotgun_ammo1 += params.second_parameter;
			break;
		case 0x80E003: // (Revolver Ammo)
			lara.num_revolver_ammo += params.second_parameter;
			break;
		case 0x80E009: // (Grenade Normal Ammo)
			lara.num_grenade_ammo1 += params.second_parameter;
			break;
		case 0x80E00B: // (Grenade Super Ammo)
			lara.num_grenade_ammo2 += params.second_parameter;
			break;
		case 0x80E00D: // (Grenade Flash Ammo)
			lara.num_grenade_ammo3 += params.second_parameter;
			break;
		case 0x80E00F: // (Crossbow Normal Ammo)
			lara.num_crossbow_ammo1 += params.second_parameter;
			break;
		case 0x80E011: // (Crossbow Poison Ammo)
			lara.num_crossbow_ammo2 += params.second_parameter;
			break;
		case 0x80E013: // (Crossbow Explosive Ammo)
			lara.num_crossbow_ammo3 += params.second_parameter;
			break;
		case 0x80DFF9: // (Small Medkit)
			lara.num_small_medipack += params.second_parameter;
			break;
		case 0x80DFFB: // (Large Medkit)
			lara.num_large_medipack += params.second_parameter;
			break;
		case 0x80DFFD: // (Flares)
			lara.num_flares += params.second_parameter;
			break;
		default:
			return FURR_RESULT_ERROR;
	}

	return FURR_RESULT_OK;
}

// Params:
// WEAPON_ID
FURRResult furr_cmd_add_weapon(FURRParameters params) {
	switch (params.first_parameter) {
		case 0x80DFD2: // (Pistol)
			lara.pistols_type_carried |= W_PRESENT;
			break;
		case 0x80DFD3: // (Uzi)
			lara.uzis_type_carried |= W_PRESENT;
			break;
		case 0x80DFD4: // (Shotgun)
			lara.shotgun_type_carried |= W_PRESENT;
			break;
		case 0x80DFD5: // (Crossbow)
			lara.crossbow_type_carried |= W_PRESENT;
			break;
		case 0x80DFD6: // (Grenade Gun)
			lara.grenade_type_carried |= W_PRESENT;
			break;
		case 0x80DFD7: // (Revolver)
			lara.sixshooter_type_carried |= W_PRESENT;
			break;
		default:
			return FURR_RESULT_ERROR;
	}

	return FURR_RESULT_OK;
}

// Params:
// WEAPON_ID
FURRResult furr_cmd_remove_weapon(FURRParameters params) {
	switch (params.first_parameter) {
		case 0x80DFD2: // (Pistol)
			lara.pistols_type_carried = W_NONE;
			break;
		case 0x80DFD3: // (Uzi)
			lara.uzis_type_carried = W_NONE;
			break;
		case 0x80DFD4: // (Shotgun)
			lara.shotgun_type_carried = W_NONE;
			break;
		case 0x80DFD5: // (Crossbow)
			lara.crossbow_type_carried = W_NONE;
			break;
		case 0x80DFD6: // (Grenade Gun)
			lara.grenade_type_carried = W_NONE;
			break;
		case 0x80DFD7: // (Revolver)
			lara.sixshooter_type_carried = W_NONE;
			break;
		default:
			return FURR_RESULT_ERROR;
	}

	return FURR_RESULT_OK;
}

// Params:
// SECONDS
FURRResult furr_cmd_start_timer(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// TIME
FURRResult furr_cmd_start_timer_from_time(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// SECONDS
FURRResult furr_cmd_set_timer(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_stop_timer(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_freeze_normal_timer(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_freeze_inverted_timer(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// LEVEL_NUM
FURRResult furr_cmd_load_level(FURRParameters params) {
	gfLevelComplete = (uint8_t)params.first_parameter;
	gfRequiredStartPos = 0;

	return FURR_RESULT_OK;
}

// The flash colours are likely not accurate. Should investigate.
// Params:
FURRResult furr_cmd_flash_red(FURRParameters params) {
	FlashFadeR = 0xff;
	FlashFadeG = 0x00;
	FlashFadeB = 0x00;
	FlashFader = 32;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_flash_orange(FURRParameters params) {
	FlashFadeR = 0xff;
	FlashFadeG = 0xbe;
	FlashFadeB = 0x00;
	FlashFader = 32;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_flash_yellow(FURRParameters params) {
	FlashFadeR = 0xff;
	FlashFadeG = 0xff;
	FlashFadeB = 0x00;
	FlashFader = 32;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_flash_green(FURRParameters params) {
	FlashFadeR = 0x00;
	FlashFadeG = 0xff;
	FlashFadeB = 0x00;
	FlashFader = 32;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_flash_lightgreen(FURRParameters params) {
	FlashFadeR = 0xbe;
	FlashFadeG = 0xff;
	FlashFadeB = 0x00;
	FlashFader = 32;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_flash_blue(FURRParameters params) {
	FlashFadeR = 0x00;
	FlashFadeG = 0x00;
	FlashFadeB = 0xff;
	FlashFader = 32;

	return FURR_RESULT_OK;
}

// Params:
// AMOUNT
FURRResult furr_cmd_shake_camera(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_shake_camera_soft(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_shake_camera_medium(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_shake_camera_heavy(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// DRIP_AMOUNT
FURRResult furr_cmd_lara_drips(FURRParameters params) {
	for (int i = 0; i < WET_COUNT; i++) {
		lara.wet[i] = (uint8_t)params.first_parameter;
	}
	return FURR_RESULT_OK;
}


// Params:
// LEGEND_TIME
FURRResult furr_cmd_legend_time(FURRParameters params) {
	gfLegendTime = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// LEGEND_STRING
FURRResult furr_cmd_legend_string(FURRParameters params) {
	gfLegend = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ANIM_INDEX
FURRResult furr_cmd_play_anim(FURRParameters params) {
	items[lara.item_number].anim_number = params.first_parameter;
	items[lara.item_number].frame_number = anims[items[lara.item_number].anim_number].frame_base;

	return FURR_RESULT_OK;
}

// Params:
// ORIENTATION
FURRResult furr_cmd_orientate(FURRParameters params) {
	items[lara.item_number].pos.y_rot = (int16_t)params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ROTATION
FURRResult furr_cmd_rotate(FURRParameters params) {
	items[lara.item_number].pos.y_rot += (int16_t)params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// X_COORDINATE
FURRResult furr_cmd_change_position_x(FURRParameters params) {
	items[lara.item_number].pos.x_pos = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// Y_COORDINATE
FURRResult furr_cmd_change_position_y(FURRParameters params) {
	items[lara.item_number].pos.y_pos = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// Z_COORDINATE
FURRResult furr_cmd_change_position_z(FURRParameters params) {
	items[lara.item_number].pos.z_pos = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ACCEL
FURRResult furr_cmd_change_accel(FURRParameters params) {
	items[lara.item_number].speed = (int16_t)params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// VERT_ACCEL
FURRResult furr_cmd_change_vert_accel(FURRParameters params) {
	items[lara.item_number].fallspeed = (int16_t)params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ROOM_NUMBER
FURRResult furr_cmd_change_room(FURRParameters params) {
	items[lara.item_number].room_number = params.first_parameter;

	return FURR_RESULT_OK;
}

// Params:
// X_AMOUNT
// Z_AMOUNT
FURRResult furr_cmd_add_position(FURRParameters params) {
	items[lara.item_number].pos.x_pos = params.first_parameter;
	items[lara.item_number].pos.z_pos = params.second_parameter;

	return FURR_RESULT_OK;
}

// Params:
// UVROTATE
FURRResult furr_cmd_uvrotate(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// ID
// POS_X
FURRResult furr_cmd_move_item_x(FURRParameters params) {
	items[params.first_parameter].pos.x_pos = params.second_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ID
// POS_Y
FURRResult furr_cmd_move_item_y(FURRParameters params) {
	items[params.first_parameter].pos.y_pos = params.second_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ID
// POS_Z
FURRResult furr_cmd_move_item_z(FURRParameters params) {
	items[params.first_parameter].pos.z_pos = params.second_parameter;

	return FURR_RESULT_OK;
}

// Params:
// ITEM_ID
FURRResult furr_cmd_kill_item(FURRParameters params) {
	KillItem(params.first_parameter);

	return FURR_RESULT_OK;
}

// Params:
// HP
// LIMIT
FURRResult furr_cmd_set_hp(FURRParameters params) {
	items[lara.item_number].hit_points = params.first_parameter;
	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_speeddn(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_speedup(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_camera_follow(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_lock_controls(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_unlock_controls(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// MIRROR_ROOM
// WORLD_COORDINATE
FURRResult furr_cmd_move_mirror(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// ID
// STATE
FURRResult furr_cmd_oneshot_state(FURRParameters params) {
	furr_oneshot_buffer[params.first_parameter] = params.second_parameter ? true : false;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_swap_inventory(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// WEATHER
FURRResult furr_cmd_change_weather(FURRParameters params) {
	if (params.first_parameter == 0) {
		t4p_rain_type = T4P_WEATHER_DISABLED;
		t4p_snow_type = T4P_WEATHER_DISABLED;
	} else if (params.first_parameter == 1) {
		t4p_rain_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
		t4p_snow_type = T4P_WEATHER_DISABLED;
	} else if (params.first_parameter == 2) {
		t4p_rain_type = T4P_WEATHER_DISABLED;
		t4p_snow_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
	}

	return FURR_RESULT_OK;
}

// Params:
// LARA_MESH
FURRResult furr_cmd_swap_lara_mesh(FURRParameters params) {
	int16_t *temp = lara.mesh_ptrs[params.second_parameter];
	lara.mesh_ptrs[params.second_parameter] = meshes[objects[params.first_parameter].mesh_index + params.second_parameter * 2];
	meshes[objects[params.first_parameter].mesh_index + params.second_parameter * 2] = temp;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_return_true(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_stop(FURRParameters params) {
	flipeffect = -1;

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_reset(FURRParameters params) {
	// TODO: this is not fully implemented.
	flipeffect = -1;

	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_ret(FURRParameters params) {
	return FURR_RESULT_RET;
}

// Params:
FURRResult furr_cmd_retn(FURRParameters params) {
	return FURR_RESULT_RET;
}

// Params:
FURRResult furr_cmd_nop(FURRParameters params) {
	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_use_binoculars(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// DIARY_ID
FURRResult furr_cmd_add_diary(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_activate_bar(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_deactivate_bar(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_call(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// SHATER_ID
FURRResult furr_cmd_shatter_item(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_lara_on_fire(FURRParameters params) {
	LaraBurn();

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_bleed(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_show_stats(FURRParameters params) {
	S_PauseMenu(2);

	return FURR_RESULT_OK;
}

// Params:
// BMP_ID
FURRResult furr_cmd_draw_bmp(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// SAMPLE_ID
FURRResult furr_cmd_play_sample(FURRParameters params) {
	SoundEffect(params.first_parameter, 0, SFX_DEFAULT);

	return FURR_RESULT_OK;
}

// Params:
// SOUNDTRACK_ID
FURRResult furr_cmd_play_soundtrack(FURRParameters params) {
	// TODO: determine if this command also supports looped tracks.
	S_CDPlay(params.first_parameter, 0);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_save_game(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_load_game(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// PICKUP_ID
FURRResult furr_cmd_pickup_item(FURRParameters params) {
	T4PlusSetInventoryCount(params.first_parameter, T4PlusGetInventoryCount(params.first_parameter) + 1, false);
	T4ShowObjectPickup(params.first_parameter, MAX_PICKUP_DISPLAYABLE_LIFETIME);

	return FURR_RESULT_OK;
}

// Params:
// DRAW_ID, TIME_TO_DRAW_IN_FRAMES
FURRResult furr_cmd_draw_item(FURRParameters params) {
	T4ShowObjectPickup(params.first_parameter, params.second_parameter);

	return FURR_RESULT_OK;
}

// Params:
FURRResult furr_cmd_fadeout(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_show_inventory(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// FMV_ID
FURRResult furr_cmd_play_fmv(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// ITEM_ID
FURRResult furr_cmd_activate_item(FURRParameters params) {
	T4PlusActivateItem(params.first_parameter, false);

	return FURR_RESULT_OK;
}

// Params:
// DAMP_LOOPED
FURRResult furr_cmd_show_damp_looped(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
FURRResult furr_cmd_only_in_water(FURRParameters params) {
	if (lara.water_status == LW_UNDERWATER || lara.water_status == LW_WADE || lara.water_status == LW_SURFACE) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}

// Params:
FURRResult furr_cmd_only_on_land(FURRParameters params) {
	if (lara.water_status == LW_UNDERWATER || lara.water_status == LW_WADE || lara.water_status == LW_SURFACE) {
		return FURR_RESULT_RET;
	}

	return FURR_RESULT_OK;
}

// Params:
// ENVIRONMENT
FURRResult furr_cmd_environment(FURRParameters params) {
	if (lara.water_status == params.first_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}

// Params:
FURRResult furr_cmd_no_fadeout(FURRParameters params) {
	return FURR_RESULT_UNIMPLEMENTED;
}

// Params:
// BUTTON
FURRResult furr_cmd_if_pressed(FURRParameters params) {
	if (input & params.first_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}

// Params:
// BUTTON
FURRResult furr_cmd_if_not_pressed(FURRParameters params) {
	if (!(input & params.first_parameter)) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

// Params:
// LARA'S PARAMETER
// VALUE
FURRResult furr_cmd_if_lara_equals(FURRParameters params) {
	if (furr_get_state_field(lara_item, params.first_parameter) == params.second_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

// Params:
// LARA'S PARAMETER
// VALUE
FURRResult furr_cmd_if_lara_not(FURRParameters params) {
	if (furr_get_state_field(lara_item, params.first_parameter) != params.second_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

// Params:
// LARA'S PARAMETER
// VALUE
FURRResult furr_cmd_if_lara_greater_than(FURRParameters params) {
	if (furr_get_state_field(lara_item, params.first_parameter) > params.second_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

// Params:
// LARA'S PARAMETER
// VALUE
FURRResult furr_cmd_if_lara_less_than(FURRParameters params) {
	if (furr_get_state_field(lara_item, params.first_parameter) < params.second_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

// Params:
// LARA'S PARAMETER
// VALUE
FURRResult furr_cmd_if_lara_equal_or_greater_than(FURRParameters params) {
	if (furr_get_state_field(lara_item, params.first_parameter) >= params.second_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

// Params:
// LARA'S PARAMETER
// VALUE
FURRResult furr_cmd_if_lara_equal_or_less_than(FURRParameters params) {
	if (furr_get_state_field(lara_item, params.first_parameter) <= params.second_parameter) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}

// Params:
FURRResult furr_cmd_if_carry_guns(FURRParameters params) {
	if (lara.gun_status == LG_READY) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}

FURRResult furr_cmd_if_not_carry_guns(FURRParameters params) {
	if (lara.gun_status != LG_READY) {
		return FURR_RESULT_OK;
	}

	return FURR_RESULT_RET;
}
//

FURRNameTableEntry furr_name_table[] = {
	{"ONESHOT", FURR_ONESHOT},
	{"EQUIP", FURR_EQUIP},
	{"HOLSTER_WEAPONS", FURR_HOLSTER_WEAPONS},
	{"INC_HP", FURR_INC_HP},
	{"DEC_HP", FURR_DEC_HP},
	{"INC_AIR", FURR_INC_AIR},
	{"DEC_AIR", FURR_DEC_AIR},
	{"INC_SPRINT", FURR_INC_SPRINT},
	{"DEC_SPRINT", FURR_DEC_SPRINT},
	{"SET_POISON", FURR_SET_POISON},
	{"KILL_LARA", FURR_KILL_LARA},
	{"REMOVE_PISTOLS", FURR_REMOVE_PISTOLS},
	{"SET_BINOCULARS", FURR_SET_BINOCULARS},
	{"SET_CROWBAR", FURR_SET_CROWBAR},
	{"SET_LASERSIGHT", FURR_SET_LASERSIGHT},
	{"DEACTIVATE_WEAPONS", FURR_DEACTIVATE_WEAPONS},
	{"ACTIVATE_PISTOLS", FURR_ACTIVATE_PISTOLS},
	{"ACTIVATE_UZIS", FURR_ACTIVATE_UZIS},
	{"ACTIVATE_REVOLVER", FURR_ACTIVATE_REVOLVER},
	{"ACTIVATE_SHOTGUN", FURR_ACTIVATE_SHOTGUN},
	{"ACTIVATE_GRENADEGUN", FURR_ACTIVATE_GRENADEGUN},
	{"ACTIVATE_CROSSBOW", FURR_ACTIVATE_CROSSBOW},
	{"EMPTY_HOLSTERS", FURR_EMPTY_HOLSTERS},
	{"FILL_HOLSTERS", FURR_FILL_HOLSTERS},
	{"EMPTY_BACKDRAW", FURR_EMPTY_BACKDRAW},
	{"FILL_BACKDRAW", FURR_FILL_BACKDRAW},
	{"REMOVE_ALL_GUNS", FURR_REMOVE_ALL_GUNS},
	{"CALL_FLIP", FURR_CALL_FLIP},
	{"MOV_BYTE", FURR_MOV_BYTE},
	{"MOV_WORD", FURR_MOV_WORD},
	{"MOV_DWORD", FURR_MOV_DWORD},
	{"ADD_KEY", FURR_ADD_KEY},
	{"REMOVE_KEY", FURR_REMOVE_KEY},
	{"ADD_QUESTITEM", FURR_ADD_QUESTITEM},
	{"REMOVE_QUESTITEM", FURR_REMOVE_QUESTITEM},
	{"ADD_EXAMINE1", FURR_ADD_EXAMINE1},
	{"REMOVE_EXAMINE1", FURR_REMOVE_EXAMINE1},
	{"ADD_EXAMINE2", FURR_ADD_EXAMINE2},
	{"REMOVE_EXAMINE2", FURR_REMOVE_EXAMINE2},
	{"ADD_EXAMINE3", FURR_ADD_EXAMINE3},
	{"REMOVE_EXAMINE3", FURR_REMOVE_EXAMINE3},
	{"SET_WATERSKIN1", FURR_SET_WATERSKIN1},
	{"SET_WATERSKIN2", FURR_SET_WATERSKIN2},
	{"SET_PUZZLEITEM", FURR_SET_PUZZLEITEM},
	{"ADD_PUZZLEITEM", FURR_ADD_PUZZLEITEM},
	{"ADD_PUZZLECOMBO", FURR_ADD_PUZZLECOMBO},
	{"SET_SUPPLY", FURR_SET_SUPPLY},
	{"ADD_SUPPLY", FURR_ADD_SUPPLY},
	{"ADD_WEAPON", FURR_ADD_WEAPON},
	{"REMOVE_WEAPON", FURR_REMOVE_WEAPON},
	{"START_TIMER", FURR_START_TIMER},
	{"START_TIMER_FROM", FURR_START_TIMER_FROM},
	{"SET_TIMER", FURR_SET_TIMER},
	{"STOP_TIMER", FURR_STOP_TIMER},
	{"FREEZE_NORMAL_TIMER", FURR_FREEZE_NORMAL_TIMER},
	{"FREEZE_INVERTED_TIMER", FURR_FREEZE_INVERTED_TIMER},
	{"LOAD_LEVEL", FURR_LOAD_LEVEL},
	{"FLASH_RED", FURR_FLASH_RED},
	{"FLASH_ORANGE", FURR_FLASH_ORANGE},
	{"FLASH_YELLOW", FURR_FLASH_YELLOW},
	{"FLASH_GREEN", FURR_FLASH_GREEN},
	{"FLASH_LIGHTGREEN", FURR_FLASH_LIGHTGREEN},
	{"FLASH_BLUE", FURR_FLASH_BLUE},
	{"SHAKE_CAMERA", FURR_SHAKE_CAMERA},
	{"SHAKE_CAMERA_SOFT", FURR_SHAKE_CAMERA_SOFT},
	{"SHAKE_CAMERA_MEDIUM", FURR_SHAKE_CAMERA_MEDIUM},
	{"SHAKE_CAMERA_HEAVY", FURR_SHAKE_CAMERA_HEAVY},
	{"LARA_DRIPS", FURR_LARA_DRIPS},
	{"SET_LEGEND_TIME", FURR_SET_LEGEND_TIME},
	{"SET_LEGEND_STRING", FURR_SET_LEGEND_STRING},
	{"PLAY_ANIM", FURR_PLAY_ANIM},
	{"ORIENTATE", FURR_ORIENTATE},
	{"ROTATE", FURR_ROTATE},
	{"CHANGE_POSITION_X", FURR_CHANGE_POSITION_X},
	{"CHANGE_POSITION_Y", FURR_CHANGE_POSITION_Y},
	{"CHANGE_POSITION_Z", FURR_CHANGE_POSITION_Z},
	{"CHANGE_ACCEL", FURR_CHANGE_ACCEL},
	{"CHANGE_VERT_ACCEL", FURR_CHANGE_VERT_ACCEL},
	{"CHANGE_ROOM", FURR_CHANGE_ROOM},
	{"ADD_POSITION", FURR_ADD_POSITION},
	{"UVROTATE", FURR_UVROTATE},
	{"MOVE_ITEM_X", FURR_MOVE_ITEM_X},
	{"MOVE_ITEM_Y", FURR_MOVE_ITEM_Y},
	{"MOVE_ITEM_Z", FURR_MOVE_ITEM_Z},
	{"KILL_ITEM", FURR_KILL_ITEM},
	{"SET_HP", FURR_SET_HP},
	{"SPEEDDN", FURR_SPEEDDN},
	{"SPEEDUP", FURR_SPEEDUP},
	{"CAMERA_FOLLOW", FURR_CAMERA_FOLLOW},
	{"LOCK_CONTROLS", FURR_LOCK_CONTROLS},
	{"UNLOCK_CONTROLS", FURR_UNLOCK_CONTROLS},
	{"MOVE_MIRROR", FURR_MOVE_MIRROR},
	{"ONESHOT_STATE", FURR_ONESHOT_STATE},
	{"SWAP_INVENTORY", FURR_SWAP_INVENTORY},
	{"CHANGE_WEATHER", FURR_CHANGE_WEATHER},
	{"SWAP_LARA_MESH", FURR_SWAP_LARA_MESH},
	{"RETURN_TRUE", FURR_RETURN_TRUE},
	{"STOP", FURR_STOP},
	{"RESET", FURR_RESET},
	{"RET", FURR_RET},
	{"RETN", FURR_RETN},
	{"NOP", FURR_NOP},
	{"USE_BINOCULARS", FURR_USE_BINOCULARS},
	{"ADD_DIARY", FURR_ADD_DIARY},
	{"ACTIVATE_BAR", FURR_ACTIVATE_BAR},
	{"DEACTIVATE_BAR", FURR_DEACTIVATE_BAR},
	{"CALL", FURR_CALL},
	{"SHATTER_ITEM", FURR_SHATTER_ITEM},
	{"LARA_ON_FIRE", FURR_LARA_ON_FIRE},
	{"BLEED", FURR_BLEED},
	{"SHOW_STATS", FURR_SHOW_STATS},
	{"DRAW_BMP", FURR_DRAW_BMP},
	{"PLAY_SAMPLE", FURR_PLAY_SAMPLE},
	{"PLAY_SOUNDTRACK", FURR_PLAY_SOUNDTRACK},
	{"SAVE_GAME", FURR_SAVE_GAME},
	{"LOAD_GAME", FURR_LOAD_GAME},
	{"PICKUP_ITEM", FURR_PICKUP_ITEM},
	{"DRAW_ITEM", FURR_DRAW_ITEM},
	{"FADEOUT", FURR_FADEOUT},
	{"SHOW_INVENTORY", FURR_SHOW_INVENTORY},
	{"PLAY_FMV", FURR_PLAY_FMV},
	{"ACTIVATE_ITEM", FURR_ACTIVATE_ITEM},
	{"DAMP_LOOPED", FURR_DAMP_LOOPED},
	{"ONLY_IN_WATER", FURR_ONLY_IN_WATER},
	{"ONLY_ON_LAND", FURR_ONLY_ON_LAND},
	{"ENVIRONMENT", FURR_ENVIRONMENT},
	{"NO_FADEOUT", FURR_NO_FADEOUT},
	{"IF_PRESSED", FURR_IF_PRESSED},
	{"IF_NOT_PRESSED", FURR_IF_NOT_PRESSED},
	{"IF_LARA=", FURR_IF_LARA_EQUAL},
	{"IF_LARA!=", FURR_IF_LARA_NOT},
	{"IF_LARA>", FURR_IF_LARA_GREATER_THAN},
	{"IF_LARA<", FURR_IF_LARA_LESS_THAN},
	{"IF_LARA>=", FURR_IF_LARA_EQUAL_OR_GREATER_THAN},
	{"IF_LARA<=", FURR_IF_LARA_EQUAL_OR_LESS_THAN},
	{"IF_CARRY_GUNS", FURR_IF_CARRY_GUNS},
	{"IF_NOT_CARRY_GUNS", FURR_IF_NOT_CARRY_GUNS },

	{"BYTE_IF_EQL", FURR_BYTE_IF_EQL},
	{"BYTE_IF_NOT", FURR_BYTE_IF_NOT},
	{"BYTE_IF_GT", FURR_BYTE_IF_GT},
	{"BYTE_IF_LT", FURR_BYTE_IF_LT},
	{"BYTE_IF_ELT", FURR_BYTE_IF_ELT},
	{"BYTE_IF_EGT", FURR_BYTE_IF_EGT},
	{"WORD_IF_EQL", FURR_WORD_IF_EQL},
	{"WORD_IF_NOT", FURR_WORD_IF_NOT},
	{"WORD_IF_GT", FURR_WORD_IF_GT},
	{"WORD_IF_LT", FURR_WORD_IF_LT},
	{"WORD_IF_ELT", FURR_WORD_IF_ELT},
	{"WORD_IF_EGT", FURR_WORD_IF_EGT},
	{"DWORD_IF_EQL", FURR_DWORD_IF_EQL},
	{"DWORD_IF_NOT", FURR_DWORD_IF_NOT},
	{"DWORD_IF_GT", FURR_DWORD_IF_GT},
	{"DWORD_IF_LT", FURR_DWORD_IF_LT},
	{"DWORD_IF_ELT", FURR_DWORD_IF_ELT},
	{"DWORD_IF_EGT", FURR_DWORD_IF_EGT},
};


FURRDataTable furr_data_table[] = {
	{2, furr_cmd_oneshot}, // ONESHOT
	{1, furr_cmd_equip}, // EQUIP
	{0, furr_cmd_holster_weapons}, // HOLSTER_WEAPONS
	{2, furr_cmd_inc_hp}, // INC_HP
	{1, furr_cmd_dec_hp}, // DEC_HP
	{2, furr_cmd_inc_air}, // INC_AIR
	{1, furr_cmd_dec_air}, // DEC_AIR
	{2, furr_cmd_inc_sprint}, // INC_SPRINT
	{1, furr_cmd_dec_sprint}, // DEC_SPRINT
	{1, furr_cmd_set_poison}, // SET_POISON
	{0, furr_cmd_kill_lara}, // KILL_LARA
	{0, furr_cmd_remove_pistols}, // REMOVE_PISTOLS
	{1, furr_cmd_set_binoculars}, // SET_BINOCULARS
	{1, furr_cmd_set_crowbar}, // SET_CROWBAR
	{1, furr_cmd_set_lasersight}, // SET_LASERSIGHT
	{0, furr_cmd_deactive_weapons}, // DEACTIVATE_WEAPONS
	{0, furr_cmd_activate_pistols}, // ACTIVATE_PISTOLS
	{0, furr_cmd_activate_uzis}, // ACTIVATE_UZIS
	{0, furr_cmd_activate_revolver}, // ACTIVATE_REVOLVER
	{0, furr_cmd_activate_shotgun}, // ACTIVATE_SHOTGUN
	{0, furr_cmd_activate_grenadegun}, // ACTIVATE_GRENADEGUN
	{0, furr_cmd_activate_crossbow}, // ACTIVATE_CROSSBOW
	{0, furr_cmd_empty_holsters}, // FURR_EMPTY_HOLSTERS,
	{1, furr_cmd_fill_holsters}, // FURR_FILL_HOLSTERS,
	{0, furr_cmd_empty_backdraw}, // FURR_EMPTY_BACKDRAW,
	{1, furr_cmd_fill_backdraw}, // FURR_FILL_BACKDRAW,
	{0, furr_cmd_remove_all_guns}, // FURR_REMOVE_ALL_GUNS,
	{1, furr_cmd_call_flip}, // FURR_CALL_FLIP,
	{2, furr_cmd_move_byte}, // FURR_MOV_BYTE,
	{2, furr_cmd_move_word}, // FURR_MOV_WORD,
	{2, furr_cmd_move_dword}, // FURR_MOV_DWORD,
	{1, furr_cmd_add_key}, // FURR_ADD_KEY,
	{1, furr_cmd_remove_key}, // FURR_REMOVE_KEY,
	{1, furr_cmd_add_questitem}, // FURR_ADD_QUESTITEM,
	{1, furr_cmd_remove_questitem}, // FURR_REMOVE_QUESTITEM,
	{0, furr_cmd_add_examine1}, // FURR_ADD_EXAMINE1,
	{0, furr_cmd_remove_examine1}, // FURR_REMOVE_EXAMINE1,
	{0, furr_cmd_add_examine2}, // FURR_ADD_EXAMINE2,
	{0, furr_cmd_remove_examine2}, // FURR_REMOVE_EXAMINE2,
	{0, furr_cmd_add_examine3}, // FURR_ADD_EXAMINE3,
	{0, furr_cmd_remove_examine3}, // FURR_REMOVE_EXAMINE3,
	{1, furr_cmd_set_waterskin1}, // FURR_SET_WATERSKIN1,
	{1, furr_cmd_set_waterskin2}, // FURR_SET_WATERSKIN2,
	{2, furr_cmd_set_puzzleitem}, // FURR_SET_PUZZLEITEM,
	{2, furr_cmd_add_puzzleitem}, // FURR_ADD_PUZZLEITEM,
	{1, furr_cmd_add_puzzlecombo}, // FURR_ADD_PUZZLECOMBO,
	{2, furr_cmd_set_supply},  // FURR_SET_SUPPLY,
	{2, furr_cmd_add_supply}, // FURR_ADD_SUPPLY,
	{1, furr_cmd_add_weapon}, // FURR_ADD_WEAPON,
	{1, furr_cmd_remove_weapon}, // FURR_REMOVE_WEAPON,
	{0, furr_cmd_start_timer}, // FURR_START_TIMER,
	{1, furr_cmd_start_timer_from_time}, // FURR_START_TIMER_FROM,
	{1, furr_cmd_set_timer}, // FURR_SET_TIMER,
	{0, furr_cmd_stop_timer}, // FURR_STOP_TIMER,
	{0, furr_cmd_freeze_normal_timer}, // FURR_FREEZE_NORMAL_TIMER,
	{0, furr_cmd_freeze_inverted_timer}, // FURR_FREEZE_INVERTED_TIMER,
	{1, furr_cmd_load_level}, // FURR_LOAD_LEVEL,
	{0, furr_cmd_flash_red}, // FURR_FLASH_RED,
	{0, furr_cmd_flash_orange}, // FURR_FLASH_ORANGE,
	{0, furr_cmd_flash_yellow}, // FURR_FLASH_YELLOW,
	{0, furr_cmd_flash_green}, // FURR_FLASH_GREEN,
	{0, furr_cmd_flash_lightgreen}, // FURR_FLASH_LIGHTGREEN,
	{0, furr_cmd_flash_blue}, // FURR_FLASH_BLUE,
	{1, furr_cmd_shake_camera}, // FURR_SHAKE_CAMERA,
	{0, furr_cmd_shake_camera_soft}, // FURR_SHAKE_CAMERA_SOFT,
	{0, furr_cmd_shake_camera_medium}, // FURR_SHAKE_CAMERA_MEDIUM,
	{0, furr_cmd_shake_camera_heavy}, // FURR_SHAKE_CAMERA_HEAVY,
	{1, furr_cmd_lara_drips}, // FURR_LARA_DRIPS,
	{1, furr_cmd_legend_time}, // FURR_SET_LEGEND_TIME,
	{1, furr_cmd_legend_string}, // FURR_SET_LEGEND_STRING,
	{1, furr_cmd_play_anim}, // FURR_PLAY_ANIM,
	{1, furr_cmd_orientate}, // FURR_ORIENTATE,
	{1, furr_cmd_rotate}, // FURR_ROTATE,
	{1, furr_cmd_change_position_x}, // FURR_CHANGE_POSITION_X,
	{1, furr_cmd_change_position_y}, // FURR_CHANGE_POSITION_Y,
	{1, furr_cmd_change_position_z}, // FURR_CHANGE_POSITION_Z,
	{1, furr_cmd_change_accel}, // FURR_CHANGE_ACCEL,
	{1, furr_cmd_change_vert_accel}, // FURR_CHANGE_VERT_ACCEL,
	{1, furr_cmd_change_room}, // FURR_CHANGE_ROOM,
	{2, furr_cmd_add_position}, // FURR_ADD_POSITION,
	{1, furr_cmd_uvrotate}, // FURR_UVROTATE,
	{2, furr_cmd_move_item_x},// FURR_MOVE_ITEM_X,
	{2, furr_cmd_move_item_y},// FURR_MOVE_ITEM_Y,
	{2, furr_cmd_move_item_z},// FURR_MOVE_ITEM_Z,
	{1, furr_cmd_kill_item}, // FURR_KILL_ITEM,
	{2, furr_cmd_set_hp}, // FURR_SET_HP,
	{0, furr_cmd_speeddn}, // FURR_SPEEDDN,
	{0, furr_cmd_speedup}, // FURR_SPEEDUP,
	{0, furr_cmd_camera_follow}, // FURR_CAMERA_FOLLOW,
	{0, furr_cmd_lock_controls}, // FURR_LOCK_CONTROLS,
	{0, furr_cmd_unlock_controls},// FURR_UNLOCK_CONTROLS,
	{2, furr_cmd_move_mirror}, // FURR_MOVE_MIRROR,
	{2, furr_cmd_oneshot_state}, // FURR_ONESHOT_STATE,
	{0, furr_cmd_swap_inventory}, // FURR_SWAP_INVENTORY,
	{1, furr_cmd_change_weather}, // FURR_CHANGE_WEATHER,
	{2, furr_cmd_swap_lara_mesh}, // FURR_SWAP_LARA_MESH,
	{0, furr_cmd_return_true}, // FURR_RETURN_TRUE,
	{0, furr_cmd_stop}, // FURR_STOP,
	{0, furr_cmd_reset}, // FURR_RESET,
	{0, furr_cmd_ret}, // FURR_RET,
	{0, furr_cmd_retn}, // FURR_RETN,
	{0, furr_cmd_nop}, // FURR_NOP,
	{0, furr_cmd_use_binoculars}, // FURR_USE_BINOCULARS,
	{1, furr_cmd_add_diary}, // FURR_ADD_DIARY,
	{0, furr_cmd_activate_bar}, // FURR_ACTIVATE_BAR,
	{0, furr_cmd_deactivate_bar}, // FURR_DEACTIVATE_BAR,
	{1, furr_cmd_call}, // FURR_CALL,
	{1, furr_cmd_shatter_item}, // FURR_SHATTER_ITEM,
	{0, furr_cmd_lara_on_fire}, // FURR_LARA_ON_FIRE,
	{0, furr_cmd_bleed}, // FURR_BLEED,
	{0, furr_cmd_show_stats}, // FURR_SHOW_STATS,
	{1, furr_cmd_draw_bmp}, // FURR_DRAW_BMP,
	{1, furr_cmd_play_sample}, // FURR_PLAY_SAMPLE,
	{1, furr_cmd_play_soundtrack}, // FURR_PLAY_SOUNDTRACK,
	{0, furr_cmd_save_game}, // FURR_SAVE_GAME,
	{0, furr_cmd_load_game}, // FURR_LOAD_GAME,
	{1, furr_cmd_pickup_item}, // FURR_PICKUP_ITEM,
	{2, furr_cmd_draw_item}, // FURR_DRAW_ITEM,
	{0, furr_cmd_fadeout}, // FURR_FADEOUT,
	{0, furr_cmd_show_inventory}, // FURR_SHOW_INVENTORY,
	{1, furr_cmd_play_fmv}, // FURR_PLAY_FMV,
	{1, furr_cmd_activate_item}, // FURR_ACTIVATE_ITEM,
	{2, furr_cmd_show_damp_looped}, // FURR_DAMP_LOOPED,
	{0, furr_cmd_only_in_water}, // FURR_ONLY_IN_WATER,
	{0, furr_cmd_only_on_land}, // FURR_ONLY_ON_LAND,
	{1, furr_cmd_environment}, // FURR_ENVIRONMENT,
	{1, furr_cmd_no_fadeout}, // FURR_NO_FADEOUT,

	{1, furr_cmd_if_pressed }, // FURR_IF_PRESSED,
	{1, furr_cmd_if_not_pressed }, // FURR_IF_NOT_PRESSED,

	{2, furr_cmd_if_lara_equals }, // FURR_IF_LARA_EQUAL,
	{2, furr_cmd_if_lara_not }, // FURR_IF_LARA_NOT,
	{2, furr_cmd_if_lara_greater_than }, // FURR_IF_LARA_GREATER_THAN,
	{2, furr_cmd_if_lara_less_than }, // FURR_IF_LARA_LESS_THAN,
	{2, furr_cmd_if_lara_equal_or_greater_than }, // FURR_IF_LARA_EQUAL_OR_GREATER_THAN,
	{2, furr_cmd_if_lara_equal_or_less_than }, // FURR_IF_LARA_EQUAL_OR_LESS_THAN,

	{0, furr_cmd_if_carry_guns }, // FURR_IF_CARRY_GUNS,
	{0, furr_cmd_if_not_carry_guns }, // FURR_IF_NOT_CARRY_GUNS,


	// FURR_BYTE_IF_EQL, // ==
	// FURR_BYTE_IF_NOT, // !=
	// FURR_BYTE_IF_GT, // >
	// FURR_BYTE_IF_LT, // <
	// FURR_BYTE_IF_ELT, // >=
	// FURR_BYTE_IF_EGT, // <=

	// FURR_WORD_IF_EQL, // ==
	// FURR_WORD_IF_NOT, // !=
	// FURR_WORD_IF_GT, // >
	// FURR_WORD_IF_LT, // <
	// FURR_WORD_IF_ELT, // >=
	// FURR_WORD_IF_EGT, // <=

	// FURR_DWORD_IF_EQL, // ==
	// FURR_DWORD_IF_NOT, // !=
	// FURR_DWORD_IF_GT, // >
	// FURR_DWORD_IF_LT, // <
	// FURR_DWORD_IF_ELT, // >=
	// FURR_DWORD_IF_EGT, // <=
};

//

int furr_get_opcode_for_command_string(const char* command_name) {
	for (int i = 0; i < (sizeof(furr_name_table) / sizeof(FURRNameTableEntry)); i++) {
		if (strcmp(command_name, furr_name_table[i].opcode_name) == 0) {
			return furr_name_table[i].opcode_token;
		}
	}

	return -1;
}

int furr_get_arg_count_for_opcode(const FURROpcode opcode) {
	if (opcode < FURR_OPCODE_COUNT) {
		if (opcode < (sizeof(furr_data_table) / sizeof(FURRDataTable))) {
			FURRDataTable* data_table = &furr_data_table[opcode];
			return data_table->arg_count;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

void furr_clear_oneshot_buffer() {
	memset(furr_oneshot_buffer, 0, LAST_FURR_FLIPEFFECT);
}

void furr_execute_furr_flipeffect(int flipeffect_id) {
	if (flipeffect_id < FIRST_FURR_FLIPEFFECT) {
		Log(1, "Invalid FURR flipeffect id %u!\n", flipeffect_id);
		return;
	}
	if (flipeffect_id >= LAST_FURR_FLIPEFFECT) {
		Log(1, "Invalid FURR flipeffect id %u!\n", flipeffect_id);
		return;
	}

	if (furr_oneshot_buffer[flipeffect_id]) {
		return;
	}

	flipeffect_id -= FIRST_FURR_FLIPEFFECT;

	FURRFlipeffectTable* curr_flipeffect_table = &furr_flipeffect_table[flipeffect_id];

	int idx = 0;
	FURRResult previous_result = FURR_RESULT_OK;
	while (idx < curr_flipeffect_table->size) {
		FURROpcode opcode = static_cast<FURROpcode>(curr_flipeffect_table->tokens[idx++]);
		FURRDataTable* data_table = &furr_data_table[opcode];

		int argument_1 = data_table->arg_count >= 1 ? curr_flipeffect_table->tokens[idx++] : 0;
		int argument_2 = data_table->arg_count >= 2 ? curr_flipeffect_table->tokens[idx++] : 0;

		FURRParameters params = { previous_result, argument_1, argument_2 };

		previous_result = data_table->func_ptr(params);
		switch (previous_result) {
			case FURR_RESULT_ERROR: {
				const char *opcode_name = furr_name_table[opcode].opcode_name;
				Log(0, "Opcode %s failed with an error.\n", opcode_name);
				break;
			}
			case FURR_RESULT_UNIMPLEMENTED: {
				const char* opcode_name = furr_name_table[opcode].opcode_name;
				Log(0, "Opcode %s has not yet been implemented.\n", opcode_name);
				break;
			}
			case FURR_RESULT_RET: {
				return;
			}
			case FURR_RESULT_FAILED: {
				previous_result = FURR_RESULT_FAILED;
				break;
			}
			case FURR_RESULT_OK: {
				previous_result = FURR_RESULT_OK;
				break;
			}
			default: {
				break;
			}
		}
	}
}

void furr_allocate_flipeffect_buffer(int flipeffect_id, int size) {
	if (flipeffect_id < FIRST_FURR_FLIPEFFECT) {
		Log(1, "Invalid FURR flipeffect id %u!\n", flipeffect_id);
		return;
	}
	if (flipeffect_id >= LAST_FURR_FLIPEFFECT) {
		Log(1, "Invalid FURR flipeffect id %u!\n", flipeffect_id);
		return;
	}

	int table_index = flipeffect_id - FIRST_FURR_FLIPEFFECT;

	if (furr_flipeffect_table[table_index].tokens == nullptr) {
		furr_flipeffect_table[table_index].tokens = (int*)SYSTEM_MALLOC(size * sizeof(int));
		if (furr_flipeffect_table[table_index].tokens) {
			memset(furr_flipeffect_table[table_index].tokens, 0, size * sizeof(int));
		}
	}
}

void furr_free_all_flipeffect_buffers() {
	for (int i = 0; i < LAST_FURR_FLIPEFFECT - FIRST_FURR_FLIPEFFECT; i++) {
		if (furr_flipeffect_table[i].tokens != nullptr) {
			SYSTEM_FREE(furr_flipeffect_table[i].tokens);
			furr_flipeffect_table[i].tokens = nullptr;
			furr_flipeffect_table[i].size = 0;
		}
	}
}

void furr_add_flipeffect_token(int flipeffect_id, int token) {
	if (flipeffect_id < FIRST_FURR_FLIPEFFECT)
		return;
	if (flipeffect_id >= LAST_FURR_FLIPEFFECT)
		return;

	int table_index = flipeffect_id - FIRST_FURR_FLIPEFFECT;

	if (furr_flipeffect_table[table_index].tokens) {
		furr_flipeffect_table[table_index].tokens[furr_flipeffect_table[table_index].size] = token;

		furr_flipeffect_table[table_index].size++;
	}
}