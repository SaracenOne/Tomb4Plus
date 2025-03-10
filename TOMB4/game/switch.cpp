#include "../tomb4/pch.h"
#include "switch.h"
#include "lara_states.h"
#include "collide.h"
#include "items.h"
#include "control.h"
#include "objects.h"
#include "draw.h"
#include "laramisc.h"
#include "sound.h"
#include "../specific/3dmath.h"
#include "camera.h"
#include "../specific/input.h"
#include "lara.h"
#include "newinv.h"
#include "../tomb4/mod_config.h"
#include "gameflow.h"

static PHD_VECTOR FullBlockSwitchPos = { 0, 256, 0 };
static PHD_VECTOR SwitchPos = { 0, 0, 0 };
static PHD_VECTOR Switch2Pos = { 0, 0, 108 };
static PHD_VECTOR UnderwaterSwitchPos = { 0, -736, -416 };
static PHD_VECTOR UnderwaterSwitchPos2 = { 0, -736, 416 };
static PHD_VECTOR PulleyPos = { 0, 0, -148 };
static PHD_VECTOR TurnSwitchPos = { 650, 0, 138 };
static PHD_VECTOR TurnSwitchPosA = { 650, 0, -138 };
static PHD_VECTOR RailSwitchPos = { 0, 0, -550 };
static PHD_VECTOR RailSwitchPos2 = { 0, 0, 550 };
static PHD_VECTOR JumpSwitchPos = { 0, -208, 256 };
static PHD_VECTOR CrowbarPos = { -89, 0, -328 };
static PHD_VECTOR CrowbarPos2 = { 89, 0, 328 };
static PHD_VECTOR CogSwitchPos = { 0, 0, -856 };

static int16_t FullBlockSwitchBounds[] = {
	-(CLICK_SIZE + HALF_CLICK_SIZE),
	(CLICK_SIZE + HALF_CLICK_SIZE),
	0,
	CLICK_SIZE,
	0, HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t SwitchBounds[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t Switch2Bounds[] = {
	-BLOCK_SIZE,
	BLOCK_SIZE,
	-BLOCK_SIZE,
	BLOCK_SIZE,
	-BLOCK_SIZE,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80)
};

static int16_t UnderwaterSwitchBounds[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	-1280,
	-HALF_BLOCK_SIZE,
	-HALF_BLOCK_SIZE,
	0,
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80)
};

static int16_t UnderwaterSwitchBounds2[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	-(BLOCK_SIZE + CLICK_SIZE),
	-HALF_BLOCK_SIZE,
	0,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80)
};

static int16_t PulleyBounds[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	0,
	0,
	-HALF_BLOCK_SIZE,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t TurnSwitchBoundsA[] = {
	HALF_BLOCK_SIZE,
	(HALF_BLOCK_SIZE + CLICK_SIZE + HALF_CLICK_SIZE),
	0,
	0,
	-HALF_BLOCK_SIZE,
	0,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t TurnSwitchBoundsC[] = {
	HALF_BLOCK_SIZE,
	(HALF_BLOCK_SIZE + CLICK_SIZE + HALF_CLICK_SIZE),
	0,
	0,
	0,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t RailSwitchBounds[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	0,
	0,
	-(HALF_BLOCK_SIZE + CLICK_SIZE),
	-(CLICK_SIZE - (QUARTER_CLICK_SIZE / 2)),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t RailSwitchBounds2[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	0,
	0,
	(CLICK_SIZE - (QUARTER_CLICK_SIZE / 2)),
	(HALF_BLOCK_SIZE + CLICK_SIZE),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t JumpSwitchBounds[] = {
	-HALF_CLICK_SIZE,
	HALF_CLICK_SIZE,
	-CLICK_SIZE,
	CLICK_SIZE,
	(CLICK_SIZE + HALF_CLICK_SIZE),
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t CrowbarBounds[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	0,
	0,
	-HALF_BLOCK_SIZE,
	-CLICK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t CrowbarBounds2[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	0,
	0,
	CLICK_SIZE,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static int16_t CogSwitchBounds[] = {
	-HALF_BLOCK_SIZE,
	HALF_BLOCK_SIZE,
	0,
	0,
	-(BLOCK_SIZE + HALF_BLOCK_SIZE),
	-HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

PHD_VECTOR OldPickupPos;
uint8_t CurrentSequence;
uint8_t Sequences[MAX_SEQUENCES];
uint8_t SequenceUsed[MAX_USED_SEQUENCES];
uint8_t SequenceResults[MAX_SEQUENCES][MAX_SEQUENCES][MAX_SEQUENCES];

void FullBlockSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (input & IN_ACTION && item->status == ITEM_INACTIVE && !(item->flags & IFL_INVISIBLE) && CurrentSequence < 3 && lara.gun_status == LG_NO_ARMS &&
	        l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH || lara.IsMoving && lara.GeneralPtr == item_number) {
		if (TestLaraPosition(FullBlockSwitchBounds, item, l)) {
			if (MoveLaraPosition(&FullBlockSwitchPos, item, l)) {
				if (item->current_anim_state == 1) {
					l->current_anim_state = AS_SWITCHON;
					l->anim_number = ANIM_BLOCKSWITCH;
					item->goal_anim_state = 0;
				}

				l->goal_anim_state = AS_STOP;
				l->frame_number = anims[l->anim_number].frame_base;
				item->status = ITEM_ACTIVE;
				AddActiveItem(item_number);
				AnimateItem(item);
				lara.IsMoving = 0;
				lara.head_y_rot = 0;
				lara.head_x_rot = 0;
				lara.torso_y_rot = 0;
				lara.torso_x_rot = 0;
				lara.gun_status = LG_HANDS_BUSY;
			} else
				lara.GeneralPtr = item_number;
		} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}
	}
}

int32_t SwitchTrigger(int16_t item_number, int16_t timer) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->status == ITEM_DEACTIVATED) {
		if ((item->current_anim_state || item->object_number == JUMP_SWITCH) &&
		        (item->current_anim_state != 1 || item->object_number != JUMP_SWITCH) || timer <= 0) {
			RemoveActiveItem(item_number);
			item->status = ITEM_INACTIVE;

			if (item->item_flags[0])
				item->flags |= IFL_INVISIBLE;
		} else {
			item->timer = timer;
			item->status = ITEM_ACTIVE;

			if (timer != 1)
				item->timer *= 30;
		}

		return 1;
	}

	if (item->status != ITEM_INACTIVE) {
		if (item->flags & IFL_INVISIBLE)
			return 1;
	}

	return 0;
}

int32_t GetSwitchTrigger(ITEM_INFO* item, int16_t* ItemNos, int32_t AttatchedToSwitch) {
	FLOOR_INFO* floor;
	int16_t* data;
	int32_t num;

	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &item->room_number);
	GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (!trigger_index)
		return 0;

	data = trigger_index;

	while ((*data & 0x1F) != TRIGGER_TYPE && !(*data & 0x8000)) data++;	//get to trigger

	if (!(*data & TRIGGER_TYPE))	//no triggers, bye
		return 0;

	data += 2;
	num = 0;

	while (1) {
		if ((*data & 0x3C00) == TO_OBJECT && item != &items[*data & 0x3FF]) {
			*ItemNos++ = *data & 0x3FF;
			num++;
		}

		if (*data & 0x8000)	//gottem all
			break;

		data++;
	}

	return num;
}

void TestTriggersAtXYZ(int32_t x, int32_t y, int32_t z, int16_t room_number, bool heavy, int16_t flags) {
	GetHeight(GetFloor(x, y, z, &room_number), x, y, z);
	TestTriggers(trigger_index, heavy, flags);
}

void SwitchControl(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	item->flags |= IFL_CODEBITS;

	if (!TriggerActive(item) && !(item->flags & IFL_INVISIBLE)) {
		if (item->object_number == JUMP_SWITCH)
			item->goal_anim_state = 0;
		else
			item->goal_anim_state = 1;

		item->timer = 0;
	}

	AnimateItem(item);
}

void SwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int16_t* bounds;

	item = &items[item_number];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && lara.gun_status == LG_NO_ARMS
	        && item->status == ITEM_INACTIVE && !(item->flags & IFL_INVISIBLE) && item->trigger_flags >= 0
	        || lara.IsMoving && lara.GeneralPtr == item_number) {
		bounds = GetBoundsAccurate(item);

		if (item->trigger_flags == 3 && item->current_anim_state == 1)
			return;

		SwitchBounds[0] = bounds[0] - CLICK_SIZE;
		SwitchBounds[1] = bounds[1] + CLICK_SIZE;

		if (item->trigger_flags) {
			SwitchBounds[4] = bounds[4] - HALF_BLOCK_SIZE;
			SwitchBounds[5] = bounds[5] + HALF_BLOCK_SIZE;

			if (item->trigger_flags == 3)
				SwitchPos.z = bounds[4] - CLICK_SIZE;
			else
				SwitchPos.z = bounds[4] - HALF_CLICK_SIZE;
		} else {
			SwitchBounds[4] = bounds[4] - 200;
			SwitchBounds[5] = bounds[5] + 200;
			SwitchPos.z = bounds[4] - QUARTER_CLICK_SIZE;
		}

		if (TestLaraPosition(SwitchBounds, item, l)) {
			if (MoveLaraPosition(&SwitchPos, item, l)) {
				// TRNG
				MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
				bool inverted_state = false;
				if (global_info->trng_switch_extended_ocb && item->trigger_flags & 0x1000)
					inverted_state = true;

				MOD_LEVEL_MISC_INFO *misc_info = get_game_mod_level_misc_info(gfCurrentLevel);

				if (item->current_anim_state == (inverted_state ? 0 : 1)) {
					if (item->trigger_flags) {
						// TREP switch maker
						if (misc_info->trep_switch_maker) {
							switch (item->trigger_flags) {
								case 1:
									l->anim_number = misc_info->trep_switch_off_ocb_1_anim;
									l->current_anim_state = AS_SWITCHOFF;
									break;
								case 2:
									l->anim_number = misc_info->trep_switch_off_ocb_2_anim;
									l->current_anim_state = AS_SWITCHOFF;
									break;
								case 3:
									l->anim_number = misc_info->trep_switch_off_ocb_3_anim;
									l->current_anim_state = AS_SWITCHOFF;
									break;
								case 4:
									l->anim_number = misc_info->trep_switch_off_ocb_4_anim;
									l->current_anim_state = AS_SWITCHOFF;
									break;
								case 5:
									l->anim_number = misc_info->trep_switch_off_ocb_5_anim;
									l->current_anim_state = AS_SWITCHOFF;
									break;
								case 6:
									l->anim_number = misc_info->trep_switch_off_ocb_6_anim;
									l->current_anim_state = AS_SWITCHOFF;
									break;
								default:
									l->anim_number = item->trigger_flags + 1;
									l->current_anim_state = AS_SWITCHOFF;
									break;
							}
						} else {
							// TRNG - custom animation overwrite
							if (global_info->trng_switch_extended_ocb && (item->trigger_flags) >= 4) {
								l->anim_number = (item->trigger_flags & 0xfff); // TRNG
								if (item->trigger_flags & 0x2000)
									l->anim_number++;
								l->current_anim_state = AS_SWITCHOFF;
								l->goal_anim_state = AS_STOP;
							} else {
								l->anim_number = ANIM_HIDDENPICKUP;
								l->current_anim_state = AS_HIDDENPICKUP;
							}
						}
					} else {
						l->anim_number = ANIM_SWITCHOFF;
						l->current_anim_state = AS_SWITCHOFF;
					}

					item->goal_anim_state = inverted_state ? 1 : 0; // TRNG
				} else {
					if (!item->trigger_flags) {
						l->anim_number = ANIM_SWITCHON;
						l->current_anim_state = AS_SWITCHON;
					} else if (item->trigger_flags == 3) {
						l->anim_number = ANIM_SMALLSWITCH;
						if (get_game_mod_global_info()->fix_lara_small_switch_rotation) {
							l->current_anim_state = AS_SWITCHON;
						}
					} else {
						// TREP switch maker
						if (misc_info->trep_switch_maker) {
							switch (item->trigger_flags) {
								case 1:
									l->anim_number = misc_info->trep_switch_on_ocb_1_anim;
									l->current_anim_state = AS_SWITCHON;
									break;
								case 2:
									l->anim_number = misc_info->trep_switch_on_ocb_2_anim;
									l->current_anim_state = AS_SWITCHON;
									break;
								case 3:
									l->anim_number = misc_info->trep_switch_on_ocb_3_anim;
									l->current_anim_state = AS_SWITCHON;
									break;
								case 4:
									l->anim_number = misc_info->trep_switch_on_ocb_4_anim;
									l->current_anim_state = AS_SWITCHON;
									break;
								case 5:
									l->anim_number = misc_info->trep_switch_on_ocb_5_anim;
									l->current_anim_state = AS_SWITCHON;
									break;
								case 6:
									l->anim_number = misc_info->trep_switch_on_ocb_6_anim;
									l->current_anim_state = AS_SWITCHON;
									break;
								default:
									l->anim_number = item->trigger_flags;
									l->current_anim_state = AS_SWITCHOFF;
									break;
							}
						} else {
							// TRNG - custom animation overwrite
							if (global_info->trng_switch_extended_ocb && (item->trigger_flags) >= 4) {
								l->anim_number = item->trigger_flags & 0xfff;
								l->current_anim_state = AS_SWITCHON;
								l->goal_anim_state = AS_STOP;
							} else {
								l->anim_number = ANIM_HIDDENPICKUP;
								l->current_anim_state = AS_HIDDENPICKUP;
							}
						}
					}

					item->goal_anim_state = inverted_state ? 0 : 1; // TRNG
				}

				l->frame_number = anims[l->anim_number].frame_base;
				lara.IsMoving = 0;
				lara.gun_status = LG_HANDS_BUSY;
				lara.head_x_rot = 0;
				lara.head_y_rot = 0;
				lara.torso_x_rot = 0;
				lara.torso_y_rot = 0;
				AddActiveItem(item_number);
				item->status = ITEM_ACTIVE;
				AnimateItem(item);
			} else
				lara.GeneralPtr = item_number;
		} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}
	} else if (l->current_anim_state != AS_SWITCHON && l->current_anim_state != AS_SWITCHOFF)
		ObjectCollision(item_number, l, coll);
}

void SwitchCollision2(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (input & IN_ACTION && item->status == ITEM_INACTIVE && lara.water_status == LW_UNDERWATER &&
	        lara.gun_status == LG_NO_ARMS && l->current_anim_state == AS_TREAD) {
		if (TestLaraPosition(Switch2Bounds, item, l)) {
			if (!item->current_anim_state || item->current_anim_state == 1) {
				if (MoveLaraPosition(&Switch2Pos, item, l)) {
					l->fallspeed = 0;
					l->goal_anim_state = AS_SWITCHON;

					do AnimateLara(l);
					while (l->current_anim_state != AS_SWITCHON);

					l->goal_anim_state = AS_TREAD;
					lara.gun_status = LG_HANDS_BUSY;
					item->goal_anim_state = item->current_anim_state != 1;
					item->status = ITEM_ACTIVE;
					AddActiveItem(item_number);
					AnimateItem(item);
				}
			}
		}
	}
}

void SwitchType78Collision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {

}

void UnderwaterSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t flag;

	item = &items[item_number];

	if (input & IN_ACTION && lara.water_status == LW_UNDERWATER && l->current_anim_state == AS_TREAD && l->anim_number == ANIM_TREAD &&
	        lara.gun_status == LG_NO_ARMS && item->current_anim_state == 0 || lara.IsMoving && lara.GeneralPtr == item_number) {
		flag = 0;

		if (TestLaraPosition(UnderwaterSwitchBounds, item, l)) {
			if (!MoveLaraPosition(&UnderwaterSwitchPos, item, l))
				lara.GeneralPtr = item_number;
			else
				flag = 1;
		} else {
			l->pos.y_rot ^= 0x8000;

			if (TestLaraPosition(UnderwaterSwitchBounds2, item, l)) {
				if (MoveLaraPosition(&UnderwaterSwitchPos2, item, l))
					flag = 1;
				else
					lara.GeneralPtr = item_number;
			}

			l->pos.y_rot ^= 0x8000;
		}

		if (flag) {
			l->anim_number = ANIM_WATERSWITCH;
			l->frame_number = anims[ANIM_WATERSWITCH].frame_base;
			l->current_anim_state = AS_SWITCHON;
			l->fallspeed = 0;
			lara.IsMoving = 0;
			lara.gun_status = LG_HANDS_BUSY;
			item->goal_anim_state = 1;
			item->status = ITEM_ACTIVE;
			AddActiveItem(item_number);
			ForcedFixedCamera.x = item->pos.x_pos - ((BLOCK_SIZE * phd_sin(item->pos.y_rot + DEGREES_TO_ROTATION(90))) >> W2V_SHIFT);
			ForcedFixedCamera.y = item->pos.y_pos - BLOCK_SIZE;
			ForcedFixedCamera.z = item->pos.z_pos - ((BLOCK_SIZE * phd_cos(item->pos.y_rot + DEGREES_TO_ROTATION(90))) >> W2V_SHIFT);
			ForcedFixedCamera.room_number = item->room_number;
		}
	}
}

void PulleyCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int16_t roty;

	item = &items[item_number];

	if (!(item->flags & IFL_INVISIBLE) && (input & IN_ACTION && lara.gun_status == LG_NO_ARMS && l->current_anim_state == AS_STOP &&
	                                       l->anim_number == ANIM_BREATH && !l->gravity_status || lara.IsMoving && lara.GeneralPtr == item_number)) {
		roty = item->pos.y_rot;
		item->pos.y_rot = l->pos.y_rot;

		if (TestLaraPosition(PulleyBounds, item, l)) {
			if (item->item_flags[1]) {
				if (OldPickupPos.x != l->pos.x_pos || OldPickupPos.y != l->pos.y_pos || OldPickupPos.z != l->pos.z_pos) {
					OldPickupPos.x = l->pos.x_pos;
					OldPickupPos.y = l->pos.y_pos;
					OldPickupPos.z = l->pos.z_pos;
					SayNo();
				}
			} else {
				if (MoveLaraPosition(&PulleyPos, item, l)) {
					l->anim_number = ANIM_STAT2PULLEY;
					l->frame_number = anims[ANIM_STAT2PULLEY].frame_base;
					l->current_anim_state = AS_PULLEY;
					AddActiveItem(item_number);
					item->status = ITEM_ACTIVE;
					item->pos.y_rot = roty;
					lara.IsMoving = 0;
					lara.head_x_rot = 0;
					lara.head_y_rot = 0;
					lara.torso_x_rot = 0;
					lara.torso_y_rot = 0;
					lara.gun_status = LG_HANDS_BUSY;
					lara.GeneralPtr = item_number;
				} else
					lara.GeneralPtr = item_number;
			}
		} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
			lara.IsMoving = 0;
			lara.gun_status = 0;
		}

		item->pos.y_rot = roty;
	} else if (l->current_anim_state != AS_PULLEY)
		ObjectCollision(item_number, l, coll);
}

void TurnSwitchControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* l;

	item = &items[item_number];
	l = lara_item;

	if (item->item_flags[0] == 2) {
		if (item->anim_number == objects[TURN_SWITCH].anim_index + 2) {
			item->pos.y_rot += 0x4000;

			if (input & IN_ACTION) {
				l->anim_number = ANIM_TURNSWITCHCB;
				l->frame_number = anims[ANIM_TURNSWITCHCB].frame_base;
				item->anim_number = objects[item->object_number].anim_index + 1;
				item->frame_number = anims[item->anim_number].frame_base;
			}
		}

		if (l->anim_number == ANIM_TURNSWITCHCD && l->frame_number == anims[ANIM_TURNSWITCHCD].frame_end && !item->item_flags[1])
			item->item_flags[1] = 1;

		if (l->frame_number >= anims[ANIM_TURNSWITCHCB].frame_base && l->frame_number <= anims[ANIM_TURNSWITCHCB].frame_base + 43 ||
		        l->frame_number >= anims[ANIM_TURNSWITCHCB].frame_base + 58 && l->frame_number <= anims[ANIM_TURNSWITCHCB].frame_base + 115)
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, SFX_ALWAYS);
	} else {
		if (item->anim_number == objects[TURN_SWITCH].anim_index + 6) {
			item->pos.y_rot -= 0x4000;

			if (input & IN_ACTION) {
				l->anim_number = ANIM_TURNSWITCHAB;
				l->frame_number = anims[ANIM_TURNSWITCHAB].frame_base;
				item->anim_number = objects[item->object_number].anim_index + 5;
				item->frame_number = anims[item->anim_number].frame_base;
			}
		}

		if (l->anim_number == ANIM_TURNSWITCHAD && l->frame_number == anims[ANIM_TURNSWITCHAD].frame_end && !item->item_flags[1])
			item->item_flags[1] = 1;

		if (l->frame_number >= anims[ANIM_TURNSWITCHAB].frame_base && l->frame_number <= anims[ANIM_TURNSWITCHAB].frame_base + 43 ||
		        l->frame_number >= anims[ANIM_TURNSWITCHAB].frame_base + 58 && l->frame_number <= anims[ANIM_TURNSWITCHAB].frame_base + 115)
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, SFX_ALWAYS);
	}

	AnimateItem(item);

	if (item->item_flags[1] == 1) {
		l->anim_number = ANIM_BREATH;
		l->frame_number = anims[ANIM_BREATH].frame_base;
		l->current_anim_state = AS_STOP;
		item->status = ITEM_INACTIVE;
		item->anim_number = objects[item->object_number].anim_index;
		item->frame_number = anims[item->anim_number].frame_base;
		RemoveActiveItem(item_number);
		lara.gun_status = LG_NO_ARMS;
		UseForcedFixedCamera = 0;
		item->item_flags[1] = 2;
	}
}

void TurnSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t flag;
	int16_t Triggers[8];

	flag = 0;
	item = &items[item_number];

	if (!item->current_anim_state && input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH
	        && !l->gravity_status && lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_number) {
		if (TestLaraPosition(TurnSwitchBoundsA, item, l)) {
			if (MoveLaraPosition(&TurnSwitchPosA, item, l)) {
				l->anim_number = ANIM_TURNSWITCHA;
				l->frame_number = anims[ANIM_TURNSWITCHA].frame_base;
				item->anim_number = objects[item->object_number].anim_index + 4;
				item->frame_number = anims[item->anim_number].frame_base;
				item->item_flags[0] = 1;
				flag = -1;
				ForcedFixedCamera.x = item->pos.x_pos - ((BLOCK_SIZE * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
				ForcedFixedCamera.z = item->pos.z_pos - ((BLOCK_SIZE * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
			} else
				lara.GeneralPtr = item_number;
		} else {
			l->pos.y_rot ^= 0x8000;

			if (TestLaraPosition(TurnSwitchBoundsC, item, l)) {
				if (MoveLaraPosition(&TurnSwitchPos, item, l)) {
					flag = 1;
					l->anim_number = ANIM_TURNSWITCHC;
					l->frame_number = anims[ANIM_TURNSWITCHC].frame_base;
					item->item_flags[0] = 2;
					ForcedFixedCamera.x = item->pos.x_pos + ((BLOCK_SIZE * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
					ForcedFixedCamera.z = item->pos.z_pos + ((BLOCK_SIZE * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
				} else
					lara.GeneralPtr = item_number;
			} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
				lara.IsMoving = 0;
				lara.gun_status = LG_NO_ARMS;
			}

			l->pos.y_rot ^= 0x8000;
		}
	}

	if (!flag) {
		GlobalCollisionBounds[0] = -HALF_BLOCK_SIZE;
		GlobalCollisionBounds[1] = HALF_BLOCK_SIZE;
		GlobalCollisionBounds[2] = -HALF_BLOCK_SIZE;
		GlobalCollisionBounds[3] = 0;
		GlobalCollisionBounds[4] = -HALF_BLOCK_SIZE;
		GlobalCollisionBounds[5] = HALF_BLOCK_SIZE;
		ItemPushLara(item, l, coll, 0, 2);
		GlobalCollisionBounds[0] = CLICK_SIZE;
		GlobalCollisionBounds[1] = BLOCK_SIZE;
		GlobalCollisionBounds[4] = -HALF_CLICK_SIZE;
		GlobalCollisionBounds[5] = HALF_CLICK_SIZE;
		ItemPushLara(item, l, coll, 0, 2);
	} else {
		lara.IsMoving = 0;
		lara.head_y_rot = 0;
		lara.head_x_rot = 0;
		lara.torso_y_rot = 0;
		lara.torso_x_rot = 0;
		lara.gun_status = LG_HANDS_BUSY;
		l->current_anim_state = AS_TURNSWITCH;
		UseForcedFixedCamera = 1;
		ForcedFixedCamera.y = item->pos.y_pos - (BLOCK_SIZE * 2);
		ForcedFixedCamera.room_number = item->room_number;
		AddActiveItem(item_number);
		item->status = ITEM_ACTIVE;
		item->item_flags[1] = 0;

		if (GetSwitchTrigger(item, Triggers, 0)) {
			item = &items[*Triggers];

			if (!TriggerActive(item)) {
				if (flag >= 0)
					item->anim_number = objects[item->object_number].anim_index + 4;
				else
					item->anim_number = objects[item->object_number].anim_index;

				item->frame_number = anims[item->anim_number].frame_base;
			}
		}
	}
}

void RailSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t flag;

	flag = 0;
	item = &items[item_number];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH &&
	        lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_number) {
		if (item->current_anim_state == 1) {
			l->pos.y_rot ^= 0x8000;

			if (TestLaraPosition(RailSwitchBounds2, item, l)) {
				if (MoveLaraPosition(&RailSwitchPos2, item, l)) {
					item->goal_anim_state = 0;
					flag = 1;
				} else
					lara.GeneralPtr = item_number;
			} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
				lara.IsMoving = 0;
				lara.gun_status = LG_NO_ARMS;
			}

			l->pos.y_rot ^= 0x8000;
		} else if (item->current_anim_state == 0) {
			if (TestLaraPosition(RailSwitchBounds, item, l)) {
				if (MoveLaraPosition(&RailSwitchPos, item, l)) {
					item->goal_anim_state = 1;
					flag = 1;
				} else
					lara.GeneralPtr = item_number;
			} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
				lara.IsMoving = 0;
				lara.gun_status = LG_NO_ARMS;
			}
		}
	}

	if (flag) {
		l->anim_number = ANIM_RAILSWITCH;
		l->frame_number = anims[ANIM_RAILSWITCH].frame_base;
		l->current_anim_state = AS_RAILSWITCH;
		l->goal_anim_state = AS_RAILSWITCH;
		lara.IsMoving = 0;
		lara.head_x_rot = 0;
		lara.head_y_rot = 0;
		lara.torso_x_rot = 0;
		lara.torso_y_rot = 0;
		lara.gun_status = LG_HANDS_BUSY;
		item->status = ITEM_ACTIVE;
		AddActiveItem(item_number);
		AnimateItem(item);
		return;
	}

	if (!lara.IsMoving)
		ObjectCollision(item_number, l, coll);
}

void JumpSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (input & IN_ACTION && lara.gun_status == LG_NO_ARMS && (l->current_anim_state == AS_REACH || l->current_anim_state == AS_UPJUMP) &&
	        l->gravity_status && l->fallspeed > 0 && !item->current_anim_state) {
		if (TestLaraPosition(JumpSwitchBounds, item, l)) {
			AlignLaraPosition(&JumpSwitchPos, item, l);
			l->anim_number = ANIM_LEAPSWITCH;
			l->frame_number = anims[ANIM_LEAPSWITCH].frame_base;
			l->current_anim_state = AS_SWITCHON;
			l->fallspeed = 0;
			l->gravity_status = 0;
			lara.gun_status = LG_HANDS_BUSY;
			item->status = ITEM_ACTIVE;
			item->goal_anim_state = 1;
			AddActiveItem(item_number);
		}
	}
}

void CrowbarSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t flag;

	flag = 0;
	item = &items[item_number];

	if (input & IN_ACTION || GLOBAL_inventoryitemchosen == CROWBAR_ITEM && l->current_anim_state == AS_STOP &&
	        l->anim_number == ANIM_BREATH && lara.gun_status == LG_NO_ARMS && !item->item_flags[0] ||
	        lara.IsMoving && lara.GeneralPtr == item_number) {
		if (item->current_anim_state == 1) {
			l->pos.y_rot ^= 0x8000;

			if (TestLaraPosition(CrowbarBounds2, item, l)) {
				if (lara.IsMoving || GLOBAL_inventoryitemchosen == CROWBAR_ITEM) {
					if (MoveLaraPosition(&CrowbarPos2, item, l)) {
						flag = 1;
						l->anim_number = ANIM_CROWSWITCH;
						l->frame_number = anims[ANIM_CROWSWITCH].frame_base;
						item->goal_anim_state = 0;
					} else
						lara.GeneralPtr = item_number;

					GLOBAL_inventoryitemchosen = NO_ITEM;
				} else
					flag = -1;
			} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
				lara.IsMoving = 0;
				lara.gun_status = LG_NO_ARMS;
			}

			l->pos.y_rot ^= 0x8000;
		} else if (!item->current_anim_state) {
			if (TestLaraPosition(CrowbarBounds, item, l)) {
				if (lara.IsMoving || GLOBAL_inventoryitemchosen == CROWBAR_ITEM) {
					if (MoveLaraPosition(&CrowbarPos, item, l)) {
						flag = 1;
						l->anim_number = ANIM_CROWSWITCH;
						l->frame_number = anims[ANIM_CROWSWITCH].frame_base;
						item->goal_anim_state = 1;
					} else
						lara.GeneralPtr = item_number;

					GLOBAL_inventoryitemchosen = NO_ITEM;
				} else
					flag = -1;
			} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
				lara.IsMoving = 0;
				lara.gun_status = LG_NO_ARMS;
			}
		}
	}

	if (!flag)
		ObjectCollision(item_number, l, coll);
	else {
		if (flag != -1) {
			l->current_anim_state = AS_SWITCHON;
			l->goal_anim_state = AS_SWITCHON;
			lara.IsMoving = 0;
			lara.head_x_rot = 0;
			lara.head_y_rot = 0;
			lara.torso_x_rot = 0;
			lara.torso_y_rot = 0;
			lara.gun_status = LG_HANDS_BUSY;
			item->status = ITEM_ACTIVE;
			AddActiveItem(item_number);
			AnimateItem(item);
		} else {
			if (lara.crowbar)
				GLOBAL_enterinventory = CROWBAR_ITEM;
			else if (OldPickupPos.x != l->pos.x_pos || OldPickupPos.y != l->pos.y_pos || OldPickupPos.z != l->pos.z_pos) {
				OldPickupPos.x = l->pos.x_pos;
				OldPickupPos.y = l->pos.y_pos;
				OldPickupPos.z = l->pos.z_pos;
				SayNo();
			}
		}
	}
}

void FullBlockSwitchControl(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->anim_number != objects[item->object_number].anim_index + 2 || CurrentSequence >= 3 || item->item_flags[0]) {
		if (CurrentSequence >= 4) {
			item->item_flags[0] = 0;
			item->goal_anim_state = 1;
			item->status = ITEM_INACTIVE;
			CurrentSequence++;

			if (CurrentSequence >= 7)
				CurrentSequence = 0;
		}
	} else {
		item->item_flags[0] = 1;
		Sequences[CurrentSequence] = (uint8_t)item->trigger_flags;
		CurrentSequence++;

		if (CurrentSequence == 3 && SequenceUsed[SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]]])
			CurrentSequence++;
	}

	AnimateItem(item);
}

void CogSwitchControl(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	AnimateItem(item);

	if (item->current_anim_state == 1) {
		if (item->goal_anim_state == 1 && !(input & IN_ACTION)) {
			lara_item->goal_anim_state = AS_STOP;
			item->goal_anim_state = 0;
		}

		if (lara_item->anim_number == ANIM_COGSWITCHL && lara_item->frame_number == anims[ANIM_COGSWITCHL].frame_base + 10) {
			item = &items[lara.GeneralPtr];
			item->item_flags[0] = 40;
		}
	} else if (item->frame_number == anims[item->anim_number].frame_end) {
		item->current_anim_state = 0;
		item->status = ITEM_INACTIVE;
		RemoveActiveItem(item_number);
		lara_item->anim_number = ANIM_STOP;
		lara_item->frame_number = anims[ANIM_STOP].frame_base;
		lara_item->current_anim_state = AS_STOP;
		lara_item->goal_anim_state = AS_STOP;
		lara.gun_status = LG_NO_ARMS;
	}
}

void CogSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	ITEM_INFO* door_item;
	DOOR_DATA* door;
	int16_t* data;

	item = &items[item_number];
	GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &item->room_number),
	          item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	data = trigger_index;

	while ((*data & 0x1F) != TRIGGER_TYPE && !(*data & 0x8000)) data++;

	int door_item_num = data[3] & 0x3FF;

	door_item = &items[door_item_num];
	door = (DOOR_DATA*)door_item->data;

	if (item->status != ITEM_INACTIVE)
		return;

	if (!(item->flags & IFL_INVISIBLE) && (input & IN_ACTION && lara.gun_status == LG_NO_ARMS && !l->gravity_status &&
	                                       l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH || lara.IsMoving && lara.GeneralPtr == item_number)) {
		if (TestLaraPosition(CogSwitchBounds, item, l)) {
			if (MoveLaraPosition(&CogSwitchPos, item, l)) {
				lara.IsMoving = 0;
				lara.head_x_rot = 0;
				lara.head_y_rot = 0;
				lara.torso_x_rot = 0;
				lara.torso_y_rot = 0;
				lara.gun_status = LG_HANDS_BUSY;
				lara.GeneralPtr = door_item_num;
				l->anim_number = ANIM_COGSWITCHS;
				l->frame_number = anims[ANIM_COGSWITCHS].frame_base;
				l->current_anim_state = AS_COGSWITCH;
				l->goal_anim_state = AS_COGSWITCH;
				AddActiveItem(item_number);
				item->status = ITEM_ACTIVE;
				item->goal_anim_state = 1;

				if (!door->Opened) {
					AddActiveItem(int16_t(door_item - items));
					door_item->status = ITEM_ACTIVE;
					*(int32_t*)&door_item->item_flags[2] = door_item->pos.y_pos;
				}
			} else
				lara.GeneralPtr = item_number;

			return;
		}

		if (lara.IsMoving && lara.GeneralPtr == item_number) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}
	}

	ObjectCollision(item_number, l, coll);
}
