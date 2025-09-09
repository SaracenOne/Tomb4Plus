#include "../../../tomb4/pch.h"
#include "harpy.h"
#include "../../effect2.h"
#include "../../../specific/function_stubs.h"
#include "../../items.h"
#include "../../objects.h"
#include "../../sphere.h"
#include "../../../specific/3dmath.h"
#include "../../box.h"
#include "../../people.h"
#include "../../effects.h"
#include "../../lara.h"
#include "../../control.h"
#include "../../lot.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"
#include "../../../tomb4/tomb4plus/t4plus_objects.h"
#include "../../tomb4fx.h"

static BITE_INFO right_hand = { 0, 128, 0, 2 };
static BITE_INFO left_hand = { 0, 128, 0, 4 };
static BITE_INFO right_hit = { 0, 0, 0, 2 };
static BITE_INFO left_hit = { 0, 0, 0, 4 };
static BITE_INFO tail_hit = { 0, 0, 0, 15 };

void TriggerHarpyMissileFlame(int16_t fx_number, int32_t xv, int32_t yv, int32_t zv) {
	SPARKS* sptr;
	int32_t dx, dz;

	dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos;
	dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;

	if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
		return;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = (GetRandomControl() & 0x7F) + 32;
	sptr->sB = sptr->dG + 64;	//uhm
	sptr->dR = (GetRandomControl() & 0x7F) + 32;
	sptr->dG = sptr->dR;
	sptr->dB = 0;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 4;
	sptr->TransType = 2;
	sptr->Life = (GetRandomControl() & 3) + 16;
	sptr->sLife = sptr->Life;
	sptr->x = (GetRandomControl() & 0xF) - 8;
	sptr->y = 0;
	sptr->z = (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (int16_t)xv;
	sptr->Yvel = (int16_t)yv;
	sptr->Zvel = (int16_t)zv;
	sptr->Friction = 68;

	sptr->Flags = SF_UNUSED2 | SF_FX | SF_ROTATE | SF_DEF | SF_SCALE;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -32 - (GetRandomControl() & 0x1F);
	else
		sptr->RotAdd = (GetRandomControl() & 0x1F) + 32;

	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->FxObj = (uint8_t)fx_number;
	sptr->Scalar = 2;
	sptr->Size = (GetRandomControl() & 7) + 64;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size >> 5;
}

void TriggerHarpyMissile(PHD_3DPOS* pos, int16_t room_number, int16_t mesh) {
	FX_INFO* fx;
	int16_t fx_num;

	fx_num = CreateEffect(room_number);

	if (fx_num != NO_ITEM) {
		fx = &effects[fx_num];
		fx->pos.x_pos = pos->x_pos;
		fx->pos.y_pos = pos->y_pos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.z_pos = pos->z_pos;
		fx->pos.x_rot = pos->x_rot;
		fx->pos.y_rot = pos->y_rot;
		fx->pos.z_rot = 0;
		fx->room_number = room_number;
		fx->counter = int16_t(2 * GetRandomControl() + 0x8000);
		fx->object_number = T4PlusGetBubblesSlotID();
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->flag1 = mesh;
		fx->frame_number = objects[T4PlusGetBubblesSlotID()].mesh_index + mesh * 2;
	}
}

void TriggerHarpySparks(int32_t x, int32_t y, int32_t z, int16_t xv, int16_t yv, int16_t zv) {
	SPARKS* sptr;
	int32_t dx, dz;

	dx = lara_item->pos.x_pos - x;
	dz = lara_item->pos.z_pos - z;

	if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
		return;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = (GetRandomControl() & 0x7F) + 64;
	sptr->dG = sptr->dR;
	sptr->dB = 0;
	sptr->Life = 16;
	sptr->sLife = 16;
	sptr->ColFadeSpeed = 4;
	sptr->TransType = 2;
	sptr->FadeToBlack = 4;
	sptr->x = x;
	sptr->y = y;
	sptr->z = z;
	sptr->Xvel = xv;
	sptr->Yvel = yv;
	sptr->Zvel = zv;
	sptr->Friction = 34;
	sptr->Scalar = 1;
	sptr->Size = (GetRandomControl() & 3) + 4;
	sptr->sSize = sptr->Size;
	sptr->dSize = (GetRandomControl() & 1) + 1;
	sptr->MaxYvel = 0;
	sptr->Gravity = 0;
	sptr->Flags = SF_NONE;
}

void TriggerHarpyFlame(int16_t item_number, uint8_t NodeNumber, int16_t size) {
	SPARKS* sptr;
	int32_t dx, dz;

	dx = lara_item->pos.x_pos - items[item_number].pos.x_pos;
	dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
		return;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = (GetRandomControl() & 0x7F) + 32;
	sptr->dG = sptr->dR;
	sptr->dB = 0;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 4;
	sptr->TransType = 2;
	sptr->Life = (GetRandomControl() & 7) + 20;
	sptr->sLife = sptr->Life;
	sptr->x = (GetRandomControl() & 0xF) - 8;
	sptr->y = 0;
	sptr->z = (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (GetRandomControl() & 0xFF) - HALF_CLICK_SIZE;
	sptr->Yvel = 0;
	sptr->Zvel = (GetRandomControl() & 0xFF) - HALF_CLICK_SIZE;
	sptr->Friction = 5;
	sptr->Flags = SF_SCALE | SF_DEF | SF_ROTATE | SF_ITEM | SF_UNUSED2 | SF_ATTACHEDNODE;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -0x20 - (GetRandomControl() & 0x1F);
	else
		sptr->RotAdd = (GetRandomControl() & 0x1F) + 0x20;

	sptr->MaxYvel = 0;
	sptr->Gravity = (GetRandomControl() & 0x1F) + 16;
	sptr->FxObj = (uint8_t)item_number;
	sptr->NodeNumber = NodeNumber;
	sptr->Scalar = 2;
	sptr->Size = uint8_t((GetRandomControl() & 0xF) + size);
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size >> 4;
}

void DoHarpyEffects(ITEM_INFO* item, int16_t item_number) {
	PHD_VECTOR pos;
	PHD_VECTOR rh;
	PHD_VECTOR lh;
	PHD_3DPOS mPos;
	int16_t xv, yv, zv, size;
	int16_t angles[2];

	item->item_flags[0]++;
	rh.x = right_hand.x;
	rh.y = right_hand.y;
	rh.z = right_hand.z;
	GetJointAbsPosition(item, &rh, right_hand.mesh_num);
	lh.x = left_hand.x;
	lh.y = left_hand.y;
	lh.z = left_hand.z;
	GetJointAbsPosition(item, &lh, left_hand.mesh_num);

	if (item->item_flags[0] >= 24 && item->item_flags[0] <= 47 && (GetRandomControl() & 0x1F) < item->item_flags[0]) {
		for (int32_t i = 0; i < 2; i++) {
			pos.x = (GetRandomControl() & 0x7FF) + rh.x - BLOCK_SIZE;
			pos.y = (GetRandomControl() & 0x7FF) + rh.y - BLOCK_SIZE;
			pos.z = (GetRandomControl() & 0x7FF) + rh.z - BLOCK_SIZE;
			xv = int16_t((rh.x - pos.x) << 3);
			yv = int16_t((rh.y - pos.y) << 3);
			zv = int16_t((rh.z - pos.z) << 3);
			TriggerHarpySparks(pos.x, pos.y, pos.z, xv, yv, zv);

			pos.x = (GetRandomControl() & 0x7FF) + lh.x - BLOCK_SIZE;
			pos.y = (GetRandomControl() & 0x7FF) + lh.y - BLOCK_SIZE;
			pos.z = (GetRandomControl() & 0x7FF) + lh.z - BLOCK_SIZE;
			xv = int16_t((lh.x - pos.x) << 3);
			yv = int16_t((lh.y - pos.y) << 3);
			zv = int16_t((lh.z - pos.z) << 3);
			TriggerHarpySparks(pos.x, pos.y, pos.z, xv, yv, zv);
		}
	}

	size = item->item_flags[0] * 2;

	if (size > QUARTER_CLICK_SIZE)
		size = QUARTER_CLICK_SIZE;

	if (item->item_flags[0] < 80) {
		if ((wibble & 0xF) == 8)
			TriggerHarpyFlame(item_number, NODE_ID_HARPY_A, size);
		else if (!(wibble & 0xF))
			TriggerHarpyFlame(item_number, NODE_ID_HARPY_B, size);
	}

	if (item->item_flags[0] >= 61 && item->item_flags[0] <= 65 && GlobalCounter & 1) {
		pos.x = right_hand.x;
		pos.y = right_hand.y << 1;
		pos.z = right_hand.z;
		GetJointAbsPosition(item, &pos, right_hand.mesh_num);
		mPos.x_pos = rh.x;
		mPos.y_pos = rh.y;
		mPos.z_pos = rh.z;
		phd_GetVectorAngles(pos.x - mPos.x_pos, pos.y - mPos.y_pos, pos.z - mPos.z_pos, angles);
		mPos.x_rot = angles[1];
		mPos.y_rot = angles[0];
		TriggerHarpyMissile(&mPos, item->room_number, 2);
	} else if (item->item_flags[0] >= 61 && item->item_flags[0] <= 65 && !(GlobalCounter & 1)) {
		pos.x = left_hand.x;
		pos.y = left_hand.y << 1;
		pos.z = left_hand.z;
		GetJointAbsPosition(item, &pos, left_hand.mesh_num);
		mPos.x_pos = lh.x;
		mPos.y_pos = lh.y;
		mPos.z_pos = lh.z;
		phd_GetVectorAngles(pos.x - mPos.x_pos, pos.y - mPos.y_pos, pos.z - mPos.z_pos, angles);
		mPos.x_rot = angles[1];
		mPos.y_rot = angles[0];
		TriggerHarpyMissile(&mPos, item->room_number, 2);
	}
}

void InitialiseHarpy(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[HARPY].anim_index + 4;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
}

void HarpyControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* enemy;
	CREATURE_INFO* harpy;
	CREATURE_INFO* baddie;
	AI_INFO info;
	int32_t dx, dy, dz, dist, max_dist;
	int16_t angle, head, torso_x, torso_y;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, HARPY);

	item = &items[item_number];
	harpy = (CREATURE_INFO*)item->data;
	angle = 0;
	head = 0;
	torso_x = 0;
	torso_y = 0;

	if (item->hit_points <= 0) {
		item->hit_points = 0;

		if (item->current_anim_state != 9) {
			if (item->current_anim_state != 10) {
				if (item->current_anim_state == 11) {
					item->pos.x_rot = 0;
					item->pos.y_pos = item->floor;
				} else {
					item->anim_number = objects[HARPY].anim_index + 5;
					item->frame_number = anims[item->anim_number].frame_base;
					item->current_anim_state = 9;
					item->gravity_status = 1;
					item->speed = 0;
					item->pos.x_rot = 0;
				}
			}
		} else
			item->goal_anim_state = 10;

		if (item->current_anim_state == 10) {
			if (item->pos.y_pos >= item->floor) {
				item->pos.y_pos = item->floor;
				item->fallspeed = 0;
				item->goal_anim_state = 11;
				item->gravity_status = 0;
			}

			item->pos.x_rot = 0;
		}
	} else {
		if (item->ai_bits)
			GetAITarget(harpy);

		harpy->enemy = 0;
		max_dist = 0x7FFFFFFF;

		for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
			baddie = &baddie_slots[i];

			if (baddie->item_num != NO_ITEM && baddie->item_num != item_number) {
				enemy = &items[baddie->item_num];

				if (enemy->object_number == T4PlusGetLaraDoubleSlotID()) {
					dx = enemy->pos.x_pos - item->pos.x_pos;
					dz = enemy->pos.z_pos - item->pos.z_pos;
					dist = SQUARE(dx) + SQUARE(dz);

					if (dist < max_dist) {
						harpy->enemy = enemy;
						max_dist = dist;
					}
				}
			}
		}

		CreatureAIInfo(item, &info);
		enemy = harpy->enemy;

		if (enemy != lara_item)
			phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, harpy->maximum_turn);

		if (info.ahead) {
			head = info.angle >> 1;
			torso_y = info.angle >> 1;
			torso_x = info.x_angle;
		}

		switch (item->current_anim_state) {
			case 1:
				harpy->flags = 0;
				harpy->maximum_turn = DEGREES_TO_ROTATION(7);

				if (enemy) {
					dy = item->pos.y_pos + (BLOCK_SIZE * 2);

					if (enemy->pos.y_pos > dy && item->floor > dy) {
						item->goal_anim_state = 3;
						break;
					}
				}

				if (info.ahead) {
					dy = abs(enemy->pos.y_pos - item->pos.y_pos);

					if (dy <= BLOCK_SIZE && info.distance < 0x1C639) {
						item->goal_anim_state = 6;
						break;
					}

					if (dy <= BLOCK_SIZE && info.distance < 0x400000) {
						item->goal_anim_state = 4;
						break;
					}
				}

				if (harpy->enemy == lara_item && Targetable(item, &info) && info.distance > 0xC40000 && GetRandomControl() & 1) {
					item->goal_anim_state = 8;
					item->item_flags[0] = 0;
				} else
					item->goal_anim_state = 2;

				break;

			case 2:
				harpy->maximum_turn = DEGREES_TO_ROTATION(7);
				harpy->flags = 0;

				if (item->required_anim_state) {
					item->goal_anim_state = item->required_anim_state;

					if (item->goal_anim_state == 8)
						item->item_flags[0] = 0;
				} else if (item->hit_status)
					item->goal_anim_state = 7;
				else if (!info.ahead) {
					if (GetRandomControl() & 1)
						item->goal_anim_state = 7;
					else if (!info.ahead)
						item->goal_anim_state = 4;
				} else if (info.distance < 0x1C639)
					item->goal_anim_state = 6;
				else if (!info.ahead || info.distance < 0x400000 || info.distance <= 0xC40000 || !(GetRandomControl() & 1))
					item->goal_anim_state = 4;
				else {
					item->goal_anim_state = 8;
					item->item_flags[0] = 0;
				}

				break;

			case 3:
				dy = item->pos.y_pos + (BLOCK_SIZE * 2);

				if (!enemy || enemy->pos.y_pos < dy || item->floor < dy)
					item->goal_anim_state = 1;

				break;

			case 4:
				harpy->maximum_turn = DEGREES_TO_ROTATION(2);

				if (info.ahead && info.distance < 0x400000)
					item->goal_anim_state = 5;
				else
					item->goal_anim_state = 13;

				break;

			case 5:
				harpy->maximum_turn = DEGREES_TO_ROTATION(2);
				item->goal_anim_state = 2;
				dy = abs(enemy->pos.y_pos - item->pos.y_pos);

				if (item->touch_bits & 0x14 || enemy && enemy != lara_item && dy <= BLOCK_SIZE && info.distance < 0x40000) {
					lara_item->hit_points -= mod_object_customization->damage_1;
					lara_item->hit_status = 1;

					if (item->touch_bits & 0x10)
						CreatureEffectT(item, &left_hit, 5, -1, DoBloodSplat);
					else
						CreatureEffectT(item, &right_hit, 5, -1, DoBloodSplat);
				}

				break;

			case 6:
				harpy->maximum_turn = DEGREES_TO_ROTATION(2);

				if (!harpy->flags) {
					dy = abs(enemy->pos.y_pos - item->pos.y_pos);

					if (item->touch_bits & 0x300000 || enemy && enemy != lara_item && dy <= BLOCK_SIZE && info.distance < 0x40000) {
						lara_item->hit_points -= mod_object_customization->damage_2;
						lara_item->hit_status = 1;
						CreatureEffectT(item, &tail_hit, 10, -1, DoBloodSplat);

						if (enemy == lara_item)
							lara.dpoisoned += 2048;

						harpy->flags = 1;
					}
				}

				break;

			case 8:
				DoHarpyEffects(item, item_number);
				break;

			case 12:

				if (info.ahead && info.distance > 0xC40000) {
					item->goal_anim_state = 2;
					item->required_anim_state = 8;
				} else if (GetRandomControl() & 1)
					item->goal_anim_state = 1;

				break;

			case 13:
				item->goal_anim_state = 2;
				break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, torso_y);
	CreatureJoint(item, 2, torso_x);
	CreatureAnimation(item_number, angle, 0);
}
