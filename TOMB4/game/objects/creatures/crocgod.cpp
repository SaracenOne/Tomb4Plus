#include "../../../tomb4/pch.h"
#include "croc.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../control.h"
#include "../../../specific/3dmath.h"
#include "../../people.h"
#include "../../effects.h"
#include "../../../specific/function_stubs.h"
#include "../../sphere.h"
#include "../../items.h"
#include "../../../specific/output.h"
#include "../../draw.h"
#include "../../sound.h"
#include "../../tomb4fx.h"
#include "../../effect2.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"
#include "../../../tomb4/tomb4plus/t4plus_objects.h"

#include "../effects/locusts.h"

void TriggerCrocgodMissile(PHD_3DPOS* pos, int16_t room_number, int16_t num) {
	FX_INFO* fx;
	int16_t fx_number;

	fx_number = CreateEffect(room_number);

	if (fx_number != NO_ITEM) {
		fx = &effects[fx_number];
		fx->pos.x_pos = pos->x_pos;
		fx->pos.y_pos = pos->y_pos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.z_pos = pos->z_pos;
		fx->pos.x_rot = pos->x_rot;
		fx->pos.y_rot = pos->y_rot;
		fx->pos.z_rot = 0;
		fx->room_number = room_number;
		fx->counter = 15 + (num << 4);
		fx->flag1 = 6;
		fx->object_number = T4PlusGetBubblesSlotID();
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->frame_number = objects[T4PlusGetBubblesSlotID()].mesh_index + 10;
	}
}

void TriggerCrocgodMissileFlame(int16_t fx_number, int32_t xv, int32_t yv, int32_t zv) {
	FX_INFO* fx;
	SPARKS* sptr;
	int32_t dx, dz;

	fx = &effects[fx_number];
	dx = lara_item->pos.x_pos - fx->pos.x_pos;
	dz = lara_item->pos.z_pos - fx->pos.z_pos;

	if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
		return;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = (GetRandomControl() & 0x3F) + 128;
	sptr->sG = sptr->sR >> 1;
	sptr->sB = 0;
	sptr->dR = (GetRandomControl() & 0x3F) + 128;
	sptr->dG = sptr->dR >> 1;
	sptr->dB = 0;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 8;
	sptr->TransType = 2;
	sptr->Dynamic = -1;
	sptr->Life = (GetRandomControl() & 7) + 32;
	sptr->sLife = sptr->Life;
	sptr->x = fx->pos.x_pos + (GetRandomControl() & 0xF) - 8;
	sptr->y = fx->pos.y_pos;
	sptr->z = fx->pos.z_pos + (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (int16_t)xv;
	sptr->Yvel = (int16_t)yv;
	sptr->Zvel = (int16_t)zv;
	sptr->Friction = 34;
	sptr->Flags = SF_SCALE | SF_DEF | SF_ROTATE | SF_UNUSED2;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -32 - (GetRandomControl() & 0x1F);
	else
		sptr->RotAdd = (GetRandomControl() & 0x1F) + 32;

	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->FxObj = (uint8_t)fx_number;
	sptr->Scalar = 2;
	sptr->Size = (GetRandomControl() & 0xF) + 128;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size >> 2;
}

void InitialiseCrocgod(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[MUTANT].anim_index;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
}

void CrocgodControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* crocgod;
	AI_INFO info;
	PHD_3DPOS mPos;
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	int16_t angles[2];
	int16_t angle, torso, neck, frame;

	if (!CreatureActive(item_number))
		return;

	angle = 0;
	torso = 0;
	neck = 0;
	item = &items[item_number];
	crocgod = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0)
		item->hit_points = 0;
	else {
		if (item->ai_bits)
			GetAITarget(crocgod);
		else if (crocgod->hurt_by_lara)
			crocgod->enemy = lara_item;

		item->pos.y_pos -= (HALF_BLOCK_SIZE + CLICK_SIZE);
		CreatureAIInfo(item, &info);
		item->pos.y_pos += (HALF_BLOCK_SIZE + CLICK_SIZE);

		if (crocgod->enemy != lara_item)
			phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);
		crocgod->maximum_turn = 0;
		angle = CreatureTurn(item, 0);

		if (item->item_flags[2] == 999) {
			torso = info.angle;
			neck = info.x_angle;
		}

		switch (item->current_anim_state) {
			case 2:

				if (item->item_flags[2] < 600) {
					item->goal_anim_state = 4;
					item->item_flags[2]++;
				} else {
					item->item_flags[2] = 999;

					if (info.distance < 0x1900000)
						item->goal_anim_state = 5;
					else if (info.distance < 0x3840000)
						item->goal_anim_state = 3;
					else if (info.distance < 0x7900000)
						item->goal_anim_state = 4;
				}

				break;

			case 3:
				frame = item->frame_number - anims[item->anim_number].frame_base;

				if (frame >= 94 && frame <= 96) {
					pos.x = 0;
					pos.y = -96;
					pos.z = 144;
					GetJointAbsPosition(item, &pos, 9);
					pos2.x = 0;
					pos2.y = -128;
					pos2.z = 288;
					GetJointAbsPosition(item, &pos2, 9);
					mPos.z_pos = pos2.z;
					mPos.y_pos = pos2.y;
					mPos.x_pos = pos2.x;
					phd_GetVectorAngles(pos2.x - pos.x, pos2.y - pos.y, pos2.z - pos.z, angles);
					mPos.y_rot = angles[0];
					mPos.x_rot = angles[1];

					if (frame == 94)
						TriggerCrocgodMissile(&mPos, item->room_number, 0);
					else {
						if (frame == 95)
							mPos.y_rot = angles[0] - (BLOCK_SIZE * 2);
						else
							mPos.y_rot = angles[0] + (BLOCK_SIZE * 2);

						TriggerCrocgodMissile(&mPos, item->room_number, 1);
					}
				}

				break;

			case 4:

				if (item->item_flags[2] < 600)
					item->item_flags[2]++;

				if (item->item_flags[2] == 999) {
					frame = item->frame_number - anims[item->anim_number].frame_base;

					if (frame >= 60 && frame <= 120)
						TriggerLocust(item);
				}

				break;

			case 5:
				frame = item->frame_number - anims[item->anim_number].frame_base;

				if (frame == 45 || frame == 60 || frame == 75) {
					pos.x = 0;
					pos.y = -96;
					pos.z = 144;
					GetJointAbsPosition(item, &pos, 9);
					pos2.x = 0;
					pos2.y = -128;
					pos2.z = 288;
					GetJointAbsPosition(item, &pos2, 9);
					mPos.z_pos = pos2.z;
					mPos.y_pos = pos2.y;
					mPos.x_pos = pos2.x;
					phd_GetVectorAngles(pos2.x - pos.x, pos2.y - pos.y, pos2.z - pos.z, angles);
					mPos.y_rot = angles[0];
					mPos.x_rot = angles[1];

					if (frame == 60)
						TriggerCrocgodMissile(&mPos, item->room_number, 0);
					else
						TriggerCrocgodMissile(&mPos, item->room_number, 1);
				}

				break;
		}
	}

	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, neck);
	CreatureJoint(item, 2, torso);
	CreatureJoint(item, 3, neck);
	CreatureAnimation(item_number, angle, 0);
}
