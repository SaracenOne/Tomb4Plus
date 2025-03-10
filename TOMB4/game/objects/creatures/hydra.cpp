#include "../../../tomb4/pch.h"
#include "hydra.h"
#include "../../objects.h"
#include "../../box.h"
#include "../../control.h"
#include "../../sound.h"
#include "../../../specific/3dmath.h"
#include "../../effects.h"
#include "../../sphere.h"
#include "../../items.h"
#include "../../effect2.h"
#include "../../../specific/function_stubs.h"
#include "../../tomb4fx.h"
#include "../../lara.h"
#include "../../../tomb4/mod_config.h"
#include "../../gameflow.h"
#include "../../../tomb4/tomb4plus/t4plus_objects.h"

static BITE_INFO hydra_hit = { 0, 0, 0, 11 };

void InitialiseHydra(int16_t item_number) {
	ITEM_INFO* item;

	int16_t hydra_slot = T4PlusGetHydraSlotID();
	if (hydra_slot < 0 || hydra_slot >= NUMBER_OBJECTS) {
		return;
	}

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[hydra_slot].anim_index;
	item->frame_number = 30 * item->trigger_flags + anims[item->anim_number].frame_base;
	item->goal_anim_state = 0;
	item->current_anim_state = 0;

	if (item->trigger_flags == 1)
		item->pos.z_pos += CLICK_SIZE + HALF_CLICK_SIZE;
	else if (item->trigger_flags == 2)
		item->pos.z_pos -= CLICK_SIZE + HALF_CLICK_SIZE;

	// TRNG behaviour
#ifndef TR5_BEHAVIOUR
	if (item->trigger_flags != 0)
#endif
		item->pos.y_rot = 0x4000;

	item->pos.x_pos -= CLICK_SIZE;
}

void HydraControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* hydra;
	PHD_VECTOR pos;
	PHD_3DPOS pos2;
	AI_INFO info;
	int16_t angles[2];
	int16_t tilt, head_z, head_x, head_y, neck_y, shade, damage, room_number;

	int16_t hydra_slot = T4PlusGetHydraSlotID();
	if (hydra_slot < 0 || hydra_slot >= NUMBER_OBJECTS) {
		return;
	}

	if (!CreatureActive(item_number))
		return;

	tilt = 0;
	head_z = 0;
	head_x = 0;
	head_y = 0;
	neck_y = 0;
	item = &items[item_number];
	hydra = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0) {
		item->hit_points = 0;

		if (item->current_anim_state != 11) {
			item->anim_number = objects[hydra_slot].anim_index + 15;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 11;
		}

		if (!((item->frame_number - anims[item->anim_number].frame_base) & 7)) {
			if (item->item_flags[3] < 12) {
				MOD_LEVEL_AUDIO_INFO* audio_info = get_game_mod_level_audio_info(gfCurrentLevel);

				ExplodeItemNode(item, 11 - item->item_flags[3], 0, 64);
				SoundEffect(audio_info->hydra_smash_sfx_id, &item->pos, 0);
				item->item_flags[3]++;
			}
		}
	} else {
		if (item->ai_bits)
			GetAITarget(hydra);
		else if (hydra->hurt_by_lara)
			hydra->enemy = lara_item;

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);

		if (item->current_anim_state != 5 && item->current_anim_state != 10 && item->current_anim_state != 11) {
			if (abs(info.angle) < DEGREES_TO_ROTATION(1))
				item->pos.y_rot += info.angle;
			else if (info.angle >= 0)
				item->pos.y_rot += DEGREES_TO_ROTATION(1);
			else
				item->pos.y_rot -= DEGREES_TO_ROTATION(1);

			if (item->trigger_flags == 1)
				tilt = -HALF_BLOCK_SIZE;
			else if (item->trigger_flags == 2)
				tilt = HALF_BLOCK_SIZE;
		}

		if (item->current_anim_state != 12) {
			head_y = info.angle >> 1;
			head_z = info.angle >> 1;
			head_x = -info.x_angle;
		}

		neck_y = -head_y;

		switch (item->current_anim_state) {
			case 0:
				hydra->maximum_turn = DEGREES_TO_ROTATION(1);
				hydra->flags = 0;

				if (item->trigger_flags == 1)
					tilt = -HALF_BLOCK_SIZE;
				else if (item->trigger_flags == 2)
					tilt = HALF_BLOCK_SIZE;

				if (info.distance < 0x310000 || !(GetRandomControl() & 0x1F))
					item->goal_anim_state = 6;
				else {
					if (info.distance < 0x400000 || !(GetRandomControl() & 0xF))
						item->goal_anim_state = 1;
					else if (!(GetRandomControl() & 0xF))
						item->goal_anim_state = 2;
				}

				break;

			case 1:
			case 7:
			case 8:
			case 9:
				hydra->maximum_turn = 0;

				if (!hydra->flags) {
					if (item->touch_bits & 0x400) {
						lara_item->hit_points -= 120;
						lara_item->hit_status = 1;
						CreatureEffectT(item, &hydra_hit, 10, item->pos.y_rot, DoBloodSplat);
						hydra->flags = 1;
					}

					if (item->hit_status && info.distance < 0x310000) {
						damage = int16_t(5 - phd_sqrt(info.distance) / 1024);

						if (lara.gun_type == WEAPON_SHOTGUN)
							damage *= 3;

						if (damage > 0) {
							item->goal_anim_state = 4;
							item->hit_points -= damage;
							CreatureEffectT(item, &hydra_hit, damage * 10, item->pos.y_rot, DoBloodSplat);
						}
					}
				}

				break;

			case 2:
				hydra->maximum_turn = 0;

				if (item->hit_status) {
					damage = int16_t(6 - phd_sqrt(info.distance) / 1024);

					if (lara.gun_type == WEAPON_SHOTGUN)
						damage *= 3;

					if ((GetRandomControl() & 0xF) < damage && info.distance < 0x6400000 && damage > 0) {
						item->hit_points -= damage;
						item->goal_anim_state = 4;
						CreatureEffectT(item, &hydra_hit, 10 * damage, item->pos.y_rot, DoBloodSplat);
					}
				}

				if (item->trigger_flags == 1)
					tilt = -HALF_BLOCK_SIZE;
				else if (item->trigger_flags == 2)
					tilt = HALF_BLOCK_SIZE;

				if (!(GlobalCounter & 3)) {
					shade = ((anims[item->anim_number].frame_base - item->frame_number) >> 3) + 1;

					if (shade > 16)
						shade = 16;

					TriggerHydraPowerupFlames(item_number, shade);
				}

				break;

			case 3:

				if (item->frame_number == anims[item->anim_number].frame_base) {
					pos.x = 0;
					pos.y = BLOCK_SIZE;
					pos.z = 40;
					GetJointAbsPosition(item, &pos, 10);

					pos2.x_pos = 0;
					pos2.y_pos = 144;
					pos2.z_pos = 40;
					GetJointAbsPosition(item, (PHD_VECTOR*)&pos2, 10);

					phd_GetVectorAngles(pos.x - pos2.x_pos, pos.y - pos2.y_pos, pos.z - pos2.z_pos, angles);
					pos2.x_rot = angles[1];
					pos2.y_rot = angles[0];
					room_number = item->room_number;
					GetFloor(pos2.x_pos, pos2.y_pos, pos2.z_pos, &room_number);
					TriggerHydraMissile(&pos2, room_number, 1);
				}
				break;

			case 6:
				hydra->maximum_turn = DEGREES_TO_ROTATION(1);
				hydra->flags = 0;

				if (item->trigger_flags == 1)
					tilt = -HALF_BLOCK_SIZE;
				else if (item->trigger_flags == 2)
					tilt = HALF_BLOCK_SIZE;

				if (info.distance < 0x90000)
					item->goal_anim_state = 7;
				else if (info.distance < 0x190000)
					item->goal_anim_state = 8;
				else if (info.distance < 0x310000)
					item->goal_anim_state = 9;
				else
					item->goal_anim_state = 0;

				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, neck_y);
	CreatureJoint(item, 1, head_y);
	CreatureJoint(item, 2, head_x);
	CreatureJoint(item, 3, head_z);
	CreatureAnimation(item_number, 0, 0);
}

void TriggerHydraMissile(PHD_3DPOS* pos, int16_t room_number, int16_t num) {
	FX_INFO* fx;
	int16_t fx_number;

	int16_t hydra_missile_slot = T4PlusGetHydraMissileSlotID();
	if (hydra_missile_slot < 0 || hydra_missile_slot >= NUMBER_OBJECTS) {
		return;
	}

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
		fx->counter = (num << 4) + 15;
		fx->speed = (GetRandomControl() & 0x1F) + 64;
		fx->flag1 = 0;
		fx->object_number = hydra_missile_slot;
		fx->frame_number = objects[hydra_missile_slot].mesh_index + 16;
	}
}

void TriggerHydraMissileFlame(PHD_VECTOR* pos, int32_t xv, int32_t yv, int32_t zv) {
	SPARKS* sptr;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = (GetRandomControl() & 0x3F) - 96;
	sptr->sG = sptr->sR >> 1;
	sptr->sB = 0;
	sptr->dR = (GetRandomControl() & 0x3F) - 96;
	sptr->dG = sptr->dR >> 1;
	sptr->dB = 0;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 8;
	sptr->TransType = 2;
	sptr->Dynamic = -1;
	sptr->Life = (GetRandomControl() & 3) + 20;
	sptr->sLife = sptr->Life;
	sptr->x = pos->x + (GetRandomControl() & 0xF) - 8;
	sptr->y = pos->y;
	sptr->z = pos->z + (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (int16_t)xv;
	sptr->Yvel = (int16_t)yv;
	sptr->Zvel = (int16_t)zv;
	sptr->Friction = 68;
	sptr->Flags = SF_ROTATE | SF_DEF | SF_SCALE;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -32 - (GetRandomControl() & 0x1F);
	else
		sptr->RotAdd = (GetRandomControl() & 0x1F) + 32;

	sptr->Gravity = 0;
	sptr->MaxYvel = 0;
	sptr->Scalar = 1;
	sptr->Size = (GetRandomControl() & 0xF) + 96;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size >> 2;
}

void TriggerHydraPowerupFlames(int16_t item_number, int32_t shade) {
	SPARKS* sptr;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sB = 0;
	sptr->sR = (GetRandomControl() & 0x3F) - 96;
	sptr->dR = (GetRandomControl() & 0x3F) - 96;
	sptr->dB = 0;

	if (shade < 16) {
		sptr->sR = (uint8_t)((shade * sptr->sR) >> 4);
		sptr->dR = (uint8_t)((shade * sptr->dR) >> 4);
	}

	sptr->sG = sptr->sR >> 1;
	sptr->dG = sptr->dR >> 1;
	sptr->FadeToBlack = 4;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 8;
	sptr->TransType = 2;
	sptr->Dynamic = -1;
	sptr->Life = (GetRandomControl() & 3) + 32;
	sptr->sLife = sptr->Life;
	sptr->x = (GetRandomControl() & 0xF) - 8;
	sptr->y = 0;
	sptr->z = (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (GetRandomControl() & 0xFF) - HALF_CLICK_SIZE;
	sptr->Yvel = 0;
	sptr->Zvel = (GetRandomControl() & 0xFF) - HALF_CLICK_SIZE;
	sptr->Friction = 4;
	sptr->Flags = SF_ATTACHEDNODE | SF_UNUSED2 | SF_ITEM | SF_ROTATE | SF_DEF | SF_SCALE;
	sptr->FxObj = (uint8_t)item_number;
	sptr->NodeNumber = NODE_ID_HYDRA;
	sptr->RotAng = GetRandomControl() & 0xFFF;
	sptr->RotAdd = (GetRandomControl() & 0x3F) - 0x20;
	sptr->MaxYvel = 0;
	sptr->Gravity = -8 - (GetRandomControl() & 7);
	sptr->Scalar = 0;
	sptr->Size = uint8_t((shade * ((GetRandomControl() & 0xF) + (QUARTER_CLICK_SIZE / 4))) >> 4);
	sptr->sSize = sptr->Size;
	sptr->dSize = 4;
}
