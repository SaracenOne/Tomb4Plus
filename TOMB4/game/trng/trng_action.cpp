#include "../../tomb4/pch.h"

#include "trng.h"
#include "trng_action.h"
#include "trng_extra_state.h"

#include "../../specific/function_stubs.h"
#include "../../specific/file.h"
#include "../../tomb4/mod_config.h"

#include "../../tomb4/tomb4plus/t4plus_items.h"

#include "../objects/creatures/baboon.h"

#include "../control.h"
#include "../effects.h"
#include "../objects.h"
#include "../door.h"
#include "../items.h"
#include "../lot.h"
#include "../effect2.h"
#include "../lara1gun.h"
#include "../sound.h"
#include "../box.h"
#include "../camera.h"
#include "../spotcam.h"
#include "../lara.h"
#include "../traps.h"
#include "../gameflow.h"
#include "trng_progressive_action.h"

uint32_t scanned_action_count;
NGScannedAction scanned_actions[NG_MAX_SCANNED_ACTIONS];
uint32_t old_action_count;
NGOldTrigger old_actions[NG_MAX_OLD_ACTIONS];

void NGHurtEnemy(uint16_t item_id, uint16_t damage) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		if (item->hit_points > 0) {
			item->hit_points -= damage;
		}
	}
}

int32_t NGPerformTRNGAction(uint16_t action_timer, uint16_t item_id, int32_t flags) {
	static uint16_t degrees_table[] = { 0x2000,0x4000,0x6000,0x8000,0xA000,0xC000,0xE000,0x0000 };
	const int32_t DEGREES_TABLE_COUNT = sizeof(degrees_table) / sizeof(uint16_t);

	uint8_t action_type = (uint8_t)action_timer & 0xff;
	uint8_t extra_timer = (uint8_t)(action_timer >> 8) & 0xff;

	int32_t repeat_type = 1;

	if (item_id < 0) {
		NGLog(NG_LOG_TYPE_ERROR, "ActionNG: Negative item ID!");
		return repeat_type;
	}

	if (item_id >= ITEM_COUNT) {
		NGLog(NG_LOG_TYPE_ERROR, "Invalid item number overflow.");
		return repeat_type;
	}

	switch (action_type) {
		// TODO: values are estimated and may not be accurate.
		// Also need to check the behaviour when an action is already
		case TURN_X_ANIMATING_MOVING_SLOWLY_IN_CLOCKWISE_OF_DEGREES:
		case TURN_X_ANIMATING_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_OF_DEGREES:
		case TURN_X_ANIMATING_MOVING_FASTLY_IN_CLOCKWISE_OF_DEGREES:
		case TURN_X_ANIMATING_MOVING_FASTLY_IN_INVERSE_CLOCKWISE_OF_DEGREES: {
			if (extra_timer >= DEGREES_TABLE_COUNT) {
				break;
			}

			bool test_abort = false;
			for (int32_t i = 0; i < progressive_action_count; i++) {
				NGProgressiveAction* progressive_action = &progressive_actions[i];
				if (progressive_action->item_index == item_id &&
				        progressive_action->type == action_type) {
					test_abort = true;
					break;
				}
			}

			if (test_abort) {
				repeat_type = 1;
				break;
			}

			NGProgressiveAction* progressive_action = NGCreateProgressiveAction();
			if (!progressive_action) {
				break;
			}

			int32_t speed = 352;
			if (action_type == TURN_X_ANIMATING_MOVING_SLOWLY_IN_CLOCKWISE_OF_DEGREES ||
			        action_type == TURN_X_ANIMATING_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_OF_DEGREES) {
				speed = 160;
			}

			progressive_action->type = AZ_ROTATE_ITEM_HORIZONTAL;
			progressive_action->duration = (degrees_table[extra_timer] / speed) & 0xffff;
			if (progressive_action->duration < 1) {
				progressive_action->duration = 1;
			}
			progressive_action->argument1_u16 = speed;

			NGAddItemMoved(item_id);

			progressive_action->item_index = item_id;
			if (action_type == TURN_X_ANIMATING_MOVING_SLOWLY_IN_CLOCKWISE_OF_DEGREES ||
			        action_type == TURN_X_ANIMATING_MOVING_FASTLY_IN_CLOCKWISE_OF_DEGREES) {
				progressive_action->argument2_i32[0] = 1;
			} else {
				progressive_action->argument2_i32[0] = -1;
			}

			repeat_type = 1;

			break;
		}
		case TURN_X_ANIMATING_MOVING_ENDLESSLY_IN_WAY: {
			NGProgressiveAction *progressive_action = nullptr;
			for (int32_t i = 0; i < progressive_action_count; i++) {
				if (progressive_actions[i].item_index == item_id &&
				        (progressive_actions[i].type == AZ_ROTATE_ITEM_HORIZONTAL || progressive_actions[i].type == AZ_TURN_FACING_HORIZONTAL)) {
					progressive_action = &progressive_actions[i];
					break;
				}
			}

			if (!progressive_action) {
				progressive_action = NGCreateProgressiveAction();
				if (!progressive_action) {
					break;
				}
			}

			int32_t speed = 352;
			if ((extra_timer & 1) == 0)
				speed = 160;

			progressive_action->duration = 0xffff;
			progressive_action->argument1_u16 = speed;
			progressive_action->type = AZ_ROTATE_ITEM_HORIZONTAL;

			NGAddItemMoved(item_id);

			if (extra_timer & 2)
				progressive_action->argument2_i32[0] = -1;
			else
				progressive_action->argument2_i32[0] = 1;

			repeat_type = 1;

			break;
		}
		case TURN_VERTICALLY_X_ANIMATING_MOVING_SLOWLY_IN_CLOCKWISE_OF_DEGREES:
		case TURN_VERTICALLY_X_ANIMATING_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_OF_DEGREES:
		case TURN_VERTICALLY_X_ANIMATING_MOVING_FASTLY_IN_CLOCKWISE_OF_DEGREES:
		case TURN_VERTICALLY_X_ANIMATING_MOVING_FASTLY_IN_INVERSE_CLOCKWISE_OF_DEGREES: {
			if (extra_timer >= DEGREES_TABLE_COUNT) {
				break;
			}

			NGProgressiveAction* progressive_action = nullptr;
			for (int32_t i = 0; i < progressive_action_count; i++) {
				if (progressive_actions[i].item_index == item_id &&
				        (progressive_actions[i].type == AZ_ROTATE_ITEM_VERTICAL || progressive_actions[i].type == AZ_TURN_FACING_VERTICAL)) {
					progressive_action = &progressive_actions[i];
					break;
				}
			}

			if (!progressive_action) {
				progressive_action = NGCreateProgressiveAction();
				if (!progressive_action) {
					break;
				}
			}

			// Huh?
			int32_t speed = 352;
			if (action_type == TURN_X_ANIMATING_MOVING_SLOWLY_IN_CLOCKWISE_OF_DEGREES ||
			        action_type == TURN_X_ANIMATING_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_OF_DEGREES) {
				speed = 160;
			}

			progressive_action->type = AZ_ROTATE_ITEM_VERTICAL;
			progressive_action->duration = (degrees_table[extra_timer] / speed) & 0xffff;
			if (progressive_action->duration < 1) {
				progressive_action->duration = 1;
			}
			progressive_action->argument1_u16 = speed;

			NGAddItemMoved(item_id);

			progressive_action->item_index = item_id;
			if (action_type == TURN_VERTICALLY_X_ANIMATING_MOVING_SLOWLY_IN_CLOCKWISE_OF_DEGREES ||
			        action_type == TURN_VERTICALLY_X_ANIMATING_MOVING_FASTLY_IN_CLOCKWISE_OF_DEGREES) {
				progressive_action->argument2_i32[0] = 1;
			} else {
				progressive_action->argument2_i32[0] = -1;
			}

			repeat_type = 1;

			break;
		}
		case TURN_VERTICALLY_X_ANIMATING_MOVING_ENDLESS_IN_WAY: {
			NGProgressiveAction* progressive_action = nullptr;
			for (int32_t i = 0; i < progressive_action_count; i++) {
				if (progressive_actions[i].item_index == item_id &&
				        (progressive_actions[i].type == AZ_ROTATE_ITEM_VERTICAL || progressive_actions[i].type == AZ_TURN_FACING_VERTICAL)) {
					progressive_action = &progressive_actions[i];
					break;
				}
			}

			if (!progressive_action) {
				progressive_action = NGCreateProgressiveAction();
				if (!progressive_action) {
					break;
				}
			}

			int32_t speed = 352;
			if ((extra_timer & 1) == 0)
				speed = 160;

			progressive_action->duration = 0xffff;
			progressive_action->argument1_u16 = speed;
			progressive_action->type = AZ_ROTATE_ITEM_VERTICAL;

			NGAddItemMoved(item_id);

			if (extra_timer & 2)
				progressive_action->argument2_i32[0] = -1;
			else
				progressive_action->argument2_i32[0] = 1;

			repeat_type = 1;

			break;
		}
		case TURN_IMMEDIATELY_X_OF_DEGREES_CLOCKWISE: {
			// Should we add NGAddItemMoved here?

			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item && extra_timer < DEGREES_TABLE_COUNT) {
				item->pos.y_rot += degrees_table[extra_timer];
			}
			break;
		}
		case TURN_IMMEDIATELY_X_OF_DEGREES_INVERSE_CLOCKWISE: {
			// Should we add NGAddItemMoved here?

			ITEM_INFO* item = T4PlusGetItemInfoForID(item_id);
			if (item && extra_timer < DEGREES_TABLE_COUNT) {
				item->pos.y_rot -= degrees_table[extra_timer];
			}
			break;
		}
		case PERFORM_FLIPEFFECT_ON_ITEM: {
			// TODO: the repeattype of this action is inaccurate.
			repeat_type = 0;
			// TODO extra item routine
			flipeffect = extra_timer;

			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				effect_routines[extra_timer](item);
			}

			NGAddItemMoved(item_id);

			if (flipeffect == -1) {
				repeat_type = 1;
			}

			break;
		}
		case KILL_OBJECT: {
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				if (item->object_number != get_game_mod_level_objects_info(gfCurrentLevel)->lara_slot
				        && !item->active
				        && item->status == ITEM_ACTIVE) {

					if (item->collidable) {
						repeat_type = 0;
					}
					break;
				}

				switch (extra_timer) {
					// 0 vitality
					case 0x00: {
						item->hit_points = 0;
						break;
					}
					// Remove immediately
					case 0x01: {
						KillItem(item_id);
						break;
					}
					// Explosion / Underwater Explosion
					case 0x02:
					case 0x03: {
						if (extra_timer == 0x02) {
							TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -2, 0, item->room_number);
							for (int32_t i = 0; i < 3; i++) {
								TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -1, 0, item->room_number);
							}
						} else {
							TriggerUnderwaterExplosion(item, 0);
						}

						KillItem(item_id);

						item->status = ITEM_DEACTIVATED;

						SoundEffect(SFX_EXPLOSION1, nullptr, 0);
						SoundEffect(SFX_EXPLOSION2, nullptr, 0);

						item->hit_points = INFINITE_HEALTH;
						break;
					}
					// Explode Creature
					case 0x04: {
						CreatureDie(item_id, true);
						SoundEffect(SFX_EXPLOSION1, nullptr, 0);
						SoundEffect(SFX_EXPLOSION2, nullptr, 0);
						item->hit_points = INFINITE_HEALTH;
						break;
					}
					// Creature Die
					case 0x05: {
						// We may not want to set this if using persistent enemy bodies.
						// T4Plus' implementation is different though.
						item->after_death = 1;
						item->status = ITEM_DEACTIVATED;
						item->hit_points = 0;

						break;
					}
					// Hide
					case 0x06: {
						// Seems accurate but WTF?
						item->pos.x_pos = 0;
						break;
					}
					// Antitrigger
					case 0x07: {
						// Accurate!
						item->flags |= IFL_CLEARBODY;
						item->item_flags[0] = 0;
						break;
					}
					// Smoke emitter
					case 0x08: {
						RemoveActiveItem(item_id);
						break;
					}
					default: {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "KILL_OBJECT: action data %u unimplemented!", extra_timer);
						break;
					}
				}
			}
			break;
		}
		case FORCE_ANIMATION_0_TO_31_ON_ITEM: {
			NGSetItemAnimation(item_id, extra_timer & 0x1f, true, false, false, true);
			break;
		}
		case FORCE_ANIMATION_32_TO_63_ON_ITEM: {
			NGSetItemAnimation(item_id, (extra_timer & 0x1f) + 32, true, false, false, true);
			break;
		}
		case FORCE_ANIMATION_64_TO_95_ON_ITEM: {
			NGSetItemAnimation(item_id, (extra_timer & 0x1f) + 64, true, false, false, true);
			break;
		}
		case TURN_X_ANIMATION_MOVING_SLOWLY_IN_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_X_ANIMATION_MOVING_SLOWLY_IN_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_X_ANIMATION_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_X_ANIMATION_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_X_ANIMATION_MOVING_FASTLY_IN_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_X_ANIMATION_MOVING_FASTLY_IN_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_X_ANIMATION_MOVING_FASTLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_X_ANIMATION_MOVING_FASTLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_VERTICALLY_X_ANIMATION_MOVING_SLOWLY_IN_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_VERTICALLY_X_ANIMATION_MOVING_SLOWLY_IN_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_VERTICALLY_X_ANIMATION_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_VERTICALLY_X_ANIMATION_MOVING_SLOWLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_VERTICALLY_X_ANIMATION_MOVING_FASTLY_IN_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_VERTICALLY_X_ANIMATION_MOVING_FASTLY_IN_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case TURN_VERTICALLY_X_ANIMATION_MOVING_FASTLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_VERTICALLY_X_ANIMATION_MOVING_FASTLY_IN_INVERSE_CLOCKWISE_UNTIL_FACING unimplemented!");
			break;
		}
		case OPEN_OR_CLOSE_DOOR_ITEM: {
			repeat_type = 0;

			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				T4PlusActivateItem(item_id, false);
				item->timer = 0;

				bool reverse = (item->flags & IFL_REVERSE) ? true : false;

				if (extra_timer) {
					if (reverse) {
						item->flags &= ~(IFL_CODEBITS);
					} else {
						item->flags |= IFL_CODEBITS;
						item->timer = (int16_t)(NGGetLastTriggerTimer() & 0xffff) * NG_TICKS_PER_SECOND;
					}
				} else {
					if (reverse) {
						item->flags |= IFL_CODEBITS;
						item->timer = (int16_t)(NGGetLastTriggerTimer() & 0xffff) * NG_TICKS_PER_SECOND;
					} else {
						item->flags &= ~(IFL_CODEBITS);
					}
				}
			}
			break;
		}
		case MOVE_CONTINUOUSLY_FORWARD_BACKWARD_X_ANIMATING_FOR_CLICKS:
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGAction: MOVE_CONTINUOUSLY_FORWARD_BACKWARD_X_ANIMATING_FOR_CLICKS may not be accurate.");

			NGAddItemMoved(item_id);

			if (!NGGetItemHorizontalMovementRemainingUnits(item_id)) {
				ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
				if (item) {
					NGSetItemHorizontalMovementAngle(item_id, item->pos.y_rot);
					NGSetItemHorizontalMovementRemainingUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
					NGSetItemHorizontalMovementRepeatUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
					NGSetItemHorizontalMovementSpeed(item_id, 32);
					NGSetItemMovementInProgressSound(item_id, -1);
					NGSetItemMovementFinishedSound(item_id, -1);
					NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
					NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
					NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
				}
			}
			break;
		case MOVE_ITEM_UP_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemVerticalMovementRemainingUnits(item_id)) {
				NGSetItemVerticalMovementRemainingUnits(item_id, (extra_timer + 1) * -CLICK_SIZE);
				NGSetItemVerticalMovementRepeatUnits(item_id, 0);
				NGSetItemVerticalMovementSpeed(item_id, -32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case MOVE_ITEM_DOWN_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemVerticalMovementRemainingUnits(item_id)) {
				NGSetItemVerticalMovementRemainingUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
				NGSetItemVerticalMovementRepeatUnits(item_id, 0);
				NGSetItemVerticalMovementSpeed(item_id, 32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case MOVE_ITEM_WEST_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemHorizontalMovementRemainingUnits(item_id)) {
				NGSetItemHorizontalMovementAngle(item_id, (int16_t)0xC000);
				NGSetItemHorizontalMovementRemainingUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
				NGSetItemHorizontalMovementRepeatUnits(item_id, 0);
				NGSetItemHorizontalMovementSpeed(item_id, 32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case MOVE_ITEM_NORTH_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemHorizontalMovementRemainingUnits(item_id)) {
				NGSetItemHorizontalMovementAngle(item_id, (int16_t)0x0000);
				NGSetItemHorizontalMovementRemainingUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
				NGSetItemHorizontalMovementRepeatUnits(item_id, 0);
				NGSetItemHorizontalMovementSpeed(item_id, 32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case MOVE_ITEM_EAST_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemHorizontalMovementRemainingUnits(item_id)) {
				NGSetItemHorizontalMovementAngle(item_id, (int16_t)0x4000);
				NGSetItemHorizontalMovementRemainingUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
				NGSetItemHorizontalMovementRepeatUnits(item_id, 0);
				NGSetItemHorizontalMovementSpeed(item_id, 32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case MOVE_ITEM_SOUTH_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemHorizontalMovementRemainingUnits(item_id)) {
				NGSetItemHorizontalMovementAngle(item_id, (int16_t)0x8000);
				NGSetItemHorizontalMovementRemainingUnits(item_id, (extra_timer + 1) * CLICK_SIZE);
				NGSetItemHorizontalMovementRepeatUnits(item_id, 0);
				NGSetItemHorizontalMovementSpeed(item_id, 32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case MOVE_CONTINUOUSLY_UPSTAIRS_DOWNSTAIRS_X_ANIMATING_FOR_CLICKS:
			NGAddItemMoved(item_id);

			if (!NGGetItemVerticalMovementRemainingUnits(item_id)) {
				NGSetItemVerticalMovementRemainingUnits(item_id, (extra_timer + 1) * -CLICK_SIZE);
				NGSetItemVerticalMovementRepeatUnits(item_id, -((extra_timer + 1) * -CLICK_SIZE));
				NGSetItemVerticalMovementSpeed(item_id, -32);
				NGSetItemMovementInProgressSound(item_id, -1);
				NGSetItemMovementFinishedSound(item_id, -1);
				NGSetItemMovementTriggerHeavyAtEnd(item_id, false);
				NGSetItemMovementTriggerNormalWhenMoving(item_id, false);
				NGSetItemMovementTriggerHeavyWhenMoving(item_id, false);
			}
			break;
		case HURT_ENEMY: {
			NGHurtEnemy(item_id, extra_timer & 0x7f);
			break;
		}
		case MOVE_ITEM_IMMEDIATELY_TO_LARA_START_POS_WITH_MATCHING_OCB_SETTINGS: {
			int32_t lara_start_pos_id = NGFindIndexForLaraStartPosWithMatchingOCB(extra_timer & 0x7f);
			if (lara_start_pos_id >= 0) {
				AIOBJECT *ai = &AIObjects[lara_start_pos_id];
				if (ai) {
					ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
					if (item) {
						item->pos.x_pos = ai->x;
						item->pos.y_pos = ai->y;
						item->pos.z_pos = ai->z;
						item->pos.y_rot = ai->y_rot;
						item->room_number = ai->room_number;
					}
				}
			}
			break;
		}
		case ACTIVATE_CAMERA_WITH_TIMER: {
			if (item_id >= number_cameras) {
				NGLog(NG_LOG_TYPE_ERROR, "Invalid camera number.");
				break;
			}

			if (camera.fixed[item_id].flags & 0x100)
				break;

			camera.number = item_id;

			if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA) {
				if (!(camera.fixed[item_id].flags & 0x01)) {
					break;
				}
			}

			if (NGGetLastTriggerTimer() != 0) {
				camera.timer = NGGetLastTriggerTimer() * NG_TICKS_PER_SECOND;
			} else {
				camera.timer = extra_timer * NG_TICKS_PER_SECOND;
			}

			camera.speed = 1;
			if (ng_camera_target_id == NO_ITEM) {
				ng_camera_target_id = lara.item_number;
			}

			ITEM_INFO *camera_item = T4PlusGetItemInfoForID(ng_camera_target_id);
			if (camera_item) {
				camera.item = camera_item;
			}

			if (flags & SCANF_BUTTON_ONE_SHOT) {
				camera.flags |= 0x100;
			}
			if (flags & SCANF_HEAVY) {
				camera.type = HEAVY_CAMERA;
			} else {
				camera.type = FIXED_CAMERA;
			}

			break;
		}
		case SET_MOVEABLE_AS_TARGET_FOR_CAMERA: {
			if (item_id >= ITEM_COUNT) {
				NGLog(NG_LOG_TYPE_ERROR, "Invalid camera target.");
				break;
			}

			ng_camera_target_id = item_id;
			break;
		}
		case TRIGGER_MOVEABLE_ACTIVATE_WITH_TIMER: {
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				item->timer = ((int16_t)(extra_timer & 0x7f)) * NG_TICKS_PER_SECOND;
				T4PlusActivateItem(item_id, false);
			}
			break;
		}
		case UNTRIGGER_MOVEABLE_ACTIVATE_WITH_TIMER: {
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				item->timer = ((int16_t)(extra_timer & 0x7f)) * NG_TICKS_PER_SECOND;
				T4PlusActivateItem(item_id, true);
			}
			break;
		}
		case ACTIVATE_OR_UNTRIGGER_FLYBY_SEQUENCE: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "ACTIVATE_OR_UNTRIGGER_FLYBY_SEQUENCE may not be implemented correctly!");

			if (item_id >= MAXIMUM_SPOTCAMS) {
				NGLog(NG_LOG_TYPE_ERROR, "Invalid spotcam!");
			}

			if (extra_timer == 0) {
				if (!(SpotCam[item_id].flags & SP_FLYBYONESHOT)) {
					bUseSpotCam = 1;

					if (flags & SCANF_BUTTON_ONE_SHOT) {
						SpotCam[item_id].flags |= SP_FLYBYONESHOT;
					}

					InitialiseSpotCam(item_id);
				}
			} else if (extra_timer == 1) {
				if (bUseSpotCam) {
					SpotcamResetFOV();
					bUseSpotCam = 0;
				}
			} else {
				NGLog(NG_LOG_TYPE_ERROR, "ACTIVATE_OR_UNTRIGGER_FLYBY_SEQUENCE invalid action data!");
			}
			break;
		}
		case EFFECT_ADD_TO_ENEMY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "EFFECT_ADD_TO_ENEMY unimplemented!");
			break;
		}
		case EFFECT_REMOVE_TO_ENEMY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "EFFECT_REMOVE_TO_ENEMY unimplemented!");
			break;
		}
		case ENEMY_SET_MESH_AS_INVISIBLE: {
			NGToggleItemMeshVisibilityMaskBit(item_id, extra_timer, false);
			break;
		}
		case ENEMY_SET_MESH_AS_VISIBLE: {
			NGToggleItemMeshVisibilityMaskBit(item_id, extra_timer, true);
			break;
		}
		case SHOW_TRIGGER_COUNTDOWN_TIMER_FOR_ENEMY: {
			NGSetDisplayTimerForMoveableWithType(item_id, (NGTimerTrackerType)(extra_timer & 0x7f));
			break;
		}
		case SET_ENEMY_TRANSPARENCY_LEVEL: {
			NGSetFadeOverride(item_id, extra_timer);
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				item->after_death = extra_timer;
			}
			break;
		}
		case FREEZE_ENEMY_FOR_SECONDS: {
			// TODO: need to re-examine this function. Right now, the implementation of 'freezing' an enemy simply disable their control
			// update, but it's possible it might affect their status instead.
			if (!NGIsItemFrozen(item_id)) {
				if (extra_timer == 0) {
					NGSetItemFreezeTimer(item_id, 0xffff);
				} else {
					NGSetItemFreezeTimer(item_id, extra_timer * NG_TICKS_PER_SECOND);
				}
			}
			break;
		}
		case UNFREEZE_ENEMY_WITH_EFFECT: {
			if (extra_timer != 0x00) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "UNFREEZE_ENEMY_WITH_EFFECT: action data %u unimplemented!", extra_timer);
			}

			if (NGIsItemFrozen(item_id)) {
				// TODO: action_data signifies a special effect when unfreezeing
				NGSetItemFreezeTimer(item_id, 0);
			}
			break;
		}
		case ENEMY_SAVE_THE_COORDINATES_AND_FACING_OF_X_MOVEABLE_IN_SAVEGAME: {
			NGAddItemMoved(item_id);
			break;
		}
		case DISABLE_ITEM_COLLISION:
			NGDisableItemCollision(item_id);
			break;
		case ENABLE_ITEM_COLLISION:
			NGEnableItemCollision(item_id);
			break;
		case MOVE_ITEM_UP_BY_UNITS_X8: {
			NGAddItemMoved(item_id);
			NGMoveItemByUnits(item_id, NG_UP, 8 * ((extra_timer)+1));
			break;
		}
		case MOVE_ITEM_DOWN_BY_UNITS_X8: {
			NGAddItemMoved(item_id);
			NGMoveItemByUnits(item_id, NG_DOWN, 8 * ((extra_timer)+1));
			break;
		}
		case MOVE_ITEM_WEST_BY_UNITS_X8: {
			NGAddItemMoved(item_id);
			NGMoveItemByUnits(item_id, NG_WEST, 8 * ((extra_timer)+1));
			break;
		}
		case MOVE_ITEM_NORTH_BY_UNITS_X8: {
			NGAddItemMoved(item_id);
			NGMoveItemByUnits(item_id, NG_NORTH, 8 * ((extra_timer)+1));
			break;
		}
		case MOVE_ITEM_EAST_BY_UNITS_X8: {
			NGAddItemMoved(item_id);
			NGMoveItemByUnits(item_id, NG_EAST, 8 * ((extra_timer)+1));
			break;
		}
		case MOVE_ITEM_SOUTH_BY_UNITS_X8: {
			NGAddItemMoved(item_id);
			NGMoveItemByUnits(item_id, NG_SOUTH, 8 * ((extra_timer)+1));
			break;
		}
		case TURN_STOP_CIRCULAR_TURNING_FOR_X_ANIMATION_ITEM_IN_WAY: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TURN_STOP_CIRCULAR_TURNING_FOR_X_ANIMATION_ITEM_IN_WAY unimplemented!");
			break;
		}
		case ENEMY_EFFECTS_APPLY_ON_X_MOVEABLE_THE_E_EFFECT: {
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			switch (extra_timer) {
				case 0:
					item->after_death = 100;
					ExplodeBaboon(item);
					break;
				case 1:
					item->item_flags[0] = 150;
					ControlMineHelicopter(item_id);
					CreatureDie(item_id, false);
					break;
				default:
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "ENEMY_EFFECTS_APPLY_ON_X_MOVEABLE_THE_E_EFFECT unsupported mode (%u)!", extra_timer);
			}
			break;
		}
		case CREATURE_FORCE_FRAME_OF_CURRENT_ANIMATION: {
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				item->frame_number = anims[item->anim_number].frame_base + extra_timer;
			}
			break;
		}
		case CREATURE_FORCE_STATE_ID: {
			ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
			if (item) {
				item->current_anim_state = extra_timer;
				item->goal_anim_state = extra_timer; // Do we need to change the goal state too or not?
			}
			break;
		}
		case TRIGGER_SET_X_MOVEABLE_AS_ACTIVE_ITEM: {
			AddActiveItem(item_id);
			break;
		}
		case TRIGGER_REMOVE_X_MOVEABLE_AS_ACTIVE_ITEM: {
			RemoveActiveItem(item_id);
			break;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Unimplemented NGAction %u", action_type);
			break;
		}
	};

	if (repeat_type != 0 && (flags & SCANF_BUTTON_ONE_SHOT)) {
		repeat_type = 2;
	}

	return repeat_type;
};

void NGResetScanActions() {
	uint32_t j = 0;
	for (uint32_t i = 0; i < scanned_action_count; i++) {
		if (scanned_actions[i].flags & SCANF_YET_TO_PERFORM) {
			scanned_actions[j++] = scanned_actions[i];
		}
	}
	scanned_action_count = j;
}

void NGPerformActionTrigger(uint16_t action_number, int32_t item_index, uint16_t flags) {
	if (item_index & NGLE_INDEX) {
		int32_t masked_index = item_index & MASK_NGLE_INDEX;
		if (masked_index < NG_SCRIPT_ID_TABLE_SIZE) {
			NGScriptIDTableEntry* table_entry = &ng_script_id_table[NG_SCRIPT_ID_TABLE_SIZE];
			item_index = table_entry->script_index;
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Script item index is out of range!");
		}
	}

	NGExecuteActionTrigger(0, action_number, item_index, flags);
}

void NGCaptureAction(uint16_t item_index, uint16_t extra_timer, uint32_t floor_offset) {
	uint32_t offset_now = floor_offset;
	uint32_t offset_sector = 0;

	if (!NGUsingLegacyNGTriggerBehaviour()) {
		offset_sector = (trigger_index - floor_data) * sizeof(uint16_t); // May not be correct
		offset_now |= (offset_sector << 24);
	}

	for (uint32_t i = 0; i < scanned_action_count; i++) {
		if (scanned_actions[i].offset_floor_data == offset_now) {
			return;
		}
	}

	int32_t current_action_id = scanned_action_count;
	scanned_actions[current_action_id].flags = SCANF_YET_TO_PERFORM;

	if (NGGetIsHeavyTesting()) {
		scanned_actions[current_action_id].flags |= SCANF_HEAVY;
	}

	if (NGGetFloorTriggerNow()[1] & IFL_INVISIBLE) {
		scanned_actions[current_action_id].flags |= SCANF_BUTTON_ONE_SHOT;
	}

	scanned_actions[current_action_id].item_index = item_index;
	scanned_actions[current_action_id].offset_floor_data = offset_now;
	scanned_actions[current_action_id].timer = extra_timer;

	if (!NGUsingLegacyNGTriggerBehaviour()) {
		offset_now = (floor_offset - (*trigger_index));
	}

	uint16_t plugin_id = NGGetPluginIDForFloorData(offset_now >> 1, false);
	scanned_actions[current_action_id].plugin_id = plugin_id;

	scanned_action_count++;
}

int32_t NGExecuteActionTrigger(uint16_t plugin_id, uint16_t action_timer, int32_t item_index, uint16_t flags) {
	uint16_t action_number = action_timer & 0xff;
	uint16_t extra_timer = (action_number >> 8) & 0x7f;
	int32_t repeat_type = 1;

	if (NGGetIsInsideDummyTrigger()) {
		return 1;
	}

	if (plugin_id > 0) {
		if (plugin_id > 255) {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid plugin ID for action trigger %d", action_number);
			return 2;
		}

		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Action plugin triggers are not yet implemented!");
		return 2;
	}

	// TODO: Callback First

	// TODO: Callback Replace

	repeat_type = NGPerformTRNGAction(action_timer, item_index, flags);

	// TODO: Callback After

	return repeat_type;
}

void NGProcessScannedActions() {
	if (scanned_action_count == 0) {
		for (uint32_t i = 0; i < old_action_count; i++) {
			if (old_actions[i].flags & SCANF_TEMP_ONE_SHOT) {
				old_actions[i].offset_floor_data = 0;
			}
		}
	}

	for (uint32_t i = 0; i < scanned_action_count; i++) {
		uint32_t offset_floor = scanned_actions[i].offset_floor_data;
		scanned_actions[i].flags &= ~SCANF_YET_TO_PERFORM;

		bool test_run = true;
		for (uint32_t j = 0; j < old_action_count; j++) {
			if (old_actions[j].offset_floor_data == offset_floor) {
				test_run = false;
				break;
			} else {
				if (!NGUsingLegacyNGTriggerBehaviour()) {
					if (old_actions[j].offset_floor_data != 0 &&
					        old_actions[j].flags & SCANF_TEMP_ONE_SHOT &&
					        (scanned_actions[i].flags & SCANF_HEAVY) == 0) {
						if ((old_actions[j].offset_floor_data & 0xff000000) != (offset_floor & 0xff000000)) {
							old_actions[j].offset_floor_data = 0;
						}
					}
				}
			}
		}

		if (test_run) {
			NGScannedAction* current_action = &scanned_actions[i];
			int32_t repeat_type = NGExecuteActionTrigger(current_action->plugin_id, current_action->timer, current_action->item_index, current_action->flags);
			if (repeat_type > 0 && !NGGetIsInsideDummyTrigger()) {
				uint32_t last_action = 0;
				for (last_action = 0; last_action < old_action_count; last_action++) {
					if (old_actions[last_action].offset_floor_data == 0) {
						break;
					}
				}

				if (last_action == old_action_count) {
					old_action_count++;
				}

				old_actions[last_action].offset_floor_data = current_action->offset_floor_data;
				old_actions[last_action].flags = 0;
				if (repeat_type == 1) {
					old_actions[last_action].flags = SCANF_TEMP_ONE_SHOT;
				}
			}
		}
	}
}