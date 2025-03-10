#include "../../tomb4/pch.h"

#include "../../global/types.h"
#include "../../specific/function_stubs.h"
#include "../../specific/3dmath.h"
#include "../control.h"
#include "../gameflow.h"
#include "../effects.h"
#include "../objects.h"
#include "../lara.h"
#include "../tomb4fx.h"
#include "../sound.h"
#include "../switch.h"
#include "../text.h"

#include "trng.h"
#include "trng_extra_state.h"
#include "trng_animation.h"
#include "trng_condition.h"
#include "trng_flipeffect.h"
#include "trng_script_parser.h"
#include "trng_organizer.h"
#include "trng_triggergroup.h"
#include "trng_globaltrigger.h"
#include "trng_progressive_action.h"
#include "../../specific/file.h"
#include "../../tomb4/tomb4plus/t4plus_environment.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"
#include "../../tomb4/tomb4plus/t4plus_objects.h"

int32_t resumed_trigger_group_count;
uint16_t resumed_trigger_groups[MAX_NG_TRIGGER_GROUPS];
int32_t last_performed_trigger_group = -1;
uint16_t performed_trigger_groups[MAX_NG_TRIGGER_GROUPS];

bool ng_loaded_savegame = false;

int16_t ng_camera_target_id = NO_ITEM;

uint32_t ng_room_offset_table[0xff];

struct NG_MOVEMENT_INFO {
	int16_t horizontal_movement_speed = 0;
	int16_t vertical_movement_speed = 0;
	int16_t movement_in_progress_sound = -1;
	int16_t movement_finished_sound = -1;
	int16_t move_horizontal_angle = 0;
	int32_t move_horizontal_remaining_units = 0;
	int32_t move_vertical_remaining_units = 0;
	int32_t move_horizontal_repeat_units = 0;
	int32_t move_vertical_repeat_units = 0;
	bool trigger_heavy_at_end = false;
	bool trigger_normal_when_moving = false;
	bool trigger_heavy_when_moving = false;
};

// NG_ITEM_EXTRADATA is persistent supplementary data used by TRNG triggers.
// The state here can subsequently be serialized as additional data for save games.
struct NG_ITEM_EXTRADATA {
	uint16_t frozen_ticks = 0;
	bool collison_disabled = false; // Will only disable the ObjectCollision routine. Doors and enemies still have collision.
	int16_t fade_override = 0;

	NG_MOVEMENT_INFO movement_info;
};

NG_ITEM_EXTRADATA *ng_items_extradata = NULL;
NG_MOVEMENT_INFO *ng_statics_movement_info = NULL;

void NGResetMovementInfo(NG_MOVEMENT_INFO* movement_info) {
	movement_info->horizontal_movement_speed = 0;
	movement_info->vertical_movement_speed = 0;
	movement_info->movement_in_progress_sound = -1;
	movement_info->movement_finished_sound = -1;
	movement_info->move_horizontal_angle = 0;
	movement_info->move_horizontal_remaining_units = 0;
	movement_info->move_vertical_remaining_units = 0;
	movement_info->move_horizontal_repeat_units = 0;
	movement_info->move_vertical_repeat_units = 0;
}

void NGResetItemExtraData(int32_t item_number) {
	NG_ITEM_EXTRADATA *current_extradata = &ng_items_extradata[item_number];

	if (current_extradata) {
		current_extradata->frozen_ticks = 0;
		current_extradata->collison_disabled = false; // Will only disable the ObjectCollision routine. Doors and enemies still have collision.

		current_extradata->fade_override = 0;

		NGResetMovementInfo(&current_extradata->movement_info);
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "NGResetItemExtraData: invalid item_number!");
	}
}

int32_t ng_animation_current_animation = -1;
int16_t ng_animation_prev_hands_state = LG_NO_ARMS;
int32_t ng_animation_target_item = -1;
int32_t ng_animation_target_test_position = -1;

NG_GLOBAL_TRIGGER_STATE ng_global_trigger_states[MAX_NG_GLOBAL_TRIGGERS];
NG_TRIGGER_GROUP_STATE ng_trigger_group_states[MAX_NG_TRIGGER_GROUPS];
NG_ORGANIZER_STATE ng_organizer_states[MAX_NG_ORGANIZERS];

// TODO: In the original, there's some behaviour which allows multiple timers to run
// at once, displaying the last activated on until it runs out. Needs investigation.
#define TIMER_TRACKER_TIMEOUT 30
NGTimerTrackerType timer_tracker_type = TTT_ONLY_SHOW_SECONDS;
int32_t timer_tracker = -1;
int32_t timer_tracker_remaining_until_timeout = 0;

int32_t ng_cinema_timer = -1;
int32_t ng_cinema_type = 0;

int32_t lara_damage_resistence = 1000;

bool ng_lara_infinite_air = false;

// Timers
int32_t ng_global_timer = 0;
int8_t ng_global_timer_frame_increment = 0;
NGTimerPosition ng_global_timer_position = NG_TIMER_POSITION_INVISIBLE;
int32_t ng_global_timer_time_until_hide = 0;

int32_t ng_local_timer = 0;
int8_t ng_local_timer_frame_increment = 0;
NGTimerPosition ng_local_timer_position = NG_TIMER_POSITION_INVISIBLE;
int32_t ng_local_timer_time_until_hide = 0;

// Level
int32_t pending_level_load_timer = -1;
int32_t pending_level_load_id = 0;

// Variables
int32_t ng_current_value = 0;
int32_t ng_global_alfa = 0;
int32_t ng_global_beta = 0;
int32_t ng_global_delta = 0;
int32_t ng_local_alfa = 0;
int32_t ng_local_beta = 0;
int32_t ng_local_delta = 0;
int32_t ng_last_input_number = 0;

int32_t ng_store_variables[STORE_VARIABLE_COUNT];

int8_t ng_last_text_input[REGULAR_TEXT_BUFFER_SIZE];

int8_t ng_string1[REGULAR_TEXT_BUFFER_SIZE];
int8_t ng_string2[REGULAR_TEXT_BUFFER_SIZE];
int8_t ng_string3[REGULAR_TEXT_BUFFER_SIZE];
int8_t ng_string4[REGULAR_TEXT_BUFFER_SIZE];

int8_t ng_text_big[BIG_TEXT_BUFFER_SIZE];

// Visual

NG_DRAW_STATE ng_drawing_state = NG_DRAW_STATE_ACTIVE;

// Inventory
uint8_t ng_selected_inventory_item_memory = 0;
int32_t ng_used_inventory_object_for_frame = NO_ITEM;
bool ng_used_large_medipack = false;
bool ng_used_small_medipack = false;

int32_t ng_looped_sound_state[NumSamples];

int32_t ng_input_to_simulate = 0;
int32_t ng_input_to_disable = 0;
int32_t ng_single_input_to_simulate = 0;

int32_t NGGetPluginIDForFloorData(size_t floor_index, bool test_condition) {
	if (test_condition) {
		floor_index++;
	}

	if (ng_floor_id_table) {
		if (floor_index < ng_floor_id_size) {
			int32_t plugin_id = ng_floor_id_table[floor_index];
			return plugin_id;
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "NGGetPluginIDForFloorData: Overflow!");
		}
	}
	return 0;
}

NG_DRAW_STATE NGGetDrawState() {
	return ng_drawing_state;
}

int32_t NGValidateAgainstBlockedInput(int32_t input) {
	if (ng_input_to_disable) {
		int32_t modified_blocked_input = ng_input_to_disable;
		if (modified_blocked_input == IN_ALL) {
			input = 0;
		} else {
			input &= ~modified_blocked_input;
		}
	}

	return input;
}

int32_t NGApplySimulatedInput(int32_t input) {
	input |= ng_single_input_to_simulate;

	if (ng_input_to_simulate) {
		int32_t modified_simulated_input = ng_input_to_simulate;
		if (modified_simulated_input != IN_ALL) {
			if (modified_simulated_input & IN_SAVE) { // SAVE_GAME
				//
			}

			if (modified_simulated_input & IN_LOAD) { // LOAD_GAME

			}

			if (modified_simulated_input & 0x10000000) { // WEAPON_KEYS
				for (int32_t i = 0; i < 6; i++) {

				}

				modified_simulated_input &= ~0x10000000;
			}
		}

		input |= modified_simulated_input;
	}

	return input;
}

bool NGValidateInputSavegame() {
	if (ng_input_to_disable) {
		int32_t modified_blocked_input = ng_input_to_disable;
		if (modified_blocked_input == IN_ALL) {
			return false;
		} else {
			if (modified_blocked_input & IN_SAVE) {
				return false;
			}
		}
	}
	return true;
}

bool NGValidateInputLoadgame() {
	if (ng_input_to_disable) {
		int32_t modified_blocked_input = ng_input_to_disable;
		if (modified_blocked_input == IN_ALL) {
			return false;
		} else {
			if (modified_blocked_input & IN_LOAD) {
				return false;
			}
		}
	}
	return true;
}

bool NGIsSimulatingInputSavegame() {
	if (ng_input_to_simulate & IN_SAVE) {
		return true;
	}
	return false;
}

bool NGIsSimulatingInputLoadgame() {
	if (ng_input_to_simulate & IN_LOAD) {
		return true;
	}
	return false;
}

bool NGValidateInputWeaponHotkeys() {
	if (ng_input_to_disable) {
		int32_t modified_blocked_input = ng_input_to_disable;
		if (modified_blocked_input == IN_ALL) {
			return false;
		} else {
			if (modified_blocked_input & 0x10000000) {
				return false;
			}
		}
	}
	return true;
}

void NGDisableInputForTime(uint32_t mask, int32_t ticks) {
	ng_input_to_disable |= mask;

	if (ticks == 0) {
		return;
	}

	NGProgressiveAction *prog_action = nullptr;
	for (int32_t i = 0; i < progressive_action_count; i++) {
		if (progressive_actions[i].type == AZ_RESET_DISABLED_INPUT && progressive_actions[i].argument2_i32[0] == mask) {
			prog_action = &progressive_actions[i];
		}
	}

	if (!prog_action) {
		prog_action = NGCreateProgressiveAction();
	}

	if (prog_action) {
		prog_action->type = AZ_RESET_DISABLED_INPUT;
		prog_action->argument2_i32[0] = mask;
		prog_action->duration = ticks;
	}
}

void NGEnableInput(uint32_t mask) {
	if (mask == -1) {
		ng_input_to_disable = 0;
	} else {
		ng_input_to_disable &= (mask ^ -1);
	}
}

void NGSimulateInputForTime(uint32_t mask, int32_t ticks) {
	ng_input_to_simulate |= mask;

	if (ticks == 0) {
		ng_single_input_to_simulate = mask;
		return;
	}

	NGProgressiveAction* prog_action = prog_action = NGCreateProgressiveAction();

	if (prog_action) {
		prog_action->type = AZ_RESET_SIMULATED_INPUT;
		prog_action->argument2_i32[0] = mask;
		prog_action->duration = ticks;
	}
}

void NGClearSimulatedSingleInputForFrame() {
	ng_single_input_to_simulate = 0;
}

void NGHandleItemMovement(uint32_t item_num) {
	if (NGGetItemHorizontalMovementRemainingUnits(item_num)) {
		ITEM_INFO* item = T4PlusGetItemInfoForID(item_num);
		if (item) {
			int32_t move_by_amount = NGGetItemHorizontalMovementSpeed(item_num);
			int32_t remaining_movement_units = NGGetItemHorizontalMovementRemainingUnits(item_num);
			if (
			    (remaining_movement_units >= 0 && move_by_amount > remaining_movement_units) ||
			    (remaining_movement_units < 0 && move_by_amount < remaining_movement_units)) {
				move_by_amount = remaining_movement_units;
			}

			NGMoveItemHorizontalByUnits(item_num, ng_items_extradata[item_num].movement_info.move_horizontal_angle, move_by_amount);
			NGSetItemHorizontalMovementRemainingUnits(item_num, remaining_movement_units - move_by_amount);

			if (NGGetItemHorizontalMovementRemainingUnits(item_num) == 0) {
				if (NGGetItemMovementFinishedSound(item_num) != -1) {
					SoundEffect(NGGetItemMovementFinishedSound(item_num), &item->pos, 0);
				}

				if (NGGetItemMovementTriggerHeavyAtEnd(item_num)) {
					TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, true, 0);
				}

				// Reset everything or loop
				if (NGGetItemHorizontalMovementRepeatUnits(item_num) != 0) {
					NGSetItemHorizontalMovementRemainingUnits(item_num, NGGetItemHorizontalMovementRepeatUnits(item_num));
					NGSetItemHorizontalMovementAngle(item_num, NGGetItemHorizontalMovementAngle(item_num) + NG_DEGREE(180));
				} else {
					NGSetItemHorizontalMovementSpeed(item_num, 0);
				}
			} else {
				if (NGGetItemMovementInProgressSound(item_num) != -1) {
					SoundEffect(NGGetItemMovementInProgressSound(item_num), &item->pos, 0);
				}

				if (NGGetItemMovementTriggerNormalWhenMoving(item_num)) {
					TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, false, 0);
				}

				if (NGGetItemMovementTriggerHeavyWhenMoving(item_num)) {
					TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, true, 0);
				}
			}
			UpdateItemRoom(item_num, -HALF_CLICK_SIZE);
		}
	}

	if (NGGetItemVerticalMovementRemainingUnits(item_num)) {
		ITEM_INFO *item = T4PlusGetItemInfoForID(item_num);
		if (item) {
			int32_t move_by_amount = NGGetItemVerticalMovementSpeed(item_num);
			int32_t remaining_movement_units = NGGetItemVerticalMovementRemainingUnits(item_num);
			if (
			    (remaining_movement_units >= 0 && move_by_amount > remaining_movement_units) ||
			    (remaining_movement_units < 0 && move_by_amount < remaining_movement_units)) {
				move_by_amount = remaining_movement_units;
			}

			NGMoveItemVerticalByUnits(item_num, move_by_amount);
			NGSetItemVerticalMovementRemainingUnits(item_num, remaining_movement_units - move_by_amount);

			if (NGGetItemVerticalMovementRemainingUnits(item_num) == 0) {
				if (NGGetItemMovementFinishedSound(item_num) != -1) {
					SoundEffect(NGGetItemMovementFinishedSound(item_num), &item->pos, 0);
				}

				if (NGGetItemMovementTriggerHeavyAtEnd(item_num)) {
					TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, true, 0);
				}

				// Reset everything or loop
				if (NGGetItemVerticalMovementRepeatUnits(item_num) != 0) {
					NGSetItemVerticalMovementSpeed(item_num, -NGGetItemVerticalMovementSpeed(item_num));
					NGSetItemVerticalMovementRemainingUnits(item_num, NGGetItemVerticalMovementRepeatUnits(item_num));
					NGSetItemVerticalMovementRepeatUnits(item_num, -NGGetItemVerticalMovementRepeatUnits(item_num));
				} else {
					NGSetItemVerticalMovementSpeed(item_num, 0);
				}
			} else {
				if (NGGetItemMovementInProgressSound(item_num) != -1) {
					SoundEffect(NGGetItemMovementInProgressSound(item_num), &item->pos, 0);
				}

				if (NGGetItemMovementTriggerNormalWhenMoving(item_num)) {
					TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, false, 0);
				}

				if (NGGetItemMovementTriggerHeavyWhenMoving(item_num)) {
					TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, true, 0);
				}
			}
			UpdateItemRoom(item_num, -HALF_CLICK_SIZE);
		}
	}
}

// Statics

void NGHandleStaticMovement(uint32_t static_num) {
	if (NGGetStaticHorizontalMovementRemainingUnits(static_num)) {
		GAME_VECTOR game_vector = NGGetGameVectorForStatic(static_num);

		int32_t move_by_amount = NGGetStaticHorizontalMovementSpeed(static_num);
		int32_t remaining_movement_units = NGGetStaticHorizontalMovementRemainingUnits(static_num);
		if (
		    (remaining_movement_units >= 0 && move_by_amount > remaining_movement_units) ||
		    (remaining_movement_units < 0 && move_by_amount < remaining_movement_units)) {
			move_by_amount = remaining_movement_units;
		}

		NGMoveStaticHorizontalByUnits(static_num, ng_statics_movement_info[static_num].move_horizontal_angle, move_by_amount);
		NGSetStaticHorizontalMovementRemainingUnits(static_num, remaining_movement_units - move_by_amount);

		if (NGGetStaticHorizontalMovementRemainingUnits(static_num) == 0) {
			if (NGGetStaticMovementFinishedSound(static_num) != -1) {
				PHD_3DPOS pos;
				pos.x_pos = game_vector.x;
				pos.y_pos = game_vector.y;
				pos.z_pos = game_vector.z;
				SoundEffect(NGGetStaticMovementFinishedSound(static_num), &pos, 0);
			}

			if (NGGetStaticMovementTriggerHeavyAtEnd(static_num)) {
				TestTriggersAtXYZ(game_vector.x, game_vector.y, game_vector.z, game_vector.room_number, true, 0);
			}

			// Reset everything or loop
			if (NGGetStaticHorizontalMovementRepeatUnits(static_num) > 0) {
				NGSetStaticHorizontalMovementRemainingUnits(static_num, NGGetStaticHorizontalMovementRepeatUnits(static_num));
				NGSetStaticHorizontalMovementAngle(static_num, NGGetStaticHorizontalMovementAngle(static_num) + NG_DEGREE(180));
			} else {
				NGSetStaticHorizontalMovementSpeed(static_num, 0);
			}
		} else {
			if (NGGetStaticMovementInProgressSound(static_num) != -1) {
				PHD_3DPOS pos;
				pos.x_pos = game_vector.x;
				pos.y_pos = game_vector.y;
				pos.z_pos = game_vector.z;
				SoundEffect(NGGetStaticMovementInProgressSound(static_num), &pos, 0);
			}

			if (NGGetStaticMovementTriggerNormalWhenMoving(static_num)) {
				TestTriggersAtXYZ(game_vector.x, game_vector.y, game_vector.z, game_vector.room_number, false, 0);
			}

			if (NGGetStaticMovementTriggerHeavyWhenMoving(static_num)) {
				TestTriggersAtXYZ(game_vector.x, game_vector.y, game_vector.z, game_vector.room_number, true, 0);
			}
		}
	}

	if (NGGetStaticVerticalMovementRemainingUnits(static_num)) {
		GAME_VECTOR game_vector = NGGetGameVectorForStatic(static_num);

		int32_t move_by_amount = NGGetStaticVerticalMovementSpeed(static_num);
		int32_t remaining_movement_units = NGGetStaticVerticalMovementRemainingUnits(static_num);
		if (
		    (remaining_movement_units >= 0 && move_by_amount > remaining_movement_units) ||
		    (remaining_movement_units < 0 && move_by_amount < remaining_movement_units)) {
			move_by_amount = remaining_movement_units;
		}

		NGMoveStaticVerticalByUnits(static_num, move_by_amount);
		NGSetStaticVerticalMovementRemainingUnits(static_num, remaining_movement_units - move_by_amount);

		if (NGGetStaticVerticalMovementRemainingUnits(static_num) == 0) {
			if (NGGetStaticMovementFinishedSound(static_num) != -1) {
				PHD_3DPOS pos;
				pos.x_pos = game_vector.x;
				pos.y_pos = game_vector.y;
				pos.z_pos = game_vector.z;
				SoundEffect(NGGetStaticMovementFinishedSound(static_num), &pos, 0);
			}

			if (NGGetStaticMovementTriggerHeavyAtEnd(static_num)) {
				TestTriggersAtXYZ(game_vector.x, game_vector.y, game_vector.z, game_vector.room_number, true, 0);
			}

			// Reset everything or loop
			if (NGGetStaticVerticalMovementRepeatUnits(static_num) > 0) {
				NGSetStaticVerticalMovementSpeed(static_num, -NGGetStaticVerticalMovementSpeed(static_num));
				NGSetStaticVerticalMovementRemainingUnits(static_num, NGGetStaticVerticalMovementRepeatUnits(static_num));
				NGSetStaticVerticalMovementRepeatUnits(static_num, -NGGetStaticVerticalMovementRepeatUnits(static_num));
			} else {
				NGSetStaticVerticalMovementSpeed(static_num, 0);
			}
		} else {
			if (NGGetStaticMovementInProgressSound(static_num) != -1) {
				PHD_3DPOS pos;
				pos.x_pos = game_vector.x;
				pos.y_pos = game_vector.y;
				pos.z_pos = game_vector.z;
				SoundEffect(NGGetStaticMovementInProgressSound(static_num), &pos, 0);
			}

			if (NGGetStaticMovementTriggerNormalWhenMoving(static_num)) {
				TestTriggersAtXYZ(game_vector.x, game_vector.y, game_vector.z, game_vector.room_number, false, 0);
			}

			if (NGGetStaticMovementTriggerHeavyWhenMoving(static_num)) {
				TestTriggersAtXYZ(game_vector.x, game_vector.y, game_vector.z, game_vector.room_number, true, 0);
			}
		}
	}
}

void NGUpdateAllItems() {
	for (int32_t i = 0; i < ITEM_COUNT; i++) {
		if (ng_items_extradata[i].frozen_ticks > 0 && ng_items_extradata[i].frozen_ticks != 0xffff) {
			ng_items_extradata[i].frozen_ticks--;
		}

		if (NGGetItemHorizontalMovementRemainingUnits(i) || NGGetItemVerticalMovementRemainingUnits(i)) {
			NGHandleItemMovement(i);
		}
	}
}

void NGUpdateAllStatics() {
	for (int32_t i = 0; i < ng_static_id_count; i++) {
		if (NGGetStaticHorizontalMovementRemainingUnits(i) || NGGetStaticVerticalMovementRemainingUnits(i)) {
			NGHandleStaticMovement(i);
		}
	}
}

NGItemCollision ng_lara_moveable_collisions[MAX_LARA_COLLISONS];
int32_t ng_lara_moveable_collision_size = 0;

int32_t ng_lara_static_collisions[MAX_LARA_STATIC_COLLISONS];
int32_t ng_lara_static_collision_size = 0;


void NGAddLaraItemCollision(ITEM_INFO *item_info, int32_t flags) {
	if (ng_lara_moveable_collision_size >= MAX_LARA_COLLISONS-1)
		return;

	for (int32_t i = 0; i < ng_lara_moveable_collision_size; i++) {
		if (ng_lara_moveable_collisions[i].item_info == item_info) {
			ng_lara_moveable_collisions[i].flags |= flags;
			return;
		}
	}

	ng_lara_moveable_collisions[ng_lara_moveable_collision_size].item_info = item_info;
	ng_lara_moveable_collisions[ng_lara_moveable_collision_size].flags = flags;
	ng_lara_moveable_collision_size++;
}

extern void NGAddLaraStaticCollision(int32_t room_number, int32_t mesh_number) {
	if (ng_lara_static_collision_size >= MAX_LARA_STATIC_COLLISONS-1)
		return;

	int32_t current_static_id = -1;

	// Very slow, replace this with caching
	for (int32_t i = 0; i < NG_STATIC_ID_TABLE_SIZE; i++) {
		if (ng_room_remap_table[ng_static_id_table[i].remapped_room_index].room_index == room_number && ng_static_id_table[i].mesh_id == mesh_number) {
			current_static_id = i;
			break;
		}
	}

	if (current_static_id >= 0) {
		for (int32_t i = 0; i < ng_lara_static_collision_size; i++) {
			if (ng_lara_static_collisions[i] == current_static_id) {
				return;
			}
		}

		ng_lara_static_collisions[ng_lara_static_collision_size] = current_static_id;
		ng_lara_static_collision_size++;
	}
}

void NGClearLaraCollisions() {
	ng_lara_moveable_collision_size = 0;
	ng_lara_static_collision_size = 0;
}

ITEM_INFO * NGIsLaraCollidingWithItem(ITEM_INFO *item, int32_t mask) {
	if (!item)
		return nullptr;

	for (int32_t i = 0; i < ng_lara_moveable_collision_size; i++) {
		if (ng_lara_moveable_collisions[i].item_info == item) {
			if ((ng_lara_moveable_collisions[i].flags & mask) != 0)
				return ng_lara_moveable_collisions[i].item_info;
			else
				return nullptr;
		}
	}

	return nullptr;
}

ITEM_INFO *NGIsLaraCollidingWithMoveableSlot(int32_t slot_number, int32_t mask) {
	for (int32_t i = 0; i < ng_lara_moveable_collision_size; i++) {
		if (ng_lara_moveable_collisions[i].item_info->object_number == slot_number) {
			if ((ng_lara_moveable_collisions[i].flags & mask) != 0)
				return ng_lara_moveable_collisions[i].item_info;
		}
	}

	return nullptr;
}

bool NGIsObjectMortalType(int32_t object_id) {
	if (object_id == BADDY_1 ||
	        object_id == BADDY_2 ||
	        (object_id >= CROCODILE && object_id <= SCORPION) ||
	        object_id == TROOPS ||
	        object_id == BABOON_NORMAL ||
	        object_id == WILD_BOAR ||
	        object_id == HARPY ||
	        object_id == BIG_BEETLE ||
	        object_id == BAT ||
	        object_id == DOG ||
	        object_id == SAS ||
	        object_id == SAS_CAPTAIN ||
	        object_id == SMALL_SCORPION) {
		return true;
	} else {
		return false;
	}
}

bool NGIsObjectImmortalType(int32_t object_id) {
	if (object_id == SKELETON ||
	        object_id == SETHA ||
	        object_id == MUMMY ||
	        object_id == SPHINX ||
	        object_id == KNIGHTS_TEMPLAR ||
	        object_id == MUTANT ||
	        object_id == BIG_BEETLE ||
	        object_id == HORSE ||
	        (object_id >= DEMIGOD1 && object_id <= LITTLE_BEETLE) ||
	        (object_id >= WRAITH1 && object_id <= WRAITH4) ||
	        object_id == HAMMERHEAD ||
	        object_id == AHMET ||
	        object_id == FISH) {
		return true;
	} else {
		return false;
	}
}

bool NGIsObjectFriendType(int32_t object_id) {
	if (object_id == VON_CROY || object_id == GUIDE || object_id == JEAN_YVES) {
		return true;
	} else {
		return false;
	}
}

ITEM_INFO *NGIsLaraCollidingWithCreature(NGCreatureType creature_type, int32_t mask) {
	for (int32_t i = 0; i < ng_lara_moveable_collision_size; i++) {
		switch (creature_type) {
			case NG_CREATURE_TYPE_ANY: {
				if ((ng_lara_moveable_collisions[i].flags & mask) != 0)
					return ng_lara_moveable_collisions[i].item_info;
				break;
			}
			case NG_CREATURE_TYPE_MORTAL: {
				int32_t object_id = ng_lara_moveable_collisions[i].item_info->object_number;

				if(NGIsObjectMortalType(object_id)) {
					if ((ng_lara_moveable_collisions[i].flags & mask) != 0)
						return ng_lara_moveable_collisions[i].item_info;
				}
				break;
			}
			case NG_CREATURE_TYPE_IMMORTAL: {
				int32_t object_id = ng_lara_moveable_collisions[i].item_info->object_number;

				if(NGIsObjectImmortalType(object_id)) {
					if ((ng_lara_moveable_collisions[i].flags & mask) != 0)
						return ng_lara_moveable_collisions[i].item_info;
				}
				break;
			}
			case NG_CREATURE_TYPE_FRIEND: {
				int32_t object_id = ng_lara_moveable_collisions[i].item_info->object_number;

				if(NGIsObjectFriendType(object_id)) {
					if ((ng_lara_moveable_collisions[i].flags & mask) != 0)
						return ng_lara_moveable_collisions[i].item_info;
				}
				break;
			}
			default: {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGIsLaraCollidingWithCreature: creature_type %u unimplemented!", creature_type);
				break;
			}
		}
	}

	return nullptr;
}

int32_t NGIsLaraCollidingWithStaticID(int32_t id) {
	for (int32_t i = 0; i < ng_lara_static_collision_size; i++) {
		if (ng_lara_static_collisions[i] == id) {
			return ng_lara_static_collisions[i];
		}
	}

	return -1;
}

int32_t NGIsLaraCollidingWithStaticSlot(int32_t slot) {
	for (int32_t i = 0; i < ng_lara_static_collision_size; i++) {
		int32_t static_id = ng_lara_static_collisions[i];

		if (static_id >= 0) {
			NGStaticTableEntry* entry = &ng_static_id_table[static_id];

			if (entry->remapped_room_index >= 0 && entry->remapped_room_index < NG_ROOM_REMAP_TABLE_SIZE) {
				if (ng_room_remap_table[entry->remapped_room_index].room_index >= 0 && ng_room_remap_table[entry->remapped_room_index].room_index < number_rooms) {

					if (entry->mesh_id >= 0 && room[ng_room_remap_table[entry->remapped_room_index].room_index].num_meshes) {

						if (room[ng_room_remap_table[entry->remapped_room_index].room_index].mesh[entry->mesh_id].static_number == slot) {
							return static_id;
						}
					}
				}
			}
		}
	}

	return -1;
}

bool NGIsLaraHolding(int32_t hold_type) {
	switch (hold_type) {
		case 1: {
			return (lara.gun_type == WEAPON_PISTOLS && lara.gun_status == LG_READY);
			break;
		}
		case 2: {
			return (lara.gun_type == WEAPON_REVOLVER && lara.gun_status == LG_READY);
			break;
		}
		case 3: {
			return (lara.gun_type == WEAPON_UZI && lara.gun_status == LG_READY);
			break;
		}
		case 4: {
			return (lara.gun_type == WEAPON_SHOTGUN && lara.gun_status == LG_READY);
			break;
		}
		case 5: {
			return (lara.gun_type == WEAPON_GRENADE && lara.gun_status == LG_READY);
			break;
		}
		case 6: {
			return (lara.gun_type == WEAPON_CROSSBOW && lara.gun_status == LG_READY);
			break;
		}
		case 7: {
			return (lara.gun_type == WEAPON_FLARE);
			break;
		}
		case 8: {
			return (lara.gun_type == WEAPON_TORCH && !lara.LitTorch);
			break;
		}
		case 9: {
			return (lara.gun_type == WEAPON_TORCH && lara.LitTorch);
			break;
		}
		case 10: {
			if (lara.vehicle != NO_ITEM)
				return (T4PlusGetItemInfoForID(lara.vehicle)->object_number == T4PlusGetJeepSlotID());
			break;
		}
		case 11: {
			if (lara.vehicle != NO_ITEM)
				return (T4PlusGetItemInfoForID(lara.vehicle)->object_number == T4PlusGetMotorbikeSlotID());
			break;
		}
		case 12: {
			if (lara.vehicle != NO_ITEM)
				return (T4PlusGetItemInfoForID(lara.vehicle)->object_number == T4PlusGetRubberBoatSlotID());
			break;
		}
		case 13: {
			if (lara.vehicle != NO_ITEM)
				return (T4PlusGetItemInfoForID(lara.vehicle)->object_number == T4PlusGetMotorBoatSlotID());
			break;
		}
		case 14: {
			return (lara.RopePtr != -1);
			break;
		}
		case 15: {
			return (lara_item->current_anim_state == AS_POLESTAT ||
			        lara_item->current_anim_state == AS_POLEUP ||
			        lara_item->current_anim_state == AS_POLEDOWN ||
			        lara_item->current_anim_state == AS_POLELEFT ||
			        lara_item->current_anim_state == AS_POLERIGHT);
			break;
		}
		case 16: {
			return (lara.gun_type == WEAPON_TORCH);
			break;
		}
		case 17: {
			if (lara.vehicle != NO_ITEM)
				return (T4PlusGetItemInfoForID(lara.vehicle)->object_number == KAYAK);
			break;
		}
		default: {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid HOLD_ id %u.", hold_type);
			return false;
		}
	}

	return false;
}

bool NGProcessGlobalTriggers(int32_t selected_inventory_object_id) {
	bool management_replaced = false;
	if (ng_levels[gfCurrentLevel].records) {
		int32_t global_trigger_count = ng_levels[gfCurrentLevel].records->global_trigger_count;
		for (int32_t i = 0; i < global_trigger_count; i++) {
			if (NGExecuteSingleGlobalTrigger(i, selected_inventory_object_id)) {
				management_replaced = true;
			}
		}
	}

	return management_replaced;
}

void NGFrameStartExtraState() {
	//ng_triggered_items_for_timerfield_count = 0;

	if (pending_level_load_timer >= 0) {
		if (pending_level_load_timer == 0) {
			gfLevelComplete = pending_level_load_id;
			gfRequiredStartPos = 0;
		}
		pending_level_load_timer--;
	}

	// If a timer has reached zero, reset its incrementation value
	if (ng_local_timer <= 0)
		ng_local_timer_frame_increment = 0;
	if (ng_global_timer <= 0)
		ng_global_timer_frame_increment = 0;

	NGClearLaraCollisions();

	if (ng_cinema_timer > 0 || ng_cinema_type != 0) {
		switch (ng_cinema_type) {
			case -1: { // Fullscreen
				SetFadeClipImmediate(150);
				break;
			}
			case 0: {
				SetFadeClipImmediate(10);
				break;
			}
			case 1: { // Tiny
				SetFadeClipImmediate(20);
				break;
			}
			case 2: { // Middle
				SetFadeClipImmediate(30);
				break;
			}
			case 3: { // Big
				SetFadeClipImmediate(40);
				break;
			}
			case 4: { // Huge
				SetFadeClipImmediate(60);
				break;
			}
			case 5: { // Fissure
				SetFadeClipImmediate(90);
				break;
			}
			default: {
				SetFadeClipImmediate(10);
				break;
			}
		}
		ng_cinema_timer--;
		if (ng_cinema_timer == 0) {
			ng_cinema_type = 0;
			SetFadeClip(0, 1);
			ng_cinema_timer--;
		}
	}

	// Looping sounds
	for (int32_t i = 0; i < NumSamples; i++) {
		if (ng_looped_sound_state[i] > 0) {
			SoundEffect(i, NULL, SFX_ALWAYS);
			ng_looped_sound_state[i] -= 1;
		}
	}

	// Cold and damage rooms
	{
		ROOM_INFO* r = &room[lara_item->room_number];
		if (r) {
			// Lara is in a damage room.
			if (T4PlusIsRoomDamage(r)) {
				// TODO
			} else {

			}

			// Lara is in a cold room.
			if (T4PlusIsRoomCold(r)) {
				// TODO
			} else {

			}
		}
	}

	// Timers
	ng_global_timer += ng_global_timer_frame_increment;
	if (ng_global_timer < 0)
		ng_global_timer = 0;

	if (ng_global_timer_time_until_hide > 0)
		ng_global_timer_time_until_hide--;

	ng_local_timer += ng_local_timer_frame_increment;
	if (ng_local_timer < 0)
		ng_local_timer = 0;

	if (ng_local_timer_time_until_hide > 0)
		ng_local_timer_time_until_hide--;
}

void NGDrawTimer(int32_t timer, NGTimerPosition timer_position, int32_t timer_time_until_hide) {
	if (timer_time_until_hide != 0) {
		if (timer_position != NG_TIMER_POSITION_INVISIBLE) {
			char format_buffer[80];
			int32_t remainder = timer % 30;
			int32_t seconds = timer / 30;

			sprintf(format_buffer, "%.2d:%.2d:%.1d", seconds / 60, seconds % 60, (334 * (remainder)) / 1000);

			// TODO: the scaling of the text is not correct at all
			switch (timer_position) {
				case NG_TIMER_POSITION_BOTTOM_CENTER:
					PrintString(phd_centerx, int32_t(phd_winymax - font_height * 0.25), 0, format_buffer, FF_CENTER);
					break;
				case NG_TIMER_POSITION_TOP_CENTER:
					PrintString(phd_centerx, font_height, 0, format_buffer, FF_CENTER);
					break;
				case NG_TIMER_POSITION_CENTER_CENTER:
					PrintString(phd_centerx, int32_t(phd_centery - font_height * 0.5), 0, format_buffer, FF_CENTER);
					break;
				case NG_TIMER_POSITION_TOP_LEFT:
					PrintString(0, font_height, 0, format_buffer, 0);
					break;
				case NG_TIMER_POSITION_TOP_RIGHT:
					PrintString(phd_winxmax, font_height, 0, format_buffer, FF_RJUSTIFY);
					break;
				case NG_TIMER_POSITION_BOTTOM_LEFT:
					PrintString(0, int32_t(phd_winymax - font_height * 0.25), 0, format_buffer, 0);
					break;
				case NG_TIMER_POSITION_BOTTOM_RIGHT:
					PrintString(phd_winxmax, int32_t(phd_winymax - font_height * 0.25), 0, format_buffer, FF_RJUSTIFY);
					break;
				case NG_TIMER_POSITION_DOWN_DAMAGE_BAR:
				case NG_TIMER_POSITION_DOWN_COLD_BAR:
					PrintString(phd_centerx, int32_t(phd_winymax - font_height * 0.25), 0, format_buffer, FF_CENTER);
					break;
				case NG_TIMER_POSITION_DOWN_LEFT_BARS:
					PrintString(0, font_height, 0, format_buffer, 0);
					break;
				case NG_TIMER_POSITION_DOWN_RIGHT_BARS:
					PrintString(phd_winxmax, font_height, 0, format_buffer, FF_RJUSTIFY);
					break;
				default:
					break;
			}
		}
	}
}

void NGDrawPhase() {
	if (timer_tracker >= 0) {
		ITEM_INFO *time_tracker_item = T4PlusGetItemInfoForID(timer_tracker);
		if (time_tracker_item) {
			if (time_tracker_item->timer <= 0) {
				if (timer_tracker_remaining_until_timeout > 0)
					timer_tracker_remaining_until_timeout--;
			}

			if (time_tracker_item->timer > 0 || timer_tracker_remaining_until_timeout > 0) {
				char format_buffer[64];
				int32_t current_timer = time_tracker_item->timer;
				if (current_timer < 0)
					current_timer = 0;

				int32_t remainder = (current_timer % 30) * (100 / 30);
				int32_t seconds = current_timer / 30;

				switch (timer_tracker_type) {
					case TTT_ONLY_SHOW_SECONDS:
						sprintf(format_buffer, "%d", seconds);
						break;
					case TTT_SECONDS_AND_ONE_DECIMAL_POINT_SEPERATOR:
						sprintf(format_buffer, "%d.%01d", seconds, remainder);
						break;
					case TTT_SECONDS_AND_TWO_DECIMAL_POINT_SEPERATOR:
						sprintf(format_buffer, "%d.%02d", seconds, remainder);
						break;
					case TTT_SECONDS_AND_ONE_DECIMAL_COLON_SEPERATOR:
						sprintf(format_buffer, "%d:%01d", seconds, remainder);
						break;
					case TTT_SECONDS_AND_TWO_DECIMAL_COLON_SEPERATOR:
						sprintf(format_buffer, "%d:%02d", seconds, remainder);
						break;
					case TTT_SECONDS_WITH_THREE_NOUGHTS:
						sprintf(format_buffer, "%03d", seconds);
						break;
					default:
						sprintf(format_buffer, "%d", time_tracker_item->timer);
						break;
				}
				if (NGGetDrawState() != NG_DRAW_STATE_BLANK) {
					PrintString(phd_centerx, int32_t(phd_winymax - font_height * 0.25), 0, format_buffer, FF_CENTER);
				}
			}
		}
	}

	// Timers
	if (NGGetDrawState() != NG_DRAW_STATE_BLANK) {
		NGDrawTimer(ng_local_timer, ng_local_timer_position, ng_local_timer_time_until_hide);
		NGDrawTimer(ng_global_timer, ng_global_timer_position, ng_global_timer_time_until_hide);
	}
}

bool NGIsItemFrozen(uint32_t item_num) {
	if ((uint16_t)(ng_items_extradata[item_num].frozen_ticks) > 0) {
		return true;
	}

	return false;
}

int32_t NGGetItemFrozenTimer(uint32_t item_num) {
	return ng_items_extradata[item_num].frozen_ticks;
}

void NGSetItemFreezeTimer(uint32_t item_num, uint32_t ticks) {
	ng_items_extradata[item_num].frozen_ticks = ticks;
}

// Items

void NGSetItemHorizontalMovementAngle(uint32_t item_num, int16_t angle) {
	ng_items_extradata[item_num].movement_info.move_horizontal_angle = angle;
}

int16_t NGGetItemHorizontalMovementAngle(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.move_horizontal_angle;
}

int32_t NGGetItemHorizontalMovementRemainingUnits(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.move_horizontal_remaining_units;
}

void NGSetItemHorizontalMovementRemainingUnits(uint32_t item_num, int32_t units) {
	ng_items_extradata[item_num].movement_info.move_horizontal_remaining_units = units;
}

int32_t NGGetItemVerticalMovementRemainingUnits(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.move_vertical_remaining_units;
}

void NGSetItemVerticalMovementRemainingUnits(uint32_t item_num, int32_t units) {
	ng_items_extradata[item_num].movement_info.move_vertical_remaining_units = units;
}

int32_t NGGetItemHorizontalMovementRepeatUnits(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.move_horizontal_repeat_units;
}

void NGSetItemHorizontalMovementRepeatUnits(uint32_t item_num, int32_t units) {
	ng_items_extradata[item_num].movement_info.move_horizontal_repeat_units = units;
}

int32_t NGGetItemVerticalMovementRepeatUnits(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.move_vertical_repeat_units;
}

void NGSetItemVerticalMovementRepeatUnits(uint32_t item_num, int32_t units) {
	ng_items_extradata[item_num].movement_info.move_vertical_repeat_units = units;
}

int32_t NGGetItemHorizontalMovementSpeed(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.horizontal_movement_speed;
}

void NGSetItemHorizontalMovementSpeed(uint32_t item_num, uint32_t movement_speed) {
	ng_items_extradata[item_num].movement_info.horizontal_movement_speed = movement_speed;
}

int32_t NGGetItemVerticalMovementSpeed(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.vertical_movement_speed;
}

void NGSetItemVerticalMovementSpeed(uint32_t item_num, uint32_t movement_speed) {
	ng_items_extradata[item_num].movement_info.vertical_movement_speed = movement_speed;
}

int32_t NGGetItemMovementInProgressSound(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.movement_in_progress_sound;
}

void NGSetItemMovementInProgressSound(uint32_t item_num, int32_t sound_effect_id) {
	ng_items_extradata[item_num].movement_info.movement_in_progress_sound = sound_effect_id;
}

int32_t NGGetItemMovementFinishedSound(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.movement_finished_sound;
}

void NGSetItemMovementFinishedSound(uint32_t item_num, int32_t sound_effect_id) {
	ng_items_extradata[item_num].movement_info.movement_finished_sound = sound_effect_id;
}

bool NGGetItemMovementTriggerHeavyAtEnd(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.trigger_heavy_at_end;
}

void NGSetItemMovementTriggerHeavyAtEnd(uint32_t item_num, bool trigger_heavy_at_end) {
	ng_items_extradata[item_num].movement_info.trigger_heavy_at_end = trigger_heavy_at_end;
}

bool NGGetItemMovementTriggerNormalWhenMoving(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.trigger_normal_when_moving;
}

void NGSetItemMovementTriggerNormalWhenMoving(uint32_t item_num, bool trigger_normal_when_moving) {
	ng_items_extradata[item_num].movement_info.trigger_normal_when_moving = trigger_normal_when_moving;
}

bool NGGetItemMovementTriggerHeavyWhenMoving(uint32_t item_num) {
	return ng_items_extradata[item_num].movement_info.trigger_heavy_when_moving;
}

void NGSetItemMovementTriggerHeavyWhenMoving(uint32_t item_num, bool trigger_heavy_when_moving) {
	ng_items_extradata[item_num].movement_info.trigger_heavy_when_moving = trigger_heavy_when_moving;
}


// Statics

//
void NGSetStaticHorizontalMovementAngle(uint32_t static_num, int16_t angle) {
	ng_statics_movement_info[static_num].move_horizontal_angle = angle;
}

int16_t NGGetStaticHorizontalMovementAngle(uint32_t static_num) {
	return ng_statics_movement_info[static_num].move_horizontal_angle;
}

int32_t NGGetStaticHorizontalMovementRemainingUnits(uint32_t static_num) {
	return ng_statics_movement_info[static_num].move_horizontal_remaining_units;
}

void NGSetStaticHorizontalMovementRemainingUnits(uint32_t static_num, int32_t units) {
	ng_statics_movement_info[static_num].move_horizontal_remaining_units = units;
}

int32_t NGGetStaticVerticalMovementRemainingUnits(uint32_t static_num) {
	return ng_statics_movement_info[static_num].move_vertical_remaining_units;
}

void NGSetStaticVerticalMovementRemainingUnits(uint32_t static_num, int32_t units) {
	ng_statics_movement_info[static_num].move_vertical_remaining_units = units;
}

int32_t NGGetStaticHorizontalMovementRepeatUnits(uint32_t static_num) {
	return ng_statics_movement_info[static_num].move_horizontal_repeat_units;
}

void NGSetStaticHorizontalMovementRepeatUnits(uint32_t static_num, int32_t units) {
	ng_statics_movement_info[static_num].move_horizontal_repeat_units = units;
}

int32_t NGGetStaticVerticalMovementRepeatUnits(uint32_t static_num) {
	return ng_statics_movement_info[static_num].move_vertical_repeat_units;
}

void NGSetStaticVerticalMovementRepeatUnits(uint32_t static_num, int32_t units) {
	ng_statics_movement_info[static_num].move_vertical_repeat_units = units;
}

int32_t NGGetStaticHorizontalMovementSpeed(uint32_t static_num) {
	return ng_statics_movement_info[static_num].horizontal_movement_speed;
}

void NGSetStaticHorizontalMovementSpeed(uint32_t static_num, uint32_t movement_speed) {
	ng_statics_movement_info[static_num].horizontal_movement_speed = movement_speed;
}

int32_t NGGetStaticVerticalMovementSpeed(uint32_t static_num) {
	return ng_statics_movement_info[static_num].vertical_movement_speed;
}

void NGSetStaticVerticalMovementSpeed(uint32_t static_num, uint32_t movement_speed) {
	ng_statics_movement_info[static_num].vertical_movement_speed = movement_speed;
}

int32_t NGGetStaticMovementInProgressSound(uint32_t static_num) {
	return ng_statics_movement_info[static_num].movement_in_progress_sound;
}

void NGSetStaticMovementInProgressSound(uint32_t static_num, int32_t sound_effect_id) {
	ng_statics_movement_info[static_num].movement_in_progress_sound = sound_effect_id;
}

int32_t NGGetStaticMovementFinishedSound(uint32_t static_num) {
	return ng_statics_movement_info[static_num].movement_finished_sound;
}

void NGSetStaticMovementFinishedSound(uint32_t static_num, int32_t sound_effect_id) {
	ng_statics_movement_info[static_num].movement_finished_sound = sound_effect_id;
}

bool NGGetStaticMovementTriggerHeavyAtEnd(uint32_t static_num) {
	return ng_statics_movement_info[static_num].trigger_heavy_at_end;
}

void NGSetStaticMovementTriggerHeavyAtEnd(uint32_t static_num, bool trigger_heavy_at_end) {
	ng_statics_movement_info[static_num].trigger_heavy_at_end = trigger_heavy_at_end;
}

bool NGGetStaticMovementTriggerNormalWhenMoving(uint32_t static_num) {
	return ng_statics_movement_info[static_num].trigger_normal_when_moving;
}

void NGSetStaticMovementTriggerNormalWhenMoving(uint32_t static_num, bool trigger_normal_when_moving) {
	ng_statics_movement_info[static_num].trigger_normal_when_moving = trigger_normal_when_moving;
}

bool NGGetStaticMovementTriggerHeavyWhenMoving(uint32_t static_num) {
	return ng_statics_movement_info[static_num].trigger_heavy_when_moving;
}

void NGSetStaticMovementTriggerHeavyWhenMoving(uint32_t static_num, bool trigger_heavy_when_moving) {
	ng_statics_movement_info[static_num].trigger_heavy_when_moving = trigger_heavy_when_moving;
}

//

bool NGIsItemCollisionDisabled(uint32_t item_num) {
	return ng_items_extradata[item_num].collison_disabled;
}

void NGDisableItemCollision(uint32_t item_num) {
	ng_items_extradata[item_num].collison_disabled = true;
}

void NGEnableItemCollision(uint32_t item_num) {
	ng_items_extradata[item_num].collison_disabled = false;
}

void NGToggleItemMeshVisibilityMaskBit(uint32_t item_num, uint32_t mask_bit, bool enabled) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_num);
	if (!item) {
		return;
	}

	if (enabled)
		item->mesh_bits |= (1 << mask_bit);
	else
		item->mesh_bits &= ~(1 << mask_bit);
}

uint32_t NGGetItemMeshVisibilityMask(uint32_t item_num) {
	ITEM_INFO* item = T4PlusGetItemInfoForID(item_num);
	if (!item) {
		return 0;
	}

	return item->mesh_bits;
}

void NGSetFullscreenCurtainTimer(int32_t ticks) {
	if (ng_cinema_timer <= 0) {
		ng_cinema_type = -1;
		ng_cinema_timer = ticks;
	}
}

void NGSetCinemaTypeAndTimer(int32_t type, int32_t ticks) {
	ng_cinema_type = type;
	ng_cinema_timer = ticks;
	if (ng_cinema_type == 0 && ng_cinema_timer == 0) {
		SetFadeClip(0, 1);
	}
}

void NGToggleOrganizer(int32_t organizer_id, bool is_enabled) {
	ng_organizer_states[organizer_id].is_enabled = is_enabled;
}

bool NGIsOrganizerEnabled(int32_t organizer_id) {
	return ng_organizer_states[organizer_id].is_enabled;
}

void NGResetOrganizer(int32_t organizer_id) {
	ng_organizer_states[organizer_id].current_tick = 0;
}

extern bool NGIsTriggerGroupContinuous(int32_t trigger_group_id) {
	return ng_trigger_group_states[trigger_group_id].continuous;
}

extern void NGSetTriggerGroupContinuous(int32_t trigger_group_id, bool is_continuous) {
	ng_trigger_group_states[trigger_group_id].continuous = is_continuous;
}

void NGSetDisplayTimerForMoveableWithType(int32_t item_id, NGTimerTrackerType new_timer_tracker_type) {
	timer_tracker = item_id;
	timer_tracker_type = new_timer_tracker_type;
#if 0
	if (items[timer_tracker].timer > 0) {
		timer_tracker_remaining_until_timeout = 0;
	} else {
		timer_tracker_remaining_until_timeout = TIMER_TRACKER_TIMEOUT;
	}
#endif
}

extern void NGSetFadeOverride(int32_t item_id, int16_t fade_override) {
	ng_items_extradata[item_id].fade_override = fade_override;
}

extern int16_t NGGetFadeOverride(int32_t item_id) {
	return ng_items_extradata[item_id].fade_override;
}

int32_t ng_draw_item_number = NO_ITEM;

void NGSetCurrentDrawItemNumber(int32_t item_num) {
	ng_draw_item_number = item_num;
}

int32_t NGGetCurrentDrawItemNumber() {
	return ng_draw_item_number;
}

void NGSetupLevelExtraState() {
	ng_camera_target_id = NO_ITEM;

	ng_draw_item_number = NO_ITEM;
	ng_lara_infinite_air = false;

	// Animation
	ng_animation_current_animation = -1;
	ng_animation_prev_hands_state = LG_NO_ARMS;
	ng_animation_target_item = -1;
	ng_animation_target_test_position = -1;

	// Timers
	ng_global_timer = 0;
	ng_global_timer_frame_increment = 0;
	ng_global_timer_position = NG_TIMER_POSITION_INVISIBLE;
	ng_global_timer_time_until_hide = 0;

	ng_local_timer = 0;
	ng_local_timer_frame_increment = 0;
	ng_local_timer_position = NG_TIMER_POSITION_INVISIBLE;
	ng_local_timer_time_until_hide = 0;

	// Level
	pending_level_load_timer = -1;
	pending_level_load_id = 0;

	// Variables
	ng_current_value = 0;
	ng_global_alfa = 0;
	ng_global_beta = 0;
	ng_global_delta = 0;
	ng_local_alfa = 0;
	ng_local_beta = 0;
	ng_local_delta = 0;
	ng_last_input_number = 0;

	for (int32_t i = 0; i < STORE_VARIABLE_COUNT; i++) {
		ng_store_variables[i] = 0;
	}

	memset(ng_last_text_input, 0x00, REGULAR_TEXT_BUFFER_SIZE);

	memset(ng_string1, 0x00, REGULAR_TEXT_BUFFER_SIZE);
	memset(ng_string2, 0x00, REGULAR_TEXT_BUFFER_SIZE);
	memset(ng_string3, 0x00, REGULAR_TEXT_BUFFER_SIZE);
	memset(ng_string4, 0x00, REGULAR_TEXT_BUFFER_SIZE);

	memset(ng_text_big, 0x00, BIG_TEXT_BUFFER_SIZE);

	// Visual
	ng_drawing_state = NG_DRAW_STATE_ACTIVE;

	// Inventory
	ng_selected_inventory_item_memory = 0;
	ng_used_inventory_object_for_frame = NO_ITEM;
	ng_used_large_medipack = false;
	ng_used_small_medipack = false;

	// Timer Trackers
	timer_tracker_type = TTT_ONLY_SHOW_SECONDS;
	timer_tracker = -1;
	timer_tracker_remaining_until_timeout = 0;

	// Items
	ng_items_extradata = (NG_ITEM_EXTRADATA*)game_malloc(ITEM_COUNT * sizeof(NG_ITEM_EXTRADATA));
	for (int32_t i = 0; i < ITEM_COUNT; i++) {
		NGResetItemExtraData(i);
	}

	// Statics
	ng_statics_movement_info = (NG_MOVEMENT_INFO*)game_malloc(NG_STATIC_ID_TABLE_SIZE * sizeof(NG_MOVEMENT_INFO));
	for (int32_t i = 0; i < NG_STATIC_ID_TABLE_SIZE; i++) {
		NGResetMovementInfo(&ng_statics_movement_info[i]);
	}

	// Records
	NG_LEVEL_RECORD_DATA *current_record_data = ng_levels[gfCurrentLevel].records;

	// Global triggers
	{
		for (int32_t i = 0; i < MAX_NG_GLOBAL_TRIGGERS; i++) {
			ng_global_trigger_states[i].is_disabled = false;
			ng_global_trigger_states[i].is_halted = false;
		}

		if (current_record_data) {
			int32_t global_trigger_count = current_record_data->global_trigger_count;
			for (int32_t i = 0; i < global_trigger_count; i++) {
				NG_GLOBAL_TRIGGER* global_trigger = &current_record_data->global_trigger_table[i].record;
				int32_t record_id = current_record_data->global_trigger_table[i].record_id;
				if (global_trigger->flags != 0xffff) {
					if (global_trigger->flags & FGT_DISABLED) {
						ng_global_trigger_states[record_id].is_disabled = true;
					}
				}
			}
		}
	}

	// Trigger groups
	{
		for (int32_t i = 0; i < MAX_NG_TRIGGER_GROUPS; i++) {
			ng_trigger_group_states[i].continuous = false;
			ng_trigger_group_states[i].one_shot = false;
		}
	}

	// Organizers
	{
		for (int32_t i = 0; i < MAX_NG_ORGANIZERS; i++) {
			ng_organizer_states[i].is_enabled = false;
			ng_organizer_states[i].current_tick = 0;
		}

		if (current_record_data) {
			int32_t organizer_count = current_record_data->organizer_count;
			for (int32_t i = 0; i < organizer_count; i++) {
				NG_ORGANIZER* organizer = &current_record_data->organizer_table[i].record;
				int32_t record_id = current_record_data->organizer_table[i].record_id;
				if (organizer->flags != 0xffff) {
					// FO_ENABLED
					if (organizer->flags & 0x0001) {
						ng_organizer_states[record_id].is_enabled = true;
					}
				}
			}
		}
	}

	// Cinema
	ng_cinema_timer = -1;
	ng_cinema_type = 0;


	// Input lock and simulator
	ng_input_to_simulate = 0;
	ng_input_to_disable = 0;
	ng_single_input_to_simulate = 0;

	// Looped samples
	memset(ng_looped_sound_state, 0x00, NumSamples * sizeof(int32_t));

	ng_lara_moveable_collision_size = 0;
	ng_lara_static_collision_size = 0;

	// Damage
	lara_damage_resistence = 1000;

	// Inventory
	ng_used_inventory_object_for_frame = NO_ITEM;
	ng_used_large_medipack = false;
	ng_used_small_medipack = false;
}

void NGFrameFinishExtraState() {
	// Savegame
	ng_loaded_savegame = false;
}