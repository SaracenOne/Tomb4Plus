#include "../../tomb4/pch.h"

#include "trng.h"
#include "trng_action.h"
#include "trng_condition.h"
#include "trng_flipeffect.h"
#include "trng_savegame.h"
#include "trng_progressive_action.h"
#include "trng_extra_state.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"

#include "../../specific/3dmath.h"
#include "../gameflow.h"
#include "../../specific/function_table.h"
#include "trng_globaltrigger.h"
#include "../../specific/audio.h"
#include "../control.h"

#define MAX_NG_SAVEGAME_BUFFER_SIZE 0x8000

uint32_t ng_savegame_buffer_size = 0;
char ng_savegame_buffer[MAX_NG_SAVEGAME_BUFFER_SIZE];

bool NGIsNGSavegame() {
	return ng_savegame_buffer_size > 0;
}

uint32_t NGWriteOldFlipeffects(uint32_t position) {
	uint32_t old_flipeffect_size = 0;
	old_flipeffect_size += sizeof(uint16_t);
	old_flipeffect_size += sizeof(uint16_t);
	old_flipeffect_size += sizeof(uint16_t);
	for (int32_t i = 0; i < old_flipeffect_count; i++) {
		if (i < NG_MAX_OLD_FLIPEFFECTS) {
			old_flipeffect_size += sizeof(uint16_t);
			old_flipeffect_size += sizeof(uint32_t);
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Old flipeffect overflow!");
		}
	}

	old_flipeffect_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, old_flipeffect_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x8003);

	NG_WRITE_16(ng_savegame_buffer, position, old_flipeffect_count);
	for (int32_t i = 0; i < old_flipeffect_count; i++) {
		if (i < NG_MAX_OLD_FLIPEFFECTS) {
			NG_WRITE_16(ng_savegame_buffer, position, old_flipeffects[i].flags);
			NG_WRITE_32(ng_savegame_buffer, position, old_flipeffects[i].offset_floor_data);
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Old flipeffect overflow!");
		}
	}

	return position;
}

uint32_t NGWriteOldFMV(uint32_t position) {
	uint32_t old_fmv_size = 0;
	old_fmv_size += sizeof(uint16_t);
	old_fmv_size += sizeof(uint16_t);
	for (int32_t i = 0; i < old_fmv_size; i++) {
		if (i < 128) {
			old_fmv_size += sizeof(uint8_t);
		}
		else {
			NGLog(NG_LOG_TYPE_ERROR, "Old fmv overflow!");
		}
	}

	old_fmv_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, old_fmv_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x8004);

	for (int32_t i = 0; i < 128; i++) {
		NG_WRITE_8(ng_savegame_buffer, position, 0);
	}

	return position;
}

uint32_t NGWriteCoordinates(uint32_t position) {
	if (moved_item_indicies_count > 0) {
		uint32_t coordinates_size = 0;
		coordinates_size += sizeof(uint16_t);
		coordinates_size += sizeof(uint16_t);
		coordinates_size += sizeof(uint16_t);
		for (int32_t i = 0; i < moved_item_indicies_count; i++) {
			coordinates_size += sizeof(uint16_t);
		}

		for (int32_t i = 0; i < moved_item_indicies_count; i++) {
			coordinates_size += sizeof(uint16_t);
			coordinates_size += sizeof(uint16_t);
			coordinates_size += sizeof(uint16_t);

			coordinates_size += sizeof(uint32_t);
			coordinates_size += sizeof(uint32_t);
			coordinates_size += sizeof(uint32_t);
			coordinates_size += sizeof(uint16_t);
		}

		coordinates_size /= sizeof(uint16_t);

		NG_WRITE_16(ng_savegame_buffer, position, coordinates_size);
		NG_WRITE_16(ng_savegame_buffer, position, 0x8006);

		NG_WRITE_16(ng_savegame_buffer, position, moved_item_indicies_count);
		for (int32_t i = 0; i < moved_item_indicies_count; i++) {
			NG_WRITE_16(ng_savegame_buffer, position, moved_item_indicies[i]);
		}

		for (int32_t i = 0; i < moved_item_indicies_count; i++) {
			ITEM_INFO* item = T4PlusGetItemInfoForID(moved_item_indicies[i]);
			if (item) {
				NG_WRITE_16(ng_savegame_buffer, position, item->pos.x_rot);
				NG_WRITE_16(ng_savegame_buffer, position, item->pos.y_rot);
				if (item->status & 0x02) {
					NG_WRITE_16(ng_savegame_buffer, position, 1);
				}
				else {
					NG_WRITE_16(ng_savegame_buffer, position, 0);
				}
				NG_WRITE_32(ng_savegame_buffer, position, item->pos.x_pos);
				NG_WRITE_32(ng_savegame_buffer, position, item->pos.y_pos);
				NG_WRITE_32(ng_savegame_buffer, position, item->pos.z_pos);
				NG_WRITE_16(ng_savegame_buffer, position, item->room_number);
			}
			else {
				NG_WRITE_16(ng_savegame_buffer, position, 0);
				NG_WRITE_16(ng_savegame_buffer, position, 0);
				NG_WRITE_16(ng_savegame_buffer, position, 0);
				NG_WRITE_32(ng_savegame_buffer, position, 0);
				NG_WRITE_32(ng_savegame_buffer, position, 0);
				NG_WRITE_32(ng_savegame_buffer, position, 0);
				NG_WRITE_16(ng_savegame_buffer, position, 0);
			}
		}
	}
	return position;
}

uint32_t NGWriteProgressiveActions(uint32_t position) {
	uint32_t old_progressive_actions_size = 0;
	old_progressive_actions_size += sizeof(uint16_t);
	old_progressive_actions_size += sizeof(uint16_t);
	old_progressive_actions_size += sizeof(uint16_t);
	for (int32_t i = 0; i < progressive_action_count; i++) {
		if (i < NG_MAX_PROGRESSIVE_ACTIONS) {
			old_progressive_actions_size += sizeof(uint16_t);
			old_progressive_actions_size += sizeof(uint16_t);
			old_progressive_actions_size += sizeof(uint16_t);
			old_progressive_actions_size += sizeof(uint16_t);
			for (int32_t j = 0; j < NG_PROGRESSIVE_ACTION_ARGUMENT_2_COUNT; j++) {
				old_progressive_actions_size += sizeof(uint32_t);
			}
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Progressive action overflow!");
		}
	}

	old_progressive_actions_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, old_progressive_actions_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x8007);

	NG_WRITE_16(ng_savegame_buffer, position, progressive_action_count);
	for (int32_t i = 0; i < progressive_action_count; i++) {
		if (i < NG_MAX_PROGRESSIVE_ACTIONS) {
			NG_WRITE_16(ng_savegame_buffer, position, progressive_actions[i].type);
			NG_WRITE_16(ng_savegame_buffer, position, progressive_actions[i].item_index);
			NG_WRITE_16(ng_savegame_buffer, position, progressive_actions[i].duration);
			NG_WRITE_16(ng_savegame_buffer, position, progressive_actions[i].argument1_u16);
			for (int32_t j = 0; j < NG_PROGRESSIVE_ACTION_ARGUMENT_2_COUNT; j++) {
				NG_WRITE_32(ng_savegame_buffer, position, progressive_actions[i].argument2_u32[j]);
			}
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Progressive action overflow!");
		}
	}

	return position;
}

uint32_t NGWriteOldActions(uint32_t position) {
	uint32_t old_action_size = 0;
	old_action_size += sizeof(uint16_t);
	old_action_size += sizeof(uint16_t);
	old_action_size += sizeof(uint16_t);
	for (int32_t i = 0; i < old_action_count; i++) {
		if (i < NG_MAX_OLD_ACTIONS) {
			old_action_size += sizeof(uint16_t);
			old_action_size += sizeof(uint32_t);
		}
		else {
			NGLog(NG_LOG_TYPE_ERROR, "Old action overflow!");
		}
	}

	old_action_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, old_action_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x8008);

	NG_WRITE_16(ng_savegame_buffer, position, old_action_count);
	for (int32_t i = 0; i < old_action_count; i++) {
		if (i < NG_MAX_OLD_ACTIONS) {
			NG_WRITE_16(ng_savegame_buffer, position, old_actions[i].flags);
			NG_WRITE_32(ng_savegame_buffer, position, old_actions[i].offset_floor_data);
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Old action overflow!");
		}
	}

	return position;
}

uint32_t NGWriteOldConditions(uint32_t position) {
	uint32_t old_condition_size = 0;
	old_condition_size += sizeof(uint16_t);
	old_condition_size += sizeof(uint16_t);
	old_condition_size += sizeof(uint16_t);
	for (int32_t i = 0; i < old_condition_count; i++) {
		if (i < NG_MAX_OLD_CONDITIONS) {
			old_condition_size += sizeof(uint16_t);
			old_condition_size += sizeof(uint32_t);
		}
		else {
			NGLog(NG_LOG_TYPE_ERROR, "Old condition overflow!");
		}
	}

	old_condition_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, old_condition_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x800E);

	NG_WRITE_16(ng_savegame_buffer, position, old_condition_count);
	for (int32_t i = 0; i < old_condition_count; i++) {
		if (i < NG_MAX_OLD_CONDITIONS) {
			NG_WRITE_16(ng_savegame_buffer, position, old_conditions[i].flags);
			NG_WRITE_32(ng_savegame_buffer, position, old_conditions[i].offset_floor_data);
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Old condition overflow!");
		}
	}

	return position;
}

uint32_t NGWriteVariableData(uint32_t position) {
	uint32_t variable_data_size = 248;

	int start_pos = position;

	NG_WRITE_16(ng_savegame_buffer, position, variable_data_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x800F);

	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Cold intensity
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Cold flags
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Damage intensity
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Damage flags
	NG_WRITE_32(ng_savegame_buffer, position, ng_input_to_disable);
	NG_WRITE_32(ng_savegame_buffer, position, 0); // TODO: Status NG
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Disable feature flags
	NG_WRITE_32(ng_savegame_buffer, position, 0); // TODO: Counter game
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Level now flags

	int32_t layer1_color = (gfLayer1Col.a << 24) & 0xff000000 | (gfLayer1Col.r << 16) & 0x00ff0000 | (gfLayer1Col.g << 8) & 0x0000ff00 | (gfLayer1Col.b) & 0x000000ff;
	NG_WRITE_32(ng_savegame_buffer, position, layer1_color);

	int32_t layer2_color = (gfLayer2Col.a << 24) & 0xff000000 | (gfLayer2Col.r << 16) & 0x00ff0000 | (gfLayer2Col.g << 8) & 0x0000ff00 | (gfLayer2Col.b) & 0x000000ff;
	NG_WRITE_32(ng_savegame_buffer, position, layer2_color);

	NG_WRITE_8(ng_savegame_buffer, position, gfLayer1Vel);
	NG_WRITE_8(ng_savegame_buffer, position, gfLayer2Vel);

	if (S_CDGetChannelIsActive(1) && S_CDGetChannelIsLooping(1)) {
		NG_WRITE_16(ng_savegame_buffer, position, S_CDGetTrackID(1));
	} else {
		NG_WRITE_16(ng_savegame_buffer, position, -1);
	}

	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Level NG Flags

	if (S_CDGetChannelIsActive(1) && !S_CDGetChannelIsLooping(1)) {
		NG_WRITE_16(ng_savegame_buffer, position, S_CDGetTrackID(1));
	} else {
		NG_WRITE_16(ng_savegame_buffer, position, -1);
	}

	uint64_t channel_1_position = S_CDGetChannelPosition(1);

	if (S_CDGetChannelIsActive(1)) {
		NG_WRITE_32(ng_savegame_buffer, position, uint32_t((channel_1_position * sizeof(int32_t)) & 0xffffffff));
	} else {
		NG_WRITE_32(ng_savegame_buffer, position, -1);
	}


	if (S_CDGetChannelIsActive(0) && S_CDGetChannelIsLooping(0)) {
		NG_WRITE_16(ng_savegame_buffer, position, S_CDGetTrackID(0));
	} else {
		NG_WRITE_16(ng_savegame_buffer, position, -1);
	}

	if (S_CDGetChannelIsActive(0) && !S_CDGetChannelIsLooping(0)) {
		NG_WRITE_16(ng_savegame_buffer, position, S_CDGetTrackID(0));
	} else {
		NG_WRITE_16(ng_savegame_buffer, position, -1);
	}

	uint64_t channel_0_position = S_CDGetChannelPosition(0);
	if (S_CDGetChannelIsActive(0)) {
		NG_WRITE_32(ng_savegame_buffer, position, uint32_t((channel_0_position * sizeof(int32_t)) & 0xffffffff));
	} else {
		NG_WRITE_32(ng_savegame_buffer, position, -1);
	}

	NG_WRITE_FLOAT(ng_savegame_buffer, position, LevelFogStart);

	NG_WRITE_32(ng_savegame_buffer, position, 0); // Unused
	NG_WRITE_32(ng_savegame_buffer, position, 0); // Unused

	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Current pushable index

	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Parallel bar rounds.
	NG_WRITE_32(ng_savegame_buffer, position, 0); // TODO: Parallel bar frames.


	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Test Pop up image.
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Pop up counter.
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Pop up image index.

	NG_WRITE_8(ng_savegame_buffer, position, 1); // TODO: Volumetric FX
	NG_WRITE_8(ng_savegame_buffer, position, 1); // TODO: Hardware Fog
	NG_WRITE_16(ng_savegame_buffer, position, 0); // Unused

	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: End Fog Sectors

	int32_t fog_color = (gfDistanceFog.a << 24) & 0xff000000 | (gfDistanceFog.r << 16) & 0x00ff0000 | (gfDistanceFog.g << 8) & 0x0000ff00 | (gfDistanceFog.b) & 0x000000ff;
	NG_WRITE_32(ng_savegame_buffer, position, fog_color);

	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Start Fog Sectors
	NG_WRITE_16(ng_savegame_buffer, position, 0); // TODO: Max Fog Bulb Distance

	for (int32_t i = 0; i < 100; i++) {
		NG_WRITE_32(ng_savegame_buffer, position, 0); // TODO: Unused
	}

	int end_pos = position;

	int total_pos = end_pos - start_pos;

	return position;
}

uint32_t NGWriteGlobalVariables(uint32_t position) {
	uint32_t global_variable_size_size = 0;
	global_variable_size_size += sizeof(uint16_t);
	global_variable_size_size += sizeof(uint16_t);

	global_variable_size_size += sizeof(uint32_t);
	global_variable_size_size += sizeof(uint32_t);
	global_variable_size_size += sizeof(uint32_t);
	global_variable_size_size += sizeof(uint32_t);

	global_variable_size_size += sizeof(ng_string1);
	global_variable_size_size += sizeof(ng_string2);
	global_variable_size_size += sizeof(ng_string3);
	global_variable_size_size += sizeof(ng_string4);

	for (int32_t i = 0; i < STORE_VARIABLE_COUNT; i++) {
		global_variable_size_size += sizeof(uint32_t);
	}

	global_variable_size_size += sizeof(ng_last_text_input);
	global_variable_size_size += sizeof(uint32_t);
	global_variable_size_size += sizeof(uint32_t);
	global_variable_size_size += sizeof(ng_text_big);


	for (int32_t i = 0; i < 20; i++) {
		global_variable_size_size += sizeof(uint32_t);
	}

	global_variable_size_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, global_variable_size_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x8038);

	NG_WRITE_32(ng_savegame_buffer, position, ng_global_alfa);
	NG_WRITE_32(ng_savegame_buffer, position, ng_global_beta);
	NG_WRITE_32(ng_savegame_buffer, position, ng_global_delta);
	NG_WRITE_32(ng_savegame_buffer, position, ng_global_timer);

	NG_WRITE_FIXED_STRING(ng_savegame_buffer, &ng_string1, sizeof(ng_string1), position);
	NG_WRITE_FIXED_STRING(ng_savegame_buffer, &ng_string2, sizeof(ng_string2), position);
	NG_WRITE_FIXED_STRING(ng_savegame_buffer, &ng_string3, sizeof(ng_string3), position);
	NG_WRITE_FIXED_STRING(ng_savegame_buffer, &ng_string4, sizeof(ng_string4), position);

	for (int32_t i = 0; i < STORE_VARIABLE_COUNT; i++) {
		NG_READ_32(ng_savegame_buffer, position, ng_store_variables[i]);
	}

	NG_WRITE_FIXED_STRING(ng_savegame_buffer, &ng_last_text_input, sizeof(ng_last_text_input), position);

	NG_WRITE_32(ng_savegame_buffer, position, ng_last_input_number);
	NG_WRITE_32(ng_savegame_buffer, position, ng_current_value);

	NG_WRITE_FIXED_STRING(ng_savegame_buffer, &ng_text_big, sizeof(ng_text_big), position);

	for (int32_t i = 0; i < 20; i++) {
		NG_WRITE_32(ng_savegame_buffer, position, 0);
	}

	return position;
}

uint32_t NGWriteGlobalTriggerState(uint32_t position) {
	uint32_t global_variable_size = 0;
	global_variable_size += sizeof(uint16_t);
	global_variable_size += sizeof(uint16_t);

	global_variable_size += sizeof(uint16_t);

	for (int32_t i = 0; i < MAX_NG_GLOBAL_TRIGGERS; i++) {
		global_variable_size += sizeof(uint16_t);
	}

	global_variable_size /= sizeof(uint16_t);

	NG_WRITE_16(ng_savegame_buffer, position, global_variable_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x801A);

	NG_WRITE_16(ng_savegame_buffer, position, MAX_NG_GLOBAL_TRIGGERS);
	for (int32_t i = 0; i < MAX_NG_GLOBAL_TRIGGERS; i++) {
		uint16_t flags = 0;
		if (ng_global_trigger_states[i].is_disabled) {
			flags |= FGT_DISABLED;
		}
		if (ng_global_trigger_states[i].is_halted) {
			flags |= FGT_PAUSE_ONE_SHOT;
		}

		NG_WRITE_16(ng_savegame_buffer, position, flags);
	}

	return position;
}

uint32_t NGWriteRoomFlags(uint32_t position) {
	uint32_t room_flags_size = 0;
	room_flags_size += sizeof(uint16_t);
	room_flags_size += sizeof(uint16_t);

	room_flags_size += sizeof(uint16_t);
	for (int32_t i = 0; i < ng_total_flip_rooms; i++) {
		room_flags_size += sizeof(uint16_t);
	}

	room_flags_size /= sizeof(uint16_t);;

	NG_WRITE_16(ng_savegame_buffer, position, room_flags_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x801C);

	NG_WRITE_16(ng_savegame_buffer, position, ng_total_flip_rooms);
	for (int32_t i = 0; i < ng_total_flip_rooms; i++) {
		int32_t room_index = NGFindIndexForRoom(i);
		if (room_index != -1 && room_index < number_rooms) {
			int16_t flags = room[room_index].flags & 0x1C35;
			NG_WRITE_16(ng_savegame_buffer, position, flags);
		} else {
			NG_WRITE_16(ng_savegame_buffer, position, 0xFFFF);
		}
	}

	return position;
}

uint32_t NGWriteOCBItems(uint32_t position) {
	uint32_t ocb_item_size = 0;
	ocb_item_size += sizeof(uint16_t);
	ocb_item_size += sizeof(uint16_t);

	ocb_item_size += sizeof(uint16_t);
	for (int32_t i = 0; i < level_items; i++) {
		ocb_item_size += sizeof(uint16_t);
	}

	ocb_item_size /= 2;

	NG_WRITE_16(ng_savegame_buffer, position, ocb_item_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x804D);

	NG_WRITE_16(ng_savegame_buffer, position, level_items);
	for (int32_t i = 0; i < level_items; i++) {
		ITEM_INFO* item = T4PlusGetItemInfoForID(i);
		if (item) {
			NG_WRITE_16(ng_savegame_buffer, position, item->trigger_flags);
		} else {
			NG_WRITE_16(ng_savegame_buffer, position, 0);
		}
	}

	return position;
}

uint32_t NGWriteLocalVariables(uint32_t position) {
	uint32_t local_variables_size = 0;
	local_variables_size += sizeof(uint16_t);
	local_variables_size += sizeof(uint16_t);

	local_variables_size += sizeof(uint32_t);
	local_variables_size += sizeof(uint32_t);
	local_variables_size += sizeof(uint32_t);
	local_variables_size += sizeof(uint32_t);

	local_variables_size /= 2;

	NG_WRITE_16(ng_savegame_buffer, position, local_variables_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x8039);

	NG_WRITE_32(ng_savegame_buffer, position, ng_local_alfa);
	NG_WRITE_32(ng_savegame_buffer, position, ng_local_beta);
	NG_WRITE_32(ng_savegame_buffer, position, ng_local_delta);
	NG_WRITE_32(ng_savegame_buffer, position, ng_local_timer);

	return position;
}

uint32_t NGWriteFrozenItems(uint32_t position) {
	uint32_t frozen_items_size = 0;
	uint16_t frozen_item_count = 0;
	for (int i = 0; i < level_items; i++) {
		if (NGIsItemFrozen(i)) {
			frozen_item_count++;
		}
	}

	frozen_items_size += sizeof(uint16_t);
	frozen_items_size += sizeof(uint16_t);
	frozen_items_size += sizeof(uint16_t);

	for (int i = 0; i < frozen_item_count; i++) {
		frozen_items_size += sizeof(uint16_t);
		frozen_items_size += sizeof(uint16_t);
	}

	frozen_items_size /= 2;

	NG_WRITE_16(ng_savegame_buffer, position, frozen_items_size);
	NG_WRITE_16(ng_savegame_buffer, position, 0x803A);
	NG_WRITE_16(ng_savegame_buffer, position, frozen_item_count);

	for (int i = 0; i < level_items; i++) {
		if (NGIsItemFrozen(i)) {
			NG_WRITE_16(ng_savegame_buffer, position, i);
			NG_WRITE_16(ng_savegame_buffer, position, NGGetItemFrozenTimer(i));
		}
	}

	return position;
}

void NGWriteNGSavegameInfo() {
	memset(ng_savegame_buffer, 0x00, MAX_NG_SAVEGAME_BUFFER_SIZE);
	ng_savegame_buffer_size = 0;

	NG_WRITE_16(ng_savegame_buffer, ng_savegame_buffer_size, NGLE_START_SIGNATURE);

	ng_savegame_buffer_size = NGWriteOldFlipeffects(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteOldFMV(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteCoordinates(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteProgressiveActions(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteOldActions(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteOldConditions(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteVariableData(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteGlobalVariables(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteGlobalTriggerState(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteRoomFlags(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteOCBItems(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteLocalVariables(ng_savegame_buffer_size);
	ng_savegame_buffer_size = NGWriteFrozenItems(ng_savegame_buffer_size);

	// End
	NG_WRITE_32(ng_savegame_buffer, ng_savegame_buffer_size, 0);

	NG_WRITE_32(ng_savegame_buffer, ng_savegame_buffer_size, NGLE_END_SIGNATURE);
	NG_WRITE_32(ng_savegame_buffer, ng_savegame_buffer_size, ng_savegame_buffer_size + sizeof(uint32_t));
}

void NGWriteNGSavegameBuffer(FILE* file) {
	if (ng_savegame_buffer_size > 0) {
		fwrite(ng_savegame_buffer, ng_savegame_buffer_size, 1, file);
	}
}

void NGReadNGSavegameInfo() {
	size_t offset = 0;

	if (NGIsNGSavegame()) {
		uint16_t header_ident = NG_READ_16(ng_savegame_buffer, offset);
		if (header_ident != NGLE_START_SIGNATURE) { // NGLE
			return;
		}

		uint16_t total_causale = 0;

		while (1) {
			size_t start_offset = offset;

			uint16_t size_info = NG_READ_16(ng_savegame_buffer, offset);
			uint32_t block_size = 0;
			uint32_t extra_words = 0;
			if (size_info & 0x8000) {
				block_size = ((size_info & 0x7fff) * 65536) + NG_READ_16(ng_savegame_buffer, offset);
			} else {
				block_size = size_info;
			}

			if (offset + (block_size - sizeof(uint16_t)) >= ng_savegame_buffer_size) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGSavegameBuffer: overflow!");
				return;
			}

			uint16_t block_type = NG_READ_16(ng_savegame_buffer, offset);

			if (block_type == 0) {
				break;
			}

			switch (block_type) {
				case 0x8003: { // OLD_EFFECTS
					old_flipeffect_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < old_flipeffect_count; i++) {
						if (i < NG_MAX_OLD_FLIPEFFECTS) {
							old_flipeffects[i].flags = NG_READ_16(ng_savegame_buffer, offset);
							old_flipeffects[i].offset_floor_data = NG_READ_32(ng_savegame_buffer, offset);
						} else {
							NGLog(NG_LOG_TYPE_ERROR, "Old flipeffect overflow!");
						}
					}
					break;
				}
				case 0x8004: { // OLD_FMV
					for (int32_t i = 0; i < 128; i++) {
						uint8_t performed_fmv = NG_READ_8(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x8006: { // COORDINATES
					moved_item_indicies_count = NG_READ_16(ng_savegame_buffer, offset);
					if (moved_item_indicies_count >= NG_MAX_SAVED_COORDINATES) {
						NGLog(NG_LOG_TYPE_ERROR, "Saved coordinate overflow.");
					}

					for (int32_t i = 0; i < moved_item_indicies_count; i++) {
						moved_item_indicies[i] = NG_READ_16(ng_savegame_buffer, offset);
					}

					for (int32_t i = 0; i < moved_item_indicies_count; i++) {
						uint16_t x_rot = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t y_rot = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t invisible_flag = NG_READ_16(ng_savegame_buffer, offset);

						int32_t x_pos = NG_READ_32(ng_savegame_buffer, offset);
						int32_t y_pos = NG_READ_32(ng_savegame_buffer, offset);
						int32_t z_pos = NG_READ_32(ng_savegame_buffer, offset);
						uint16_t room_number = NG_READ_16(ng_savegame_buffer, offset);

						ITEM_INFO *item = T4PlusGetItemInfoForID(moved_item_indicies[i]);
						if (item) {
							item->pos.x_pos = x_pos;
							item->pos.y_pos = y_pos;
							item->pos.z_pos = z_pos;

							item->pos.x_rot = x_rot;
							item->pos.y_rot = y_rot;

							if (invisible_flag) {
								item->status |= 0x03;
							} else {
								item->status &= ~0x02;
							}

							UpdateItemRoom(moved_item_indicies[i], -256);
						}
					}
					break;
				}
				case 0x8007: { // PROGR_ACTIONS
					uint32_t progressive_action_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < progressive_action_count; i++) {
						if (i < NG_MAX_PROGRESSIVE_ACTIONS) {
							uint16_t progressive_action_type = NG_READ_16(ng_savegame_buffer, offset);
							progressive_actions[i].type = static_cast<NGProgressiveActionType>(progressive_action_type);
							progressive_actions[i].item_index = NG_READ_16(ng_savegame_buffer, offset);
							progressive_actions[i].duration = NG_READ_16(ng_savegame_buffer, offset);
							
							progressive_actions[i].argument1_u16 = NG_READ_16(ng_savegame_buffer, offset);
							for (int32_t j = 0; j < NG_PROGRESSIVE_ACTION_ARGUMENT_2_COUNT; j++) {
								progressive_actions[i].argument2_u32[j] = NG_READ_32(ng_savegame_buffer, offset);
							}
						} else {
							NGLog(NG_LOG_TYPE_ERROR, "Progressive action overflow!");
						}
					}
					break;
				}
				case 0x8008: { // OLD_ACTIONS
					old_action_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < old_action_count; i++) {
						if (i < NG_MAX_OLD_ACTIONS) {
							old_actions[i].flags = NG_READ_16(ng_savegame_buffer, offset);
							old_actions[i].offset_floor_data = NG_READ_32(ng_savegame_buffer, offset);
						} else {
							NGLog(NG_LOG_TYPE_ERROR, "Old action overflow!");
						}
					}
					break;
				}
				case 0x800E: { // OLD_CONDITION
					old_condition_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < old_condition_count; i++) {
						if (i < NG_MAX_OLD_CONDITIONS) {
							old_conditions[i].flags = NG_READ_16(ng_savegame_buffer, offset);
							old_conditions[i].offset_floor_data = NG_READ_32(ng_savegame_buffer, offset);
						}
						else {
							NGLog(NG_LOG_TYPE_ERROR, "Old condition overflow!");
						}
					}
					break;
					break;
				}
				case 0x800F: { // VARIABLE_DATA
					uint16_t cold_intensity = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t cold_flags = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t damage_intensity = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t damage_flags = NG_READ_16(ng_savegame_buffer, offset);
					ng_input_to_disable = NG_READ_32(ng_savegame_buffer, offset);
					uint32_t status_ng = NG_READ_32(ng_savegame_buffer, offset);
					uint16_t disable_features_flags = NG_READ_16(ng_savegame_buffer, offset);
					uint32_t counter_game = NG_READ_32(ng_savegame_buffer, offset);
					uint16_t level_now_flags = NG_READ_16(ng_savegame_buffer, offset);

					int32_t layer1_color = NG_READ_32(ng_savegame_buffer, offset);
					gfLayer1Col.a = ((layer1_color & 0xff000000) >> 24);
					gfLayer1Col.r = ((layer1_color & 0x00ff0000) >> 16);
					gfLayer1Col.g = ((layer1_color & 0x0000ff00) >> 8);
					gfLayer1Col.b = ((layer1_color & 0x000000ff) >> 0);
					int32_t layer2_color = NG_READ_32(ng_savegame_buffer, offset);
					gfLayer2Col.a = ((layer2_color & 0xff000000) >> 24);
					gfLayer2Col.r = ((layer2_color & 0x00ff0000) >> 16);
					gfLayer2Col.g = ((layer2_color & 0x0000ff00) >> 8);
					gfLayer2Col.b = ((layer2_color & 0x000000ff) >> 0);

					gfLayer1Vel = NG_READ_8(ng_savegame_buffer, offset);
					gfLayer2Vel = NG_READ_8(ng_savegame_buffer, offset);

					int16_t secondary_loop_cd_track = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t level_ng_flags = NG_READ_16(ng_savegame_buffer, offset);
					int16_t secondary_single_cd_track = NG_READ_16(ng_savegame_buffer, offset);
					uint32_t cd_channel_2_offset = NG_READ_32(ng_savegame_buffer, offset);

					if (secondary_loop_cd_track != -1) {
						S_CDPlayExt(secondary_loop_cd_track, 1, true, false);
					}

					if (secondary_single_cd_track != -1) {
						S_CDPlayExt(secondary_single_cd_track, 1, false , false);
					}

					if (secondary_loop_cd_track != -1 || secondary_single_cd_track != -1) {
						if (cd_channel_2_offset != 0xffffffff) {
							S_CDSeek(1, cd_channel_2_offset / sizeof(int32_t));
						}
					}

					int16_t main_loop_cd_track = NG_READ_16(ng_savegame_buffer, offset);
					int16_t main_single_cd_track = NG_READ_16(ng_savegame_buffer, offset);

					if (main_loop_cd_track != -1) {
						S_CDPlayExt(main_loop_cd_track, 0, true, false);
					}

					if (main_single_cd_track != -1) {
						S_CDPlayExt(main_single_cd_track, 0, false, false);
					}

					uint32_t cd_channel_1_offset = NG_READ_32(ng_savegame_buffer, offset);

					if (main_loop_cd_track != -1 || main_single_cd_track != -1) {
						if (cd_channel_1_offset != 0xffffffff) {
							S_CDSeek(0, cd_channel_1_offset / sizeof(int32_t));
						}
					}

					LevelFogStart = NG_READ_FLOAT(ng_savegame_buffer, offset);

					int32_t unused_1 = NG_READ_32(ng_savegame_buffer, offset);
					int32_t unused_2 = NG_READ_32(ng_savegame_buffer, offset);

					int16_t current_pushable_index = NG_READ_16(ng_savegame_buffer, offset);

					int16_t parallel_bar_rounds = NG_READ_16(ng_savegame_buffer, offset);
					int32_t parallel_bar_frames = NG_READ_32(ng_savegame_buffer, offset);
					int16_t test_pop_up_image = NG_READ_16(ng_savegame_buffer, offset);
					int16_t pop_up_counter = NG_READ_16(ng_savegame_buffer, offset);
					int16_t pop_up_image_index = NG_READ_16(ng_savegame_buffer, offset);

					int8_t volumetric_fx = NG_READ_8(ng_savegame_buffer, offset);
					int8_t hardware_fog = NG_READ_8(ng_savegame_buffer, offset);
					int16_t unused_3 = NG_READ_16(ng_savegame_buffer, offset);

					int16_t end_fog_sectors = NG_READ_16(ng_savegame_buffer, offset);

					int32_t fog_color = NG_READ_32(ng_savegame_buffer, offset);

					SetDistanceFogColor(
						((fog_color & 0x00ff0000) >> 16),
						((fog_color & 0x0000ff00) >> 8),
						((fog_color & 0x000000ff) >> 0)
					);

					int16_t start_fog_sectors = NG_READ_16(ng_savegame_buffer, offset);
					int16_t max_fog_bulb_distance = NG_READ_16(ng_savegame_buffer, offset);

					for (int32_t i = 0; i < 100; i++) {
						int32_t unused_4 = NG_READ_32(ng_savegame_buffer, offset);
					}

					break;
				}
				case 0x8019: { // SWAP_MESH
					break;
				}
				case 0x801A: { // STATUS_GTRIGGERS
					uint16_t global_trigger_count = NG_READ_16(ng_savegame_buffer, offset);
					for (uint32_t i = 0; i < global_trigger_count; i++) {
						if (i < MAX_NG_GLOBAL_TRIGGERS) {
							uint16_t flags = NG_READ_16(ng_savegame_buffer, offset);

							ng_global_trigger_states[i].is_disabled = flags & FGT_DISABLED;
							ng_global_trigger_states[i].is_halted = flags & FGT_PAUSE_ONE_SHOT;
						} else {
							NGLog(NG_LOG_TYPE_ERROR, "GTrigger overflow!");
						}
					}
					break;
				}
				case 0x801C: { // ROOM_FLAGS
					uint16_t room_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < room_count; i++) {
						uint16_t room_flags = NG_READ_16(ng_savegame_buffer, offset);
						if (i < number_rooms) {
							int32_t room_index = NGFindIndexForRoom(i);
							if (room_index != -1 && room_index < number_rooms) {
								uint16_t new_room_flags = room[room_index].flags & ~0x1C35;
								new_room_flags |= room_flags;
								room[room_index].flags = new_room_flags;
							}
						}
					}
					break;
				}
				case 0x801D: { // WEATHER_INTENSITY
					uint16_t room_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < room_count; i++) {
						uint8_t weather_intensity = NG_READ_8(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x804D: { // OCB_ITEMS
					uint16_t item_ocb_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < item_ocb_count; i++) {
						uint16_t item_ocb = NG_READ_16(ng_savegame_buffer, offset);
						ITEM_INFO *item = T4PlusGetItemInfoForID(i);
						if (item) {
							item->trigger_flags = item_ocb;
						}
					}
					break;
				}
				case 0x801E: { // STATUS_ORGANIZER
					uint16_t status_organizer_count = NG_READ_16(ng_savegame_buffer, offset);
					break;
				}
				case 0x8011: { // PRINT_STRING
					uint32_t color = NG_READ_32(ng_savegame_buffer, offset);
					uint16_t position = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t flags = NG_READ_16(ng_savegame_buffer, offset);
					uint8_t blink_speed = NG_READ_8(ng_savegame_buffer, offset);
					uint8_t flags_micro = NG_READ_8(ng_savegame_buffer, offset);
					uint8_t def_flags_micro = NG_READ_8(ng_savegame_buffer, offset);
					uint8_t def_all_flags_micro = NG_READ_8(ng_savegame_buffer, offset);
					uint32_t def_color = NG_READ_32(ng_savegame_buffer, offset);
					uint16_t def_position = NG_READ_16(ng_savegame_buffer, offset);
					uint8_t def_blink_speed = NG_READ_8(ng_savegame_buffer, offset);
					uint16_t def_flags = NG_READ_16(ng_savegame_buffer, offset);
					uint8_t unused = NG_READ_8(ng_savegame_buffer, offset);
					break;
				}
				case 0x8014: { // BLIND_SAVE
					uint16_t blind_savegame_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < blind_savegame_count; i++) {
						uint8_t blind_savegame = NG_READ_8(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x8015: { // CASUALE
					total_causale = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < total_causale; i++) {
						uint16_t causale = NG_READ_16(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x8022: { // STATICS
					uint16_t changed_statics_count = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < changed_statics_count; i++) {
						uint16_t room_index = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t static_index = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t flags = NG_READ_16(ng_savegame_buffer, offset);
						uint32_t pos_x = NG_READ_32(ng_savegame_buffer, offset);
						uint32_t pos_y = NG_READ_32(ng_savegame_buffer, offset);
						uint32_t pos_z = NG_READ_32(ng_savegame_buffer, offset);
						uint16_t rotation = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t color = NG_READ_16(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x8023: { // OBJECT_TIMER
					uint16_t timer_count = NG_READ_16(ng_savegame_buffer, offset);
					uint32_t long_timer = NG_READ_32(ng_savegame_buffer, offset);
					uint16_t short_timer = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < timer_count; i++) {
						uint16_t item_index = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t custom_a = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t custom_b = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t custom_c = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t custom_d = NG_READ_16(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x8024: { // VERSION_HEADER
					uint16_t version_a = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t version_b = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t version_c = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t version_d = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t flags = NG_READ_16(ng_savegame_buffer, offset);
					break;
				}
				case 0x8030: { // STATUS_ANIM_RANGES
					break;
				}
				case 0x8031: { // SAVEGAME_INFOS
					uint16_t flags = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t first_shatter = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t last_shatter = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t lara_state_id = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t lara_hp = NG_READ_16(ng_savegame_buffer, offset);

					char tr4_name[32];
					for (int32_t i = 0; i < sizeof(tr4_name); i++) {
						tr4_name[i] = NG_READ_8(ng_savegame_buffer, offset);
					}

					int16_t vehicle = NG_READ_16(ng_savegame_buffer, offset);

					int8_t buffer_a[157];
					for (int32_t i = 0; i < sizeof(buffer_a); i++) {
						buffer_a[i] = NG_READ_8(ng_savegame_buffer, offset);
					}

					int8_t buffer_b[68];
					for (int32_t i = 0; i < sizeof(buffer_b); i++) {
						buffer_b[i] = NG_READ_8(ng_savegame_buffer, offset);
					}

					uint16_t room_flags = NG_READ_16(ng_savegame_buffer, offset);
					uint32_t offset_lara = NG_READ_32(ng_savegame_buffer, offset);

					uint16_t buffer_c[117];
					for (int32_t i = 0; i < (sizeof(buffer_c) / sizeof(uint16_t)); i++) {
						buffer_c[i] = NG_READ_16(ng_savegame_buffer, offset);
					}

					uint8_t dummy = NG_READ_8(ng_savegame_buffer, offset);

					break;
				}
				case 0x8035: { // HUB_HEADERS
					uint16_t hub_num_levels = NG_READ_16(ng_savegame_buffer, offset);
					uint16_t hub_last_entry = NG_READ_16(ng_savegame_buffer, offset);

					uint16_t lara_hub_word_count = NG_READ_16(ng_savegame_buffer, offset);

					for (int32_t i = 0; i < 10; i++) {
						uint16_t level_number = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t word_count = NG_READ_16(ng_savegame_buffer, offset);
					}

					break;
				}
				case 0x8038: { // VAR_GLOBAL_TRNG
					ng_global_alfa = NG_READ_32(ng_savegame_buffer, offset);
					ng_global_beta = NG_READ_32(ng_savegame_buffer, offset);
					ng_global_delta = NG_READ_32(ng_savegame_buffer, offset);
					ng_global_timer = NG_READ_32(ng_savegame_buffer, offset);

					NG_READ_FIXED_STRING(ng_savegame_buffer, &ng_string1, sizeof(ng_string1), offset);
					NG_READ_FIXED_STRING(ng_savegame_buffer, &ng_string2, sizeof(ng_string2), offset);
					NG_READ_FIXED_STRING(ng_savegame_buffer, &ng_string3, sizeof(ng_string3), offset);
					NG_READ_FIXED_STRING(ng_savegame_buffer, &ng_string4, sizeof(ng_string4), offset);

					for (int32_t i = 0; i < STORE_VARIABLE_COUNT; i++) {
						ng_store_variables[i] = NG_READ_32(ng_savegame_buffer, offset);
					}

					NG_READ_FIXED_STRING(ng_savegame_buffer, &ng_last_text_input, sizeof(ng_last_text_input), offset);
					
					ng_last_input_number = NG_READ_32(ng_savegame_buffer, offset);
					ng_current_value = NG_READ_32(ng_savegame_buffer, offset);

					NG_READ_FIXED_STRING(ng_savegame_buffer, &ng_text_big, sizeof(ng_text_big), offset);

					int32_t extra[20];
					for (int32_t i = 0; i < 20; i++) {
						extra[i] = NG_READ_32(ng_savegame_buffer, offset);
					}

					break;
				}
				case 0x8039: { // VAR_LOCAL_TRNG
					ng_local_alfa = NG_READ_32(ng_savegame_buffer, offset);
					ng_local_beta = NG_READ_32(ng_savegame_buffer, offset);
					ng_local_delta = NG_READ_32(ng_savegame_buffer, offset);
					ng_local_timer = NG_READ_32(ng_savegame_buffer, offset);
					break;
				}
				case 0x803A: { // FROZEN_ITEMS
					uint16_t total_frozen_items = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < total_frozen_items; i++) {
						uint16_t item_index = NG_READ_16(ng_savegame_buffer, offset);
						uint16_t duration = NG_READ_16(ng_savegame_buffer, offset);
						ITEM_INFO* item = T4PlusGetItemInfoForID(i);
						if (item) {
							NGSetItemFreezeTimer(item_index, duration);
						}
					}
					break;
				}
				case 0x803C: { // NO_COLL_ITEMS
					uint16_t total_no_coll_items = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < total_no_coll_items; i++) {
						uint16_t item_index = NG_READ_16(ng_savegame_buffer, offset);
						ITEM_INFO* item = T4PlusGetItemInfoForID(i);
						if (item) {
							NGDisableItemCollision(item_index);
						}
					}
					break;
				}
				case 0x8043: { // DIARY_DATA
					for (int32_t i = 0; i < 2; i++) {
						uint16_t diary_data = NG_READ_16(ng_savegame_buffer, offset);
					}
					break;
				}
				case 0x804C: { // SLOT_FLAGS
					uint16_t total_slot_flags = NG_READ_16(ng_savegame_buffer, offset);
					for (int32_t i = 0; i < total_slot_flags; i++) {
						uint16_t slot_flags = NG_READ_16(ng_savegame_buffer, offset);
					}
					break;
				}
				default: {
					break;
				}
			}

			size_t expected_block_size = start_offset + (block_size * sizeof(uint16_t));

			if (offset != expected_block_size) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGSavegameBuffer: size of block (0x%04X) mismatch!", block_type);
				offset = expected_block_size;
			}

			if (offset >= ng_savegame_buffer_size) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGSavegameBuffer: overflow!");
				return;
			}
		}
	}
}

void NGReadNGSavegameBuffer(FILE *file) {
	memset(ng_savegame_buffer, 0x00, MAX_NG_SAVEGAME_BUFFER_SIZE);
	ng_savegame_buffer_size = 0;

	int32_t original_file_position = ftell(file);
	if (fseek(file, -int32_t((sizeof(uint32_t) * 2)), SEEK_END) == 0) {
		uint32_t ngle_ident = 0;
		fread(&ngle_ident, sizeof(uint32_t), 1, file);
		if (ngle_ident == NGLE_END_SIGNATURE) {
			uint32_t ngle_buffer_end = ftell(file);

			uint32_t footer_offset = 0;
			fread(&footer_offset, sizeof(uint32_t), 1, file);
			if (fseek(file, -int32_t(footer_offset), SEEK_END) == 0) {
				int32_t ngle_buffer_start = ftell(file);
				uint16_t header_ident;
				fread(&header_ident, sizeof(uint16_t), 1, file);
				if (header_ident == NGLE_START_SIGNATURE) {
					if (fseek(file, -int32_t(sizeof(uint16_t)), SEEK_CUR) == 0) {
						uint32_t buffer_size = ngle_buffer_end - ngle_buffer_start;
						if (buffer_size < MAX_NG_SAVEGAME_BUFFER_SIZE) {
							if (fread(ng_savegame_buffer, sizeof(char), buffer_size, file) == buffer_size) {
								ng_savegame_buffer_size = buffer_size;
							}
						} else {
							NGLog(NG_LOG_TYPE_ERROR, "NGReadNGSavegameBuffer: overflow!");
						}
					}
				}
			}
		}
	}
	fseek(file, original_file_position, SEEK_SET);
}