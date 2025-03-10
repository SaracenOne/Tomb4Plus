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
#include "trng_globaltrigger.h"
#include "../../specific/input.h"
#include "../../specific/dxshell.h"
#include "../newinv.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"

bool NGExecuteSingleGlobalTrigger(int32_t global_trigger_id, int32_t selected_inventory_object_id) {
	NG_GLOBAL_TRIGGER* global_trigger = &ng_levels[gfCurrentLevel].records->global_trigger_table[global_trigger_id].record;
	int32_t record_id = ng_levels[gfCurrentLevel].records->global_trigger_table[global_trigger_id].record_id;

	bool global_trigger_condition_passed = false;

	uint16_t condition_trigger_group_id = global_trigger->condition_trigger_group;

	if (selected_inventory_object_id == NO_ITEM) {
		// What the difference between GT_CONDITION_GROUP and GT_ALWAYS?
		switch (global_trigger->type) {
			case GT_USED_INVENTORY_ITEM: {
				if (ng_used_inventory_object_for_frame != NO_ITEM) {
					if (ng_used_inventory_object_for_frame == global_trigger->parameter)
						global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_USED_BIG_MEDIPACK: {
				if (ng_used_large_medipack) {
					global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_USED_LITTLE_MEDIPACK: {
				if (ng_used_small_medipack) {
					global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_ENEMY_KILLED: {
				int32_t enemy_id = ng_script_id_table[global_trigger->parameter].script_index;
				ITEM_INFO *item = T4PlusGetItemInfoForID(enemy_id);
				if (item) {
					if (item->after_death > 0)
						global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_LARA_HP_LESS_THAN: {
				if (lara_item->hit_points < global_trigger->parameter)
					global_trigger_condition_passed = true;
				break;
			}
			case GT_LARA_HP_HIGHER_THAN: {
				if (lara_item->hit_points > global_trigger->parameter)
					global_trigger_condition_passed = true;
				break;
			}
			case GT_LARA_POISONED: {
				if (lara_item->poisoned > (uint32_t)global_trigger->parameter)
					global_trigger_condition_passed = true;
				break;
			}
			case GT_CONDITION_GROUP: {
				global_trigger_condition_passed = true;
				break;
			}
			case GT_DISTANCE_FROM_ITEM: {
				if (is_mod_trng_version_equal_or_greater_than_target(1, 2, 2, 4)) {
					NGScriptIDTableEntry *entry = &ng_script_id_table[global_trigger->parameter  & 0x1fff];
					int32_t distance = (global_trigger->parameter >> 13) & 0x1ffff;
					if (entry->script_index != -1) {
						ITEM_INFO *item = T4PlusGetItemInfoForID(entry->script_index);
						if (item) {
							global_trigger_condition_passed = NGIsSourcePositionLessThanDistanceToTargetPosition(&lara_item->pos, &item->pos, distance, global_trigger->parameter & GTD_IGNORE_HEIGHT);
						}
					}
				} else {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "GT_DISTANCE_FROM_ITEM for version prior to (1, 2, 2, 4) is unimplemented.");
				}
				break;
			}
			case GT_COLLIDE_ITEM: {
				ITEM_INFO *item = T4PlusGetItemInfoForID(ng_script_id_table[global_trigger->parameter].script_index);

				ITEM_INFO* collided_item = NGIsLaraCollidingWithItem(item, global_trigger->flags & FGT_PUSHING_COLLISION ? NG_COLLISION_TYPE_PUSH : NG_COLLISION_TYPE_BOUNDS);
				if (collided_item) {
					global_trigger_condition_passed = true;
					NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(collided_item));
				}
				break;
			}
			case GT_COLLIDE_SLOT: {
				ITEM_INFO *collided_item = NGIsLaraCollidingWithMoveableSlot(global_trigger->parameter, global_trigger->flags & FGT_PUSHING_COLLISION ? NG_COLLISION_TYPE_PUSH : NG_COLLISION_TYPE_BOUNDS);
				if (collided_item) {
					global_trigger_condition_passed = true;
					NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(collided_item));
				}
				break;
			}
			case GT_COLLIDE_CREATURE: {
				ITEM_INFO *collided_item = NGIsLaraCollidingWithCreature(NG_CREATURE_TYPE_ANY, global_trigger->flags & FGT_PUSHING_COLLISION ? NG_COLLISION_TYPE_PUSH : NG_COLLISION_TYPE_BOUNDS);
				if (collided_item) {
					global_trigger_condition_passed = true;
					NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(collided_item));
				}
				break;
			}
			case GT_LOADED_SAVEGAME: {
				int32_t result = ng_loaded_savegame == true;
				if (result >= 0) {
					global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_COLLIDE_STATIC_SLOT: {
				int32_t result = NGIsLaraCollidingWithStaticSlot(global_trigger->parameter);
				if (result >= 0) {
					global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_LARA_HOLDS_ITEM: {
				global_trigger_condition_passed = NGIsLaraHolding(global_trigger->parameter);
				break;
			}
			case GT_KEYBOARD_CODE: {
				if (IsKeyPressed(global_trigger->parameter) != 0) {
					global_trigger_condition_passed = true;
				}
				break;
			}
			case GT_ALWAYS:
				global_trigger_condition_passed = true;
				break;
			case GT_TRNG_G_TIMER_EQUALS:
				// We don't evaluate AT ALL if the timer is inactive
				if (ng_global_timer_frame_increment == 0)
					return false;

				if (ng_global_timer == global_trigger->parameter) {
					global_trigger_condition_passed = true;
				}
				break;
			case GT_TRNG_L_TIMER_EQUALS:
				// We don't evaluate AT ALL if the timer is inactive
				if (ng_local_timer_frame_increment == 0)
					return false;

				if (ng_local_timer == global_trigger->parameter) {
					global_trigger_condition_passed = true;
				}
				break;
			case GT_SELECTED_INVENTORY_ITEM:
				return false;
			default:
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented GlobalTrigger type %u!", global_trigger->type);
				return false;
		}
	} else {
		switch (global_trigger->type) {
			case GT_SELECTED_INVENTORY_ITEM:
				if (selected_inventory_object_id == global_trigger->parameter)
					global_trigger_condition_passed = true;
				break;
			default:
				return false;
		}
	}

	bool management_replaced = false;

	if (global_trigger->flags & FGT_NOT_TRUE)
		global_trigger_condition_passed = !global_trigger_condition_passed;

	if (global_trigger->flags & FGT_REMOVE_INPUT) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented FGT_REMOVE_INPUT!");
		return false;
	}

	if (ng_global_trigger_states[record_id].is_disabled) {
		return false;
	}

	if (global_trigger->flags & FGT_REPLACE_MANAGEMENT) {
		if (global_trigger_condition_passed)
			management_replaced = true;
	}

	bool global_trigger_should_on_false_triggergroup = false;

	if (global_trigger_condition_passed) {
		if ((condition_trigger_group_id == 0xffff || NGTriggerGroupFunction(condition_trigger_group_id, TRIGGER_GROUP_EXECUTION_MULTIPLE))) {
			if (!ng_global_trigger_states[record_id].is_halted) {
				uint32_t perform_trigger_group_id = global_trigger->perform_trigger_group;

				if (perform_trigger_group_id != 0xffff) {
					NGTriggerGroupFunction(perform_trigger_group_id, TRIGGER_GROUP_EXECUTION_MULTIPLE);
				} else {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented 'default' perform parameter in GlobalTrigger");
				}

				if (global_trigger->flags & FGT_SINGLE_SHOT || global_trigger->flags & FGT_SINGLE_SHOT_RESUMED)
					ng_global_trigger_states[record_id].is_halted = true;
			} else {
				// I assume a halted GlobalTrigger should run the on false triggergroup, but may need to check this.
				global_trigger_should_on_false_triggergroup = true;
			}
		} else {
			if (global_trigger->flags & FGT_SINGLE_SHOT_RESUMED)
				ng_global_trigger_states[record_id].is_halted = false;

			global_trigger_should_on_false_triggergroup = true;
		}
	} else {
		global_trigger_should_on_false_triggergroup = true;
	}

	if (global_trigger_should_on_false_triggergroup) {
		uint32_t on_false_trigger_group_id = global_trigger->on_false_trigger_group;
		if (on_false_trigger_group_id != 0xffff) {
			NGTriggerGroupFunction(on_false_trigger_group_id, TRIGGER_GROUP_EXECUTION_MULTIPLE);
		}
	}

	return management_replaced;
}