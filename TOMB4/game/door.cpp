#include "../tomb4/pch.h"
#include "door.h"
#include "draw.h"
#include "control.h"
#include "objects.h"
#include "lara_states.h"
#include "collide.h"
#include "newinv.h"
#include "sound.h"
#include "items.h"
#include "sphere.h"
#include "box.h"
#include "../specific/input.h"
#include "lara.h"
#include "switch.h"
#include "lot.h"
#include "../specific/3dmath.h"

static PHD_VECTOR CrowbarDoorPos = { -412, 0, 140 };
static PHD_VECTOR PullDoorPos = { -201, 0, 322 };
static PHD_VECTOR PushDoorPos = { 201, 0, -702 };
static PHD_VECTOR KickDoorPos = { 0, 0, -917 };
static PHD_VECTOR DoubleDoorPos = { 0, 0, 220 };
static PHD_VECTOR UnderwaterDoorPos = { -251, -760, -46 };

static int16_t CrowbarDoorBounds[] = {
	-HALF_BLOCK_SIZE,
	HALF_BLOCK_SIZE,
	-BLOCK_SIZE,
	0,
	0,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80)
};
static int16_t PushPullKickDoorBounds[] = {
	-(CLICK_SIZE + HALF_CLICK_SIZE),
	(CLICK_SIZE + HALF_CLICK_SIZE),
	0,
	0,
	-BLOCK_SIZE,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};
static int16_t UnderwaterDoorBounds[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	-BLOCK_SIZE,
	0,
	-BLOCK_SIZE,
	0,
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80),
	-DEGREES_TO_ROTATION(80),
	DEGREES_TO_ROTATION(80)
};

void ShutThatDoor(DOORPOS_DATA* d) {
	CREATURE_INFO* cinfo;

	if (d->floor) {
		d->floor->box = -1;
		d->floor->ceiling = -127;
		d->floor->floor = -127;
		d->floor->index = 0;
		d->floor->pit_room = -1;
		d->floor->sky_room = -1;

		if (d->block != NO_BOX) {
			boxes[d->block].overlap_index |= BLOCKED;

			for (int16_t slot = 0; slot < MAXIMUM_BADDIES; slot++) {
				cinfo = &baddie_slots[slot];
				// T4Plus: additional check for NULL slots
				if (cinfo) {
					cinfo->LOT.target_box = NO_BOX;
				} else {
					break;
				}
			}
		}
	}
}

void OpenThatDoor(DOORPOS_DATA* d) {
	CREATURE_INFO* cinfo;

	if (d->floor) {
		*d->floor = d->data;

		if (d->block != NO_BOX) {
			boxes[d->block].overlap_index &= ~BLOCKED;

			for (int16_t slot = 0; slot < MAXIMUM_BADDIES; slot++) {
				cinfo = &baddie_slots[slot];
				// T4Plus: additional check for NULL slots
				if (cinfo) {
					cinfo->LOT.target_box = NO_BOX;
				} else {
					break;
				}
			}
		}
	}
}

void DoorControl(int16_t item_number) {
	ITEM_INFO* item;
	DOOR_DATA* door;
	int16_t* bounds;

	item = &items[item_number];
	door = (DOOR_DATA*)item->data;

	if (item->trigger_flags == 1) {
		if (item->item_flags[0]) {
			bounds = GetBoundsAccurate(item);
			item->item_flags[0]--;
			item->pos.y_pos -= 12;

			if (item->pos.y_pos < bounds[2] + *(int32_t*)&item->item_flags[2] - CLICK_SIZE) {
				item->pos.y_pos = bounds[2] + *(int32_t*)&item->item_flags[2] - CLICK_SIZE;
				item->item_flags[0] = 0;
			}

			if (!door->Opened) {
				OpenThatDoor(&door->d1);
				OpenThatDoor(&door->d2);
				OpenThatDoor(&door->d1flip);
				OpenThatDoor(&door->d2flip);
				door->Opened = 1;
			}
		} else {
			if (item->pos.y_pos < item->item_flags[2])
				item->pos.y_pos += 4;

			if (item->pos.y_pos >= item->item_flags[2]) {
				item->pos.y_pos = item->item_flags[2];

#ifdef FLYCHEAT_NOCLIP
				if (lara.water_status != LW_FLYCHEAT) {
					if (door->Opened) {
						ShutThatDoor(&door->d1);
						ShutThatDoor(&door->d2);
						ShutThatDoor(&door->d1flip);
						ShutThatDoor(&door->d2flip);
						door->Opened = 0;
					}
				} else {
					if (!door->Opened) {
						OpenThatDoor(&door->d1);
						OpenThatDoor(&door->d2);
						OpenThatDoor(&door->d1flip);
						OpenThatDoor(&door->d2flip);
						door->Opened = 1;
					}
				}
#else
				if (door->Opened) {
					ShutThatDoor(&door->d1);
					ShutThatDoor(&door->d2);
					ShutThatDoor(&door->d1flip);
					ShutThatDoor(&door->d2flip);
					door->Opened = 0;
				}
#endif
			}
		}
	} else {
		if (TriggerActive(item)) {
			if (item->current_anim_state == 0)
				item->goal_anim_state = 1;
			else {
				if (!door->Opened) {
					OpenThatDoor(&door->d1);
					OpenThatDoor(&door->d2);
					OpenThatDoor(&door->d1flip);
					OpenThatDoor(&door->d2flip);
					door->Opened = 1;
				}
			}
		} else {
			if (item->current_anim_state == 1)
				item->goal_anim_state = 0;
#ifdef FLYCHEAT_NOCLIP
			else {
				if (lara.water_status != LW_FLYCHEAT) {
					if (door->Opened) {
						ShutThatDoor(&door->d1);
						ShutThatDoor(&door->d2);
						ShutThatDoor(&door->d1flip);
						ShutThatDoor(&door->d2flip);
						door->Opened = 0;
					}
				} else {
					if (!door->Opened) {
						OpenThatDoor(&door->d1);
						OpenThatDoor(&door->d2);
						OpenThatDoor(&door->d1flip);
						OpenThatDoor(&door->d2flip);
						door->Opened = 1;
					}
				}
			}
#else
			else if (door->Opened) {
				ShutThatDoor(&door->d1);
				ShutThatDoor(&door->d2);
				ShutThatDoor(&door->d1flip);
				ShutThatDoor(&door->d2flip);
				door->Opened = 0;
			}
#endif
		}

		AnimateItem(item);
	}
}

#ifdef FLYCHEAT_NOCLIP
void DoorNoclipCollision(int16_t item_number) {
	ITEM_INFO* item;
	DOOR_DATA* door;

	item = &items[item_number];
	door = (DOOR_DATA*)item->data;

	if (item->trigger_flags == 1) {
		if (item->item_flags[0]) {
			if (!door->Opened) {
				OpenThatDoor(&door->d1);
				OpenThatDoor(&door->d2);
				OpenThatDoor(&door->d1flip);
				OpenThatDoor(&door->d2flip);
				door->Opened = 1;
			}
		} else {
			if (item->pos.y_pos >= item->item_flags[2]) {
				if (lara.water_status != LW_FLYCHEAT) {
					if (door->Opened) {
						ShutThatDoor(&door->d1);
						ShutThatDoor(&door->d2);
						ShutThatDoor(&door->d1flip);
						ShutThatDoor(&door->d2flip);
						door->Opened = 0;
					}
				} else {
					if (!door->Opened) {
						OpenThatDoor(&door->d1);
						OpenThatDoor(&door->d2);
						OpenThatDoor(&door->d1flip);
						OpenThatDoor(&door->d2flip);
						door->Opened = 1;
					}
				}
			}
		}
	} else {
		if (TriggerActive(item)) {
			if (item->current_anim_state != 0) {
				if (!door->Opened) {
					OpenThatDoor(&door->d1);
					OpenThatDoor(&door->d2);
					OpenThatDoor(&door->d1flip);
					OpenThatDoor(&door->d2flip);
					door->Opened = 1;
				}
			}
		} else {
			if (item->current_anim_state != 1) {
				if (lara.water_status != LW_FLYCHEAT) {
					if (door->Opened) {
						ShutThatDoor(&door->d1);
						ShutThatDoor(&door->d2);
						ShutThatDoor(&door->d1flip);
						ShutThatDoor(&door->d2flip);
						door->Opened = 0;
					}
				} else {
					if (!door->Opened) {
						OpenThatDoor(&door->d1);
						OpenThatDoor(&door->d2);
						OpenThatDoor(&door->d1flip);
						OpenThatDoor(&door->d2flip);
						door->Opened = 1;
					}
				}
			}
		}
	}
}
#endif

void DoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_num];

#ifdef FLYCHEAT_NOCLIP
	if (item->status != ITEM_ACTIVE) {
		DoorNoclipCollision(item_num);
	}
#endif

	if (item->trigger_flags == 2 && item->status != ITEM_ACTIVE && ((input & IN_ACTION || GLOBAL_inventoryitemchosen == CROWBAR_ITEM) &&
	        l->current_anim_state == AS_STOP && l->anim_number == LARA_ANIM_BREATH && !l->gravity_status && lara.gun_status == LG_NO_ARMS ||
	        lara.IsMoving && lara.GeneralPtr == item_num)) {
		item->pos.y_rot ^= 0x8000;

		if (TestLaraPosition(CrowbarDoorBounds, item, l)) {
			if (!lara.IsMoving) {
				if (GLOBAL_inventoryitemchosen == NO_ITEM) {
					if (have_i_got_object(CROWBAR_ITEM))
						GLOBAL_enterinventory = CROWBAR_ITEM;
					else {
						if (OldPickupPos.x != l->pos.x_pos || OldPickupPos.y != l->pos.y_pos || OldPickupPos.z != l->pos.z_pos) {
							OldPickupPos.x = l->pos.x_pos;
							OldPickupPos.y = l->pos.y_pos;
							OldPickupPos.z = l->pos.z_pos;
							SayNo();
						}
					}

					item->pos.y_rot ^= 0x8000;
					return;
				}

				if (GLOBAL_inventoryitemchosen != CROWBAR_ITEM) {
					item->pos.y_rot ^= 0x8000;
					return;
				}
			}

			GLOBAL_inventoryitemchosen = NO_ITEM;

			if (MoveLaraPosition(&CrowbarDoorPos, item, l)) {
				l->anim_number = LARA_ANIM_CROWBARDOOR;
				l->frame_number = anims[LARA_ANIM_CROWBARDOOR].frame_base;
				l->current_anim_state = AS_CONTROLLED;
				item->pos.y_rot ^= 0x8000;
				AddActiveItem(item_num);
				item->flags |= IFL_CODEBITS;
				item->status = ITEM_ACTIVE;
				item->goal_anim_state = 1;
				lara.IsMoving = 0;
				lara.gun_status = LG_HANDS_BUSY;
				return;
			}

			lara.GeneralPtr = item_num;
		} else if (lara.IsMoving && lara.GeneralPtr == item_num) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}

		item->pos.y_rot ^= 0x8000;
	}

	if (TestBoundsCollide(item, l, coll->radius)) {
		if (TestCollision(item, l)) {
			if (coll->enable_baddie_push) {
				if (item->current_anim_state == item->goal_anim_state)
					ItemPushLara(item, l, coll, 0, 1);
				else
					ItemPushLara(item, l, coll, coll->enable_spaz, 1);
			}
		}
	}
}

void PushPullKickDoorControl(int16_t item_number) {
	ITEM_INFO* item;
	DOOR_DATA* door;

	item = &items[item_number];
	door = (DOOR_DATA*)item->data;

	if (!door->Opened) {
		OpenThatDoor(&door->d1);
		OpenThatDoor(&door->d2);
		OpenThatDoor(&door->d1flip);
		OpenThatDoor(&door->d2flip);
		door->Opened = 1;
	}

	AnimateItem(item);
}

void PushPullKickDoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t pull, goin;

	item = &items[item_num];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == LARA_ANIM_BREATH && item->status != ITEM_ACTIVE &&
	        !l->gravity_status && lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_num) {
		pull = 0;

		if (l->room_number == item->room_number) {
			item->pos.y_rot ^= 0x8000;
			pull = 1;
		}

		if (TestLaraPosition(PushPullKickDoorBounds, item, l)) {
			goin = 0;

			if (pull) {
				if (MoveLaraPosition(&PullDoorPos, item, l)) {
					l->anim_number = LARA_ANIM_PULLDOOR;
					l->frame_number = anims[LARA_ANIM_PULLDOOR].frame_base;
					item->goal_anim_state = 3;
					goin = 1;
				} else
					lara.GeneralPtr = item_num;
			} else {
				if (item->object_number < KICK_DOOR1) {
					if (MoveLaraPosition(&PushDoorPos, item, l)) {
						l->anim_number = LARA_ANIM_PUSHDOOR;
						l->frame_number = anims[LARA_ANIM_PUSHDOOR].frame_base;
						item->goal_anim_state = 2;
						goin = 1;
					} else
						lara.GeneralPtr = item_num;
				} else {
					if (MoveLaraPosition(&KickDoorPos, item, l)) {
						l->anim_number = LARA_ANIM_KICKDOOR;
						l->frame_number = anims[LARA_ANIM_KICKDOOR].frame_base;
						item->goal_anim_state = 2;
						goin = 1;
					} else
						lara.GeneralPtr = item_num;
				}
			}

			if (goin) {
				AddActiveItem(item_num);
				item->status = ITEM_ACTIVE;
				l->current_anim_state = AS_CONTROLLED;
				l->goal_anim_state = AS_STOP;
				lara.IsMoving = 0;
				lara.gun_status = LG_HANDS_BUSY;
			}
		} else if (lara.IsMoving && lara.GeneralPtr == item_num) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}

		if (pull)
			item->pos.y_rot ^= 0x8000;
	} else if (!item->current_anim_state)
		DoorCollision(item_num, l, coll);
}

void DoubleDoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_num];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == LARA_ANIM_BREATH && item->status != ITEM_ACTIVE &&
	        !l->gravity_status && lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_num) {
		item->pos.y_rot ^= 0x8000;

		if (TestLaraPosition(PushPullKickDoorBounds, item, l)) {
			if (MoveLaraPosition(&DoubleDoorPos, item, l)) {
				l->anim_number = LARA_ANIM_TWODOOR;
				l->frame_number = anims[LARA_ANIM_TWODOOR].frame_base;
				l->current_anim_state = AS_TWODOOR;
				AddActiveItem(item_num);
				item->status = ITEM_ACTIVE;
				lara.IsMoving = 0;
				lara.gun_status = LG_HANDS_BUSY;
				lara.head_x_rot = 0;
				lara.head_y_rot = 0;
				lara.torso_x_rot = 0;
				lara.torso_y_rot = 0;
			} else
				lara.GeneralPtr = item_num;
		} else if (lara.IsMoving && lara.GeneralPtr == item_num) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}

		item->pos.y_rot ^= 0x8000;
	}
}

void UnderwaterDoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_num];

	if (input & IN_ACTION && item->status != ITEM_ACTIVE && l->current_anim_state == AS_TREAD && lara.water_status == LW_UNDERWATER &&
	        lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_num) {
		l->pos.y_rot ^= 0x8000;

		if (TestLaraPosition(UnderwaterDoorBounds, item, l)) {
			if (MoveLaraPosition(&UnderwaterDoorPos, item, l)) {
				l->anim_number = LARA_ANIM_WATERDOOR;
				l->frame_number = anims[LARA_ANIM_WATERDOOR].frame_base;
				l->current_anim_state = AS_CONTROLLED;
				l->fallspeed = 0;
				item->status = ITEM_ACTIVE;
				AddActiveItem(item_num);
				item->goal_anim_state = 1;
				AnimateItem(item);
				lara.IsMoving = 0;
				lara.gun_status = LG_HANDS_BUSY;
			} else
				lara.GeneralPtr = item_num;
		} else if (lara.IsMoving && lara.GeneralPtr == item_num) {
			lara.IsMoving = 0;
			lara.gun_status = LG_NO_ARMS;
		}

		l->pos.y_rot ^= 0x8000;
	} else if (item->status == ITEM_ACTIVE)
		ObjectCollision(item_num, l, coll);
}

void SequenceDoorControl(int16_t item_number) {
	ITEM_INFO* item;
	DOOR_DATA* door;

	item = &items[item_number];
	door = (DOOR_DATA*)item->data;

	if (item->item_flags[0]) {
		if (TriggerActive(item)) {
			if (!item->current_anim_state)
				item->goal_anim_state = 1;
			else {
				if (!door->Opened) {
					OpenThatDoor(&door->d1);
					OpenThatDoor(&door->d2);
					OpenThatDoor(&door->d1flip);
					OpenThatDoor(&door->d2flip);
					door->Opened = 1;
				}

				if (CurrentSequence == 3) {
					if (SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]] == item->trigger_flags &&
					        !Sequences[0] && Sequences[1] == 1 && Sequences[2] == 2) {
						CurrentSequence = 4;
						SequenceUsed[item->trigger_flags] = Sequences[1];
					}
				} else if ((CurrentSequence == 1 || CurrentSequence == 2) && item->trigger_flags == 2) {
					item->flags &= ~(IFL_INVISIBLE | IFL_ANTITRIGGER_ONESHOT);
					item->goal_anim_state = 0;
					item->item_flags[0] = 0;
				}
			}
		} else {
			if (item->current_anim_state == 1)
				item->goal_anim_state = 0;
			else {
				if (CurrentSequence == 3 && SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]] == item->trigger_flags) {
					CurrentSequence = 4;

					if (item->trigger_flags != 2)
						SequenceUsed[item->trigger_flags] = 1;
				}

				if (door->Opened) {
					ShutThatDoor(&door->d1);
					ShutThatDoor(&door->d2);
					ShutThatDoor(&door->d1flip);
					ShutThatDoor(&door->d2flip);
					door->Opened = 0;
				}
			}
		}
	} else if (!item->current_anim_state && CurrentSequence == 3 && SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]] == item->trigger_flags) {
		if (item->trigger_flags && item->trigger_flags != 2 && !SequenceUsed[0]) {
			Sequences[1] = 0;
			Sequences[0] = 1;
			Sequences[2] = 2;
			return;
		}

		item->goal_anim_state = 1;
		item->item_flags[0] = 1;
	}

	AnimateItem(item);
}
