#include "../../tomb4/pch.h"

#include "../../specific/3dmath.h"
#include "../../specific/function_stubs.h"
#include "../../specific/dxshell.h"
#include "../../specific/file.h"

#include "../control.h"
#include "../effects.h"
#include "../objects.h"
#include "../draw.h"
#include "../door.h"
#include "../items.h"
#include "../lot.h"
#include "../lara.h"
#include "../lara_states.h"
#include "../newinv.h"
#include "../savegame.h"

#include "trng.h"
#include "trng_arithmetic.h"
#include "trng_condition.h"
#include "trng_env_condition.h"
#include "trng_extra_state.h"
#include "trng_triggergroup.h"

#include "../../tomb4/tomb4plus/t4plus_inventory.h"
#include "../../tomb4/tomb4plus/t4plus_objects.h"

#include "../../specific/input.h"
#include "../../specific/dxsound.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"

NGOldTrigger old_conditions[NG_MAX_OLD_CONDITIONS];
int32_t old_condition_count;

enum GRID_FRAGMENT_TYPE {
	FRAGMENT_TYPE_SQUARE_FRAGMENT,
	FRAGMENT_TYPE_HORIZONTAL_STRIP_FRAGMENT,
	FRAGMENT_TYPE_VERTICAL_STRIP_FRAGMENT,
	FRAGMENT_TYPE_TWO_CROSSED_STRIPES,
	FRAGMENT_TYPE_DIAGONAL_UPPER_RIGHT_LOWER_LEFT_PASSING_LINE, // DIAGONAL_BOTTOM_UP_PASSING_POINT
	FRAGMENT_TYPE_DIAGONAL_UPPER_LEFT_LOWER_RIGHT_PASSING_LINE, // DIAGONAL_TOP_DOWN_PASSING_POINT
	FRAGMENT_TYPE_TWO_CROSS_DIAGONALS_PASSING_LINE
};

bool NGIsCreatureActive(int16_t item_num) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_num);

	if (item->flags & IFL_CLEARBODY) {
		return false;
	}

	if (item->status == ITEM_INVISIBLE) {
		return false;
	}

	return true;
}

bool NGIsTriggerActive(ITEM_INFO *item) {
	if (!(item->flags & IFL_CODEBITS)) {
		return false;
	}

	return true;
}

bool NGGridFragmentCondition(int32_t x_pos, int32_t y_pos, int32_t grid_size, int32_t x_target_coordinate, int32_t y_target_coordinate, GRID_FRAGMENT_TYPE grid_fragment_type, bool inverted) {
	int fragment_size = BLOCK_SIZE / grid_size;
	// Flipped these around to match
	int touching_fragment_x = x_pos / fragment_size;
	int touching_fragment_y = y_pos / fragment_size;

	if (grid_fragment_type == FRAGMENT_TYPE_SQUARE_FRAGMENT) {
		return (touching_fragment_x == x_target_coordinate && touching_fragment_y == y_target_coordinate) != inverted;
	} else if (grid_fragment_type == FRAGMENT_TYPE_VERTICAL_STRIP_FRAGMENT) {
		return (touching_fragment_x == x_target_coordinate) != inverted;
	} else if (grid_fragment_type == FRAGMENT_TYPE_HORIZONTAL_STRIP_FRAGMENT) {
		return (touching_fragment_y == y_target_coordinate) != inverted;
	} else if (grid_fragment_type == FRAGMENT_TYPE_TWO_CROSSED_STRIPES) {
		return (touching_fragment_x == x_target_coordinate || touching_fragment_y == y_target_coordinate) != inverted;
	} else if (grid_fragment_type == FRAGMENT_TYPE_DIAGONAL_UPPER_RIGHT_LOWER_LEFT_PASSING_LINE) {
		return (touching_fragment_x + touching_fragment_y == x_target_coordinate + y_target_coordinate) != inverted;
	} else if (grid_fragment_type == FRAGMENT_TYPE_DIAGONAL_UPPER_LEFT_LOWER_RIGHT_PASSING_LINE) {
		return (touching_fragment_x - touching_fragment_y == x_target_coordinate - y_target_coordinate) != inverted;
	} else if (grid_fragment_type == FRAGMENT_TYPE_TWO_CROSS_DIAGONALS_PASSING_LINE) {
		return ((touching_fragment_x + touching_fragment_y == x_target_coordinate + y_target_coordinate) || (touching_fragment_x - touching_fragment_y == x_target_coordinate - y_target_coordinate)) != inverted;
	} else {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGGridFragmentCondition: grid fragment type unsupported %u!", (uint32_t)grid_fragment_type);
	}

	return false;
}

bool NGGridFragmentConditionTrigger(int32_t param, uint32_t extra, int32_t grid_size) {
	int lara_sector_displacement_y = (lara_item->pos.x_pos & 0x3ff);
	int lara_sector_displacement_x = (lara_item->pos.z_pos & 0x3ff);

	bool inverted = extra & 0x02;
	bool pad_trigger = extra & 0x01;

	if (pad_trigger) {
		if (lara_item->pos.y_pos != lara_item->floor)
			return false;
	}

	if (param >= 0 && param < 16) {
		int32_t index = param;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, index / 4, index % 4, FRAGMENT_TYPE_SQUARE_FRAGMENT, inverted);
	} else if (param >= 32 && param < 36) {
		int32_t index = param - 32;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, 0, index, FRAGMENT_TYPE_HORIZONTAL_STRIP_FRAGMENT, inverted);
	} else if (param >= 48 && param < 52) {
		int32_t index = param - 48;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, index, 0, FRAGMENT_TYPE_VERTICAL_STRIP_FRAGMENT, inverted);
	} else if (param >= 96 && param < 112) {
		int32_t index = param - 96;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, index / 4, index % 4, FRAGMENT_TYPE_TWO_CROSSED_STRIPES, inverted);
	} else if (param >= 128 && param < 144) {
		int32_t index = param - 128;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, index / 4, index % 4, FRAGMENT_TYPE_DIAGONAL_UPPER_RIGHT_LOWER_LEFT_PASSING_LINE, inverted);
	} else if (param >= 144 && param < 160) {
		int32_t index = param - 144;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, index / 4, index % 4, FRAGMENT_TYPE_DIAGONAL_UPPER_LEFT_LOWER_RIGHT_PASSING_LINE, inverted);
	} else if (param >= 192 && param < 208) {
		int32_t index = param - 192;
		return NGGridFragmentCondition(lara_sector_displacement_x, lara_sector_displacement_y, grid_size, index / 4, index % 4, FRAGMENT_TYPE_TWO_CROSS_DIAGONALS_PASSING_LINE, inverted);
	} else {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGGridFragmentTrigger: param %u unsupported!", param);
	}

	return false;
}

int32_t NGPerformTRNGCondition(uint16_t condition_number, uint16_t main_argument, uint16_t extra, bool *test_restore, bool *test_skip, int *repeat_type, uint16_t flags) {
	if (NGGetIsInsideDummyTrigger()) {
		if (!NGIsValidConditionForDummy(condition_number, main_argument, true)) {
			*test_restore = false;
			*test_skip = false;
			*repeat_type = 0;
			return 0;
		}
	}

	int32_t result = 0;

	switch (condition_number) {
		case INVENTORY_ITEM_IS_MISSING: {
			if (T4PlusGetInventoryCount(main_argument) == 0) {
				result = 1;
			}
			break;
		}
		case INVENTORY_ITEM_IS_PRESENT: {
			if (T4PlusGetInventoryCount(main_argument) != 0) {
				result = 1;
			}
			break;
		}
		case INVENTORY_ITEM_HAS_AT_LEAST: {
			*test_restore = true;
			if (T4PlusGetInventoryCount(main_argument) >= extra) {
				result = 1;
			}
			break;
		}
		case INVENTORY_ITEM_HAS_LESS_THAN: {
			*test_restore = true;
			if (T4PlusGetInventoryCount(main_argument) < extra) {
				result = 1;
			}
			break;
		}
		case LARA_IS_PERFORMING_X_ACTION: {
			*test_restore = true;

			bool is_lara_performing = !extra;
			switch (main_argument) {
				case 0: // Climbing
					if ((
					            lara_item->current_anim_state >= AS_CLIMBSTNC &&
					            lara_item->current_anim_state <= AS_CLIMBDOWN) == is_lara_performing) {
						result = 1;
					}
					break;
				case 1: // Swimming underwater
					if ((
					            lara_item->current_anim_state == AS_TREAD ||
					            lara_item->current_anim_state == AS_SWIM ||
					            lara_item->current_anim_state == AS_GLIDE) == is_lara_performing) {
						result = 1;
					}
					break;
				case 2: // Floating on water
					if ((
					            lara_item->current_anim_state == AS_SURFTREAD ||
					            lara_item->current_anim_state == AS_SURFSWIM ||
					            lara_item->current_anim_state == AS_SURFBACK ||
					            lara_item->current_anim_state == AS_SURFLEFT ||
					            lara_item->current_anim_state == AS_SURFRIGHT) == is_lara_performing) {
						result = 1;
					}
					break;
				case 3: // Falling
					if ((lara_item->current_anim_state == AS_FASTFALL ||
					        lara_item->current_anim_state == AS_FALLBACK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 4: // Jumping
					if ((
					            lara_item->current_anim_state == AS_BACKJUMP ||
					            lara_item->current_anim_state == AS_UPJUMP ||
					            lara_item->current_anim_state == AS_FORWARDJUMP) == is_lara_performing) {
						result = 1;
					}
					break;
				case 5: // Moving on all fours
					if ((
					            lara_item->current_anim_state == AS_ALL4S ||
					            lara_item->current_anim_state == AS_CRAWL ||
					            lara_item->current_anim_state == AS_ALL4TURNL ||
					            lara_item->current_anim_state == AS_ALL4TURNR ||
					            lara_item->current_anim_state == AS_CRAWLBACK ||
					            lara_item->current_anim_state == AS_DUCK ||
					            lara_item->current_anim_state == AS_DUCKROLL ||
					            lara_item->current_anim_state == AS_DUCKROTL ||
					            lara_item->current_anim_state == AS_DUCKROTR) == is_lara_performing) {
						result = 1;
					}
					break;
				case 6: // Sliding
					if ((lara_item->current_anim_state == AS_SLIDE ||
					        lara_item->current_anim_state == AS_SLIDEBACK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 7: // Rolling
					if ((lara_item->current_anim_state == AS_FASTBACK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 8: // Running
					if ((lara_item->current_anim_state == AS_RUN) == is_lara_performing) {
						result = 1;
					}
					break;
				case 9: // Walking
					if ((lara_item->current_anim_state == AS_WALK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 10: // Dying
					if ((lara_item->current_anim_state == AS_DEATH) == is_lara_performing) {
						result = 1;
					}
					break;
				case 11: // Stopping
					if ((lara_item->current_anim_state == AS_STOP) == is_lara_performing) {
						result = 1;
					}
					break;
				case 12: // Monkeying
					if ((
					            lara_item->current_anim_state == AS_HANG2 ||
					            lara_item->current_anim_state == AS_MONKEYSWING ||
					            lara_item->current_anim_state == AS_MONKEYL ||
					            lara_item->current_anim_state == AS_MONKEYR ||
					            lara_item->current_anim_state == AS_MONKEY180 ||
					            lara_item->current_anim_state == AS_HANGTURNL ||
					            lara_item->current_anim_state == AS_HANGTURNR) == is_lara_performing) {
						result = 1;
					}
					break;
				case 13: // Pushing block
					if ((lara_item->current_anim_state == AS_PUSHBLOCK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 14: // Pulling block
					if ((lara_item->current_anim_state == AS_PULLBLOCK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 15: // Pulling or pushing block
					if ((lara_item->current_anim_state == AS_PUSHBLOCK || lara_item->current_anim_state == AS_PULLBLOCK) == is_lara_performing) {
						result = 1;
					}
					break;
				case 16: // Swimming underwater or on water
					if ((
					            lara_item->current_anim_state == AS_TREAD ||
					            lara_item->current_anim_state == AS_SWIM ||
					            lara_item->current_anim_state == AS_SURFTREAD ||
					            lara_item->current_anim_state == AS_SURFSWIM) == is_lara_performing) {
						result = 1;
					}
					break;
				case 17: // Shooting
					if (lara.has_fired == is_lara_performing) {
						result = 1;
					}
					break;
				case 18: // Hanging
					if ((
					            lara_item->current_anim_state == AS_HANG ||
					            lara_item->current_anim_state == AS_HANGLEFT ||
					            lara_item->current_anim_state == AS_HANGRIGHT ||
					            lara_item->current_anim_state == AS_HANGTURNL ||
					            lara_item->current_anim_state == AS_HANGTURNR) == is_lara_performing) {
						result = 1;
					}
					break;
				case 19: { // Dripping
					bool is_dripping = false;
					for (int32_t i = 0; i < WET_COUNT; i++) {
						if (lara.wet[i]) {
							is_dripping = true;
						}
					}
					result = (is_dripping == is_lara_performing);
				}
				break;
				default:
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "LARA_IS_PERFORMING_X_ACTION is not currently implemented!");
					break;
			}
			break;
		}
		case FRAGMENTED_TRIGGGER_CHECK_IN_WAY_IF_LARA_IS_IN_FRAGMENT_OF_2X2_SECTOR_GRID: {
			*test_restore = true;
			if (NGGridFragmentConditionTrigger(main_argument, extra, 2)) {
				result = 1;
			}
			break;
		}
		case FRAGMENTED_TRIGGGER_CHECK_IN_WAY_IF_LARA_IS_IN_FRAGMENT_OF_3X3_SECTOR_GRID: {
			*test_restore = true;
			if (NGGridFragmentConditionTrigger(main_argument, extra, 3)) {
				result = 1;
			}
			break;
		}
		case FRAGMENTED_TRIGGGER_CHECK_IN_WAY_IF_LARA_IS_IN_FRAGMENT_OF_4X4_SECTOR_GRID: {
			*test_restore = true;
			if (NGGridFragmentConditionTrigger(main_argument, extra, 4)) {
				result = 1;
			}
			break;
		}
		case VERTICAL_TRIGGER_ZONE: {
			// TODO: make more accurate to TRNG
			*test_restore = true;
			int16_t* bounds = GetBoundsAccurate(lara_item);
			int32_t item_top_y = lara_item->pos.y_pos + bounds[2];
			int32_t item_bottom_y = lara_item->pos.y_pos; // + bounds[3];

			int32_t bottom_trigger_bounds = lara_item->floor - (main_argument * 128);
			int32_t top_trigger_bounds = bottom_trigger_bounds - ((extra + 1) * 128);

			if (item_top_y <= bottom_trigger_bounds && item_bottom_y >= top_trigger_bounds) {
				result = 1;
			}
			break;
		}
		case VERTICAL_TRIGGER_ZONE_INVERSE: {
			// TODO: make more accurate to TRNG
			*test_restore = true;
			int16_t* bounds = GetBoundsAccurate(lara_item);
			int32_t item_top_y = lara_item->pos.y_pos + bounds[2];
			int32_t item_bottom_y = lara_item->pos.y_pos; // + bounds[3];

			int32_t bottom_trigger_bounds = lara_item->floor - (main_argument * 128);
			int32_t top_trigger_bounds = bottom_trigger_bounds - ((extra + 1) * 128);

			if (item_top_y < top_trigger_bounds || item_bottom_y > bottom_trigger_bounds) {
				result = 1;
			}
			break;
		}
		case VERTICAL_TRIGGER_ANTI_ZONE: {
			// TODO: make more accurate to TRNG
			*test_restore = true;
			int16_t* bounds = GetBoundsAccurate(lara_item);
			int item_top_y = lara_item->pos.y_pos + bounds[2];
			int item_bottom_y = lara_item->pos.y_pos; // + bounds[3];

			int bottom_trigger_bounds = lara_item->floor - (main_argument * 128);
			int top_trigger_bounds = bottom_trigger_bounds - ((extra + 1) * 128);

			if (!(item_top_y < bottom_trigger_bounds && item_bottom_y > top_trigger_bounds)) {
				result = 1;
			}
			break;
		}
		case KEYBOARD_SCANCODE_IS_CURRENTLY: {
			*test_restore = true;
			int scancode = main_argument;

			scancode = convert_tomb_keycode_to_sdl_scancode(scancode);
			if (!keymap) {
				result = false;
			}
			if (scancode == SDLK_UNKNOWN) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unknown keyboard scancode %u!", scancode);
				result = false;
			}

			// TODO: find the difference between multishot and singleshot
			switch (extra) {
				// Inactive single shot
				case 0: {
					result = (keymap[scancode] == 0);
					break;
				}
				// Active single shot
				case 1: {
					result = (keymap[scancode] != 0);
					break;
				}
				// Inactive multi shot
				case 2: {
					result = (keymap[scancode] == 0);
					break;
				}
				// Inactive single shot
				case 3: {
					result = (keymap[scancode] != 0);
					break;
				}
			}
			break;
		}
		case KEYBOARD_COMMAND_GAME_IS_CURRENTLY: {
			*test_restore = true;
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "KEYBOARD_COMMAND_GAME_IS_CURRENTLY is not currently implemented!");
			break;
		}
		case CREATURE_IS_CURRENTLY: {
			*test_restore = true;
			*test_skip = true;
			switch (extra) {
				// Enemy is dead
				case 0x00: {
					result = (T4PlusGetItemInfoForID(main_argument)->status == ITEM_DEACTIVATED);
					break;
				}
				// Enemy has not yet been activated
				case 0x01: {
					result = (T4PlusGetItemInfoForID(main_argument)->status == ITEM_INVISIBLE);
					break;
				}
				// Enemy is living
				case 0x02: {
					result = NGIsCreatureActive(main_argument);
					break;
				}
				// Enemy is active
				case 0x03: {
					result = (NGIsCreatureActive(main_argument) && NGIsTriggerActive(T4PlusGetItemInfoForID(main_argument)));
					break;
				}
				// Enemy is not active
				case 0x04: {
					result = (!NGIsCreatureActive(main_argument) || !NGIsTriggerActive(T4PlusGetItemInfoForID(main_argument)));
					break;
				}
				default: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "CREATURE_IS_CURRENTLY %u is not currently implemented!", extra);
					break;
				}
			}
			break;
		}
		case MULTIPLE_CONDITION_OF_X_TRIGGERGROUP_SCRIPT_COMMAND: {
			result = NGTriggerGroupFunction(main_argument, TRIGGER_GROUP_EXECUTION_MULTIPLE);
			break;
		}
		case MULTIPLE_CONDITION_OF_X_MULTIENVCONDITION_SCRIPT_COMMAND: {
			*test_restore = true;
			result = TestMultiEnvCondition(main_argument, extra, -1);
			break;
		}
		case LARA_HAS_FOUND_AT_LEAST_X_SECRETS: {
			result = (savegame.Game.Secrets >= main_argument);
			result = true; // TEST
			break;
		}
		case LARA_HAS_FOUND_EXACTLY_X_SECRETS: {
			result = (savegame.Game.Secrets == main_argument);
			break;
		}
		case KEYPAD_LAST_NUMBER_TYPED_IN_KEYPAD_IS_X_VALUE: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "KEYPAD_LAST_NUMBER_TYPED_IN_KEYPAD_IS_X_VALUE is not currently implemented!");
			break;
		}
		case TIMER_TIMER_SCREEN_VALUE_IS_Y_THAN_X_SECONDS: {
			*test_restore = true;
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TIMER_TIMER_SCREEN_VALUE_IS_Y_THAN_X_SECONDS is not currently implemented!");
			break;
		}
		case CREATURE_CURRENT_ANIMATION_0_31_IS:
		case CREATURE_CURRENT_ANIMATION_32_63_IS:
		case CREATURE_CURRENT_ANIMATION_64_95_IS:
			*test_restore = true;
			*test_skip = true;
			result = (T4PlusGetItemInfoForID(main_argument)->anim_number - objects[T4PlusGetItemInfoForID(main_argument)->object_number].anim_index == extra);
			break;
		case CREATURE_IS_CURRENTLY_OF_STATE: {
			*test_restore = true;
			*test_skip = true;
			result = (T4PlusGetItemInfoForID(main_argument)->current_anim_state == extra);
			break;
		}
		// Lara status is enabled/disabled
		case LARA_STATUS_IS_ENABLED_OR_DISABLED: {
			*test_restore = true;
			switch (main_argument) {
				// Infinite Air
				case 1:
					if (NGLaraHasInfiniteAir() == (bool)extra)
						result = 1;
					break;
				// Poisoned
				case 2:
					if ((lara_item->poisoned) == (bool)extra)
						result = 1;
					break;
				// Lara touching floor
				case 4:
					if ((lara_item->pos.y_pos == lara_item->floor) == (bool)extra)
						result = 1;
					break;
				default:
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: Unimplemented NGCondition Lara Status %u!", main_argument);
					break;
			}
			break;
		}
		case LARA_IS_TOUCHING_MOVEABLE_ID: {
			*test_skip = true;
			ITEM_INFO *item = NGIsLaraCollidingWithItem(T4PlusGetItemInfoForID(main_argument), NG_COLLISION_TYPE_PUSH);
			if (item) {
				NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(item));
				result = 1;
			}
			break;
		}
		case LARA_IS_TOUCHING_MOVEABLE_SLOT: {
			ITEM_INFO *item = NGIsLaraCollidingWithMoveableSlot(main_argument, NG_COLLISION_TYPE_PUSH);
			if (item) {
				NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(item));
				result = 1;
			}
			break;
		}
		case LARA_IS_TOUCHING_CREATURE_TYPE: {
			switch (main_argument) {
				// Mortal creatures
				case 0x00: {
					ITEM_INFO *item = NGIsLaraCollidingWithCreature(NG_CREATURE_TYPE_MORTAL, NG_COLLISION_TYPE_PUSH);
					if (item) {
						NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(item));
						result = 1;
					}
					break;
				}
				// Immortal creatures
				case 0x01: {
					ITEM_INFO *item = NGIsLaraCollidingWithCreature(NG_CREATURE_TYPE_IMMORTAL, NG_COLLISION_TYPE_PUSH);
					if (item) {
						NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(item));
						result = 1;
					}
					break;
				}
				// Friends
				case 0x02: {
					ITEM_INFO *item = NGIsLaraCollidingWithCreature(NG_CREATURE_TYPE_FRIEND, NG_COLLISION_TYPE_PUSH);
					if (item) {
						NGStoreItemIndexConditional(T4PlusGetIDForItemInfo(item));
						result = 1;
					}
					break;
				}
				default: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_TOUCHING_CREATURE_TYPE %u unimplemented!", main_argument);
					break;
				}
			}
			break;
		}
		case LARA_IS_VITALITY_IS_X_THAN: {
			*test_restore = true;
			switch (extra) {
				// Equal than
				case 0: {
					result = (lara_item->hit_points == main_argument);
					break;
				}
				// Higher than
				case 1: {
					result = (lara_item->hit_points > main_argument);
					break;
				}
				// Less than
				case 2: {
					result = (lara_item->hit_points < main_argument);
					break;
				}
				default: {
					NGLog(NG_LOG_TYPE_ERROR, "NGCondition: LARA_IS_VITALITY_IS_X_THAN unknown main_argument!");
					break;
				}
			}
			break;
		}
		case LARA_IS_PERFORMING_ANIMATION: {
			if (lara_item->anim_number - objects[T4PlusGetLaraSlotID()].anim_index == main_argument) {
				result = 1;
			}
			break;
		}
		case LARA_IS_STATE: {
			if (lara_item->current_anim_state == main_argument) {
				result = 1;
			}
			break;
		}
		case ANIMTEXTURE_THE_X_ANIMRANGE_TEXTURE_IS_ENABLED_OR_DISABLED: {
			*test_restore = true;
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ANIMTEXTURE_THE_X_ANIMRANGE_TEXTURE_IS_ENABLED_OR_DISABLED is not currently implemented!");
			break;
		}
		case LARA_IS_TOUCHING_STATIC_SLOT: {
			int id = NGIsLaraCollidingWithStaticID(main_argument);
			if (id >= 0) {
				result = 1;
			}
			break;
		}
		case LARA_IS_TOUCHING_STATIC_ID: {
			int id = NGIsLaraCollidingWithStaticID(main_argument);
			if (id >= 0) {
				result = 1;
			}
			break;
		}
		case LARA_IS_HOLDING_OR_DRIVING_ITEMS: {
			result = NGIsLaraHolding(main_argument);
			break;
		}
		case CREATURE_THE_X_CREATURE_HAS_THE_TRANSPARENCY_LEVEL: {
			*test_restore = true;
			*test_skip = true;
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: CREATURE_THE_X_CREATURE_HAS_THE_TRANSPARENCY_LEVEL unimplemented type: 0x%02x!", extra);
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VERIABLE_IS_EQUAL_OR_GREATER_TO_BIG_NUMBER_VALUE: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) >= current_big_numbers[extra].big_number)
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VERIABLE_IS_LESS_THAN_BIG_NUMBER_VALUE: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) < current_big_numbers[extra].big_number)
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VERIABLE_IS_EQUAL_TO_BIG_NUMBER_VALUE: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) == current_big_numbers[extra].big_number)
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VARIABLE_IS_EQUAL_OR_GREATER_TO: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) >= extra)
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VARIABLE_IS_LESS_THAN: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) < extra)
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VARIABLE_IS_EQUAL_TO: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) == extra)
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VARIABLE_HAS_THE_BIT_SET: {
			*test_restore = true;
			*test_skip = false;
			if (NGNumericGetVariable(main_argument) & (1 << extra))
				result = 1;
			break;
		}
		case VARIABLES_THE_X_NUMERIC_VARIABLE_HAS_THE_BIT_CLEAR: {
			*test_restore = true;
			*test_skip = false;
			if (!(NGNumericGetVariable(main_argument) & (1 << extra)))
				result = 1;
			break;
		}
		case VARIABLES_THE_CURRENT_VALUE_VARIABLE_IS_GREATER_OR_EQUAL_THAN_X_VALUE: {
			*test_restore = true;
			*test_skip = false;
			if (ng_current_value >= main_argument)
				result = 1;
			break;
		}
		case VARIABLES_THE_CURRENT_VALUE_VARIABLE_IS_LESS_THAN_X_VALUE: {
			*test_restore = true;
			*test_skip = false;
			if (ng_current_value < main_argument)
				result = 1;
			break;
		}
		case VARIABLES_THE_CURRENT_VALUE_VARIABLE_IS_EQUAL_THAN_X_VALUE: {
			*test_restore = true;
			*test_skip = false;
			if (ng_current_value == main_argument)
				result = 1;
			break;
		}
		case LARA_IS_LESS_OR_EVEN_CLICKS_DISTANT_TO_MOVEABLE: {
			*test_restore = true;
			*test_skip = true;
			result = NGIsSourcePositionLessThanDistanceToTargetPosition(&lara_item->pos, &T4PlusGetItemInfoForID(main_argument)->pos, extra * CLICK_SIZE, false);
			break;
		}
		case LARA_IS_LESS_OR_EVEN_UNITS_DISTANT_TO_MOVEABLE: {
			*test_restore = true;
			*test_skip = true;
			result = NGIsSourcePositionLessThanDistanceToTargetPosition(&lara_item->pos, &T4PlusGetItemInfoForID(main_argument)->pos, extra, false);
			break;
		}
		case LARA_IS_IN_ROOM_TYPE: {
			*test_restore = false;
			*test_skip = false;
			ROOM_INFO* r = &room[lara_item->room_number];
			if (r) {
				int room_flags = r->flags;
				switch (main_argument) {
					case 0: {
						result = room_flags & ROOM_UNDERWATER;
						break;
					}
					case 1: {
						result = room_flags & 0x02;
						break;
					}
					case 2: {
						result = room_flags & ROOM_SWAMP;
						break;
					}
					case 3: {
						result = room_flags & ROOM_OUTSIDE;
						break;
					}
					case 4: {
						result = room_flags & ROOM_DAMAGE;
						break;
					}
					case 5: {
						result = room_flags & ROOM_NOT_INSIDE;
						break;
					}
					case 6: {
						result = room_flags & ROOM_INSIDE;
						break;
					}
					case 7: {
						result = room_flags & ROOM_NO_LENSFLARE;
						break;
					}
					case 8: {
						result = room_flags & ROOM_CAUSTICS;
						break;
					}
					case 9: {
						result = room_flags & ROOM_REFLECTIONS;
						break;
					}
					case 10: {
						result = room_flags & ROOM_SNOW;
						break;
					}
					case 11: {
						result = room_flags & ROOM_RAIN;
						break;
					}
					case 12: {
						result = room_flags & ROOM_COLD;
						break;
					}
					default: {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_IN_ROOM_TYPE unsupported room type!");
						break;
					}
				}
				break;
			}
			break;
		}
		case LARA_IS_TOUCHING_MOVEABLE_WITH_MESH_NUMBER: {
			*test_restore = true;
			*test_skip = true;
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_TOUCHING_MOVEABLE_WITH_MESH_NUMBER unimplemented!");
			break;
		}
		case SOUND_THE_X_SOUND_EFFECT_IS_PLAYING: {
			*test_restore = false;
			*test_skip = false;
			result = DSIsSamplePlaying(main_argument);
			break;
		}
		default:
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: %u unimplemented!", condition_number);
			break;
	};

	if (NGGetIsInsideDummyTrigger())
		*repeat_type = 0;

	return result;
};

int NGRunCondition(uint16_t plugin_id, uint16_t condition_number, uint16_t index, uint16_t extra_buttons, bool *test_restore, bool *test_skips, int *repeat_type, uint16_t flags) {
	int result = 0;

	if (NGGetInsideConditionCount() != 0) {
		*repeat_type = 0;
		*test_restore = false;
		*test_skips = false;
		return 0;
	}

	if (NGGetIsInsideDummyTrigger()) {
		if (plugin_id > 0) {
			*repeat_type = 0;
			*test_restore = false;
			*test_skips = false;
			return 0;
		}

		if (!NGIsValidConditionForDummy(condition_number, index, true)) {
			*repeat_type = 0;
			*test_restore = false;
			*test_skips = false;
			return 0;
		}
	}

	if (plugin_id > 0) {
		if (plugin_id > 255) {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid plugin ID for action trigger %d", condition_number);
			*repeat_type = 0;
			*test_restore = false;
			*test_skips = false;
			return 0;
		}

		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Condition plugin triggers are not yet implemented!");
		return 2;
	}

	// TODO: CALLBACK FIRST

	// TODO: CALLBACK REPLACE

	result = NGPerformTRNGCondition(condition_number, index, extra_buttons, test_restore, test_skips, repeat_type, flags);

	// TODO: CALLBACK AFTER

	return result;
}

int NGRunConditionTrigger(uint16_t *current_floor_data) {
	NGStoreTestConditionsFound(true);

	size_t offset_condition = (size_t)current_floor_data;
	size_t offset_now = offset_condition - (size_t)floor_data;

	for (int32_t i = 0; i < old_condition_count; i++) {
		if (old_conditions[i].offset_floor_data == offset_now) {
			return 0;
		}
	}

	uint16_t plugin_id = NGGetPluginIDForFloorData((offset_now >> 1), true);

	bool is_oneshot = false;
	if (NGGetFloorTriggerNow()[1] & IFL_INVISIBLE) {
		is_oneshot = true;
	}

	bool is_heavy = false;
	if (NGGetIsHeavyTesting()) {
		is_heavy = true;
	}

	int repeat_type = 0;

	uint16_t condition = current_floor_data[0] & 0xff;
	uint16_t index = current_floor_data[1] & 0x3ff;
	uint16_t extra = current_floor_data[0] >> 9;
	extra &= 0x1f;

	bool test_restore = false;

	uint16_t flags = SCANF_FLOOR_DATA;
	if (is_oneshot) {
		flags |= SCANF_BUTTON_ONE_SHOT;
	}
	if (is_heavy) {
		flags |= SCANF_HEAVY;
	}

	bool test_skips = true;

	// Run condition
	int32_t result = NGRunCondition(plugin_id, condition, index, extra, &test_restore, &test_skips, &repeat_type, flags);

	if (test_restore) {
		stored_save_trigger_buttons |= IFL_CODEBITS;
		stored_save_trigger_buttons &= (IFL_CODEBITS | IFL_INVISIBLE);
	}

	if (result) {
		if (test_skips) {
			result = 2;
		}
	}

	if (!NGGetIsInsideDummyTrigger() && (result || repeat_type == 3) && (repeat_type != 0 || is_oneshot)) {
		if (repeat_type == 3) {
			repeat_type = 1;
		}

		if (is_oneshot && result) {
			repeat_type = 2;
		}

		int32_t last_condition = 0;
		for (int32_t i = 0; i < old_condition_count; i++) {
			last_condition = i;
			if (old_conditions[i].offset_floor_data == 0) {
				break;
			}
		}

		if (last_condition == old_condition_count) {
			old_condition_count++;
		}

		flags = SCANF_FLOOR_DATA;
		if (is_heavy) {
			flags |= SCANF_HEAVY;
		}
		if (repeat_type == 1) {
			flags |= SCANF_TEMP_ONE_SHOT;
		}

		old_conditions[last_condition].flags = flags;
		old_conditions[last_condition].offset_floor_data = offset_now;
	}

	return result;
}

bool NGIsValidConditionForDummy(int32_t condition_number, int32_t main_argument, bool test_first) {
	if (test_first) {
		if (condition_number == MULTIPLE_CONDITION_OF_X_TRIGGERGROUP_SCRIPT_COMMAND) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "MULTIPLE_CONDITION_OF_X_TRIGGERGROUP_SCRIPT_COMMAND unimplemented as dummy condition");
			return false;
		}

		if (condition_number == MULTIPLE_CONDITION_OF_X_MULTIENVCONDITION_SCRIPT_COMMAND) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "MULTIPLE_CONDITION_OF_X_MULTIENVCONDITION_SCRIPT_COMMAND unimplemented as dummy condition");
			return false;
		}
	}

	// TODO add missing conditions.
	if (condition_number == FRAGMENTED_TRIGGGER_CHECK_IN_WAY_IF_LARA_IS_IN_FRAGMENT_OF_2X2_SECTOR_GRID ||
	        condition_number == FRAGMENTED_TRIGGGER_CHECK_IN_WAY_IF_LARA_IS_IN_FRAGMENT_OF_3X3_SECTOR_GRID ||
	        condition_number == FRAGMENTED_TRIGGGER_CHECK_IN_WAY_IF_LARA_IS_IN_FRAGMENT_OF_4X4_SECTOR_GRID) {
		return true;
	}

	return false;
}

bool NGAnalyzeDummyCondition(uint16_t *current_floor_data) {
	size_t offset_condition = (size_t)current_floor_data;
	size_t offset_now = offset_condition - (size_t)floor_data;

	uint16_t plugin_id = NGGetPluginIDForFloorData((offset_now >> 1), true);
	if (plugin_id > 0) {
		return false;
	}

	uint16_t condition_number = current_floor_data[0] & 0xff;
	uint16_t index = current_floor_data[1] & 0x3ff;
	uint16_t extra = current_floor_data[0] >> 9;
	extra &= 0x1f;

	if (!NGIsValidConditionForDummy(condition_number, index, true)) {
		return false;
	}

	bool test_restore = false;
	bool test_skip = true;
	int32_t repeat_type = 0;
	uint32_t result = 0;

	// TODO - store

	result = NGRunCondition(0, condition_number, index, extra, &test_restore, &test_skip, &repeat_type, SCANF_DIRECT_CALL);

	// TODO - restore

	if (result == 0) {
		NGStoreTestDummyFailed(true);
		return false;
	}

	return true;
}

void NGProcessConditions() {
	if (!NGGetTestConditionsFound()) {
		for (int i = 0; i < old_condition_count; i++) {
			if (old_conditions[i].flags & SCANF_TEMP_ONE_SHOT) {
				old_conditions[i].offset_floor_data = 0;
			}
		}
	}
}