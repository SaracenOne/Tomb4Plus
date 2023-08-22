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
#include "../text.h"
#include "trng.h"
#include "trng_extra_state.h"
#include "trng_flipeffect.h"
#include "trng_script_parser.h"
#include "../sound.h"

unsigned int ng_room_offset_table[0xff];

// NG_ITEM_EXTRADATA is persistent supllementary data used by TRNG triggers.
// The state here can subseqeuently be serialized as additional data for savegames.
struct NG_ITEM_EXTRADATA {
	short frozen_ticks = 0;
	short auto_rotation_per_frame = 0;
	bool collison_disabled = false; // Will only disable the ObjectCollision routine. Doors and enemies stll have collision.
	unsigned short movement_speed = 0;
	short movement_in_progress_sound = -1;
	short movement_finished_sound = -1;
	short move_north_south_units = 0;
	short move_east_west_units = 0;
	short move_up_down_units = 0;
};

NG_ITEM_EXTRADATA *ng_items_extradata = NULL;

struct NG_GLOBAL_TRIGGER_STATE {
	bool is_disabled = false;
	bool is_halted = false;
};

NG_GLOBAL_TRIGGER_STATE ng_global_trigger_states[MAX_NG_GLOBAL_TRIGGERS];

struct NG_TRIGGER_GROUP_STATE {
	bool continuous = false;
	bool one_shot = false;
};

NG_TRIGGER_GROUP_STATE ng_trigger_group_states[MAX_NG_TRIGGER_GROUPS];

struct NG_ORGANIZER_STATE {
	bool is_enabled = false;
	int current_tick = 0;
};

NG_ORGANIZER_STATE ng_organizer_states[MAX_NG_ORGANIZERS];

// TODO: In the original, there's some behaviour which allows multiple timers to run
// at once, displaying the last activated on until it runs out. Needs investigation.
#define TIMER_TRACKER_TIMEOUT 30
NGTimerTrackerType timer_tracker_type = TTT_ONLY_SHOW_SECONDS;
int timer_tracker = -1;
int timer_tracker_remaining_until_timeout = 0;

int ng_cinema_timer = -1;
int ng_cinema_type = 0;

struct TriggerState {
	int index = -1;
	int room = -1;
};

#define BACKUP_TRIGGER_STATE_COUNT 2
TriggerState ng_backup_trigger_state[BACKUP_TRIGGER_STATE_COUNT];
int ng_backup_trigger_state_count = 0;

TriggerState ng_current_trigger_state;

int pending_ng_room = -1;

int ng_floorstate_data_size = 0;
char *ng_oneshot_floorstate = NULL;

int ng_last_flipeffect_floor_trigger = -1;
int ng_current_flipeffect_floor_trigger = -1;

int ng_last_action_floor_trigger = -1;
int ng_current_action_floor_trigger = -1;

int ng_heavy_last_flipeffect_floor_trigger = -1;
int ng_heavy_current_flipeffect_floor_trigger = -1;

int ng_heavy_last_action_floor_trigger = -1;
int ng_heavy_current_action_floor_trigger = -1;

int lara_damage_resistence = 1000;

// Variables
int current_value = 0;
int global_alfa = 0;
int global_beta = 0;
int global_delta = 0;
int global_timer = 0;
int local_alfa = 0;
int local_beta = 0;
int local_delta = 0;
int local_timer = 0;
int last_input_number = 0;

enum TRNG_INPUT {
	TRNG_INPUT_UP,
	TRNG_INPUT_DOWN,
	TRNG_INPUT_LEFT,
	TRNG_INPUT_RIGHT,
	TRNG_INPUT_DUCK,
	TRNG_INPUT_DASH,
	TRNG_INPUT_WALK,
	TRNG_INPUT_JUMP,
	TRNG_INPUT_ACTION,
	TRNG_INPUT_DRAW_WEAPON,
	TRNG_INPUT_USE_FLARE,
	TRNG_INPUT_LOOK,
	TRNG_INPUT_ROLL,
	TRNG_INVENTORY_AND_DESELECT,
	TRNG_STEP_LEFT,
	TRNG_STEP_RIGHT,
	TRNG_PAUSE,
	TRNG_SAVE_GAME,
	TRNG_LOAD_GAME,
	TRNG_WEAPON_KEYS,
	TRNG_INPUT_COUNT
};

int ng_looped_sound_state[NumSamples];

#define NG_INPUT_LOCK_TIMER_COUNT TRNG_INPUT_COUNT
int ng_input_lock_timers[NG_INPUT_LOCK_TIMER_COUNT];

void NGStorePendingRoomNumber(int room_number) {
	pending_ng_room = room_number;
}

int NGRestorePendingRoomNumber() {
	return pending_ng_room;
}

void NGUpdateCurrentTriggerRoomAndIndex(int new_room, int new_index) {
	ng_current_trigger_state.index = new_index;
	ng_current_trigger_state.room = new_room;
}

void NGClearCurrentTriggerRoomAndIndex() {
	ng_current_trigger_state.index = -1;
	ng_current_trigger_state.room = -1;
}

// These may be needed since Lara's trigger index derived from earlier may get overwritten by something.
void NGStoreBackupTriggerRoomAndIndex() {
	ng_backup_trigger_state[ng_backup_trigger_state_count].index = ng_current_trigger_state.index;
	ng_backup_trigger_state[ng_backup_trigger_state_count].room = ng_current_trigger_state.room;

	ng_backup_trigger_state_count++;
	if (ng_backup_trigger_state_count >= BACKUP_TRIGGER_STATE_COUNT) {
		NGLog(NG_LOG_TYPE_ERROR, "NGStoreBackupTriggerRoomAndIndex: Overflow!");
		exit(-1);
	}
}

void NGRestoreBackupTriggerRoomAndIndex() {
	ng_current_trigger_state.index = ng_backup_trigger_state[ng_backup_trigger_state_count - 1].index;
	ng_current_trigger_state.room = ng_backup_trigger_state[ng_backup_trigger_state_count - 1].room;

	ng_backup_trigger_state_count--;
	if (ng_backup_trigger_state_count < 0) {
		NGLog(NG_LOG_TYPE_ERROR, "NGRestoreBackupTriggerRoomAndIndex: Underflow!");
		exit(-1);
	}
}

bool NGIsOneShotTriggeredForTile() {
	int index = ng_room_offset_table[ng_current_trigger_state.room] + ng_current_trigger_state.index;

	bool result = ng_oneshot_floorstate[index];
	if (!result)
		printf("");

	return result;
}

// This method is not accurate since it seems like rollingballs can interrupted the check.
bool NGCheckFlipeffectFloorStatePressedThisFrameOrLastFrame(bool heavy) {
	int index = ng_room_offset_table[ng_current_trigger_state.room] + ng_current_trigger_state.index;

	if (!heavy) {
		if (ng_current_flipeffect_floor_trigger == index || ng_last_flipeffect_floor_trigger == index)
			return true;
	} else {
		if (ng_heavy_current_flipeffect_floor_trigger == index || ng_heavy_last_flipeffect_floor_trigger == index)
			return true;
	}

	return false;
}

extern bool NGCheckActionFloorStatePressedThisFrameOrLastFrame(bool heavy) {
	int index = ng_room_offset_table[ng_current_trigger_state.room] + ng_current_trigger_state.index;

	if (!heavy) {
		if (ng_current_action_floor_trigger == index || ng_last_action_floor_trigger == index)
			return true;
	} else {
		//if (ng_heavy_current_action_floor_trigger == index || ng_heavy_last_action_floor_trigger == index)
		//	return true;
	}

	return false;
}

int NGValidateInputAgainstLockTimers(int input) {
	for (int i = 0; i < TRNG_SAVE_GAME; i++) {
		if (ng_input_lock_timers[i] != 0) {
			switch (i) {
				case TRNG_INPUT_UP:
					input &= ~IN_FORWARD;
					break;
				case TRNG_INPUT_DOWN:
					input &= ~IN_BACK;
					break;
				case TRNG_INPUT_LEFT:
					input &= ~IN_LEFT;
					break;
				case TRNG_INPUT_RIGHT:
					input &= ~IN_RIGHT;
					break;
				case TRNG_INPUT_DUCK:
					input &= ~IN_DUCK;
					break;
				case TRNG_INPUT_DASH:
					input &= ~IN_SPRINT;
					break;
				case TRNG_INPUT_WALK:
					input &= ~(IN_WALK | IN_LSTEP | IN_RSTEP); // TRNG bug?
					break;
				case TRNG_INPUT_JUMP:
					input &= ~IN_JUMP;
					break;
				case TRNG_INPUT_ACTION:
					input &= ~(IN_ACTION | IN_SELECT);
					break;
				case TRNG_INPUT_DRAW_WEAPON:
					input &= ~IN_DRAW;
					break;
				case TRNG_INPUT_USE_FLARE:
					input &= ~IN_FLARE;
					break;
				case TRNG_INPUT_LOOK:
					input &= ~IN_LOOK;
					break;
				case TRNG_INPUT_ROLL:
					input &= ~IN_ROLL;
					break;
				case TRNG_INVENTORY_AND_DESELECT:
					input &= ~(IN_OPTION | IN_DESELECT);
					break;
				case TRNG_STEP_LEFT:
					input &= ~(IN_WALK | IN_LSTEP | IN_RSTEP); // TRNG bug?
					break;
				case TRNG_STEP_RIGHT:
					input &= ~(IN_WALK | IN_LSTEP | IN_RSTEP); // TRNG bug?
					break;
				case TRNG_PAUSE:
					input &= ~IN_PAUSE;
					break;
				default:
					NGLog(NG_LOG_TYPE_ERROR, "Invalid input type %u!", i);
					break;
			}
		}
	}

	return input;
}

bool NGValidateInputSavegame() {
	return ng_input_lock_timers[TRNG_SAVE_GAME] == 0;
}

bool NGValidateInputLoadgame() {
	return ng_input_lock_timers[TRNG_LOAD_GAME] == 0;
}

bool NGValidateInputWeaponHotkeys() {
	return ng_input_lock_timers[TRNG_WEAPON_KEYS] == 0;
}

void NGDisableInputForTime(unsigned char input, int ticks) {
	if (input > NG_INPUT_LOCK_TIMER_COUNT) {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid input type %u!", input);
		return;
	}

	int final_ticks = -1;
	if (ticks > 0) {
		final_ticks = ticks;
	}

	if (input == 0) {
		for (int i = 0; i < NG_INPUT_LOCK_TIMER_COUNT; i++) {
			ng_input_lock_timers[i] = final_ticks;
		}
	} else {
		ng_input_lock_timers[input - 1] = final_ticks;
	}
}

void NGEnableInput(unsigned char input) {
	if (input == 0) {
		for (int i = 0; i < NG_INPUT_LOCK_TIMER_COUNT; i++) {
			ng_input_lock_timers[i] = 0;
		}
	} else {
		ng_input_lock_timers[input - 1] = 0;
	}
}

void NGUpdateAllItems() {
	for (int i = 0; i < ITEM_COUNT; i++) {
		if (ng_items_extradata[i].frozen_ticks > 0) {
			ng_items_extradata[i].frozen_ticks--;
		}

		NGRotateItemY(i, NGGetAutoRotationPerFrame(i));

		if (ng_items_extradata[i].move_north_south_units > 0) {
			int move_by_amount = ng_items_extradata[i].movement_speed;
			if (move_by_amount > ng_items_extradata[i].move_north_south_units)
				move_by_amount = ng_items_extradata[i].move_north_south_units;

			NGMoveItemByUnits(i, NG_NORTH, move_by_amount);
			ng_items_extradata[i].move_north_south_units -= move_by_amount;

			if (move_by_amount == 0) {
				if (ng_items_extradata[i].movement_finished_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_finished_sound, &items[i].pos, 0);
			} else {
				if (ng_items_extradata[i].movement_in_progress_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_in_progress_sound, &items[i].pos, 0);
			}
		}
		else if (ng_items_extradata[i].move_north_south_units < 0) {
			int move_by_amount = -ng_items_extradata[i].movement_speed;
			if (move_by_amount < ng_items_extradata[i].move_north_south_units)
				move_by_amount = ng_items_extradata[i].move_north_south_units;

			NGMoveItemByUnits(i, NG_NORTH, move_by_amount);
			ng_items_extradata[i].move_north_south_units -= move_by_amount;

			if (move_by_amount == 0) {
				if (ng_items_extradata[i].movement_finished_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_finished_sound, &items[i].pos, 0);
			} else {
				if (ng_items_extradata[i].movement_in_progress_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_in_progress_sound, &items[i].pos, 0);
			}
		}

		if (ng_items_extradata[i].move_east_west_units > 0) {
			int move_by_amount = ng_items_extradata[i].movement_speed;
			if (move_by_amount > ng_items_extradata[i].move_east_west_units)
				move_by_amount = ng_items_extradata[i].move_east_west_units;

			NGMoveItemByUnits(i, NG_EAST, move_by_amount);
			ng_items_extradata[i].move_east_west_units -= move_by_amount;

			if (move_by_amount == 0) {
				if (ng_items_extradata[i].movement_finished_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_finished_sound, &items[i].pos, 0);
			} else {
				if (ng_items_extradata[i].movement_in_progress_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_in_progress_sound, &items[i].pos, 0);
			}
		}
		else if (ng_items_extradata[i].move_east_west_units < 0) {
			int move_by_amount = -ng_items_extradata[i].movement_speed;
			if (move_by_amount < ng_items_extradata[i].move_east_west_units)
				move_by_amount = ng_items_extradata[i].move_east_west_units;

			NGMoveItemByUnits(i, NG_EAST, move_by_amount);
			ng_items_extradata[i].move_east_west_units -= move_by_amount;

			if (move_by_amount == 0) {
				if (ng_items_extradata[i].movement_finished_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_finished_sound, &items[i].pos, 0);
			} else {
				if (ng_items_extradata[i].movement_in_progress_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_in_progress_sound, &items[i].pos, 0);
			}
		}

		if (ng_items_extradata[i].move_up_down_units > 0) {
			int move_by_amount = ng_items_extradata[i].movement_speed;
			if (move_by_amount > ng_items_extradata[i].move_up_down_units)
				move_by_amount = ng_items_extradata[i].move_up_down_units;


			NGMoveItemByUnits(i, NG_UP, move_by_amount);
			ng_items_extradata[i].move_up_down_units -= move_by_amount;

			if (move_by_amount == 0) {
				if (ng_items_extradata[i].movement_finished_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_finished_sound, &items[i].pos, 0);
			} else {
				if (ng_items_extradata[i].movement_in_progress_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_in_progress_sound, &items[i].pos, 0);
			}
		}
		else if (ng_items_extradata[i].move_up_down_units < 0) {
			int move_by_amount = -ng_items_extradata[i].movement_speed;
			if (move_by_amount < ng_items_extradata[i].move_up_down_units)
				move_by_amount = ng_items_extradata[i].move_up_down_units;

			NGMoveItemByUnits(i, NG_UP, move_by_amount);
			ng_items_extradata[i].move_up_down_units -= move_by_amount;

			if (move_by_amount == 0) {
				if (ng_items_extradata[i].movement_finished_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_finished_sound, &items[i].pos, 0);
			} else {
				if (ng_items_extradata[i].movement_in_progress_sound != -1)
					SoundEffect(ng_items_extradata[i].movement_in_progress_sound, &items[i].pos, 0);
			}
		}
	}
}

void NGExecuteSingleGlobalTrigger(int global_trigger_id) {
	NG_GLOBAL_TRIGGER* global_trigger = &ng_levels[gfCurrentLevel].records->global_triggers_table[global_trigger_id].global_trigger;
	int record_id = ng_levels[gfCurrentLevel].records->global_triggers_table[global_trigger_id].record_id;

	bool global_trigger_condition_passed = false;
	
	unsigned short condition_trigger_group_id = global_trigger->condition_trigger_group;

	// What the difference between GT_CONDITION_GROUP and GT_ALWAYS?
	switch (global_trigger->type) {
		case 0x0003: { // GT_ENEMY_KILLED
			int enemy_id = ng_script_id_table[global_trigger->parameter];
			if (items[enemy_id].after_death > 0)
				global_trigger_condition_passed = true;
			break;
		}
		case 0x0004: { // GT_LARA_HP_LESS_THAN
			if (lara_item->hit_points < global_trigger->parameter)
				global_trigger_condition_passed = true;
			break;
		}
		case 0x0005: { // GT_LARA_HP_HIGHER_THAN
			if (lara_item->hit_points > global_trigger->parameter)
				global_trigger_condition_passed = true;
			break;
		}
		case 0x000a: { // GT_LARA_IS_POISONED
			if (lara_item->poisoned > global_trigger->parameter)
				global_trigger_condition_passed = true;
			break;
		}
		case 0x000b: // GT_CONDITION_GROUP
			global_trigger_condition_passed = true;
			break;
		case 0x0020: // GT_ALWAYS
			global_trigger_condition_passed = true;
			break;
		default:
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented GlobalTrigger type %u!", global_trigger->type);
			return;
	}

	// FGT_NOT_TRUE
	if (global_trigger->flags & 0x0002)
		global_trigger_condition_passed = !global_trigger_condition_passed;

	if (ng_global_trigger_states[record_id].is_disabled) {
		return;
	}

	if (global_trigger_condition_passed) {
		if (!ng_global_trigger_states[record_id].is_halted && (condition_trigger_group_id == 0xffff || NGTriggerGroupFunction(condition_trigger_group_id, 0))) {
			unsigned int perform_trigger_group_id = global_trigger->perform_trigger_group;

			if (perform_trigger_group_id != 0xffff) {
				NGTriggerGroupFunction(perform_trigger_group_id, 0);
			}

			// FGT_SINGLE_SHOT / FGT_SINGLE_SHOT_RESUMED
			if (global_trigger->flags & 0x0001 || global_trigger->flags & 0x0020)
				ng_global_trigger_states[record_id].is_halted = true;
		} else {
			// FGT_SINGLE_SHOT_RESUMED
			if (global_trigger->flags & 0x0020)
				ng_global_trigger_states[record_id].is_halted = false;

			unsigned int on_false_trigger_group_id = global_trigger->on_false_trigger_group;
			if (on_false_trigger_group_id != 0xffff) {
				NGTriggerGroupFunction(on_false_trigger_group_id, 0);
			}
		}
	} else {
		unsigned int on_false_trigger_group_id = global_trigger->on_false_trigger_group;
		if (on_false_trigger_group_id != 0xffff) {
			NGTriggerGroupFunction(on_false_trigger_group_id, 0);
		}
	}
}

void NGProcessGlobalTriggers() {
	if (ng_levels[gfCurrentLevel].records) {
		int global_trigger_count = ng_levels[gfCurrentLevel].records->global_trigger_count;
		for (int i = 0; i < global_trigger_count; i++) {
			NGExecuteSingleGlobalTrigger(i);
		}
	}
}

void NGProcessTriggerGroups() {
	for (int i = 0; i < MAX_NG_TRIGGER_GROUPS; i++) {
		if (NGIsTriggerGroupContinuous(i)) {
			NGTriggerGroupFunction(i, 0);
		}
	}
}

/*
void NGProcessTriggerGroups() {
	for (int i = 0; i < MAX_NG_TRIGGER_GROUPS; i++) {
		if (1) {
			if (!current_trigger_groups[i].data[0].first_field == 0x0000) {
				//if (i == 2 || i == 4 || i == 6) {
				//if (i == 22 || i == 23 || i == 24) {
				if (NGIsTriggerGroupContinuous(i)) {
					NGTriggerGroupFunction(i, 0);
				}
			}
		}
	}
}
*/

void NGExecuteOrganizer(int organizer_id) {
	NG_ORGANIZER* organizer = &ng_levels[gfCurrentLevel].records->organizer_table[organizer_id].organizer;
	int record_id = ng_levels[gfCurrentLevel].records->organizer_table[organizer_id].record_id;

	bool global_trigger_condition_passed = false;

	for (unsigned int i = 0; i < organizer->appointment_count; i++) {
		if (ng_organizer_states[record_id].current_tick == organizer->appointments[i].time) {
			NGTriggerGroupFunction(organizer->appointments[i].trigger_group, 0);
			if (i == organizer->appointment_count-1) {
				// FO_LOOP
				if (organizer->flags & 0x02) {
					ng_organizer_states[record_id].current_tick = -1;
				} else {
					ng_organizer_states[record_id].is_enabled = false;
				}
			}
		}
	}

	ng_organizer_states[record_id].current_tick += 1;
}

void NGProcessOrganizers() {
	if (ng_levels[gfCurrentLevel].records) {
		int organizer_count = ng_levels[gfCurrentLevel].records->organizer_count;
		for (int i = 0; i < organizer_count; i++) {
			int record_id = ng_levels[gfCurrentLevel].records->organizer_table[i].record_id;

			if (ng_organizer_states[record_id].is_enabled) {
				NGExecuteOrganizer(i);
			}
		}
	}
}

void NGFrameStartUpdate() {
	NGProcessGlobalTriggers();
	NGProcessTriggerGroups();
	NGProcessOrganizers();

	if (ng_cinema_timer > 0 || ng_cinema_type > 0) {
		switch (ng_cinema_type) {
			case 0: { // None / curtain effect
				SetFadeClipImmediate(150);
				break;
			// TODO: these are not accurate
			} case 1: { // Tiny
				SetFadeClipImmediate(8);
				break;
			} case 2: { // Middle
				SetFadeClipImmediate(16);
				break;
			} case 3: { // Big
				SetFadeClipImmediate(32);
				break;
			} case 4: { // Huge
				SetFadeClipImmediate(64);
				break;
			} case 5: { // Fissure
				SetFadeClipImmediate(128);
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

	// Input locks
	for (int i = 0; i < NG_INPUT_LOCK_TIMER_COUNT; i++) {
		if (ng_input_lock_timers[i] > 0) {
			ng_input_lock_timers[i] -= 1;
		}
	}

	// Looping sounds
	for (int i = 0; i < NumSamples; i++) {
		if (ng_looped_sound_state[i] > 0) {
			SoundEffect(i, NULL, SFX_ALWAYS);
			ng_looped_sound_state[i] -= 1;
		}
	}

	// Cold and damage rooms
	{
		ROOM_INFO* r = &room[lara_item->room_number];
		if (r) {
			int room_flags = r->flags;

			// Lara is in a damage room.
			if (room_flags & 0x10) {
				// TODO
			} else {

			}

			// Lara is in a damage room.
			if (room_flags & ROOM_COLD) {
				// TODO
			}
			else {

			}
		}
	}
}

void NGDrawPhase() {
	if (timer_tracker >= 0) {
		if (items[timer_tracker].timer <= 0) {
			if (timer_tracker_remaining_until_timeout > 0)
				timer_tracker_remaining_until_timeout--;
		}

		if (items[timer_tracker].timer > 0 || timer_tracker_remaining_until_timeout > 0) {
			char format_buffer[64];
			int remainder = items[timer_tracker].timer % 30;
			int seconds = items[timer_tracker].timer / 30;

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
					sprintf(format_buffer, "%d", items[timer_tracker].timer);
					break;
			}
			PrintString(phd_centerx, phd_winymax - font_height * 0.25, 0, format_buffer, FF_CENTER);
		}
	}
}

bool NGIsItemFrozen(unsigned int item_num) {
	if ((unsigned short)(ng_items_extradata[item_num].frozen_ticks) > 0) {
		return true;
	}

	return false;
}

void NGSetItemFreezeTimer(unsigned int item_num, int ticks) {
	ng_items_extradata[item_num].frozen_ticks = ticks;
}

short NGGetAutoRotationPerFrame(unsigned int item_num) {
	return ng_items_extradata[item_num].auto_rotation_per_frame;
}

void NGSetAutoRotationPerFrame(unsigned int item_num, short degress_per_frame) {
	ng_items_extradata[item_num].auto_rotation_per_frame = degress_per_frame;
}

//
short NGGetItemNorthSouthUnits(unsigned int item_num) {
	return ng_items_extradata[item_num].move_north_south_units;
}

void NGSetItemNorthSouthUnits(unsigned int item_num, short units) {
	ng_items_extradata[item_num].move_north_south_units = units;
}

short NGGetItemEastWestUnits(unsigned int item_num) {
	return ng_items_extradata[item_num].move_east_west_units;
}

void NGSetItemEastWestUnits(unsigned int item_num, short units) {
	ng_items_extradata[item_num].move_east_west_units = units;
}

short NGGetItemUpDownUnits(unsigned int item_num) {
	return ng_items_extradata[item_num].move_up_down_units;
}

void NGSetItemUpDownUnits(unsigned int item_num, short units) {
	ng_items_extradata[item_num].move_up_down_units = units;
}

extern void NGSetItemMovementSpeed(unsigned int item_num, unsigned int movement_speed) {
	ng_items_extradata[item_num].movement_speed = movement_speed;
}

extern void NGSetItemMovementInProgressSound(unsigned int item_num, int sound_effect_id) {
	ng_items_extradata[item_num].movement_in_progress_sound = sound_effect_id;
}

extern void NGSetItemMovementFinishedSound(unsigned int item_num, int sound_effect_id) {
	ng_items_extradata[item_num].movement_finished_sound = sound_effect_id;
}

bool NGIsItemCollisionDisabled(unsigned int item_num) {
	return ng_items_extradata[item_num].collison_disabled;
}

void NGDisableItemCollision(unsigned int item_num) {
	ng_items_extradata[item_num].collison_disabled = true;
}

void NGEnableItemCollision(unsigned int item_num) {
	ng_items_extradata[item_num].collison_disabled = false;
}

void NGSetCurtainTimer(int ticks) {
	if (ng_cinema_type == 0) {
		ng_cinema_timer = ticks;
	}
}

void NGSetCinemaTypeAndTimer(int type, int ticks) {
	ng_cinema_type = type;
	ng_cinema_timer = ticks;
}

void NGToggleOrganizer(int organizer_id, bool is_enabled) {
	ng_organizer_states[organizer_id].is_enabled = is_enabled;
}

extern bool NGIsTriggerGroupContinuous(int trigger_group_id) {
	return ng_trigger_group_states[trigger_group_id].continuous;
}

extern void NGSetTriggerGroupContinuous(int trigger_group_id, bool is_continuous) {
	ng_trigger_group_states[trigger_group_id].continuous = is_continuous;
}

void NGSetDisplayTimerForMoveableWithType(int item_id, NGTimerTrackerType new_timer_tracker_type) {
	timer_tracker = item_id;
	timer_tracker_type = new_timer_tracker_type;
	if (items[timer_tracker].timer > 0) {
		timer_tracker_remaining_until_timeout = 0;
	} else {
		timer_tracker_remaining_until_timeout = TIMER_TRACKER_TIMEOUT;
	}
}

void NGUpdateFlipeffectFloorstateData(bool heavy) {
	int index = ng_room_offset_table[ng_current_trigger_state.room] + ng_current_trigger_state.index;

	if (heavy) {
		ng_heavy_current_flipeffect_floor_trigger = index;
		ng_heavy_last_flipeffect_floor_trigger = index;
	} else {
		ng_current_flipeffect_floor_trigger = index;
		ng_last_flipeffect_floor_trigger = index;
	}
}

void NGUpdateActionFloorstateData(bool heavy) {
	int index = ng_room_offset_table[ng_current_trigger_state.room] + ng_current_trigger_state.index;

	if (heavy) {
		ng_heavy_current_action_floor_trigger = index;
		ng_heavy_last_action_floor_trigger = index;
	} else {
		ng_current_action_floor_trigger = index;
		ng_last_action_floor_trigger = index;
	}
}

extern void NGUpdateOneshot() {
	int index = ng_room_offset_table[ng_current_trigger_state.room] + ng_current_trigger_state.index;
	ng_oneshot_floorstate[index] = true;
}

void NGSetupExtraState() {
	// Variables
	current_value = 0;
	global_alfa = 0;
	global_beta = 0;
	global_delta = 0;
	global_timer = 0;
	local_alfa = 0;
	local_beta = 0;
	local_delta = 0;
	local_timer = 0;
	last_input_number = 0;

	// Timer Trackers
	timer_tracker_type = TTT_ONLY_SHOW_SECONDS;
	timer_tracker = -1;
	timer_tracker_remaining_until_timeout = 0;

	// Items
	ng_items_extradata = (NG_ITEM_EXTRADATA*)game_malloc(ITEM_COUNT * sizeof(NG_ITEM_EXTRADATA));
	memset(ng_items_extradata, 0x00, ITEM_COUNT * sizeof(NG_ITEM_EXTRADATA));

	// Records
	NG_LEVEL_RECORD_DATA *current_record_data = ng_levels[gfCurrentLevel].records;

	// Global triggers
	{
		for (int i = 0; i < MAX_NG_GLOBAL_TRIGGERS; i++) {
			ng_global_trigger_states[i].is_disabled = false;
			ng_global_trigger_states[i].is_halted = false;
		}

		if (current_record_data) {
			int global_trigger_count = current_record_data->global_trigger_count;
			for (int i = 0; i < global_trigger_count; i++) {
				NG_GLOBAL_TRIGGER* global_trigger = &current_record_data->global_triggers_table[i].global_trigger;
				int record_id = current_record_data->global_triggers_table[i].record_id;
				if (global_trigger->flags != 0xffff) {
					// FGT_DISABLED
					if (global_trigger->flags & 0x0008) {
						ng_global_trigger_states[record_id].is_disabled = true;
					}
				}
			}
		}
	}

	// Trigger groups
	{
		for (int i = 0; i < MAX_NG_TRIGGER_GROUPS; i++) {
			ng_trigger_group_states[i].continuous = false;
			ng_trigger_group_states[i].one_shot = false;
		}
	}

	// Organizers
	{
		for (int i = 0; i < MAX_NG_ORGANIZERS; i++) {
			ng_organizer_states[i].is_enabled = false;
			ng_organizer_states[i].current_tick = 0;
		}

		if (current_record_data) {
			int organizer_count = current_record_data->organizer_count;
			for (int i = 0; i < organizer_count; i++) {
				NG_ORGANIZER* organizer = &current_record_data->organizer_table[i].organizer;
				int record_id = current_record_data->organizer_table[i].record_id;
				if (organizer->flags != 0xffff) {
					// FO_ENABLED
					if (organizer->flags & 0x0001) {
						ng_organizer_states[record_id].is_enabled = true;
					}
				}
			}
		}
	}

	// Looped samples
	memset(ng_looped_sound_state, 0x00, NumSamples * sizeof(int));

	// Cinema
	ng_cinema_timer = -1;
	ng_cinema_type = 0;

	// Floorstate
	ng_last_flipeffect_floor_trigger = -1;
	ng_current_flipeffect_floor_trigger = -1;
	ng_last_action_floor_trigger = -1;
	ng_current_action_floor_trigger = -1;
	ng_heavy_last_flipeffect_floor_trigger = -1;
	ng_heavy_current_flipeffect_floor_trigger = -1;
	ng_heavy_last_action_floor_trigger = -1;
	ng_heavy_current_action_floor_trigger = -1;

	ng_floorstate_data_size = 0;
	for (int i = 0; i < number_rooms; i++) {
		ng_room_offset_table[i] = ng_floorstate_data_size;
		ng_floorstate_data_size += room[i].x_size * room[i].y_size;
	}
	ng_oneshot_floorstate = (char*)game_malloc(ng_floorstate_data_size);
	memset(ng_oneshot_floorstate, 0x00, ng_floorstate_data_size);

	// Input lock
	memset(ng_input_lock_timers, 0x00, sizeof(ng_input_lock_timers));

	// Damage
	lara_damage_resistence = 1000;
}

void NGFrameFinishExtraState() {
	// Lara
	ng_last_flipeffect_floor_trigger = ng_current_flipeffect_floor_trigger;
	ng_current_flipeffect_floor_trigger = -1;

	ng_last_action_floor_trigger = ng_current_action_floor_trigger;
	ng_current_action_floor_trigger = -1;

	// Heavy
	ng_heavy_last_flipeffect_floor_trigger = ng_heavy_current_flipeffect_floor_trigger;
	ng_heavy_current_flipeffect_floor_trigger = -1;

	ng_heavy_last_action_floor_trigger = ng_heavy_current_action_floor_trigger;
	ng_heavy_current_action_floor_trigger = -1;
}
