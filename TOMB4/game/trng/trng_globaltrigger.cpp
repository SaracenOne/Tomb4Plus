#include "../../tomb4/pch.h"

#include "../control.h"
#include "../lara.h"
#include "../gameflow.h"

#include "trng.h"
#include "trng_condition.h"
#include "trng_flipeffect.h"
#include "trng_extra_state.h"
#include "trng_script_parser.h"
#include "trng_triggergroup.h"

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
	case 0x000b: { // GT_CONDITION_GROUP
		global_trigger_condition_passed = true;
		break;
	}
	case 0x000d: { // GT_COLLIDE_ITEM
		int result = NGIsLaraCollidingWithItem(ng_script_id_table[global_trigger->parameter]);
		if (result >= 0) {
			global_trigger_condition_passed = true;
			ng_found_item_index = result;
		}
		break;
	}
	case 0x000e: { // GT_COLLIDE_SLOT
		int result = NGIsLaraCollidingWithSlot(global_trigger->parameter);
		if (result >= 0) {
			global_trigger_condition_passed = true;
			ng_found_item_index = result;
		}
		break;
	}
	case 0x000f: { // GT_COLLIDE_CREATURE
		int result = NGIsLaraCollidingWithCreature();
		if (result >= 0) {
			global_trigger_condition_passed = true;
			ng_found_item_index = result;
		}
		break;
	}
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

	// FGT_PUSHING_COLLISION
	if (global_trigger->flags & 0x0004) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented FGT_PUSHING_COLLISION!");
		return;
	}

	// FGT_REMOVE_INPUT
	if (global_trigger->flags & 0x0010) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented FGT_REMOVE_INPUT!");
		return;
	}

	// FGT_REPLACE_MANAGEMENT
	if (global_trigger->flags & 0x0040) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented FGT_REPLACE_MANAGEMENT!");
		return;
	}


	if (ng_global_trigger_states[record_id].is_disabled) {
		return;
	}

	if (global_trigger_condition_passed) {
		if (!ng_global_trigger_states[record_id].is_halted && (condition_trigger_group_id == 0xffff || NGTriggerGroupFunction(condition_trigger_group_id, 0))) {
			unsigned int perform_trigger_group_id = global_trigger->perform_trigger_group;

			if (perform_trigger_group_id != 0xffff) {
				NGTriggerGroupFunction(perform_trigger_group_id, 0);
			} else {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented 'default' perform parameter in GlobalTrigger");
			}

			// FGT_SINGLE_SHOT / FGT_SINGLE_SHOT_RESUMED
			if (global_trigger->flags & 0x0001 || global_trigger->flags & 0x0020)
				ng_global_trigger_states[record_id].is_halted = true;
		}
		else {
			// FGT_SINGLE_SHOT_RESUMED
			if (global_trigger->flags & 0x0020)
				ng_global_trigger_states[record_id].is_halted = false;

			unsigned int on_false_trigger_group_id = global_trigger->on_false_trigger_group;
			if (on_false_trigger_group_id != 0xffff) {
				NGTriggerGroupFunction(on_false_trigger_group_id, 0);
			}
		}
	}
	else {
		unsigned int on_false_trigger_group_id = global_trigger->on_false_trigger_group;
		if (on_false_trigger_group_id != 0xffff) {
			NGTriggerGroupFunction(on_false_trigger_group_id, 0);
		}
	}
}