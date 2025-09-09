#include "../../tomb4/pch.h"

#include "../../specific/function_stubs.h"
#include "../../specific/audio.h"
#include "../camera.h"
#include "../control.h"
#include "../debris.h"
#include "../effects.h"
#include "../effect2.h"
#include "../flmtorch.h"
#include "../gameflow.h"
#include "../items.h"
#include "../lara.h"
#include "../objects.h"
#include "../savegame.h"
#include "../sound.h"
#include "../tomb4fx.h"
#include "../traps.h"
#include "../../specific/file.h"
#include "../../specific/dxsound.h"

#include "trng.h"
#include "trng_action.h"
#include "trng_arithmetic.h"
#include "trng_extra_state.h"
#include "trng_flipeffect.h"
#include "trng_script_parser.h"
#include "trng_triggergroup.h"

#include "../../tomb4/mod_config.h"
#include "../../tomb4/tomb4plus/t4plus_inventory.h"
#include "../../tomb4/tomb4plus/t4plus_savegame.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"
#include "../../tomb4/tomb4plus/t4plus_objects.h"

#include "../../specific/function_table.h"
#include "../../specific/3dmath.h"
#include "trng_progressive_action.h"

uint32_t scanned_flipeffect_count = 0;
NGScannedFlipEffect scanned_flipeffects[NG_MAX_SCANNED_FLIPEFFECTS];
uint32_t old_flipeffect_count;
NGOldTrigger old_flipeffects[NG_MAX_OLD_FLIPEFFECTS];

void NGAttractLaraInDirection(uint8_t direction, uint8_t speed) {
	switch (direction) {
		// West
		case 0x00:
			lara_item->pos.x_pos -= speed;
			break;
		// North West
		case 0x01:
			lara_item->pos.z_pos += speed;
			lara_item->pos.x_pos -= speed;
			break;
		// North
		case 0x02:
			lara_item->pos.z_pos += speed;
			break;
		// North East
		case 0x03:
			lara_item->pos.z_pos += speed;
			lara_item->pos.x_pos += speed;
			break;
		// East
		case 0x04:
			lara_item->pos.x_pos += speed;
			break;
		// South East
		case 0x05:
			lara_item->pos.x_pos += speed;
			lara_item->pos.z_pos -= speed;
			break;
		// South
		case 0x06:
			lara_item->pos.z_pos -= speed;
			break;
		// South West
		case 0x07:
			lara_item->pos.x_pos -= speed;
			lara_item->pos.z_pos -= speed;
			break;
		default:
			NGLog(NG_LOG_TYPE_ERROR, "lara_attract_lara_in_direction_on_ground_and_in_air_with_speed: unknown direction!");
			break;
	}
}

bool NGTriggerItemGroupWithTimer(uint8_t item_group, uint8_t timer, bool anti) {
	NG_ITEM_GROUP current_item_group = current_item_groups[item_group];
	int32_t index = 0;
	for (int32_t i = 0; i < current_item_group.item_count; i++) {
		int16_t current_script_item = current_item_group.item_list[i];

		if (current_script_item >= NG_SCRIPT_ID_TABLE_SIZE || current_script_item < 0) {
			NGLog(NG_LOG_TYPE_ERROR, "Item group IDs (%u) contains and invalid script item index (%s).", i, current_script_item);
			continue;
		}

		int16_t current_item_id = ng_script_id_table[current_script_item].script_index;
		if (current_item_id < 0) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Negative item group IDs (statics) not yet supported!");
			continue;
		}
		if (current_item_id >= ITEM_COUNT) {
			NGLog(NG_LOG_TYPE_ERROR, "Item group IDs (%u) contains and invalid item id (%s).", i, current_item_id);
			continue;
		}

		ITEM_INFO *item = T4PlusGetItemInfoForID(current_item_id);
		if (item) {
			item->timer = ((int16_t)timer) * NG_TICKS_PER_SECOND;
		}
		T4PlusActivateItem(current_item_id, anti);

		index++;
	}
	return true;
}

int16_t NGGetInventoryObjectIDForByte(uint8_t inventory_id) {
	if (inventory_id >= 0x6B) {
		return ((inventory_id - 0x6B) + SIXSHOOTER_ITEM);
	} else if (inventory_id >= 0x67) {
		return ((inventory_id - 0x67) + GRENADE_GUN_ITEM);
	} else if (inventory_id >= 0x5C) {
		return ((inventory_id - 0x5C) + PISTOLS_ITEM);
	} else {
		return inventory_id + PUZZLE_ITEM1;
	}
}

// NGLE - 47
bool inventory_remove_inventory_item(uint8_t inventory_id, uint8_t _unused) {
	int16_t object_number = NGGetInventoryObjectIDForByte(inventory_id);

	T4PlusSetInventoryCount(object_number, 0, true);

	return true;
}

// NGLE - 48
bool inventory_increase_inventory_items_by_one_in_x_way(uint8_t inventory_id, uint8_t show_popup) {
	int16_t object_number = NGGetInventoryObjectIDForByte(inventory_id);

	if (show_popup)
		T4ShowObjectPickup(object_number, MAX_PICKUP_DISPLAYABLE_LIFETIME);

	int32_t current_inventory_count = T4PlusGetInventoryCount(object_number);
	current_inventory_count++;
	T4PlusSetInventoryCount(object_number, current_inventory_count, true);

	return true;
}

// NGLE - 49
bool inventory_decrease_inventory_items_by_one_in_x_way(uint8_t inventory_id, uint8_t _unused) {
	int16_t object_number = NGGetInventoryObjectIDForByte(inventory_id);

	int32_t current_inventory_count = T4PlusGetInventoryCount(object_number);
	current_inventory_count--;
	if (current_inventory_count < 0)
		current_inventory_count = 0;

	T4PlusSetInventoryCount(object_number, current_inventory_count, true);

	return true;
}

// NGLE - 50
bool inventory_set_inventory_items(uint8_t inventory_id, uint8_t count) {
	int16_t object_number = NGGetInventoryObjectIDForByte(inventory_id);

	T4PlusSetInventoryCount(object_number, count, true);

	return true;
}

// NGLE - 51
bool disable_input_for_time(uint8_t input, uint8_t timer) {
	if (input < (sizeof(NG_INPUT_CODES) / sizeof(int32_t))) {
		NGDisableInputForTime(NG_INPUT_CODES[input], (int32_t)timer * NG_TICKS_PER_SECOND);
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid id for input command!");
	}

	return true;
}

// NGLE - 52
bool keyboard_enable_input(uint8_t input, uint8_t _unused) {
	if (input < (sizeof(NG_INPUT_CODES) / sizeof(int32_t))) {
		NGEnableInput(NG_INPUT_CODES[input]);
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid id for input command!");
	}

	return true;
}

// NGLE - 53
bool keyboard_simulate_receivement_of_keyboard_command(uint8_t input, uint8_t timer) {
	if (timer < (sizeof(NG_SIMULATION_TIMES) / sizeof(int32_t))) {
		if (input < (sizeof(NG_INPUT_CODES) / sizeof(int32_t))) {
			NGSimulateInputForTime(NG_INPUT_CODES[input], NG_SIMULATION_TIMES[timer] / NG_TICKS_PER_SECOND);
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid id for input command!");
		}
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid simulation time for simulated keyboard command!");
	}

	return true;
}

// NGLE - 54
bool hide_screen_for_time(int32_t seconds, bool black_screen) {
	ng_drawing_state = NG_DRAW_STATE_FROZEN;

	if (black_screen) {
		ng_drawing_state = NG_DRAW_STATE_BLANK;
	}

	if (seconds > 0) {
		NGProgressiveAction *prog_action = prog_action = NGCreateProgressiveAction();

		if (prog_action) {
			prog_action->type = AZ_HIDE_SCREEN;
			prog_action->duration = seconds * NG_TICKS_PER_SECOND;
		}
	}

	return true;
}

// NGLE - 55
bool show_screen() {
	ng_drawing_state = NG_DRAW_STATE_ACTIVE;

	return true;
}

// NGLE - 63
bool kill_and_or_set_lara_on_fire(uint8_t death_type, uint8_t extra_timer) {
	switch (death_type) {
		case 0x00: // Zero Lara hitpoints
			lara_item->hit_points = 0;
			lara_item->hit_status = 1;
			break;
		case 0x01: // Zero Lara hitpoints and set on fire
			lara_item->hit_points = 0;
			lara_item->hit_status = 1;
			LaraBurn();
			break;
		case 0x02: // Just set Lara on fire (With the original executable, it sometimes doesn't execute. TRNG bug?)
			LaraBurn();
			break;
		default:
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "kill_and_or_set_lara_on_fire: Unsupported death type.");
			break;
	}
	return true;
}

// NGLE - 65
bool print_standard_x_string_on_screen_for_e_seconds(uint8_t string_id, uint8_t timer) {
	gfLegend = string_id;
	if (timer == 0) {
		gfLegendTime = -1;
	} else {
		gfLegendTime = timer * NG_TICKS_PER_SECOND;
	}

	return true;
}

// NGLE - 68
bool play_cd_track_channel_1(uint8_t track_id, uint8_t looping) {
	S_CDPlayExt(track_id, 0, looping, false);

	return true;
}

// NGLE - 69
bool stop_all_cd_tracks(uint8_t _unused_1, uint8_t _unused_2) {
	S_CDStopExt(0);
	S_CDStopExt(1);

	return true;
}

// NGLE - 70
bool play_sound_from_first_group(uint8_t sound_sample, uint8_t timer) {
	int32_t indexed_sound_sample = sound_sample;

	if (timer == 0) {
		// INFINITE LOOP
		ng_looped_sound_state[indexed_sound_sample] = -1;
	} else if (timer == 0x1f) {
		// ONESHOT
		ng_looped_sound_state[indexed_sound_sample] = 0;
	} else {
		ng_looped_sound_state[indexed_sound_sample] = timer * NG_TICKS_PER_SECOND;
	}

	SoundEffect(indexed_sound_sample, NULL, SFX_ALWAYS);

	return true;
}

// NGLE - 71
bool play_sound_from_second_group(uint8_t sound_sample, uint8_t timer) {
	int32_t indexed_sound_sample = 256 + sound_sample;

	if (timer == 0) {
		// INFINITE LOOP
		ng_looped_sound_state[indexed_sound_sample] = -1;
	} else if (timer == 0x1f) {
		// ONESHOT
		ng_looped_sound_state[indexed_sound_sample] = 0;
	} else {
		ng_looped_sound_state[indexed_sound_sample] = timer * NG_TICKS_PER_SECOND;
	}

	SoundEffect(indexed_sound_sample, NULL, SFX_ALWAYS);

	return true;
}

// NGLE - 72
bool stop_sound_from_first_group(uint8_t sound_sample, uint8_t _unused_2) {
	int32_t indexed_sound_sample = sound_sample;

	StopSoundEffect(indexed_sound_sample);
	ng_looped_sound_state[indexed_sound_sample] = 0;

	return true;
}

// NGLE - 73
bool stop_sound_from_second_group(uint8_t sound_sample, uint8_t _unused_2) {
	int32_t indexed_sound_sample = 256 + sound_sample;

	StopSoundEffect(indexed_sound_sample);
	ng_looped_sound_state[indexed_sound_sample] = 0;

	return true;
}

// NGLE - 74
bool stop_all_sound_samples(uint8_t _unused_1, uint8_t _unused_2) {
	S_SoundStopAllSamples();

	memset(ng_looped_sound_state, 0x00, NumSamples * sizeof(int32_t));

	return true;
}

// NGLE - 77
bool force_lara_animation_0_255_of_slot_animation(uint8_t animation_index, uint8_t object_id) {
	int32_t animation_index_offset = objects[object_id].anim_index + animation_index;

	NGSetItemAnimation(lara.item_number, animation_index_offset, true, false, false, true);

	return true;
}

// NGLE - 78
bool lara_force_x_state_id_and_e_next_state_id_for_lara(uint8_t state_id, uint8_t next_state_id) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(lara.item_number);
	if (item) {
		item->current_anim_state = state_id;
		item->goal_anim_state = next_state_id;
	}

	return true;
}

// NGLE - 79
bool move_lara_to_lara_start_pos_in_x_way(uint8_t ocb, uint8_t teleport_type) {
	int32_t lara_start_pos_id = NGFindIndexForLaraStartPosWithMatchingOCB(ocb);
	if (lara_start_pos_id >= 0) {
		AIOBJECT* ai = &AIObjects[lara_start_pos_id];
		if (ai) {
			if (teleport_type == 1) {
				// Keep sector displacement
				lara_item->pos.x_pos = (ai->x & ~0x3ff) | (lara_item->pos.x_pos & 0x3ff);
				int32_t lara_y_offset =  lara_item->floor - lara_item->pos.y_pos;
				lara_item->pos.z_pos = (ai->z & ~0x3ff) | (lara_item->pos.z_pos & 0x3ff);

				FLOOR_INFO* ai_floor_info = GetFloor(lara_item->pos.x_pos, ai->y, lara_item->pos.z_pos, &ai->room_number);
				lara_item->pos.y_pos = GetHeight(ai_floor_info, lara_item->pos.x_pos, ai->y, lara_item->pos.z_pos) - lara_y_offset;

				camera.fixed_camera = 1;

				if (lara_item->room_number != ai->room_number)
					ItemNewRoom(lara.item_number, ai->room_number);

				return true;
			} else {
				// Warp directly
				lara_item->pos.x_pos = ai->x;
				lara_item->pos.y_pos = ai->y;
				lara_item->pos.z_pos = ai->z;
				lara_item->pos.y_rot = ai->y_rot;

				if (lara_item->room_number != ai->room_number)
					ItemNewRoom(lara.item_number, ai->room_number);

				camera.fixed_camera = 1;

				return true;
			}
		}
	}

	return false;
}

// NGLE - 80
bool force_lara_animation_256_512_of_slot_animation(uint8_t animation_index, uint8_t object_id) {
	int32_t animation_index_offset = objects[object_id].anim_index + animation_index + 256;

	NGSetItemAnimation(lara.item_number, animation_index_offset, true, false, false, true);

	return true;
}

// NGLE - 82
bool delay_load_x_level_in_seconds(uint8_t level_id, uint8_t level_timer) {
	if (level_timer < 0x1f) {
		pending_level_load_timer = level_timer * NG_TICKS_PER_SECOND;
	} else {
		// Not sure how this works. Editor calls it 'perform one single time', but it doesn't seem to do anything.
		pending_level_load_timer = -1;
	}
	pending_level_load_id = level_id;

	return true;
}


// NGLE - 83
bool remove_weapons_or_flares_from_laras_hands(uint8_t _unused1, uint8_t _unused2) {
	lara.request_gun_type = WEAPON_NONE;

	return true;
}

// NGLE - 84
bool cutscene_set_fade_in_for_x_time(uint8_t fade_time, uint8_t _unused2) {
	SetScreenFadeIn(fade_time);

	return true;
}

// NGLE - 85
bool cutscene_set_fade_out_for_x_time_in_way(uint8_t fade_time, uint8_t fade_type) {
	SetScreenFadeOut(fade_time, fade_type);

	return true;
}

// NGLE - 89
bool damage_lara_life_by_percentage(uint8_t timer, uint8_t _unused2) {
	const int32_t MAX_LARA_HEALTH = 1000; // May need this to be customizable.

	if (lara_item->hit_points > 0) {
		int32_t new_hit_points = lara_item->hit_points;
		if (timer <= 9) {
			int32_t health_multiple = (MAX_LARA_HEALTH / 1000);
			new_hit_points -= (int32_t)(health_multiple * (timer + 1));
		} else if (timer <= 18) {
			int32_t health_multiple = (MAX_LARA_HEALTH / 100);
			new_hit_points -= (int32_t)(health_multiple * (timer - 8));
		} else {
			int32_t health_multiple = (MAX_LARA_HEALTH / 10);
			new_hit_points -= (int32_t)(health_multiple * (timer - 17));
		}

		if (new_hit_points < 0)
			new_hit_points = 0;

		lara_item->hit_points = new_hit_points;
		lara_item->hit_status = true;
	}

	return true;
}

// NGLE - 90
bool recharge_lara_life_by_percentage(uint8_t timer, uint8_t _unused2) {
	const int32_t MAX_LARA_HEALTH = 1000; // May need this to be customizable.

	if (lara_item->hit_points > 0) {
		int32_t new_hit_points = lara_item->hit_points;
		if (timer <= 9) {
			int32_t health_multiple = (MAX_LARA_HEALTH / 1000);
			new_hit_points += (int32_t)(health_multiple * (timer + 1));
		} else if (timer <= 18) {
			int32_t health_multiple = (MAX_LARA_HEALTH / 100);
			new_hit_points += (int32_t)(health_multiple * (timer - 8));
		} else {
			int32_t health_multiple = (MAX_LARA_HEALTH / 10);
			new_hit_points += (int32_t)(health_multiple * (timer - 17));
		}

		if (new_hit_points > MAX_LARA_HEALTH)
			new_hit_points = MAX_LARA_HEALTH;

		lara_item->hit_points = new_hit_points;
		lara_item->hit_status = true;
	}

	return true;
}

// NGLE - 96
bool lara_disarm_lara(uint8_t remove_weapons_only, uint8_t _unusued) {
	lara.request_gun_type = WEAPON_NONE;
	lara.last_gun_type = WEAPON_NONE;
	lara.gun_status = LG_NO_ARMS;

	lara.pistols_type_carried = W_NONE;
	lara.uzis_type_carried = W_NONE;
	lara.shotgun_type_carried = W_NONE;
	lara.crossbow_type_carried = W_NONE;
	lara.grenade_type_carried = W_NONE;
	lara.sixshooter_type_carried = W_NONE;

	lara.weapon_item = NO_ITEM;

	lara.back_gun = 0;
	lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersSlotID() : T4PlusGetLaraSlotID();

	lara.mesh_ptrs[LM_RHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_RHAND * 2];
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_LHAND * 2];

	lara.left_arm.frame_number = 0;
	lara.right_arm.frame_number = 0;
	lara.target = 0;
	lara.right_arm.lock = 0;
	lara.left_arm.lock = 0;

	if (remove_weapons_only != 1) {
		lara.num_uzi_ammo = 0;
		lara.num_revolver_ammo = 0;
		lara.num_shotgun_ammo1 = 0;
		lara.num_shotgun_ammo2 = 0;
		lara.num_grenade_ammo1 = 0;
		lara.num_grenade_ammo2 = 0;
		lara.num_grenade_ammo3 = 0;
		lara.num_crossbow_ammo1 = 0;
		lara.num_crossbow_ammo2 = 0;
		lara.num_crossbow_ammo3 = 0;
	}
	return true;
}

// NGLE - 104
bool lara_toggle_infinite_air(uint8_t enabled, uint8_t _unused) {
	ng_lara_infinite_air = enabled;

	return true;
}

// NGLE - 109
bool global_trigger_enable_disable(uint8_t enable, uint8_t global_trigger_id) {
	ng_global_trigger_states[global_trigger_id].is_disabled = enable > 0 ? false : true;

	return true;
}

// NGLE - 115
bool set_room_type(uint8_t room_number, uint8_t room_type) {
	int32_t indexed_room_number = NGFindIndexForRoom(room_number);
	if (indexed_room_number >= number_rooms || indexed_room_number < 0) {
		NGLog(NG_LOG_TYPE_ERROR, "Room number overflow!");
		return false;
	}

	ROOM_INFO* r = &room[indexed_room_number];
	if (r) {
		switch (room_type) {
			case 0: {
				r->flags |= ROOM_UNDERWATER;
				break;
			}
			case 1: {
				r->flags |= 0x02;
				break;
			}
			case 2: {
				r->flags |= ROOM_SWAMP;
				break;
			}
			case 3: {
				r->flags |= ROOM_OUTSIDE;
				break;
			}
			case 4: {
				r->flags |= ROOM_DAMAGE;
				break;
			}
			case 5: {
				r->flags |= ROOM_NOT_INSIDE;
				break;
			}
			case 6: {
				r->flags |= ROOM_INSIDE;
				break;
			}
			case 7: {
				r->flags |= ROOM_NO_LENSFLARE;
				break;
			}
			case 8: {
				r->flags |= ROOM_CAUSTICS;
				break;
			}
			case 9: {
				r->flags |= ROOM_REFLECTIONS;
				break;
			}
			case 10: {
				r->flags |= ROOM_SNOW;
				break;
			}
			case 11: {
				r->flags |= ROOM_RAIN;
				break;
			}
			case 12: {
				r->flags |= ROOM_COLD;
				break;
			}
			default: {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SET_ROOM_TYPE: unsupported room type!");
				return false;
			}
		}
	}

	return true;
}

// NGLE - 116
bool remove_room_type(uint8_t room_number, uint8_t room_type) {
	int32_t indexed_room_number = NGFindIndexForRoom(room_number);
	if (indexed_room_number >= number_rooms || indexed_room_number < 0) {
		NGLog(NG_LOG_TYPE_ERROR, "Room number overflow!");
		return false;
	}

	ROOM_INFO* r = &room[indexed_room_number];
	if (r) {
		switch (room_type) {
			case 0: {
				r->flags &= ~ROOM_UNDERWATER;
				break;
			}
			case 1: {
				r->flags &= ~0x02;
				break;
			}
			case 2: {
				r->flags &= ~ROOM_SWAMP;
				break;
			}
			case 3: {
				r->flags &= ~ROOM_OUTSIDE;
				break;
			}
			case 4: {
				r->flags &= ~ROOM_DAMAGE;
				break;
			}
			case 5: {
				r->flags &= ~ROOM_NOT_INSIDE;
				break;
			}
			case 6: {
				r->flags &= ~ROOM_INSIDE;
				break;
			}
			case 7: {
				r->flags &= ~ROOM_NO_LENSFLARE;
				break;
			}
			case 8: {
				r->flags &= ~ROOM_CAUSTICS;
				break;
			}
			case 9: {
				r->flags &= ~ROOM_REFLECTIONS;
				break;
			}
			case 10: {
				r->flags &= ~ROOM_SNOW;
				break;
			}
			case 11: {
				r->flags &= ~ROOM_RAIN;
				break;
			}
			case 12: {
				r->flags &= ~ROOM_COLD;
				break;
			}
			default: {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "REMOVE_ROOM_TYPE: unsupported room type!");
				return false;
			}
		}
	}

	return true;
}

// NGLE - 118
bool perform_triggergroup_from_script_in_specific_way(uint8_t trigger_group_id, uint8_t execution_type) {
	return NGTriggerGroupFunction(trigger_group_id, execution_type);
}

// NGLE - 125
bool flipmap_on(uint8_t flipmap_id, uint8_t _unused_2) {
	flipmap[flipmap_id] |= IFL_CODEBITS;

	if (!flip_stats[flipmap_id])
		FlipMap(flipmap_id);

	return true;
}

// NGLE - 126
bool flipmap_off(uint8_t flipmap_id, uint8_t _unused_2) {
	flipmap[flipmap_id] &= ~(IFL_CODEBITS);

	if (flip_stats[flipmap_id])
		FlipMap(flipmap_id);

	return true;
}

// NGLE - 127
bool organizer_enable(uint8_t organizer_id_lower, uint8_t organizer_id_upper) {
	uint16_t organizer_id = ((int16_t)organizer_id_upper << 8) | (int16_t)organizer_id_lower;
	bool should_reset = NGIsOrganizerEnabled(organizer_id) == false;

	NGToggleOrganizer(organizer_id, true);

	// Docs say that if the organizer is disabled, if enable with flipeffect, it should always start from the beginning.
	if (should_reset)
		NGResetOrganizer(organizer_id);

	return true;
}

// NGLE - 128
bool organizer_disable(uint8_t organizer_id_lower, uint8_t organizer_id_upper) {
	uint16_t organizer_id = ((int16_t)organizer_id_upper << 8) | (int16_t)organizer_id_lower;
	NGToggleOrganizer(organizer_id, false);

	return true;
}

// NGLE - 129
bool sound_play_cd_track_channel_2(uint8_t track_id, uint8_t looping) {
	S_CDPlayExt(track_id, 1, looping, false);

	return true;
}

// NGLE - 130
bool sound_stop_cd_track_on_channel(uint8_t channel_id, uint8_t _unused) {
	S_CDStopExt(channel_id);

	return true;
}


// NGLE - 133
bool sound_set_x_volume_for_audio_track_on_channel(uint8_t volume, uint8_t channel) {
	// Not sure how accurate this behaviour is.
	if (!is_mod_trng_version_equal_or_greater_than_target(1, 1, 8, 7)) {
		if (volume == 0) {
			volume = 100;
		}
	}

	S_CDSetChannelVolume(volume, channel);
	return true;
}

// NGLE - 134
bool lara_attract_lara_in_direction_on_ground_with_speed(uint8_t direction, uint8_t speed) {
	if (lara_item->pos.y_pos >= lara_item->floor)
		NGAttractLaraInDirection(direction, speed);

	return true;
}

// NGLE - 135
bool lara_attract_lara_in_direction_in_air_with_speed(uint8_t direction, uint8_t speed) {
	if (lara_item->pos.y_pos < lara_item->floor)
		NGAttractLaraInDirection(direction, speed);

	return true;
}

// NGLE - 145
bool itemgroup_activate_item_group_with_timer(uint8_t item_group, uint8_t timer) {
	return NGTriggerItemGroupWithTimer(item_group, timer, false);
}

// NGLE - 146
bool itemgroup_untrigger_item_group_with_timer(uint8_t item_group, uint8_t timer) {
	return NGTriggerItemGroupWithTimer(item_group, timer, true);
}

// NGLE - 158
bool lara_attract_lara_in_direction_on_ground_and_in_air_with_speed(uint8_t direction, uint8_t speed) {
	NGAttractLaraInDirection(direction, speed);

	return true;
}

// NGLE - 159
bool distance_set_level_far_view_distance_to_x_number_of_sectors(uint8_t sectors, uint8_t _unused) {
	ClipRange = (float)sectors * float(BLOCK_SIZE);

	return true;
}

// NGLE - 160
bool static_shatter(uint8_t static_id_lower, uint8_t static_id_upper) {
	uint16_t static_id = ((int16_t)static_id_upper << 8) | (int16_t)static_id_lower;

	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];

		PHD_3DPOS pos;
		pos.x_pos = mesh->x;
		pos.y_pos = mesh->y;
		pos.z_pos = mesh->z;

		if (mesh->Flags & 1) {
			ShatterObject(0, mesh, 128, room_number, 0);
			SmashedMeshRoom[SmashedMeshCount] = room_number;
			SmashedMesh[SmashedMeshCount] = mesh;
			SmashedMeshCount++;
			mesh->Flags &= ~1;

			MOD_LEVEL_STATIC_INFO* static_info = &get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number];
			if (static_info->shatter_sound_id >= 0) {
				SoundEffect(static_info->shatter_sound_id, &pos, SFX_DEFAULT);
			}
		}
	}

	return true;
}

// NGLE - 161
bool static_remove_collision_for_x_static(uint8_t static_id_lower, uint8_t static_id_upper) {
	uint16_t static_id = ((int16_t)static_id_upper << 8) | (int16_t)static_id_lower;

	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];

		mesh->Flags |= 4; // Add no-collision flag
	}

	return true;
}

// NGLE - 162
bool static_restore_collision_for_x_static(uint8_t static_id_lower, uint8_t static_id_upper) {
	uint16_t static_id = ((int16_t)static_id_upper << 8) | (int16_t)static_id_lower;

	NGStaticTableEntry *entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];

		mesh->Flags &= ~4; // Removes no-collision flag
	}

	return true;
}

// NGLE - 166
bool static_move_static_with_data_in_x_parameter_list(uint8_t move_param_id_lower, uint8_t move_param_id_upper) {
	uint16_t move_param_id = ((int16_t)move_param_id_upper << 8) | (int16_t)move_param_id_lower;

	if (move_param_id >= MAX_NG_MOVE_ITEMS)
		return false;

	bool is_valid = false;

	NG_MOVE_ITEM *move_item = &current_move_items[move_param_id];
	int32_t direction = move_item->direction & 0xff;
	int32_t script_static = move_item->index_item;

	if (move_item->flags != 0xffff) {
		if (move_item->flags & ~(FMOV_INFINITE_LOOP | FMOV_HEAVY_AT_END | FMOV_TRIGGERS_ALL | FMOV_HEAVY_ALL)) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "static_move_static_with_data_in_x_parameter_list: unimplemented flags 0x%04x", move_item->flags);
			return false;
		}
	}

	switch (direction) {
		case DIR_NORTH: {
			if (!NGGetStaticHorizontalMovementRemainingUnits(script_static)) {
				int16_t current_angle = (int16_t)0xC000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += (int16_t)0x8000;
				}
				NGSetStaticHorizontalMovementAngle(script_static, (int16_t)current_angle);
				NGSetStaticHorizontalMovementRemainingUnits(script_static, move_item->distance);
				NGSetStaticHorizontalMovementSpeed(script_static, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, move_item->distance);
				} else {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, 0);
				}
				is_valid = true;
			}
			break;
		}
		case DIR_EAST: {
			if (!NGGetStaticHorizontalMovementRemainingUnits(script_static)) {
				int16_t current_angle = 0x0000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += 0x8000;
				}
				NGSetStaticHorizontalMovementAngle(script_static, (int16_t)current_angle);
				NGSetStaticHorizontalMovementRemainingUnits(script_static, move_item->distance);
				NGSetStaticHorizontalMovementSpeed(script_static, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, move_item->distance);
				} else {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, 0);
				}
				is_valid = true;
			}
			break;
		}
		case DIR_SOUTH: {
			if (!NGGetStaticHorizontalMovementRemainingUnits(script_static)) {
				int16_t current_angle = 0x4000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += 0x8000;
				}
				NGSetStaticHorizontalMovementAngle(script_static, (int16_t)current_angle);
				NGSetStaticHorizontalMovementRemainingUnits(script_static, move_item->distance);
				NGSetStaticHorizontalMovementSpeed(script_static, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, move_item->distance);
				} else {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, 0);
				}
				is_valid = true;
			}
			break;
		}
		case DIR_WEST: {
			if (!NGGetStaticHorizontalMovementRemainingUnits(script_static)) {
				int16_t current_angle = (int16_t)0x8000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += 0x8000;
				}
				NGSetStaticHorizontalMovementAngle(script_static, (int16_t)current_angle);
				NGSetStaticHorizontalMovementRemainingUnits(script_static, move_item->distance);
				NGSetStaticHorizontalMovementSpeed(script_static, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, move_item->distance);
				} else {
					NGSetStaticHorizontalMovementRepeatUnits(script_static, 0);
				}
				is_valid = true;
			}
			break;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "moveable_move_moveable_with_data_in_x_parameter_list: unimplemented direction 0x%04x", move_item->direction);
			break;
		}
	}

	if (is_valid) {
		NGSetStaticMovementInProgressSound(script_static, move_item->moving_sound);
		NGSetStaticMovementFinishedSound(script_static, move_item->final_sound);

		if (move_item->flags & FMOV_HEAVY_AT_END) {
			NGSetStaticMovementTriggerHeavyAtEnd(script_static, true);
		} else {
			NGSetStaticMovementTriggerHeavyAtEnd(script_static, false);
		}

		if (move_item->flags & FMOV_TRIGGERS_ALL) {
			NGSetStaticMovementTriggerNormalWhenMoving(script_static, true);
		} else {
			NGSetStaticMovementTriggerNormalWhenMoving(script_static, false);
		}

		if (move_item->flags & FMOV_HEAVY_ALL) {
			NGSetStaticMovementTriggerHeavyWhenMoving(script_static, true);
		} else {
			NGSetStaticMovementTriggerHeavyWhenMoving(script_static, false);
		}
	}

	return true;
}

// NGLE - 167
bool moveable_move_moveable_with_data_in_x_parameter_list(uint8_t move_param_id_lower, uint8_t move_param_id_upper) {
	uint16_t move_param_id = ((int16_t)move_param_id_upper << 8) | (int16_t)move_param_id_lower;

	if (move_param_id >= MAX_NG_MOVE_ITEMS)
		return false;

	bool is_valid = false;

	NG_MOVE_ITEM *move_item = &current_move_items[move_param_id];
	int32_t direction = move_item->direction & 0xff;
	int32_t script_item = ng_script_id_table[move_item->index_item].script_index;
	if (script_item == -1) {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid script ID!");
		return false;
	}

	if (move_item->flags != 0xffff) {
		if (move_item->flags & ~(FMOV_INFINITE_LOOP | FMOV_HEAVY_AT_END | FMOV_TRIGGERS_ALL | FMOV_HEAVY_ALL)) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "moveable_move_moveable_with_data_in_x_parameter_list: unimplemented flags 0x%04x", move_item->flags);
			return false;
		}
	}

	switch (direction) {
		case DIR_NORTH: {
			if (!NGGetItemHorizontalMovementRemainingUnits(script_item)) {
				int16_t current_angle = (int16_t)0xC000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += (int16_t)0x8000;
				}
				NGSetItemHorizontalMovementAngle(script_item, (int16_t)current_angle);
				NGSetItemHorizontalMovementRemainingUnits(script_item, move_item->distance);
				NGSetItemHorizontalMovementSpeed(script_item, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetItemHorizontalMovementRepeatUnits(script_item, move_item->distance);
				} else {
					NGSetItemHorizontalMovementRepeatUnits(script_item, 0);
				}
				is_valid = true;
			}
			break;
		}
		case DIR_EAST: {
			if (!NGGetItemHorizontalMovementRemainingUnits(script_item)) {
				int16_t current_angle = (int16_t)0x0000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += (int16_t)0x8000;
				}
				NGSetItemHorizontalMovementAngle(script_item, (int16_t)current_angle);
				NGSetItemHorizontalMovementRemainingUnits(script_item, move_item->distance);
				NGSetItemHorizontalMovementSpeed(script_item, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetItemHorizontalMovementRepeatUnits(script_item, move_item->distance);
				} else {
					NGSetItemHorizontalMovementRepeatUnits(script_item, 0);
				}
				is_valid = true;
			}
			break;
		}
		case DIR_SOUTH: {
			if (!NGGetItemHorizontalMovementRemainingUnits(script_item)) {
				int16_t current_angle = (int16_t)0x4000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += (int16_t)0x8000;
				}
				NGSetItemHorizontalMovementAngle(script_item, (int16_t)current_angle);
				NGSetItemHorizontalMovementRemainingUnits(script_item, move_item->distance);
				NGSetItemHorizontalMovementSpeed(script_item, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetItemHorizontalMovementRepeatUnits(script_item, move_item->distance);
				} else {
					NGSetItemHorizontalMovementRepeatUnits(script_item, 0);
				}
				is_valid = true;
			}
			break;
		}
		case DIR_WEST: {
			if (!NGGetItemHorizontalMovementRemainingUnits(script_item)) {
				int16_t current_angle = (int16_t)0x8000;
				if (move_item->direction & DIR_INVERT_DIRECTION) {
					current_angle += (int16_t)0x8000;
				}
				NGSetItemHorizontalMovementAngle(script_item, (int16_t)current_angle);
				NGSetItemHorizontalMovementRemainingUnits(script_item, move_item->distance);
				NGSetItemHorizontalMovementSpeed(script_item, move_item->speed);
				if (move_item->flags & FMOV_INFINITE_LOOP) {
					NGSetItemHorizontalMovementRepeatUnits(script_item, move_item->distance);
				} else {
					NGSetItemHorizontalMovementRepeatUnits(script_item, 0);
				}
				is_valid = true;
			}
			break;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "moveable_move_moveable_with_data_in_x_parameter_list: unimplemented direction 0x%04x", move_item->direction);
			break;
		}
	}

	if (is_valid) {
		NGSetItemMovementInProgressSound(script_item, move_item->moving_sound);
		NGSetItemMovementFinishedSound(script_item, move_item->final_sound);

		if (move_item->flags & FMOV_HEAVY_AT_END) {
			NGSetItemMovementTriggerHeavyAtEnd(script_item, true);
		} else {
			NGSetItemMovementTriggerHeavyAtEnd(script_item, false);
		}

		if (move_item->flags & FMOV_TRIGGERS_ALL) {
			NGSetItemMovementTriggerNormalWhenMoving(script_item, true);
		} else {
			NGSetItemMovementTriggerNormalWhenMoving(script_item, false);
		}

		if (move_item->flags & FMOV_HEAVY_ALL) {
			NGSetItemMovementTriggerHeavyWhenMoving(script_item, true);
		} else {
			NGSetItemMovementTriggerHeavyWhenMoving(script_item, false);
		}
	}

	return true;
}

// NGLE - 168
bool sound_play_sound_single_playback_of_global_sound_map(uint8_t lower_sample_id, uint8_t upper_sample_id) {
	int32_t indexed_sound_sample = (int32_t)lower_sample_id | ((int32_t)(upper_sample_id) << 8 & 0xff00);

	SoundEffect(indexed_sound_sample, NULL, SFX_ALWAYS);
	return true;
}

// NGLE - 169
bool lara_force_x_animation_for_lara_preserve_state_id(uint8_t lower_anim_id, uint8_t upper_anim_id) {
	int32_t animation_index = (int32_t)lower_anim_id | ((int32_t)(upper_anim_id) << 8 & 0xff00);

	int32_t animation_index_offset = objects[T4PlusGetLaraSlotID()].anim_index + animation_index;

	NGSetItemAnimation(lara.item_number, animation_index_offset, false, false, true, false);

	return true;
}

// NGLE - 170
bool lara_force_x_animation_for_lara_set_new_state_id(uint8_t lower_anim_id, uint8_t upper_anim_id) {
	int32_t animation_index = (int32_t)lower_anim_id | ((int32_t)(upper_anim_id) << 8 & 0xff00);
	int32_t animation_index_offset = objects[T4PlusGetLaraSlotID()].anim_index + animation_index;

	NGSetItemAnimation(lara.item_number, animation_index_offset, true, false, true, false);

	return true;
}

// NGLE - 171
bool lara_force_x_animation_for_lara_set_neutral_state_id(uint8_t lower_anim_id, uint8_t upper_anim_id) {
	int32_t animation_index = (int32_t)lower_anim_id | ((int32_t)(upper_anim_id) << 8 & 0xff00);
	int32_t animation_index_offset = objects[T4PlusGetLaraSlotID()].anim_index + animation_index;

	NGSetItemAnimation(lara.item_number, animation_index_offset, false, false, true, false);
	ITEM_INFO* item = T4PlusGetItemInfoForID(lara.item_number);
	if (item) {
		item->current_anim_state = 69;
	}

	return true;
}


// NGLE - 180
bool static_explode(uint8_t static_id_lower, uint8_t static_id_upper) {
	uint16_t static_id = ((int16_t)static_id_upper << 8) | (int16_t)static_id_lower;

	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];

		PHD_3DPOS pos;
		pos.x_pos = mesh->x;
		pos.y_pos = mesh->y;
		pos.z_pos = mesh->z;

		if (mesh->Flags & 1) {
			ShatterObject(0, mesh, 128, room_number, 0);
			SmashedMeshRoom[SmashedMeshCount] = room_number;
			SmashedMesh[SmashedMeshCount] = mesh;
			SmashedMeshCount++;
			mesh->Flags &= ~1;

			// TODO: explosion effect is not accurate
			TriggerExplosionSparks(pos.x_pos, pos.y_pos, pos.z_pos, 3, -2, 0, room_number);
			for (int32_t i = 0; i < 3; i++)
				TriggerExplosionSparks(pos.x_pos, pos.y_pos, pos.z_pos, 3, -1, 0, room_number);

			SoundEffect(SFX_EXPLOSION1, 0, SFX_DEFAULT);
			camera.bounce = -75;
		}
	}

	return true;
}


// NGLE - 189
bool static_visibility_set_as_invisible(uint8_t static_id_lower, uint8_t static_id_upper) {
	uint16_t static_id = ((int16_t)static_id_upper << 8) | (int16_t)static_id_lower;

	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
		if (mesh)
			mesh->Flags &= ~1;
		else
			NGLog(NG_LOG_TYPE_ERROR, "static_visibility_set_as_invisible: Missing static mesh!");
	}

	return true;
}

// NGLE - 190
bool static_visibility_render_newly_visible(uint8_t static_id_lower, uint8_t static_id_upper) {
	uint16_t static_id = ((int16_t)static_id_upper << 8) | (int16_t)static_id_lower;

	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
		if (mesh)
			mesh->Flags |= 1;
		else
			NGLog(NG_LOG_TYPE_ERROR, "static_visibility_render_newly_visible: Missing static mesh!");
	}

	return true;
}

// NGLE - 193
bool play_track_on_channel_with_restore(uint8_t track_id, uint8_t channel_id) {
	S_CDPlayExt(track_id, channel_id, false, true);
	return true;
}

// NGLE - 199
bool lara_light_or_put_out_torch_in_laras_hand(uint8_t light_torch, uint8_t _unused) {
	lara.LitTorch = light_torch;
	return true;
}

// NGLE - 200
bool lara_give_or_remove_torch_to_or_from_hand_of_lara(uint8_t give_torch, uint8_t _unused) {
	if (give_torch) {
		if (lara.gun_type != WEAPON_TORCH) {
			lara.request_gun_type = WEAPON_TORCH;
			lara.gun_type = WEAPON_TORCH;
			lara.flare_control_left = 1;
			lara.left_arm.anim_number = objects[TORCH_ANIM].anim_index;
			lara.gun_status = LG_READY;
			lara.left_arm.lock = 0;
			lara.left_arm.frame_number = 0;
			lara.left_arm.frame_base = anims[objects[TORCH_ANIM].anim_index].frame_ptr;
			lara.mesh_ptrs[LM_LHAND] = meshes[objects[TORCH_ANIM].mesh_index + LM_LHAND * 2];
		}
	} else {
		if (lara.gun_type == WEAPON_TORCH) {
			lara.flare_control_left = 0;
			lara.LitTorch = 0;
			lara.left_arm.lock = 0;
			lara.gun_type = WEAPON_NONE;
			lara.request_gun_type = WEAPON_NONE;
			lara.gun_status = LG_NO_ARMS;
			lara.mesh_ptrs[LM_LHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_LHAND * 2];
		}
	}
	return true;
}

// NGLE - 231
bool variables_add_value_to_x_variable(uint8_t variable, uint8_t value) {
	NGNumericOperation(NG_ADD, variable, value);
	return true;
}

// NGLE - 232
bool variables_set_x_variable_to_value(uint8_t variable, uint8_t value) {
	NGNumericOperation(NG_SET, variable, value);
	return true;
}

// NGLE - 233
bool variables_subtract_value_from_x_variable(uint8_t variable, uint8_t value) {
	NGNumericOperation(NG_SUBTRACT, variable, value);
	return true;
}

// NGLE - 234
bool variables_set_in_x_variable_the_bit(uint8_t variable, uint8_t bit) {
	NGNumericOperation(NG_BIT_SET, variable, bit);
	return true;
}

// NGLE - 235
bool variables_clear_in_x_variable_the_bit(uint8_t variable, uint8_t bit) {
	NGNumericOperation(NG_BIT_CLEAR, variable, bit);
	return true;
}

// NGLE - 244
bool variables_copy_to_x_numeric_variable_the_savegame_memory_value(uint8_t variable, uint8_t savegame_value) {
	NGNumericOperation(NG_SET, variable, NGNumericGetSavegameValue(savegame_value));
	return true;
}

// NGLE - 245
bool variables_copy_from_x_numeric_variable_the_savegame_memory_value(uint8_t numeric_variable, uint8_t savegame_variable) {
	NGNumericSetSavegameValue(savegame_variable, NGNumericGetVariable(numeric_variable));
	return true;
}

// NGLE - 246
bool variables_set_in_x_savegame_memory_the_value(uint32_t variable, int32_t value) {
	NGNumericSetSavegameValue(variable, value);
	return true;
}

// NGLE - 247
bool variables_set_in_x_savegame_memory_the_bit(uint32_t variable, uint8_t bit) {
	int32_t value = NGNumericGetSavegameValue(variable);
	value |= (1 << bit);
	NGNumericSetSavegameValue(variable, value);
	return true;
}

// NGLE - 248
bool variables_clear_in_x_savegame_memory_the_bit(uint32_t variable, uint8_t bit) {
	int32_t value = NGNumericGetSavegameValue(variable);
	value &= ~(1 << bit);
	NGNumericSetSavegameValue(variable, value);
	return true;
}

// NGLE - 249
bool variables_add_to_x_savegame_memory_the_value(uint32_t variable, int32_t value) {
	int32_t savegame_value = NGNumericGetSavegameValue(variable);
	savegame_value += value;
	NGNumericSetSavegameValue(variable, savegame_value);
	return true;
}

// NGLE - 250
bool variables_subtract_to_x_savegame_memory_the_value(uint32_t variable, int32_t value) {
	int32_t savegame_value = NGNumericGetSavegameValue(variable);
	savegame_value -= value;
	NGNumericSetSavegameValue(variable, savegame_value);
	return true;
}

// NGLE - 251
bool variables_multiply_x_variable_by_value(uint8_t variable, uint8_t value) {
	NGNumericOperation(NG_MULTIPLY, variable, value);
	return true;
}

// NGLE - 252
bool variables_set_in_x_numeric_variable_the_negative_value(uint8_t variable, uint8_t value) {
	NGNumericOperation(NG_SET, variable, -128 + value);
	return true;
}

// NGLE - 253
bool variables_divide_x_variable_by_value(uint8_t variable, uint8_t value) {
	NGNumericOperation(NG_DIVIDE, variable, value);
	return true;
}

// NGLE - 264
bool variables_start_the_x_trng_timer_in_mode(uint8_t set_global_timer, uint8_t countdown_timer) {
	if (set_global_timer) {
		if (countdown_timer) {
			ng_global_timer_frame_increment = -1;
		} else {
			ng_global_timer_frame_increment = 1;
		}
	} else {
		if (countdown_timer) {
			ng_local_timer_frame_increment = -1;
		} else {
			ng_local_timer_frame_increment = 1;
		}
	}

	return true;
}

// NGLE - 265
bool variables_stop_the_x_trng_timer(uint8_t set_global_timer, uint8_t _unused) {
	if (set_global_timer) {
		ng_global_timer_frame_increment = 0;
	} else {
		ng_local_timer_frame_increment = 0;
	}

	return true;
}

// NGLE - 266
bool variables_initialize_the_x_trng_timer_to_seconds(uint8_t set_global_timer, uint8_t seconds) {
	if (set_global_timer)
		ng_global_timer = (int32_t)(seconds) * NG_TICKS_PER_SECOND;
	else
		ng_local_timer = (int32_t)(seconds) * NG_TICKS_PER_SECOND;

	return true;
}

// NGLE - 267
bool variables_initialize_the_x_trng_timer_to_big_number_seconds(uint8_t set_global_timer, uint8_t big_number_id) {
	NGLog(NG_LOG_TYPE_ERROR, "variables_initialize_the_x_trng_timer_to_big_number_seconds: unimplemented!");
	return true;
}

// NGLE - 268
bool variables_initialize_the_x_trng_timer_to_frame_ticks(uint8_t set_global_timer, uint8_t frame_ticks) {
	if (set_global_timer)
		ng_global_timer = (int32_t)(frame_ticks);
	else
		ng_local_timer = (int32_t)(frame_ticks);

	return true;
}

// NGLE - 269
bool variables_show_x_trng_timer_in_position(uint8_t set_global_timer, uint8_t position) {

	if (position == NGTimerPosition::NG_TIMER_POSITION_DOWN_DAMAGE_BAR) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NG_TIMER_POSITION_DOWN_DAMAGE_BAR unimplemented!");
	} else if (position == NGTimerPosition::NG_TIMER_POSITION_DOWN_COLD_BAR) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NG_TIMER_POSITION_DOWN_COLD_BAR unimplemented!");
	} else if (position == NGTimerPosition::NG_TIMER_POSITION_DOWN_LEFT_BARS) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NG_TIMER_POSITION_DOWN_LEFT_BARS unimplemented!");
	} else if (position == NGTimerPosition::NG_TIMER_POSITION_DOWN_RIGHT_BARS) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NG_TIMER_POSITION_DOWN_RIGHT_BARS unimplemented!");
	}

	if (set_global_timer) {
		ng_global_timer_position = (NGTimerPosition)(position);
		ng_global_timer_time_until_hide = -1;
	} else {
		ng_local_timer_position = (NGTimerPosition)(position);
		ng_local_timer_time_until_hide = -1;
	}

	return true;
}

// NGLE - 270
bool variables_hide_the_x_trng_timer_in_seconds(uint8_t set_global_timer, uint8_t seconds) {
	if (set_global_timer) {
		if (ng_global_timer_time_until_hide < 0) {
			ng_global_timer_time_until_hide = (int32_t)(seconds) * NG_TICKS_PER_SECOND;
		}
	} else {
		if (ng_local_timer_time_until_hide < 0) {
			ng_local_timer_time_until_hide = (int32_t)(seconds) * NG_TICKS_PER_SECOND;
		}
	}

	return true;
}

// NGLE - 284
bool variables_numeric_invert_the_sign_of_x_numeric_value(uint8_t variable, uint8_t _unused) {
	NGNumericOperation(NG_INVERT_SIGN, variable, NGNumericGetVariable(variable));
	return true;
}

// NGLE - 335
bool variables_set_the_x_inventory_item_as_selected_inventory_memory(uint8_t inventory_item, uint8_t _unused) {
	ng_selected_inventory_item_memory = inventory_item;
	return true;
}

// NGLE - 345
bool triggergroup_enable_newly_the_oneshot_x_triggergroup_already_performed(uint8_t trigger_group_id_upper, uint8_t trigger_group_id_lower) {
	uint16_t trigger_group_id = (trigger_group_id_upper << 8) | trigger_group_id_lower;
	if (trigger_group_id >= MAX_NG_TRIGGER_GROUPS) {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid trigger group.");
		return false;
	}

	current_trigger_groups[trigger_group_id].oneshot_triggered = false;

	return true;
}

// NGLE - 355
bool screen_flash_screen_with_light_color_for_duration(uint8_t flash_color, uint8_t duration) {
	switch (flash_color) {
		case 0: {
			// Red Light
			FlashFadeR = 0xff;
			FlashFadeG = 0x40;
			FlashFadeB = 0x00;
			break;
		}
		case 1: {
			// Orange
			FlashFadeR = 0xff;
			FlashFadeG = 0x80;
			FlashFadeB = 0x00;
			break;
		}
		case 2: {
			// Yellow
			FlashFadeR = 0xff;
			FlashFadeG = 0xc0;
			FlashFadeB = 0x40;
			break;
		}
		case 3: {
			// White
			FlashFadeR = 0xff;
			FlashFadeG = 0xff;
			FlashFadeB = 0xff;
			break;
		}
		case 4: {
			// Green
			FlashFadeR = 0x0a;
			FlashFadeG = 0xe5;
			FlashFadeB = 0x0a;
			break;
		}
		case 5: {
			// Purple
			FlashFadeR = 0xf0;
			FlashFadeG = 0x1d;
			FlashFadeB = 0xd5;
			break;
		}
		case 6: {
			// Light Green
			FlashFadeR = 0x55;
			FlashFadeG = 0xea;
			FlashFadeB = 0x59;
			break;
		}
		case 7: {
			// Blue
			FlashFadeR = 0x23;
			FlashFadeG = 0x27;
			FlashFadeB = 0xe8;
			break;
		}
		case 8: {
			// Azure
			FlashFadeR = 0x4e;
			FlashFadeG = 0xd5;
			FlashFadeB = 0xe1;
			break;
		}
		case 9: {
			// Grey
			FlashFadeR = 0x8c;
			FlashFadeG = 0x8c;
			FlashFadeB = 0x8c;
			break;
		}
		case 10: {
			// Brown
			FlashFadeR = 0x71;
			FlashFadeG = 0x28;
			FlashFadeB = 0x32;
			break;
		}
		default:
			FlashFadeR = 0x00;
			FlashFadeG = 0x00;
			FlashFadeB = 0x00;
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "screen_flash_screen_with_light_color_for_duration: flash colour is out of range!");
			break;
	}

	FlashFader = duration;
	return true;
}


// NGLE - 367
bool camera_show_black_screen_for_seconds_with_final_curtain_effect(uint8_t timer, uint8_t _unused) {
	NGSetFullscreenCurtainTimer(timer * NG_TICKS_PER_SECOND);

	return true;
}

// NGLE - 369
bool camera_set_cinema_effect_type_for_seconds(uint8_t timer, uint8_t extra_timer) {
	NGSetCinemaTypeAndTimer(timer, extra_timer * NG_TICKS_PER_SECOND);

	return true;
}

// NGLE - 371
bool perform_triggergroup_from_script_in_single_execution_mode(uint8_t trigger_group_id_lower, uint8_t trigger_group_id_upper) {
	uint16_t trigger_group_id = (trigger_group_id_upper << 8) | trigger_group_id_lower;
	return NGTriggerGroupFunction(trigger_group_id, TRIGGER_GROUP_EXECUTION_SINGLE);
}

// NGLE - 372
bool perform_triggergroup_from_script_in_multi_execution_mode(uint8_t trigger_group_id_lower, uint8_t trigger_group_id_upper) {
	uint16_t trigger_group_id = (trigger_group_id_upper << 8) | trigger_group_id_lower;
	return NGTriggerGroupFunction(trigger_group_id, TRIGGER_GROUP_EXECUTION_MULTIPLE);
}

// NGLE - 373
bool perform_triggergroup_from_script_in_continuous_mode(uint8_t trigger_group_id_lower, uint8_t trigger_group_id_upper) {
	uint16_t trigger_group_id = (trigger_group_id_upper << 8) | trigger_group_id_lower;
	return NGTriggerGroupFunction(trigger_group_id, TRIGGER_GROUP_EXECUTION_CONTINUOUS);
}


// NGLE - 374
bool enable_global_trigger_with_id(uint8_t global_trigger_id_lower, uint8_t global_trigger_id_upper) {
	uint16_t global_trigger_id = (global_trigger_id_upper << 8) | global_trigger_id_lower;
	ng_global_trigger_states[global_trigger_id].is_disabled = false;
	return true;
}

// NGLE - 375
bool disable_global_trigger_with_id(uint8_t global_trigger_id_lower, uint8_t global_trigger_id_upper) {
	uint16_t global_trigger_id = (global_trigger_id_upper << 8) | global_trigger_id_lower;
	ng_global_trigger_states[global_trigger_id].is_disabled = true;
	return true;
}

// NGLE - 404
bool trigger_secret(uint8_t secret_number, uint8_t _unused) {
	T4PTriggerSecret(secret_number);
	return true;
}

// NGLE - 407
bool set_lara_holsters(uint8_t holster_type, uint8_t _unused) {
	switch (holster_type) {
		case 0x0d: {
			lara.holster = T4PlusGetLaraHolstersSlotID();
			break;
		};
		case 0x0e: {
			lara.holster = T4PlusGetLaraHolstersPistolsSlotID();
			break;
		};
		case 0x10: {
			lara.holster = T4PlusGetLaraHolstersRevolverSlotID();
			break;
		};
		case 0x0f: {
			lara.holster = T4PlusGetLaraHolstersUzisSlotID();
			break;
		};
		case 0x00: {
			lara.holster = T4PlusGetLaraSlotID();
			break;
		};
		default: {
			lara.holster = holster_type;
		}
	}
	return true;
}

// NGLE - 411
bool lara_set_x_opacity_for_lara_for_seconds(uint8_t opacity, uint8_t seconds) {
	return false;
}

int32_t NGPerformTRNGFlipEffect(uint16_t flip_number, int16_t full_timer, uint32_t flags) {
	int8_t timer = (int8_t)full_timer & 0xff;
	int8_t extra_timer = (int8_t)(full_timer >> 8) & 0xff;

	int32_t repeat_type = 1;

	switch (flip_number) {
		case INVENTORY_REMOVE_INVENTORY_ITEM: {
			inventory_remove_inventory_item(timer, extra_timer);
			break;
		}
		case INVENTORY_INCREASE_INVENTORY_ITEMS_BY_ONE_IN_X_WAY: {
			inventory_increase_inventory_items_by_one_in_x_way(timer, extra_timer);
			break;
		}
		case INVENTORY_DECREASE_INVENTORY_ITEMS_BY_ONE: {
			inventory_decrease_inventory_items_by_one_in_x_way(timer, extra_timer);
			break;
		}
		case INVENTORY_SET_INVENTORY_ITEMS: {
			inventory_set_inventory_items(timer, extra_timer);
			break;
		}
		case KEYBOARD_DISABLE_INPUT_FOR_TIME: {
			disable_input_for_time(timer, extra_timer);
			break;
		}
		case KEYBOARD_ENABLE_INPUT: {
			keyboard_enable_input(timer, extra_timer);
			break;
		}
		case KEYBOARD_SIMULATE_RECEIVEMENT_OF_KEYBOARD_COMMAND: {
			keyboard_simulate_receivement_of_keyboard_command(timer, extra_timer);
			break;
		}
		case SCREEN_HIDE_SCREEN_FOR_X_TIME_IN_WAY: {
			hide_screen_for_time(timer, extra_timer == 0);
			break;
		}
		case SCREEN_SHOW_SCREEN: {
			show_screen();
			break;
		}
		case ANIMATED_TEXTURES_STOP_ANIMATION_RANGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_STOP_ANIMATION_RANGE unimplemented!");
			break;
		}
		case ANIMATED_TEXTURES_RESTART_ANIMATION_RANGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_RESTART_ANIMATION_RANGE unimplemented!");
			break;
		}
		case ANIMATED_TEXTURES_SET_THE_FRAME_OF_TEXTURE_OF_FIRST_P_RANGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_SET_THE_FRAME_OF_TEXTURE_OF_FIRST_P_RANGE unimplemented!");
			break;
		}
		case ANIMATED_TEXTURES_SET_THE_FRAME_OF_TEXTURE_OF_SECOND_P_RANGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_SET_THE_FRAME_OF_TEXTURE_OF_SECOND_P_RANGE unimplemented!");
			break;
		}
		case ANIMATED_TEXTURES_INVERT_SCROLL_DIRECTION: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_INVERT_SCROLL_DIRECTION unimplemented!");
			break;
		}
		case WEATHER_FOG_ENABLE_VOLUMETRIC_FX: {
			t4_override_fog_mode = T4P_FOG_FORCE_VOLUMETRIC;
			UpdateDistanceFogColor();
			break;
		}
		case WEATHER_FOG_DISABLE_VOLUMETRIC_FX: {
			t4_override_fog_mode = T4P_FOG_FORCE_DISTANT;
			UpdateDistanceFogColor();
			break;
		}
		case KILL_AND_OR_SET_LARA_ON_FIRE: {
			kill_and_or_set_lara_on_fire(timer, extra_timer);
			break;
		}
		case TEXT_PRINT_NGEXTRA_STRING_ON_SCREEN_FOR_X_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_NGEXTRA_STRING_ON_SCREEN_FOR_X_SECONDS unimplemented!");
			break;
		}
		case TEXT_PRINT_STANDARD_STRING_ON_SCREEN_FOR_X_SECONDS: {
			print_standard_x_string_on_screen_for_e_seconds(timer, extra_timer);
			break;
		}
		case TEXT_SET_COLOR_AND_POSITION_FOR_NEXT_PRINT_STRING_FLIPEFFECT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_SET_COLOR_AND_POSITION_FOR_NEXT_PRINT_STRING_FLIPEFFECT unimplemented!");
			break;
		}
		case TEXT_ERASE_ALL_STRINGS_SHOWED_WITH_PRINT_STRING_EFFECT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_ERASE_ALL_STRINGS_SHOWED_WITH_PRINT_STRING_EFFECT unimplemented!");
			break;
		}
		case PLAY_CD_TRACK_ON_CHANNEL_1: {
			play_cd_track_channel_1(timer, extra_timer);
			break;
		}
		case STOP_ALL_CD_TRACKS: {
			stop_all_cd_tracks(timer, extra_timer);
			break;
		}
		case PLAY_SOUND_FROM_FIRST_GROUP: {
			play_sound_from_first_group(timer, extra_timer);
			break;
		}
		case PLAY_SOUND_FROM_SECOND_GROUP: {
			play_sound_from_second_group(timer, extra_timer);
			break;
		}
		case STOP_SOUND_FROM_FIRST_GROUP: {
			stop_sound_from_first_group(timer, extra_timer);
			break;
		}
		case STOP_SOUND_FROM_SECOND_GROUP: {
			stop_sound_from_second_group(timer, extra_timer);
			break;
		}
		case STOP_ALL_SOUND_SAMPLES: {
			stop_all_sound_samples(timer, extra_timer);
			break;
		}
		case TEXT_SET_BLINK_STATUS_AND_SPEED: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_SET_BLINK_STATUS_AND_SPEED unimplemented!");
			break;
		}
		case TEXT_RESET_ALL_TEXT_FORMATTING_SETTINGS_WITH_DEFAULT_VALUES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_RESET_ALL_TEXT_FORMATTING_SETTINGS_WITH_DEFAULT_VALUES unimplemented!");
			break;
		}
		case FORCE_LARA_ANIMATION_0_255_OF_SLOT_ANIMATION: {
			force_lara_animation_0_255_of_slot_animation(timer, extra_timer);
			break;
		}
		case FORCE_LARA_STATE_ID_AND_NEXT_STATE_ID: {
			lara_force_x_state_id_and_e_next_state_id_for_lara(timer, extra_timer);
			break;
		}
		case MOVE_LARA_TO_START_POS_WITH_OCB_VALUE_IN_X_WAY: {
			// TODO: check repeat type for this effect.
			move_lara_to_lara_start_pos_in_x_way(timer, extra_timer);
			break;
		}
		case FORCE_LARA_ANIMATION_256_512_OF_SLOT_ANIMATION: {
			force_lara_animation_256_512_of_slot_animation(timer, extra_timer);
			break;
		}
		case TEXT_SET_SIZE_CHARACTERS_FOR_NEXT_PRINT_STRING_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_SET_SIZE_CHARACTERS_FOR_NEXT_PRINT_STRING_COMMAND unimplemented!");
			break;
		}
		case DELAY_LOAD_X_LEVEL_IN_SECONDS: {
			delay_load_x_level_in_seconds(timer, extra_timer);
			break;
		}
		case REMOVE_WEAPONS_OR_FLARES_FROM_LARAS_HANDS: {
			remove_weapons_or_flares_from_laras_hands(timer, extra_timer);
			break;
		}
		case CUTSCENE_SET_FADEIN_FOR_X_TIME: {
			cutscene_set_fade_in_for_x_time(timer, extra_timer);
			break;
		}
		case CUTSCENE_SET_FADEOUT_FOR_X_TIME_IN_WAY: {
			cutscene_set_fade_out_for_x_time_in_way(timer, extra_timer);
			break;
		}
		case TIMER_SHOW_OR_HIDE_SCREEN_TIMER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TIMER_SHOW_OR_HIDE_SCREEN_TIMER unimplemented!");
			break;
		}
		case ANIMCOMMAND_ADD_TO_CURRENT_OBJECT_EFFECT_FOR_X_TIME: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_ADD_TO_CURRENT_OBJECT_EFFECT_FOR_X_TIME unimplemented!");
			break;
		}
		case ENABLE_OR_DISABLE_MIRROR_WITH_X_HIDDEN_ROOM: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ENABLE_OR_DISABLE_MIRROR_WITH_X_HIDDEN_ROOM unimplemented!");
			break;
		}
		case DAMAGE_LARA_LIFE_BY_PERCENTAGE_OF_FULL_VITALITY: {
			damage_lara_life_by_percentage(timer, extra_timer);
			if (extra_timer == 1) {
				repeat_type = 0;
			}
			break;
		}
		case RECHARGE_LARA_LIFE_BY_PERCENTAGE_OF_FULL_VITALITY: {
			recharge_lara_life_by_percentage(timer, extra_timer);

			if (extra_timer == 1) {
				repeat_type = 0;
			}
			break;
		}

		case LARA_INVULNERABLE_FOR_X_TIME_WITH_EFFECT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_INVULNERABLE_FOR_X_TIME_WITH_EFFECT unimplemented!");
			break;
		}
		case LARA_REMOVE_FLAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_REMOVE_FLAMES unimplemented!");
			break;
		}
		case LARA_REMOVE_INVULNERABLE_STATUS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_REMOVE_INVULNERABLE_STATUS unimplemented!");
			break;
		}
		case ANIMATED_TEXTURES_START_THE_X_TEXTURE_SEQUENCE_FOR_ANIMATED_RANGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_START_THE_X_TEXTURE_SEQUENCE_FOR_ANIMATED_RANGE unimplemented!");
			break;
		}
		case ANIMATED_TEXTURES_STOP_TEXTURE_SEQUENCE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMATED_TEXTURES_STOP_TEXTURE_SEQUENCE unimplemented!");
			break;
		}
		case LARA_DISARM_LARA: {
			lara_disarm_lara(timer, extra_timer);
			break;
		}
		case BACKUP_SAVE_IN_SILENT_WAY_THE_CURRENT_GAME_IN_X_BACKUP_FILE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "BACKUP_SAVE_IN_SILENT_WAY_THE_CURRENT_GAME_IN_X_BACKUP_FILE unimplemented!");
			break;
		}
		case BACKUP_RESTORE_THE_X_BACKUP_FILE_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "BACKUP_RESTORE_THE_X_BACKUP_FILE_IN_WAY unimplemented!");
			break;
		}
		case LARA_SWAP_MESHES_OF_LARA_WITH_SLOT_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_SWAP_MESHES_OF_LARA_WITH_SLOT_IN_WAY unimplemented!");
			break;
		}
		case LARA_SET_SINGLE_X_LARA_MESH_WITH_MESH_GOT_FROM_SLOT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_SET_SINGLE_X_LARA_MESH_WITH_MESH_GOT_FROM_SLOT unimplemented!");
			break;
		}
		case ANIMCOMMAND_ACTIVATE_HEAVY_TRIGGERS_IN_SECTOR_WHERE_LARA_IS: {
			int16_t room_num = lara_item->room_number;
			FLOOR_INFO* floor_info = GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &room_num);
			GetHeight(floor_info, lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
			TestTriggers(trigger_index, true, 0);
			break;
		}
		case ANIMCOMMAND_TURN_CURRENT_OBJECT_OF_X_DEGREES_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_TURN_CURRENT_OBJECT_OF_X_DEGREES_IN_WAY unimplemented!");
			break;
		}
		case ANIMCOMMAND_UPDATE_ALSO_ORIGINAL_LARA_POSITION_AFTER_A_SETPOSITION: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_UPDATE_ALSO_ORIGINAL_LARA_POSITION_AFTER_A_SETPOSITION unimplemented!");
			break;
		}
		case LARA_TOGGLE_INFINITE_AIR: {
			lara_toggle_infinite_air(timer, extra_timer);
			break;
		}
		case LARA_COPY_MESHES_FROM_X_SLOT_TO_LARA_MESHES_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_COPY_MESHES_FROM_X_SLOT_TO_LARA_MESHES_IN_WAY unimplemented!");
			break;
		}
		case LARA_BACKUP_MESHES_OF_LARA_IN_X_SLOT_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_BACKUP_MESHES_OF_LARA_IN_X_SLOT_IN_WAY unimplemented!");
			break;
		}
		case LARA_ENABLE_OR_DISABLE_WEAPONS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_ENABLE_OR_DISABLE_WEAPONS unimplemented!");
			break;
		}
		case LARA_HIDE_OR_SHOW_HOLSTER_MESHES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_HIDE_OR_SHOW_HOLSTER_MESHES unimplemented!");
			break;
		}
		case GLOBAL_TRIGGER_ENABLE_DISABLE: {
			global_trigger_enable_disable(timer, extra_timer);
			break;
		}
		case LARA_INCREASE_AIR_FOR_LARA_OF_X_UNITS_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_INCREASE_AIR_FOR_LARA_OF_X_UNITS_IN_WAY unimplemented!");
			break;
		}
		case LARA_DECREASE_DAMAGE_BAR_OF_DAMAGE_ROOM_OF_X_UNITS_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_DECREASE_DAMAGE_BAR_OF_DAMAGE_ROOM_OF_X_UNITS_IN_WAY unimplemented!");
			break;
		}
		case LARA_DECREASE_COLD_BAR_OF_COLD_ROOM_OF_X_UNITS_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_DECREASE_COLD_BAR_OF_COLD_ROOM_OF_X_UNITS_IN_WAY unimplemented!");
			break;
		}
		case LARA_HEALTH_POISON_LARA_WITH_X_INTENSITY_OF_POISON: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_HEALTH_POISON_LARA_WITH_X_INTENSITY_OF_POISON unimplemented!");
			break;
		}
		case LARA_REMOVE_POSION_FROM_LARA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_REMOVE_POSION_FROM_LARA unimplemented!");
			break;
		}
		case SET_ROOM_TYPE: {
			set_room_type(timer, extra_timer);
			break;
		}
		case REMOVE_ROOM_TYPE: {
			remove_room_type(timer, extra_timer);
			break;
		}
		case WEATHER_SET_RAIN_OR_SNOW_INTENSITY_FOR_X_ROOM_WITH_NEW_INTENSITY_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SET_RAIN_OR_SNOW_INTENSITY_FOR_X_ROOM_WITH_NEW_INTENSITY_VALUE unimplemented!");
			break;
		}
		case PERFORM_TRIGGERGROUP_FROM_SCRIPT_IN_SPECIFIC_WAY:
			perform_triggergroup_from_script_in_specific_way(timer, extra_timer);
			break;

		case CAMERA_SET_CURRENT_CAMERA_AS_FOLLOW_CAMERA_ON_AXIS_UNTIL_CONDITION: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_SET_CURRENT_CAMERA_AS_FOLLOW_CAMERA_ON_AXIS_UNTIL_CONDITION unimplemented!");
			break;
		}
		case CAMERA_STOP_CURRENT_AXIS_OR_EFFECT_CAMERA_AND_THE_CAMERA_LINKED_WITH_IT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_STOP_CURRENT_AXIS_OR_EFFECT_CAMERA_AND_THE_CAMERA_LINKED_WITH_IT unimplemented!");
			break;
		}
		case LARA_DISABLE_X_SKILL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_DISABLE_X_SKILL unimplemented!");
			break;
		}
		case LARA_RESTORE_X_SKILL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_RESTORE_X_SKILL unimplemented!");
			break;
		}
		case CAMERA_USE_CURRENT_CAMERA_TO_PERFORM_X_EFFECT_AT_DISTANCE_FROM_TARGET: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_USE_CURRENT_CAMERA_TO_PERFORM_X_EFFECT_AT_DISTANCE_FROM_TARGET unimplemented!");
			break;
		}
		case FLIPMAP_ENABLED_X_FLIPMAP_WITH_BUTTONS_FOR_ACTIVATION: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "FLIPMAP_ENABLED_X_FLIPMAP_WITH_BUTTONS_FOR_ACTIVATION unimplemented!");
			break;
		}
		case FLIPMAP_ON: {
			flipmap_on(timer, extra_timer);
			break;
		}
		case FLIPMAP_OFF: {
			flipmap_off(timer, extra_timer);
			break;
		}
		case ORGANIZER_ENABLE:
			organizer_enable(timer, extra_timer);
			break;
		case ORGANIZER_DISABLE:
			organizer_disable(timer, extra_timer);
			break;
		case SOUND_PLAY_CD_TRACK_ON_CHANNEL_2: {
			sound_play_cd_track_channel_2(timer, extra_timer);
			break;
		}
		case SOUND_STOP_CD_TRACK_ON_CHANNEL: {
			sound_stop_cd_track_on_channel(timer, extra_timer);
			break;
		}
		case SOUND_PLAY_X_IMPORTED_FILE_IN_LOOP_MODE_ON_CHANNEL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SOUND_PLAY_X_IMPORTED_FILE_IN_LOOP_MODE_ON_CHANNEL unimplemented!");
			break;
		}
		case SOUND_PLAY_X_IMPORTED_FILE_IN_SINGLE_PLAY_MODE_ON_CHANNEL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SOUND_PLAY_X_IMPORTED_FILE_IN_SINGLE_PLAY_MODE_ON_CHANNEL unimplemented!");
			break;
		}
		case SOUND_SET_X_VOLUME_OF_AUDIO_TRACK_ON_CHANNEL: {
			sound_set_x_volume_for_audio_track_on_channel(timer, extra_timer);
			break;
		}
		case LARA_ATTRACT_LARA_IN_DIRECTION_ON_GROUND_WITH_SPEED: {
			lara_attract_lara_in_direction_on_ground_with_speed(timer, extra_timer);
			repeat_type = 0;
			break;
		}
		case LARA_ATTRACT_LARA_IN_DIRECTION_IN_AIR_WITH_SPEED: {
			lara_attract_lara_in_direction_in_air_with_speed(timer, extra_timer);
			repeat_type = 0;
			break;
		}
		case LARA_ATTRACT_LARA_UP_DOWN: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_ATTRACT_LARA_UP_DOWN unimplemented!");
			repeat_type = 0;
			break;
		}
		case ITEMGROUP_MOVE_CONTINUOUSLY_FORWARD_BACKWARD_X_ITEMGROUP_OF_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_CONTINUOUSLY_FORWARD_BACKWARD_X_ITEMGROUP_OF_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_MOVE_UP_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_UP_X_ITEMGROUP_FOR_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_MOVE_DOWN_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_DOWN_X_ITEMGROUP_FOR_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_MOVE_TO_WEST_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_TO_WEST_X_ITEMGROUP_FOR_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_MOVE_TO_NORTH_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_TO_NORTH_X_ITEMGROUP_FOR_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_MOVE_TO_EAST_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_TO_EAST_X_ITEMGROUP_FOR_CLICKS unimplemented!");

			break;
		}
		case ITEMGROUP_MOVE_TO_SOUTH_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_TO_EAST_X_ITEMGROUP_FOR_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_MOVE_CONTINOUSLY_UPSTAIRS_DOWNSTAIRS_X_ITEMGROUP_FOR_CLICKS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_MOVE_TO_EAST_X_ITEMGROUP_FOR_CLICKS unimplemented!");
			break;
		}
		case ITEMGROUP_ACTIVATE_ITEMGROUP_WITH_TIMER: {
			itemgroup_activate_item_group_with_timer(timer, extra_timer);
			break;
		}
		case ITEMGROUP_UNTRIGGER_ITEMGROUP_WITH_TIMER: {
			itemgroup_untrigger_item_group_with_timer(timer, extra_timer);
			break;
		}
		case ITEMGROUP_DISABLE_ALL_CONTINUOUS_ACTIONS_X_ITEMGROUP: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_DISABLE_ALL_CONTINUOUS_ACTIONS_X_ITEMGROUP unimplemented!");

			break;
		}
		case TEXT_PRINT_NGEXTRA_STRING_ONSCREEN_FREEZE_GAME_AND_WAIT_FOR_ESCAPE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_NGEXTRA_STRING_ONSCREEN_FREEZE_GAME_AND_WAIT_FOR_ESCAPE unimplemented!");
			break;
		}
		case TEXT_PRINT_STANDARD_STRING_ONSCREEN_FREEZE_GAME_AND_WAIT_FOR_ESCAPE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_STANDARD_STRING_ONSCREEN_FREEZE_GAME_AND_WAIT_FOR_ESCAPE unimplemented!");
			break;
		}
		case WEATHER_SKY_X_ENABLE_DISABLE_THE_LAYER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SKY_X_ENABLE_DISABLE_THE_LAYER unimplemented!");
			break;
		}
		case WEATHER_SKY_X_ENABLE_DISABLE_LIGHTNING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SKY_X_ENABLE_DISABLE_LIGHTNING unimplemented!");
			break;
		}
		case WEATHER_SKY_SET_NEW_X_COLOR_FOR_LAYER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SKY_SET_NEW_X_COLOR_FOR_LAYER unimplemented!");
			break;
		}
		case WEATHER_SKY_SET_NEW_X_SPEED_FOR_LAYER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SKY_SET_NEW_X_SPEED_FOR_LAYER unimplemented!");
			break;
		}
		case WEATHER_SKY_CHANGE_SLOWLY_THE_COLOR_OF_LAYER1_TO_X_COLOR_IN_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SKY_CHANGE_SLOWLY_THE_COLOR_OF_LAYER1_TO_X_COLOR_IN_SECONDS unimplemented!");
			break;
		}
		case WEATHER_SKY_CHANGE_SLOWLY_THE_COLOR_OF_LAYER2_TO_X_COLOR_IN_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SKY_CHANGE_SLOWLY_THE_COLOR_OF_LAYER2_TO_X_COLOR_IN_SECONDS unimplemented!");
			break;
		}
		case WEATHER_SET_X_NEW_STATE_FOR_SNOW_IN_CURRENT_LEVEL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SET_X_NEW_STATE_FOR_SNOW_IN_CURRENT_LEVEL unimplemented!");
			break;
		}
		case WEATHER_SET_X_NEW_STATE_FOR_RAIN_IN_CURRENT_LEVEL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SET_X_NEW_STATE_FOR_RAIN_IN_CURRENT_LEVEL unimplemented!");
			break;
		}
		case LARA_ATTRACT_LARA_IN_DIRECTION_ON_GROUND_AND_IN_AIR_WITH_SPEED: {
			repeat_type = 0;
			lara_attract_lara_in_direction_on_ground_and_in_air_with_speed(timer, extra_timer);
			break;
		}
		case DISTANCE_SET_LEVEL_FAR_VIEW_DISTANCE_TO_X_NUMBER_OF_SECTORS: {
			distance_set_level_far_view_distance_to_x_number_of_sectors(timer, extra_timer);
			break;
		}
		case STATIC_SHATTER: {
			static_shatter(timer, extra_timer);
			break;
		}
		case STATIC_REMOVE_COLLISION_FROM: {
			static_remove_collision_for_x_static(timer, extra_timer);
			break;
		}
		case STATIC_RESTORE_COLLISION_TO: {
			static_restore_collision_for_x_static(timer, extra_timer);
			break;
		}
		case STATIC_SET_ICE_TRANSPARENCY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_ICE_TRANSPARENCY unimplemented!");
			break;
		}
		case STATIC_SET_GLASS_TRANSPARENCY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_GLASS_TRANSPARENCY unimplemented!");
			break;
		}
		case STATIC_REMOVE_TRANSPARENCY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_REMOVE_TRANSPARENCY unimplemented!");
			break;
		}
		case STATIC_MOVE_STATIC_WITH_DATA_IN_X_PARAMETERS_LIST: {
			static_move_static_with_data_in_x_parameter_list(timer, extra_timer);
			break;
		}
		case MOVEABLE_MOVE_MOVEABLE_WITH_DATA_IN_X_PARAMETERS_LIST: {
			moveable_move_moveable_with_data_in_x_parameter_list(timer, extra_timer);
			break;
		}
		case SOUND_PLAY_SOUND_SINGLE_PLAYBACK_OF_GLOBAL_SOUND_MAP: {
			sound_play_sound_single_playback_of_global_sound_map(timer, extra_timer);
			break;
		}
		case LARA_FORCE_X_ANIMATION_FOR_LARA_PRESERVE_STATE_ID: {
			lara_force_x_animation_for_lara_preserve_state_id(timer, extra_timer);
			break;
		}
		case LARA_FORCE_X_ANIMATION_FOR_LARA_SET_NEW_STATE_ID: {
			lara_force_x_animation_for_lara_set_new_state_id(timer, extra_timer);
			break;
		}
		case LARA_FORCE_X_ANIMATION_FOR_LARA_SET_NEUTRAL_STATE_ID: {
			lara_force_x_animation_for_lara_set_neutral_state_id(timer, extra_timer);
			break;
		}
		case STATIC_ROTATE_STATIC_WITH_DATA_OF_X_PARAMETER_LIST: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_ROTATE_STATIC_WITH_DATA_OF_X_PARAMETER_LIST unimplemented!");
			break;
		}
		case MOVEABLE_ROTATE_MOVEABLE_WITH_DATA_IN_X_PARAMETERS_LIST: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "MOVEABLE_ROTATE_MOVEABLE_WITH_DATA_IN_X_PARAMETERS_LIST unimplemented!");
			break;
		}
		case STATIC_STOP_ALL_ROTATION_FOR_X_STATIC: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_STOP_ALL_ROTATION_FOR_X_STATIC unimplemented!");
			break;
		}
		case STATIC_STOP_ALL_ROTATION_FOR_STATICS_IN_X_ROOM: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_STOP_ALL_ROTATION_FOR_STATICS_IN_X_ROOM unimplemented!");
			break;
		}
		case MOVEABLE_STOP_ALL_ROTATION_FOR_MOVEABLES_IN_X_ROOM: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "MOVEABLE_STOP_ALL_ROTATION_FOR_MOVEABLES_IN_X_ROOM unimplemented!");
			break;
		}
		case STATIC_STOP_THE_MOTION_OF_X_STATIC: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_STOP_THE_MOTION_OF_X_STATIC unimplemented!");
			break;
		}
		case MOVEABLE_STOP_ALL_MOTIONS_OF_ALL_MOVEABLES_IN_X_ROOM: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "MOVEABLE_STOP_ALL_MOTIONS_OF_ALL_MOVEABLES_IN_X_ROOM unimplemented!");
			break;
		}
		case STATIC_STOP_THE_MOVEMENTS_OF_ALL_STATICS_IN_X_ROOM: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_STOP_THE_MOVEMENTS_OF_ALL_STATICS_IN_X_ROOM unimplemented!");
			break;
		}
		case STATIC_EXPLODE: {
			static_explode(timer, extra_timer);
			break;
		}
		case STATIC_SET_EXPLOSIVE_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_EXPLOSIVE_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_REMOVE_EXPLOSIVE_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_REMOVE_EXPLOSIVE_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_SET_POISON_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_POISON_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_REMOVE_POISON_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_REMOVE_POISON_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_SET_DAMAGE_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_DAMAGE_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_REMOVE_DAMAGE_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_REMOVE_DAMAGE_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_SET_BURNING_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_BURNING_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_REMOVE_BURNING_ATTRIBUTE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_REMOVE_BURNING_ATTRIBUTE unimplemented!");
			break;
		}
		case STATIC_VISIBILITY_SET_AS_INVISIBLE: {
			static_visibility_set_as_invisible(timer, extra_timer);
			break;
		}
		case STATIC_VISIBILITY_RENDER_NEWLY_VISIBLE: {
			static_visibility_render_newly_visible(timer, extra_timer);
			break;
		}
		case STATIC_SET_COLOR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATIC_SET_COLOR unimplemented!");
			break;
		}
		case TRIGGERGROUP_STOP_X_TRIGGERGROUP: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TRIGGERGROUP_STOP_X_TRIGGERGROUP unimplemented!");
			break;
		}
		case PLAY_TRACK_ON_CHANNEL_WITH_RESTORE: {
			play_track_on_channel_with_restore(timer, extra_timer);
			break;
		}
		case WEATHER_SET_X_DISTANCE_FOG_VALUE: {
			LevelFogStart = (float(BLOCK_SIZE) * 120.0F) - (float(BLOCK_SIZE) * (float)((uint8_t)timer));
			break;
		}
		case WEATHER_CHANGE_START_FOR_DISTANCE_TO_X_DISTANCE_IN_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_CHANGE_START_FOR_DISTANCE_TO_X_DISTANCE_IN_SECONDS unimplemented!");
			break;
		}
		case WEATHER_PULSE_START_DISTANCE_FOG_FROM_CURRENT_TO_X_DISTANCE_IN_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_PULSE_START_DISTANCE_FOG_FROM_CURRENT_TO_X_DISTANCE_IN_SECONDS unimplemented!");
			break;
		}
		case WEATHER_ENABLE_OR_DISABLE_ALL_FOG: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_ENABLE_OR_DISABLE_ALL_FOG unimplemented!");
			break;
		}
		case WEATHER_STOP_THE_PULSE_START_DISTANCE_FOG_AND_SET_NEW_X_FOG_DISTANCE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_STOP_THE_PULSE_START_DISTANCE_FOG_AND_SET_NEW_X_FOG_DISTANCE unimplemented!");
			break;
		}
		case LARA_LIGHT_OR_PUT_OUT_THE_TORCH_IN_LARAS_HAND: {
			lara_light_or_put_out_torch_in_laras_hand(timer, extra_timer);
			break;
		}
		case LARA_GIVE_OR_REMOVE_TORCH_TO_OR_FROM_HAND_OF_LARA: {
			lara_give_or_remove_torch_to_or_from_hand_of_lara(timer, extra_timer);
			break;
		}
		case TEXT_VERTICAL_SCROLLING_OF_NGEXTRA_X_STRING_WITH_SPEED: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_VERTICAL_SCROLLING_OF_NGEXTRA_X_STRING_WITH_SPEED unimplemented!");
			break;
		}
		case TEXT_ABORT_ALL_VERTICAL_SCROLLING_TEXT_OPERATIONS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_ABORT_ALL_VERTICAL_SCROLLING_TEXT_OPERATIONS unimplemented!");
			break;
		}
		case TEXT_PRINT_FORMATTED_TEXT_NGEXTRA_STRING_WITH_FORMATTING_DATA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_FORMATTED_TEXT_NGEXTRA_STRING_WITH_FORMATTING_DATA unimplemented!");
			break;
		}
		case TEXT_REMOVE_X_EXTRA_NGSTRING_FROM_SCREEN: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_REMOVE_X_EXTRA_NGSTRING_FROM_SCREEN unimplemented!");
			break;
		}
		case TEXT_VERTICAL_SCROLLING_OF_NGEXTRA_X_STRING_WITH_FORMATTING_DATA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_VERTICAL_SCROLLING_OF_NGEXTRA_X_STRING_WITH_FORMATTING_DATA unimplemented!");
			break;
		}
		case TEXT_HORIZONTAL_SCROLLING_OF_NGEXTRA_X_STRING_WITH_FORMATTING_DATA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_HORIZONTAL_SCROLLING_OF_NGEXTRA_X_STRING_WITH_FORMATTING_DATA unimplemented!");
			break;
		}
		case TEXT_PRINT_PSX_X_STRING_WITH_FORMATTING_DATA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_PSX_X_STRING_WITH_FORMATTING_DATA unimplemented!");
			break;
		}
		case TEXT_PRINT_PC_X_STRING_WITH_FORMATTING_DATA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_PC_X_STRING_WITH_FORMATTING_DATA unimplemented!");
			break;
		}
		case TEXT_PRINT_PC_X_STRING_WITH_FORMATTING_DATA_AND_WAIT_FOR_ESCAPE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_PC_X_STRING_WITH_FORMATTING_DATA_AND_WAIT_FOR_ESCAPE unimplemented!");
			break;
		}
		case TEXT_PRINT_NGEXTRA_X_STRING_WITH_FORMATTING_DATA_AND_WAIT_FOR_ESCAPE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_NGEXTRA_X_STRING_WITH_FORMATTING_DATA_AND_WAIT_FOR_ESCAPE unimplemented!");
			break;
		}
		case ANIMCOMMAND_SET_TEMPORARY_FREE_HANDS_UNTIL_IS_PERFORMING_X_ANIMATION: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_SET_TEMPORARY_FREE_HANDS_UNTIL_IS_PERFORMING_X_ANIMATION unimplemented!");
			break;
		}
		case ANIMCOMMAND_SET_TEMPORARY_FREE_HANDS_FOR_X_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_SET_TEMPORARY_FREE_HANDS_FOR_X_SECONDS unimplemented!");
			break;
		}
		case ANIMCOMMAND_REMOVE_FREE_HANDS_AND_RESTORE_PREVIOUS_STATUS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_REMOVE_FREE_HANDS_AND_RESTORE_PREVIOUS_STATUS unimplemented!");
			break;
		}
		case CAMERA_CHANGE_CAMERA_MODE_WITH_X_PARAMETER_FOR_TIME: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_CHANGE_CAMERA_MODE_WITH_X_PARAMETER_FOR_TIME unimplemented!");
			break;
		}
		case CAMERA_RESTORE_CAMERA_MODE_AFTER_A_CHANGE_CAMERA_FLIPEFFECT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_RESTORE_CAMERA_MODE_AFTER_A_CHANGE_CAMERA_FLIPEFFECT unimplemented!");
			break;
		}
		case ANIMCOMMAND_RESET_THE_NUMBER_OF_TURNS_JUMP_POWER_OF_PARALLEL_BAR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_RESET_THE_NUMBER_OF_TURNS_JUMP_POWER_OF_PARALLEL_BAR unimplemented!");
			break;
		}
		case IMAGES_SHOW_IMAGE_WITH_DATA_IN_X_IMAGE_SCRIPT_COMMAND_FOR_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "IMAGES_SHOW_IMAGE_WITH_DATA_IN_X_IMAGE_SCRIPT_COMMAND_FOR_SECONDS unimplemented!");
			break;
		}
		case IMAGES_REMOVE_FROM_SCREEN_THE_CURRENT_POP_UP_IMAGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "IMAGES_REMOVE_FROM_SCREEN_THE_CURRENT_POP_UP_IMAGE unimplemented!");
			break;
		}
		case DIARY_ADD_EXTRA_NG_STIRNG_TO_DIARY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "DIARY_ADD_EXTRA_NG_STIRNG_TO_DIARY unimplemented!");
			break;
		}
		case DIARY_CLEAR_ALL_STRINGS_IN_X_DIARY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "DIARY_CLEAR_ALL_STRINGS_IN_X_DIARY unimplemented!");
			break;
		}
		case DIARY_REMOVE_LAST_STRING_FROM_X_DIARY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "DIARY_REMOVE_LAST_STRING_FROM_X_DIARY unimplemented!");
			break;
		}
		case DIARY_SHOW_DIARY_AT_PAGE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "DIARY_SHOW_DIARY_AT_PAGE unimplemented!");
			break;
		}
		case SHOW_STATISTICS_SCREEN: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SHOW_STATISTICS_SCREEN unimplemented!");
			break;
		}
		case WEATHER_SET_THE_X_COLOR_FOR_DISTANCE_FOG: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SET_THE_X_COLOR_FOR_DISTANCE_FOG unimplemented!");
			break;
		}
		case WEATHER_ENABLE_HARDWARE_FOG: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_ENABLE_HARDWARE_FOG unimplemented!");
			break;
		}
		case WEATHER_SET_X_MAX_VISIBILITY_DISTANCE_FOR_FOG_BULBS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SET_X_MAX_VISIBILITY_DISTANCE_FOR_FOG_BULBS unimplemented!");
			break;
		}
		case WEATHER_SET_X_END_FOG_LIMIT_FOR_DISTANCE_FOG: {
			LevelFogEnd = (float(BLOCK_SIZE) * 120.0F) - (float(BLOCK_SIZE) * (float)((uint8_t)timer));
			break;
		}
		case WEATHER_CHANGE_END_LIMIT_OF_DISTANCE_FOG_IN_X_WAY_WITH_SPEED: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_CHANGE_END_LIMIT_OF_DISTANCE_FOG_IN_X_WAY_WITH_SPEED unimplemented!");
			break;
		}
		case WEATHER_CHANGE_START_LIMIT_OF_DISTANCE_FOG_IN_X_WAY_WITH_SPEED: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_CHANGE_START_LIMIT_OF_DISTANCE_FOG_IN_X_WAY_WITH_SPEED unimplemented!");
			break;
		}
		case WEATHER_STOP_THE_X_CHANGE_LIMIT_OF_DISTANEC_FOG_EFFECT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_STOP_THE_X_CHANGE_LIMIT_OF_DISTANEC_FOG_EFFECT unimplemented!");
			break;
		}
		case VARIABLES_ADD_VALUE_TO_X_VARIABLE: {
			variables_add_value_to_x_variable(timer, extra_timer);
			break;
		}
		case VARIABLES_SET_X_VARIABLE_TO_VALUE: {
			variables_set_x_variable_to_value(timer, extra_timer);
			break;
		}
		case VARIABLES_SUBTRACT_VALUE_FROM_X_VARIABLE: {
			variables_subtract_value_from_x_variable(timer, extra_timer);
			break;
		}
		case VARIABLES_SET_IN_X_VARIABLE_THE_BIT: {
			variables_set_in_x_variable_the_bit(timer, extra_timer);
			break;
		}
		case VARIABLES_CLEAR_IN_X_VARIABLE_THE_BIT: {
			variables_clear_in_x_variable_the_bit(timer, extra_timer);
			break;
		}
		case VARIABLES_COPY_CURRENTVALUE_TO_X_STORE_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_CURRENTVALUE_TO_X_STORE_VARIABLE unimplemented!");
			break;
		}
		case VARIABLES_COPY_X_STORE_VARIABLE_TO_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_X_STORE_VARIABLE_TO_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_COPY_THE_X_TEXT_VARIABLE_TO_Y_TEXT_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_THE_X_TEXT_VARIABLE_TO_Y_TEXT_VALUE unimplemented!");
			break;
		}
		case VARIABLES_COPY_THE_X_NG_STRING_TO_Y_TEXT_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_THE_X_NG_STRING_TO_Y_TEXT_VALUE unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_BIG_TEXT_THE_X_NG_STRING_WITH_SEPARATOR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_BIG_TEXT_THE_X_NG_STRING_WITH_SEPARATOR unimplemented!");
			break;
		}
		case VARIABLES_CLEAR_THE_X_VARIABLE_GROUP: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CLEAR_THE_X_VARIABLE_GROUP unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_BIG_TEXT_THE_X_TEXT_VARIABLE_WITH_SEPARATOR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_BIG_TEXT_THE_X_TEXT_VARIABLE_WITH_SEPARATOR unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_BIG_TEXT_THE_X_NUMERIC_VARIABLE_WITH_SEPARATOR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_BIG_TEXT_THE_X_TEXT_VARIABLE_WITH_SEPARATOR unimplemented!");
			break;
		}
		case VARIABLES_COPY_TO_X_NUMERIC_VARIABLE_THE_SAVEGAME_MEMORY_VALUE: {
			variables_copy_to_x_numeric_variable_the_savegame_memory_value(timer, extra_timer);
			break;
		}
		case VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_THE_SAVEGAME_MEMORY_VALUE: {
			variables_copy_from_x_numeric_variable_the_savegame_memory_value(timer, extra_timer);
			break;
		}
		case VARIABLES_SET_IN_X_SAVEGAME_MEMORY_THE_VALUE: {
			variables_set_in_x_savegame_memory_the_value(timer, extra_timer);
			break;
		}
		case VARIABLES_SET_IN_X_SAVEGAME_MEMORY_THE_BIT: {
			variables_set_in_x_savegame_memory_the_bit(timer, extra_timer);
			break;
		}
		case VARIABLES_CLEAR_IN_X_SAVEGAME_MEMORY_THE_BIT: {
			variables_clear_in_x_savegame_memory_the_bit(timer, extra_timer);
			break;
		}
		case VARIABLES_ADD_TO_X_SAVEGAME_MEMORY_THE_VALUE: {
			variables_add_to_x_savegame_memory_the_value(timer, extra_timer);
			break;
		}
		case VARIABLES_SUBTRACT_FROM_X_SAVEGAME_MEMORY_THE_VALUE: {
			variables_subtract_to_x_savegame_memory_the_value(timer, extra_timer);
			break;
		}
		case VARIABLES_MULTIPLY_X_VARIABLE_BY_VALUE: {
			variables_multiply_x_variable_by_value(timer, extra_timer);
			break;
		}
		case VARIABLES_SET_IN_X_NUMERIC_VARIABLE_THE_NEGATIVE_VALUE: {
			variables_set_in_x_numeric_variable_the_negative_value(timer, extra_timer);
			break;
		}
		case VARIABLES_DIVIDE_X_VARIABLE_BY_VALUE: {
			variables_divide_x_variable_by_value(timer, extra_timer);
			break;
		}
		case VARIABLES_SET_IN_X_SAVEGAME_MEMORY_THE_NEGATIVE_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_SAVEGAME_MEMORY_THE_NEGATIVE_VALUE unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_SELECTED_ITEM_MEMORY_THE_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_SELECTED_ITEM_MEMORY_THE_VALUE unimplemented!");
			break;
		}
		case VARIABLES_COPY_TO_X_NUMERIC_VARIABLE_THE_SELECTED_ITEM_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_TO_X_NUMERIC_VARIABLE_THE_SELECTED_ITEM_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_SELECTED_ITEM_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_SELECTED_ITEM_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_X_SELECTED_ITEM_MEMORY_THE_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_X_SELECTED_ITEM_MEMORY_THE_VALUE unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_SELECTED_ITEM_MEMORY_THE_BIT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_SELECTED_ITEM_MEMORY_THE_BIT unimplemented!");
			break;
		}
		case VARIABLES_CLEAR_IN_X_SELECTED_ITEM_MEMORY_THE_BIT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CLEAR_IN_X_SELECTED_ITEM_MEMORY_THE_BIT unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_SELECTED_ITEM_MEMORY_THE_BIG_NUMBER_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_SELECTED_ITEM_MEMORY_THE_BIG_NUMBER_VALUE unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_SAVEGAME_MEMORY_THE_BIG_NUMBER_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_SAVEGAME_MEMORY_THE_BIG_NUMBER_VALUE unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_NUMBERIC_VARIABLE_THE_BIG_NUMBER_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_NUMBERIC_VARIABLE_THE_BIG_NUMBER_VALUE unimplemented!");
			break;
		}
		case VARIABLES_START_THE_X_TRNG_TIMER_TO_MODE: {
			variables_start_the_x_trng_timer_in_mode(timer, extra_timer);
			break;
		}
		case VARIABLES_STOP_THE_X_TRNG_TIMER: {
			variables_stop_the_x_trng_timer(timer, extra_timer);
			break;
		}
		case VARIABLES_INITIALIZE_TRNG_TIMER_TO_X_SECONDS: {
			variables_initialize_the_x_trng_timer_to_seconds(timer, extra_timer);
			break;
		}
		case VARIABLES_INITIALIZE_X_TRNG_TIMER_TO_BIG_NUMBER_SECONDS: {

			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_INITIALIZE_X_TRNG_TIMER_TO_BIG_NUMBER_SECONDS unimplemented!");
			break;
		}
		case VARIABLES_INITIALIZE_X_TRNG_TIMER_TO_FRAME_TICKS: {
			variables_initialize_the_x_trng_timer_to_frame_ticks(timer, extra_timer);
			break;
		}
		case VARIABLES_SHOW_X_TRNG_TIMER_IN_POSITION: {
			variables_show_x_trng_timer_in_position(timer, extra_timer);
			break;
		}
		case VARIABLES_HIDE_X_TRNG_TIMER_IN_SECONDS: {
			variables_hide_the_x_trng_timer_in_seconds(timer, extra_timer);
			break;
		}
		case VARIABLES_COPY_X_NUMERIC_VARIABLE_TO_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_X_NUMERIC_VARIABLE_TO_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_COPY_CURRENTVALUE_TO_X_NUMBERIC_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_CURRENTVALUE_TO_X_NUMBERIC_VALUE unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_X_SAVEGAME_MEMORY_THE_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_X_SAVEGAME_MEMORY_THE_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_SUBTRACT_FROM_X_SAVEGAME_MEMORY_THE_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SUBTRACT_FROM_X_SAVEGAME_MEMORY_THE_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_SELECTED_ITEM_MEMORY_THE_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_SELECTED_ITEM_MEMORY_THE_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_SUBTRACT_TO_SELECTED_ITEM_MEMORY_THE_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SUBTRACT_TO_SELECTED_ITEM_MEMORY_THE_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_X_CODE_MEMORY_TO_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_X_CODE_MEMORY_TO_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_CURRENTVALUE_TO_X_CODE_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_CURRENTVALUE_TO_X_CODE_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_CODE_MEMORY_THE_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_CODE_MEMORY_THE_VALUE unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_X_CODE_MEMORY_THE_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_X_CODE_MEMORY_THE_VALUE unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_CODE_MEMORY_THE_BIT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_CODE_MEMORY_THE_BIT unimplemented!");
			break;
		}
		case VARIABLES_CLEAR_IN_X_CODE_MEMORY_THE_BIT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CLEAR_IN_X_CODE_MEMORY_THE_BIT unimplemented!");
			break;
		}
		case VARIABLES_ADD_TO_X_CODE_MEMORY_THE_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_X_CODE_MEMORY_THE_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_INVERT_THE_SIGN_OF_X_NUMERIC_VALUE: {
			variables_numeric_invert_the_sign_of_x_numeric_value(timer, extra_timer);
			break;
		}
		case VARIABLES_ADD_TO_CURRENTVALUE_THE_X_NUMERIC_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_ADD_TO_CURRENTVALUE_THE_X_NUMERIC_VARIABLE unimplemented!");
			break;
		}
		case VARIABLES_SUBTRACT_FROM_CURRENTVALUE_THE_X_NUMERIC_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SUBTRACT_FROM_CURRENTVALUE_THE_X_NUMERIC_VARIABLE unimplemented!");
			break;
		}
		case VARIABLES_DIVIDE_CURRENTVALUE_BY_X_NUMERIC_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_DIVIDE_CURRENTVALUE_BY_X_NUMERIC_VARIABLE unimplemented!");
			break;
		}
		case VARIABLES_MULTIPLY_CURRENTVALUE_BY_X_NUMERIC_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_MULTIPLY_CURRENTVALUE_BY_X_NUMERIC_VARIABLE unimplemented!");
			break;
		}
		case SWITCH_PERFORM_THE_X_SWITCH_SCRIPT_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SWITCH_PERFORM_THE_X_SWITCH_SCRIPT_COMMAND unimplemented!");
			break;
		}
		case ORGANIZER_RESUME_X_ORGANIZER_IN_Y_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ORGANIZER_RESUME_X_ORGANIZER_IN_Y_WAY unimplemented!");
			break;
		}
		case VARIABLES_COPY_TO_X_NUMERIC_VARIABLE_THE_Y_COLOR_RGB: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_TO_X_NUMERIC_VARIABLE_THE_Y_COLOR_RGB unimplemented!");
			break;
		}
		case VARIABLES_SET_THE_X_SLOT_AS_SELECTED_SLOT_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_THE_X_SLOT_AS_SELECTED_SLOT_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_SELECTED_SLOT_MEMORY_TO_X_NUMERIC_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_SELECTED_SLOT_MEMORY_TO_X_NUMERIC_VARIABLE unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_SELECTED_SLOT_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_SELECTED_SLOT_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_SELECTED_ANIMATION_MEMORY_TO_X_NUMERIC_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_SELECTED_ANIMATION_MEMORY_TO_X_NUMERIC_VARIABLE unimplemented!");
			break;
		}
		case VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_SELECTED_ANIMATION_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_SELECTED_ANIMATION_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_CONVERT_THE_X_NGLE_ROOM_INDEX_IN_TOMB_ROOM_INDEX_TO_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CONVERT_THE_X_NGLE_ROOM_INDEX_IN_TOMB_ROOM_INDEX_TO_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_CONVERT_THE_TOMB_ROOM_INDEX_TO_NGLE_ROOM_INDEX_IN_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CONVERT_THE_TOMB_ROOM_INDEX_TO_NGLE_ROOM_INDEX_IN_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_CONVERT_THE_NGLE_ROOM_INDEX_TO_TOMB_ROOM_INDEX_IN_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CONVERT_THE_NGLE_ROOM_INDEX_TO_TOMB_ROOM_INDEX_IN_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_CONVERT_THE_TOMB_ITEM_INDEX_TO_NGLE_ITEM_INDEX_IN_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CONVERT_THE_TOMB_ITEM_INDEX_TO_NGLE_ITEM_INDEX_IN_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_CONVERT_THE_NGLE_ITEM_INDEX_TO_TOMB_ITEM_INDEX_IN_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CONVERT_THE_NGLE_ITEM_INDEX_TO_TOMB_ITEM_INDEX_IN_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_SAVE_THE_COORDINATES_AND_FACING_OF_ITEM_INDEX_IN_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SAVE_THE_COORDINATES_AND_FACING_OF_ITEM_INDEX_IN_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_GENERATE_IN_X_NUMERIC_VARIABLE_THE_RANDOM_NUMBER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_GENERATE_IN_X_NUMERIC_VARIABLE_THE_RANDOM_NUMBER unimplemented!");
			break;
		}
		case VARIABLES_GENERATE_IN_X_NUMERIC_VARIABLE_A_RANDOM_CURRENTVALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_GENERATE_IN_X_NUMERIC_VARIABLE_A_RANDOM_CURRENTVALUE unimplemented!");
			break;
		}
		case VARIABLES_PERFORM_OPERATION_X_NUMERICVARIABLE_AND_NUMBER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_PERFORM_OPERATION_X_NUMERICVARIABLE_AND_NUMBER unimplemented!");
			break;
		}
		case INVENTORY_POP_UP_INVENTORY_TO_SELECT_X_ITEM_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "INVENTORY_POP_UP_INVENTORY_TO_SELECT_X_ITEM_IN_WAY unimplemented!");
			break;
		}
		case CUSTOM_BAR_SHOW_THE_X_CUSTOM_BAR_ON_SCREEN_FOR_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUSTOM_BAR_SHOW_THE_X_CUSTOM_BAR_ON_SCREEN_FOR_SECONDS unimplemented!");
			break;
		}
		case CUSTOM_BAR_HIDE_THE_X_CUSTOM_BAR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUSTOM_BAR_HIDE_THE_X_CUSTOM_BAR unimplemented!");
			break;
		}
		case FLIPMAP_ALTERNATIVE_CONTINOUSLY_THE_X_FLIPMAP_WITH_THE_E_FRAME_INTERVAL: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "FLIPMAP_ALTERNATIVE_CONTINOUSLY_THE_X_FLIPMAP_WITH_THE_E_FRAME_INTERVAL unimplemented!");
			break;
		}
		case FLIPMAP_STOP_THE_ALTERNATE_OF_X_FLIPMAP_LETTING_THE_E_FLIPMAP_TYPE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "FLIPMAP_STOP_THE_ALTERNATE_OF_X_FLIPMAP_LETTING_THE_E_FLIPMAP_TYPE unimplemented!");
			break;
		}
		case VARIABLES_SET_THE_X_INVENTORY_ITEM_AS_SELECTED_INVENTORY_MEMORY: {
			variables_set_the_x_inventory_item_as_selected_inventory_memory(timer, extra_timer);
			break;
		}
		case VARAIBLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_E_INVENTORY_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARAIBLES_COPY_FROM_X_NUMERIC_VARIABLE_TO_E_INVENTORY_MEMORY unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_INVENTORY_MEMORY_THE_E_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_INVENTORY_MEMORY_THE_E_VALUE unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_INVENTORY_MEMORY_THE_E_BIG_NUMBER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_INVENTORY_MEMORY_THE_E_BIG_NUMBER unimplemented!");
			break;
		}
		case VARIABLES_COPY_TO_X_NUMERIC_VARIBLE_THE_E_INVENTORY_MEMORY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_COPY_TO_X_NUMERIC_VARIBLE_THE_E_INVENTORY_MEMORY unimplemented!");
			break;
		}
		case LARA_SWAP_MESH_OF_X_LARA_SLOTS_WITH_THAT_OF_E_SLOT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_SWAP_MESH_OF_X_LARA_SLOTS_WITH_THAT_OF_E_SLOT unimplemented!");
			break;
		}
		case SWAP_MESH_OF_X_SLOT_WITH_MESH_OF_E_SLOT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SWAP_MESH_OF_X_SLOT_WITH_MESH_OF_E_SLOT unimplemented!");
			break;
		}
		case VARIABLES_SET_IN_X_CODE_MEMORY_THE_E_NEGATIVE_NUMBER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_SET_IN_X_CODE_MEMORY_THE_E_NEGATIVE_NUMBER unimplemented!");
			break;
		}
		case FISH_CLEAR_ALL_FISH_OF_X_TYPE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "FISH_CLEAR_ALL_FISH_OF_X_TYPE unimplemented!");
			break;
		}
		case SWAP_MESH_OF_CURRENT_HORIZONT_OBJECT_WITH_X_SLOT_MESH: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SWAP_MESH_OF_CURRENT_HORIZONT_OBJECT_WITH_X_SLOT_MESH unimplemented!");
			break;
		}
		case TRIGGERGROUP_ENABLE_NEWLY_THE_ONESHOT_X_TRIGGERGROUP_ALREADY_PERFORMED: {
			triggergroup_enable_newly_the_oneshot_x_triggergroup_already_performed(timer, extra_timer);
			break;
		}
		case CAMERA_ENABLE_THE_X_STANDBY_CAMERA_EFFECT_FOR_E_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_ENABLE_THE_X_STANDBY_CAMERA_EFFECT_FOR_E_SECONDS unimplemented!");
			break;
		}
		case ENEMY_FREEZE_ALL_ENEMIES_FOR_X_SECONDS_IN_E_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ENEMY_FREEZE_ALL_ENEMIES_FOR_X_SECONDS_IN_E_WAY unimplemented!");
			break;
		}
		case ENEMY_REMOVE_THE_FREEZE_ALL_ENEMIES_MODE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ENEMY_REMOVE_THE_FREEZE_ALL_ENEMIES_MODE unimplemented!");
			break;
		}
		case LARA_TRIGGER_CLOCKWISE_WHIRL_AT_X_CENTER_AND_E_DIAMETER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_TRIGGER_CLOCKWISE_WHIRL_AT_X_CENTER_AND_E_DIAMETER unimplemented!");
			break;
		}
		case LARA_TRIGGER_UNCLOCKWISE_WHIRL_AT_X_CENTER_AND_E_DIAMETER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_TRIGGER_UNCLOCKWISE_WHIRL_AT_X_CENTER_AND_E_DIAMETER unimplemented!");
			break;
		}
		case VARIABLES_CONVERT_FROM_ITEM_ADDRESS_TO_ITEM_INDEX_THE_VALUE_IN_CURRENT_VALUE_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CONVERT_FROM_ITEM_ADDRESS_TO_ITEM_INDEX_THE_VALUE_IN_CURRENT_VALUE_VARIABLE unimplemented!");
			break;
		}
		case STATICS_SCALE_A_STATIC_ITEM_USING_THE_DATA_IN_X_PARAMETERS_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATICS_SCALE_A_STATIC_ITEM_USING_THE_DATA_IN_X_PARAMETERS_COMMAND unimplemented!");
			break;
		}
		case STATICS_STOP_THE_ENDLESS_SCALING_OF_THE_STATICS_START_WITH_THE_X_PARAMETERS_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "STATICS_STOP_THE_ENDLESS_SCALING_OF_THE_STATICS_START_WITH_THE_X_PARAMETERS_COMMAND unimplemented!");
			break;
		}
		case ITEMGROUP_PERFORM_X_TRIGGER_WITH_E_ITEMGROUP_OF_STATICS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ITEMGROUP_PERFORM_X_TRIGGER_WITH_E_ITEMGROUP_OF_STATICS unimplemented!");
			break;
		}
		case SCREEN_FLASH_SCREEN_WITH_LIGHT_COLOR_FOR_DURATION: {

			screen_flash_screen_with_light_color_for_duration(timer, extra_timer);
			break;
		}
		case SCREEN_REMOVE_INFINITE_FLASH_EFFECT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SCREEN_REMOVE_INFINITE_FLASH_EFFECT unimplemented!");
			break;
		}
		case SPRITE_SHOW_SPRITE_WITH_DATA_IN_X_PARAMETERS_FOR_DURATION: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SPRITE_SHOW_SPRITE_WITH_DATA_IN_PARAMETERS_FOR_DURATION unimplemented!");
			break;
		}
		case SPRITE_REMOVE_FROM_SCREEN_THE_SPRITE_WITH_DATA_IN_X_PARAMETERS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SPRITE_REMOVE_FROM_SCREEN_THE_SPRITE_WITH_DATA_IN_X_PARAMETERS unimplemented!");
			break;
		}
		case WEATHER_PERFORM_A_LIGHTNING_WITH_DATA_IN_X_PARAMETER_FOR_E_DURATE_IN_TICK_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_PERFORM_A_LIGHTNING_WITH_DATA_IN_X_PARAMETER_FOR_E_DURATE_IN_TICK_FRAMES unimplemented!");
			break;
		}
		case TEXT_PRINT_UNLIMITED_X_EXTRA_NG_STRING_WITH_CURRENT_SETTINGS_FOR_INFINITE_TIME: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_UNLIMITED_X_EXTRA_NG_STRING_WITH_CURRENT_SETTINGS_FOR_INFINITE_TIME unimplemented!");
			break;
		}
		case WEATHER_SET_IN_ADVANCE_THE_RAIN_SNOW_SETTINGS_USING_THE_INTENSITY_OF_X_ROOM: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_SET_IN_ADVANCE_THE_RAIN_SNOW_SETTINGS_USING_THE_INTENSITY_OF_X_ROOM unimplemented!");
			break;
		}
		case WEATHER_STOP_THE_ENDLESS_LIGHTNING_EFFECT_WITH_X_PARAMETERS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "WEATHER_STOP_THE_ENDLESS_LIGHTNING_EFFECT_WITH_X_PARAMETERS unimplemented!");
			break;
		}
		case TEXT_PRINT_EXTRA_NG_X_STRING_WITH_WINDOWS_FONT_AND_FORMATTING_DATA_IN_THE_E_PARAMETERS_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_PRINT_EXTRA_NG_X_STRING_WITH_WINDOWS_FONT_AND_FORMATTING_DATA_IN_THE_E_PARAMETERS_COMMAND unimplemented!");
			break;
		}
		case TEXT_REMOVE_PRINT_EXTRA_NG_X_STRING_WITH_WINDOWS_FONT_THAT_USED_IN_THE_E_PARAMETERS_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TEXT_REMOVE_PRINT_EXTRA_NG_X_STRING_WITH_WINDOWS_FONT_THAT_USED_IN_THE_E_PARAMETERS_COMMAND unimplemented!");
			break;
		}
		case CAMERA_INCREASE_THE_ZOOM_FACTOR_FOR_BINOCULAR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_INCREASE_THE_ZOOM_FACTOR_FOR_BINOCULAR unimplemented!");
			break;
		}
		case VARIABLES_MOVE_X_FUEL_TO_BOAT_TANK_IN_E_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_MOVE_X_FUEL_TO_BOAT_TANK_IN_E_WAY unimplemented!");
			break;
		}
		case CAMERA_SHOW_BLACK_SCREEN_FOR_SECONDS_WITH_FINAL_CURTAIN_EFFECT: {
			camera_show_black_screen_for_seconds_with_final_curtain_effect(timer, extra_timer);
			break;
		}
		case CAMERA_STOP_BLACK_SCREEN_AND_OPEN_CURTAIN: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_STOP_BLACK_SCREEN_AND_OPEN_CURTAIN unimplemented!");
			break;
		}
		case CAMERA_SET_CINEMA_EFFECT_TYPE_FOR_SECONDS: {
			camera_set_cinema_effect_type_for_seconds(timer, extra_timer);
			break;
		}
		case ANIMCOMMAND_IF_CURRENT_ITEM_IS_WADING_WATER_ADD_TWIRLS_WITH_X_INTENSITY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMCOMMAND_IF_CURRENT_ITEM_IS_WADING_WATER_ADD_TWIRLS_WITH_X_INTENSITY unimplemented!");
			break;
		}
		case PERFORM_X_TRIGGERGROUP_FROM_SCRIPT_IN_SINGLE_EXECUTION: {
			perform_triggergroup_from_script_in_single_execution_mode(timer, extra_timer);
			break;
		}
		case PERFORM_X_TRIGGERGROUP_FROM_SCRIPT_IN_MULTI_EXECUTION: {
			repeat_type = 0;
			perform_triggergroup_from_script_in_multi_execution_mode(timer, extra_timer);
			break;
		}
		case PERFORM_X_TRIGGERGROUP_FROM_SCRIPT_IN_CONTINUOUS_EXECUTION: {
			perform_triggergroup_from_script_in_continuous_mode(timer, extra_timer);
			break;
		}
		case GLOBAL_TRIGGER_ENABLE_WITH_ID: {
			enable_global_trigger_with_id(timer, extra_timer);
			break;
		}
		case GLOBAL_TRIGGER_DISABLE_WITH_ID: {
			disable_global_trigger_with_id(timer, extra_timer);
			break;
		}
		case ORGANIZER_RESUME_X_ORGANIZER_FROM_THE_FIRST_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ORGANIZER_RESUME_X_ORGANIZER_FROM_THE_FIRST_COMMAND unimplemented!");
			break;
		}
		case ORGANIZER_RESUME_X_ORGANIZER_FROM_NEXT_COMMAND_IMMEDIATE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ORGANIZER_RESUME_X_ORGANIZER_FROM_NEXT_COMMAND_IMMEDIATE unimplemented!");
			break;
		}
		case ORGANIZER_RESUME_X_ORGANIZER_FROM_NEXT_COMMAND_IN_THE_GIVEN_TIME_SET_IN_NEXT_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ORGANIZER_RESUME_X_ORGANIZER_FROM_NEXT_COMMAND_IN_THE_GIVEN_TIME_SET_IN_NEXT_COMMAND unimplemented!");
			break;
		}
		case CUTSCENE_PERFORM_THE_DEMO_PAK_AT_X_INDEX_OF_DEMO_SCRIPT_COMMAND: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_PERFORM_THE_DEMO_PAK_AT_X_INDEX_OF_DEMO_SCRIPT_COMMAND unimplemented!");
			break;
		}
		case CUTSCENE_STOP_THE_CURRENT_DEMO_IN_PROGRESS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_STOP_THE_CURRENT_DEMO_IN_PROGRESS unimplemented!");
			break;
		}
		case CUTSCENE_LOOK_AT_LEADING_ACTOR_FROM_X_VIEW_ANGLE_AND_E_DISTANCE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_LOOK_AT_LEADING_ACTOR_FROM_X_VIEW_ANGLE_AND_E_DISTANCE unimplemented!");
			break;
		}
		case CUTSCENE_LOOK_EXTRA_ACTOR_FROM_X_VIEW_ANGLE_AND_E_DISTANCE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_LOOK_EXTRA_ACTOR_FROM_X_VIEW_ANGLE_AND_E_DISTANCE unimplemented!");
			break;
		}
		case CUTSCENE_LOOK_LARA_FROM_X_VIEW_ANGLE_AND_E_DISTANCE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_LOOK_LARA_FROM_X_VIEW_ANGLE_AND_E_DISTANCE unimplemented!");
			break;
		}
		case CUTSCENE_RESET_CUTSCENE_CAMERA_AND_COME_BACK_TO_LARAS_CHASE_CAMERA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_RESET_CUTSCENE_CAMERA_AND_COME_BACK_TO_LARAS_CHASE_CAMERA unimplemented!");
			break;
		}
		case CUTSCENE_MOVE_UP_CUTSCENE_CAMERA_OF_X_CLICKS_IN_E_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_MOVE_UP_CUTSCENE_CAMERA_OF_X_CLICKS_IN_E_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_MOVE_DOWN_CUTSCENE_CAMERA_OF_X_CLICKS_IN_E_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_MOVE_DOWN_CUTSCENE_CAMERA_OF_X_CLICKS_IN_E_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_ZOOM_IN_CUTSCENE_CAMERA_COVERING_X_DISTANCE_IN_E_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_ZOOM_IN_CUTSCENE_CAMERA_COVERING_X_DISTANCE_IN_E_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_ZOOM_OUT_CUTSCENE_CAMERA_COVERING_X_DISTANCE_IN_E_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_ZOOM_OUT_CUTSCENE_CAMERA_COVERING_X_DISTANCE_IN_E_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_ROTATE_AT_RIGHT_CUTSCENE_CAMERA_AROUND_TARGET_BY_X_DEGREES_IN_E_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_ROTATE_AT_RIGHT_CUTSCENE_CAMERA_AROUND_TARGET_BY_X_DEGREES_IN_E_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_ROTATE_AT_LEFT_CUTSCENE_CAMERA_AROUND_TARGET_BY_X_DEGREES_IN_E_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_ROTATE_AT_LEFT_CUTSCENE_CAMERA_AROUND_TARGET_BY_X_DEGREES_IN_E_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_FREEZE_CUTSCENE_CAMERA_FOR_X_DEMO_FRAMES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_FREEZE_CUTSCENE_CAMERA_FOR_X_DEMO_FRAMES unimplemented!");
			break;
		}
		case CUTSCENE_REMOVE_FREEZE_FROM_CUTSCENE_CAMERA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_REMOVE_FREEZE_FROM_CUTSCENE_CAMERA unimplemented!");
			break;
		}
		case CUTSCENE_SWAP_ANIMATIONS_SET_IN_DATA_OF_X_PARAMETER_COMMAND_FOR_E_ACTOR: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_SWAP_ANIMATIONS_SET_IN_DATA_OF_X_PARAMETER_COMMAND_FOR_E_ACTOR unimplemented!");
			break;
		}
		case CUTSCENE_FREE_CUTSCENE_X_RESOURCES: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_FREE_CUTSCENE_X_RESOURCES unimplemented!");
			break;
		}
		case CUTSCENE_ACTOR_WITH_X_ROLE_WILL_LOOK_AT_LARA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_ACTOR_WITH_X_ROLE_WILL_LOOK_AT_LARA unimplemented!");
			break;
		}
		case CUTSCENE_LARA_WILL_LOOK_ACTOR_WITH_X_ROLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_LARA_WILL_LOOK_ACTOR_WITH_X_ROLE unimplemented!");
			break;
		}
		case CUTSCENE_SWAP_VON_CROY_X_MESH_WITH_SAME_MESH_OF_E_SLOT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_SWAP_VON_CROY_X_MESH_WITH_SAME_MESH_OF_E_SLOT unimplemented!");
			break;
		}
		case CUTSCENE_SWAP_VON_CROY_X_DATA_WITH_E_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_SWAP_VON_CROY_X_DATA_WITH_E_VALUE unimplemented!");
			break;
		}
		case CUTSCENE_SET_X_COLOR_E_POSITION_OF_LARAS_TEXTS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_SET_X_COLOR_E_POSITION_OF_LARAS_TEXTS unimplemented!");
			break;
		}
		case CUTSCENE_SET_X_COLOR_AND_E_POSITION_FOR_LEADING_ACTORS_TEXTS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_SET_X_COLOR_AND_E_POSITION_FOR_LEADING_ACTORS_TEXTS unimplemented!");
			break;
		}
		case CUTSCENE_SET_X_COLOR_AND_E_POSITION_FOR_EXTRA_ACTORS_TEXTS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CUTSCENE_SET_X_COLOR_AND_E_POSITION_FOR_EXTRA_ACTORS_TEXTS unimplemented!");
			break;
		}
		case SPRITE_STOP_THE_ANIMATED_SPRITES_WITH_DATA_IN_X_PARAMETER_AT_E_FRAME: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SPRITE_STOP_THE_ANIMATED_SPRITES_WITH_DATA_IN_X_PARAMETER_AT_E_FRAME unimplemented!");
			break;
		}
		case SPRITE_RESUME_ANIMATION_OF_SPRITES_WITH_DATA_IN_X_PARAMETER: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "SPRITE_RESUME_ANIMATION_OF_SPRITES_WITH_DATA_IN_X_PARAMETER unimplemented!");
			break;
		}
		case TRIGGER_SECRET: {
			trigger_secret(timer, extra_timer);
			break;
		}
		case FMV_PLAY_X_FMV: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "FMV_PLAY_X_FMV unimplemented!");
			break;
		}
		case CAMERA_GET_OR_REMOVE_X_INFINITE_DURATE_FOR_CURRENT_CAMERA: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CAMERA_GET_OR_REMOVE_X_INFINITE_DURATE_FOR_CURRENT_CAMERA unimplemented!");
			break;
		}
		case SET_LARA_HOLSTER_TYPE: {
			set_lara_holsters(timer, extra_timer);
			break;
		}
		case LARA_SET_CURRENT_SELECTED_WEAPON_FOR_X: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_SET_CURRENT_SELECTED_WEAPON_FOR_X unimplemented!");
			break;
		}
		case IMAGES_PERFORM_X_INPUT_BOX_PARAMETERS_AND_WAIT_FOR_PLAYER_INPUT: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "IMAGES_PERFORM_X_INPUT_BOX_PARAMETERS_AND_WAIT_FOR_PLAYER_INPUT unimplemented!");
			break;
		}
		case VARIABLES_CLEAR_X_TEXT_VARIABLE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "VARIABLES_CLEAR_X_TEXT_VARIABLE unimplemented!");
			break;
		}
		case LARA_SET_X_OPACITY_LEVEL_OFLARA_FOR_E_SECONDS: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_SET_X_OPACITY_LEVEL_OFLARA_FOR_E_SECONDS unimplemented!");
			break;
		}
		default: {
			if (flip_number < 47) {
				{
					repeat_type = 0;

					int8_t original_trigger_timer = TriggerTimer;
					TriggerTimer = timer;
					effect_routines[flip_number](lara_item);
					TriggerTimer = original_trigger_timer;

					if (flipeffect != -1) {
						repeat_type = 1;
					}

					if (flags & SCANF_BUTTON_ONE_SHOT) {
						repeat_type = 2;
					}
				}
			} else {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented NGFlipEffect %u!", flip_number);
			}
		}
	}

	if ((flags & SCANF_BUTTON_ONE_SHOT)) {
		repeat_type = 2;
	}

	return repeat_type;
}

int32_t NGExecuteFlipEffect(uint16_t plugin_id, uint16_t flip_number, int16_t full_timer, uint32_t flags) {
	int32_t repeat_type = 0;

	if (plugin_id > 0) {
		if (plugin_id == 0xffff) {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid Plugin ID for Flipeffect %u", flip_number);
			return 0;
		}

		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TODO: Plugins are unimplemented.");

		return repeat_type;
	}

	// TODO: Callback First

	// TODO: Callback Replace

	repeat_type = NGPerformTRNGFlipEffect(flip_number, full_timer, flags);

	// TODO: Callback After

	return repeat_type;
}

void NGExecuteFlipEffects() {
	if (scanned_flipeffect_count == 0) {
		flipeffect = -1;
		for (uint32_t i = 0; i < old_flipeffect_count; i++) {
			if (old_flipeffects[i].flags & SCANF_TEMP_ONE_SHOT)
				old_flipeffects[i].offset_floor_data = 0;
		}

		return;
	}

	for (uint32_t i = 0; i < scanned_flipeffect_count; i++) {
		uint32_t offset_floor_data = scanned_flipeffects[i].offset_floor_data;

		scanned_flipeffects[i].flags &= ~SCANF_YET_TO_PERFORM;

		bool test_run = true;
		for (uint32_t j = 0; j < old_flipeffect_count; j++) {
			if (old_flipeffects[j].offset_floor_data == offset_floor_data) {
				test_run = false;
			} else {
				if (!NGUsingLegacyNGTriggerBehaviour()) {
					if (old_flipeffects[j].offset_floor_data != 0 &&
					        old_flipeffects[j].flags & SCANF_TEMP_ONE_SHOT &&
					        (scanned_flipeffects[i].flags & SCANF_HEAVY) == 0) {
						if ((old_flipeffects[j].offset_floor_data & 0xFF000000) != (offset_floor_data & 0xFF000000)) {
							old_flipeffects[j].offset_floor_data = 0;
						}
					}
				}
			}
		}

		if (test_run) {
			NGScannedFlipEffect *current_flipeffect = &scanned_flipeffects[i];
			if (current_flipeffect->flags & SCANF_HEAVY) {
				NGStoreItemIndexCurrent(current_flipeffect->indice);
			} else {
				NGStoreItemIndexCurrent(lara.item_number);
			}

			int32_t repeat_type = NGExecuteFlipEffect(current_flipeffect->plugin_id, current_flipeffect->number, current_flipeffect->timer, current_flipeffect->flags);
			if (repeat_type > 0 && !NGGetIsInsideDummyTrigger()) {
				uint32_t last_flipeffect = 0;
				for (last_flipeffect = 0; last_flipeffect < old_flipeffect_count; last_flipeffect++) {
					if (old_flipeffects[last_flipeffect].offset_floor_data == 0) {
						break;
					}
				}

				if (last_flipeffect == old_flipeffect_count) {
					old_flipeffect_count++;
				}

				old_flipeffects[last_flipeffect].offset_floor_data = current_flipeffect->offset_floor_data;
				old_flipeffects[last_flipeffect].flags = 0;
				if (repeat_type == 1) {
					old_flipeffects[last_flipeffect].flags |= SCANF_TEMP_ONE_SHOT;
				}
			}
		}
	}
}

void NGCaptureFlipEffect(uint16_t flip_number, uint16_t timer, uint32_t flip_offset) {

	bool is_testing_heavy = NGGetIsHeavyTesting();

	uint32_t offset_now = flip_offset;
	uint32_t offset_sector = 0;

	if (!NGUsingLegacyNGTriggerBehaviour()) {
		if (is_testing_heavy) {
			offset_sector = (trigger_index - floor_data) * sizeof(uint16_t); // May not be correct
		} else {
			offset_sector = uint32_t((NGGetLastFloorAddress()) - floor_data) * sizeof(uint16_t);
		}

		offset_now |= (offset_sector << 24);
	}

	for (uint32_t i = 0; i < scanned_flipeffect_count; i++) {
		if (scanned_flipeffects[i].offset_floor_data == offset_now) {
			return;
		}
	}

	size_t flipeffect_count = scanned_flipeffect_count;

	scanned_flipeffects[flipeffect_count].flags = SCANF_YET_TO_PERFORM | SCANF_FLOOR_DATA;

	if (is_testing_heavy) {
		scanned_flipeffects[flipeffect_count].flags |= SCANF_HEAVY;
		scanned_flipeffects[flipeffect_count].indice = NGGetItemIndexEnabledTrigger();
	} else {
		scanned_flipeffects[flipeffect_count].indice = lara.item_number;
	}

	if (NGGetFloorTriggerNow()[1] & IFL_INVISIBLE) {
		scanned_flipeffects[flipeffect_count].flags |= SCANF_BUTTON_ONE_SHOT;
	}

	scanned_flipeffects[flipeffect_count].number = flip_number;
	scanned_flipeffects[flipeffect_count].offset_floor_data = offset_now;
	scanned_flipeffects[flipeffect_count].timer = timer;

	if (!NGUsingLegacyNGTriggerBehaviour()) {
		offset_now = (flip_offset - (*floor_data));
	}

	uint16_t plugin_id = NGGetPluginIDForFloorData(offset_now >> 1, false);
	scanned_flipeffects[flipeffect_count].plugin_id = plugin_id;

	if (flip_number == PERFORM_TRIGGERGROUP_FROM_SCRIPT_IN_SPECIFIC_WAY && plugin_id == 0) {
		int16_t current_id = timer & 0xff;
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Captured flipeffects does not implement special behaviour for flipeffect 118");
	}

	scanned_flipeffect_count++;
}

void NGResetScanFlipEffects() {
	uint32_t j = 0;
	for (uint32_t i = 0; i < scanned_flipeffect_count; i++) {
		if (scanned_flipeffects[i].flags & SCANF_YET_TO_PERFORM) {
			scanned_flipeffects[j++] = scanned_flipeffects[i];
		}
	}

	scanned_flipeffect_count = j;
}