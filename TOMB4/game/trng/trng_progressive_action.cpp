#include "../../tomb4/pch.h"

#include "trng.h"
#include "trng_progressive_action.h"

#include "../../specific/function_stubs.h"
#include "../../specific/file.h"
#include "../../tomb4/mod_config.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"
#include "trng_extra_state.h"

NGProgressiveAction progressive_actions[NG_MAX_PROGRESSIVE_ACTIONS];
int32_t progressive_action_count = 0;

NGProgressiveAction *NGCreateProgressiveAction() {
	for (int32_t i = 0; i < progressive_action_count; i++) {
		if (progressive_actions[i].type == AZ_NONE) {
			return &progressive_actions[i];
		}
	}
	if (progressive_action_count >= NG_MAX_PROGRESSIVE_ACTIONS) {
		NGLog(NG_LOG_TYPE_ERROR, "Progressive action count has overflown");
		return nullptr;
	}

	progressive_action_count++;

	return &progressive_actions[progressive_action_count-1];
}

void NGProgressiveActionComplete(NGProgressiveAction* progressive_action) {

}

void NGExecuteProgressiveAction(NGProgressiveAction* progressive_action) {
	NGProgressiveAction original_action = *progressive_action;

	switch (progressive_action->type) {
		case AZ_ROTATE_ITEM_HORIZONTAL: {
			ITEM_INFO* item_info = T4PlusGetItemInfoForID(progressive_action->item_index);
			if (progressive_action->argument2_i32[0] > 0) {
				item_info->pos.y_rot += progressive_action->argument1_u16;
			} else {
				item_info->pos.y_rot -= progressive_action->argument1_u16;
			}

			if (progressive_action->duration != 0xffff) {
				progressive_action->duration--;
			}

			if (progressive_action->duration == 0) {
				progressive_action->type = AZ_NONE;
			}

			break;
		}
		case AZ_ROTATE_ITEM_VERTICAL: {
			ITEM_INFO* item_info = T4PlusGetItemInfoForID(progressive_action->item_index);
			if (progressive_action->argument2_i32[0] > 0) {
				item_info->pos.x_rot += progressive_action->argument1_u16;
			}
			else {
				item_info->pos.x_rot -= progressive_action->argument1_u16;
			}

			if (progressive_action->duration != 0xffff) {
				progressive_action->duration--;
			}

			if (progressive_action->duration == 0) {
				progressive_action->type = AZ_NONE;
			}

			break;
		}
		case AZ_RESET_DISABLED_INPUT: {
			progressive_action->duration--;
			if (progressive_action->duration == 0) {
				int32_t mask = progressive_action->argument2_i32[0] ^ -1;
				ng_input_to_disable &= mask;
				progressive_action->type = AZ_NONE;
			}
			break;
		}
		case AZ_RESET_SIMULATED_INPUT: {
			progressive_action->duration--;
			if (progressive_action->duration == 0) {
				int32_t mask = progressive_action->argument2_i32[0] ^ -1;
				ng_input_to_simulate &= mask;
				progressive_action->type = AZ_NONE;
			}
			break;
		}
		case AZ_HIDE_SCREEN: {
			progressive_action->duration--;
			if (progressive_action->duration == 0) {
				ng_drawing_state = NG_DRAW_STATE_ACTIVE;
				progressive_action->type = AZ_NONE;
			}
			break;
		}
	}

	if (progressive_action->type == AZ_NONE) {
		NGProgressiveActionComplete(&original_action);
	}
}

void NGExecuteProgressiveActions() {
	for (int i = 0; i < progressive_action_count; i++) {
		if (progressive_actions[i].type != AZ_NONE) {
			NGExecuteProgressiveAction(&progressive_actions[i]);
		}
	}
}