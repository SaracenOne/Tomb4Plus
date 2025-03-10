#include "../../../tomb4/pch.h"
#include "sas.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../control.h"
#include "../../lara_states.h"
#include "../../switch.h"
#include "../../items.h"
#include "../../collide.h"
#include "../../sphere.h"
#include "../../tomb4fx.h"
#include "../../box.h"
#include "../../effect2.h"
#include "../../../specific/3dmath.h"
#include "../../people.h"
#include "../../../specific/input.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"

static BITE_INFO sas_fire = { 0, 300, 64, 7 };

void InitialiseInjuredSas(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->trigger_flags) {
		item->anim_number = objects[SAS_DYING].anim_index;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
	} else {
		item->anim_number = objects[SAS_DYING].anim_index + 3;
		item->current_anim_state = 4;
		item->goal_anim_state = 4;
	}

	item->frame_number = anims[item->anim_number].frame_base;
}

void InjuredSasControl(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->current_anim_state == 1) {
		if (!(GetRandomControl() & 0x7F))
			item->goal_anim_state = 2;
		else if (!(GetRandomControl() & 0xFF))
			item->goal_anim_state = 3;
	} else if (item->current_anim_state == 4 && !(GetRandomControl() & 0x7F))
		item->goal_anim_state = 5;

	AnimateItem(item);
}

static void SasFireGrenade(ITEM_INFO* sas, int16_t xrot, int16_t yrot) {
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	PHD_VECTOR oPos;
	int32_t h;
	int16_t item_number;

	item_number = CreateItem();

	if (item_number == NO_ITEM)
		return;

	item = &items[item_number];
	item->shade = -0x3DF0;
	item->object_number = GRENADE;
	item->room_number = sas->room_number;
	pos.x = sas_fire.x;
	pos.y = sas_fire.y;
	pos.z = sas_fire.z;
	GetJointAbsPosition(sas, &pos, sas_fire.mesh_num);
	item->pos.x_pos = pos.x;
	item->pos.y_pos = pos.y;
	item->pos.z_pos = pos.z;
	oPos.x = pos.x;
	oPos.y = pos.y;
	oPos.z = pos.z;
	floor = GetFloor(pos.x, pos.y, pos.z, &item->room_number);
	h = GetHeight(floor, pos.x, pos.y, pos.z);

	if (h < pos.y) {
		item->pos.x_pos = sas->pos.x_pos;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = sas->pos.z_pos;
		item->room_number = sas->room_number;
	}

	SmokeCountL = 32;
	SmokeWeapon = WEAPON_GRENADE;

	for (int i = 0; i < 5; i++)
		TriggerGunSmoke(oPos.x, oPos.y, oPos.z, pos.x - oPos.x, pos.y - oPos.y, pos.z - oPos.z, 1, 5, 32);

	InitialiseItem(item_number);
	item->pos.x_rot = xrot + sas->pos.x_rot;
	item->pos.y_rot = yrot + sas->pos.y_rot;
	item->pos.z_rot = 0;

	if (GetRandomControl() & 3)
		item->item_flags[0] = 1;
	else
		item->item_flags[0] = 2;

	item->item_flags[2] = 1;
	item->speed = 128;
	item->fallspeed = (-128 * phd_sin(item->pos.x_rot)) >> W2V_SHIFT;
	item->current_anim_state = item->pos.x_rot;
	item->goal_anim_state = item->pos.y_rot;
	item->required_anim_state = 0;
	item->hit_points = 120;
	AddActiveItem(item_number);
}

void InitialiseSas(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[SAS].anim_index + 12;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
}

void SasControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* sas;
	AI_INFO info;
	PHD_VECTOR pos;
	int32_t iDistance, dx, dz;
	int16_t angle, tilt, head, torso_x, torso_y, iAngle, xrot, yrot;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, SAS);

	item = &items[item_number];
	sas = (CREATURE_INFO*)item->data;
	angle = 0;
	tilt = 0;
	head = 0;
	torso_x = 0;
	torso_y = 0;

	if (item->fired_weapon) {
		pos.x = sas_fire.x;
		pos.y = sas_fire.y;
		pos.z = sas_fire.z;
		GetJointAbsPosition(item, &pos, sas_fire.mesh_num);
		TriggerDynamic(pos.x, pos.y, pos.z, 2 * item->fired_weapon + 8, 24, 16, 4);
		item->fired_weapon--;
	}

	if (item->hit_points <= 0) {
		if (item->current_anim_state != 7) {
			item->anim_number = objects[item->object_number].anim_index + 19;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 7;
		}
	} else {
		if (item->ai_bits)
			GetAITarget(sas);
		else
			sas->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (sas->enemy == lara_item) {
			iDistance = info.distance;
			iAngle = info.angle;
		} else {
			dx = lara_item->pos.x_pos - item->pos.x_pos;
			dz = lara_item->pos.z_pos - item->pos.z_pos;
			iAngle = int16_t(phd_atan(dz, dx) - item->pos.y_rot);
			iDistance = SQUARE(dx) + SQUARE(dz);
		}

		if (sas->enemy == lara_item)
			GetCreatureMood(item, &info, false);
		else
			GetCreatureMood(item, &info, true);

		if (lara.vehicle != NO_ITEM && info.bite)
			sas->mood = ESCAPE_MOOD;

		if (sas->enemy == lara_item)
			CreatureMood(item, &info, false);
		else
			CreatureMood(item, &info, true);

		angle = CreatureTurn(item, sas->maximum_turn);

		if (item->hit_status)
			AlertAllGuards(item_number);

		switch (item->current_anim_state) {
			case 1:
				head = iAngle;
				sas->flags = 0;
				sas->maximum_turn = 0;

				if (item->anim_number == objects[item->object_number].anim_index + 17) {
					if (abs(info.angle) < DEGREES_TO_ROTATION(10))
						item->pos.y_rot += info.angle;
					else if (info.angle < 0)
						item->pos.y_rot -= DEGREES_TO_ROTATION(10);
					else
						item->pos.y_rot += DEGREES_TO_ROTATION(10);
				} else if (item->ai_bits == MODIFY || lara.vehicle) {
					if (abs(info.angle) < DEGREES_TO_ROTATION(2))
						item->pos.y_rot += info.angle;
					else if (info.angle < 0)
						item->pos.y_rot -= DEGREES_TO_ROTATION(2);
					else
						item->pos.y_rot += DEGREES_TO_ROTATION(2);
				}

				if (item->ai_bits & GUARD) {
					head = AIGuard(sas);

					if (!(GetRandomControl() & 0xFF)) {
						if (item->current_anim_state == 1)
							item->goal_anim_state = 4;
						else
							item->goal_anim_state = 1;
					}
				} else if (item->ai_bits & PATROL1 && item->ai_bits != MODIFY && !lara.vehicle) {
					item->goal_anim_state = 2;
					head = 0;
				} else if (Targetable(item, &info)) {
					if (info.distance >= 0x900000 && info.zone_number == info.enemy_zone) {
						if (item->ai_bits != MODIFY)
							item->goal_anim_state = 2;
					} else if (GetRandomControl() & 1)
						item->goal_anim_state = 8;
					else if (GetRandomControl() & 1)
						item->goal_anim_state = 10;
					else
						item->goal_anim_state = 12;
				} else if (item->ai_bits == MODIFY)
					item->goal_anim_state = 1;
				else if (sas->mood == ESCAPE_MOOD)
					item->goal_anim_state = 3;
				else if ((sas->alerted || sas->mood != BORED_MOOD) && (!(item->ai_bits & FOLLOW) || !sas->reached_goal && iDistance <= 0x400000)) {
					if (sas->mood != BORED_MOOD && info.distance > 0x400000)
						item->goal_anim_state = 3;
					else
						item->goal_anim_state = 2;
				} else
					item->goal_anim_state = 1;

				break;

			case 2:
				head = iAngle;
				sas->flags = 0;
				sas->maximum_turn = DEGREES_TO_ROTATION(5);

				if (item->ai_bits & PATROL1)
					item->goal_anim_state = 2;
				else if (lara.vehicle && (item->ai_bits == MODIFY || !item->ai_bits))
					item->goal_anim_state = 1;
				else if (sas->mood == ESCAPE_MOOD)
					item->goal_anim_state = 3;
				else if (item->ai_bits & GUARD || item->ai_bits & FOLLOW && (sas->reached_goal || iDistance > 0x400000))
					item->goal_anim_state = 1;
				else if (Targetable(item, &info)) {
					if (info.distance >= 0x900000 && info.zone_number == info.enemy_zone)
						item->goal_anim_state = 9;
					else
						item->goal_anim_state = 1;
				} else if (sas->mood != BORED_MOOD) {
					if (info.distance > 0x400000)
						item->goal_anim_state = 3;
				} else if (info.ahead)
					item->goal_anim_state = 1;

				break;

			case 3:

				if (info.ahead)
					head = info.angle;

				sas->maximum_turn = DEGREES_TO_ROTATION(10);
				tilt = angle >> 1;

				if (lara.vehicle) {
					if (item->ai_bits == MODIFY || !item->ai_bits) {
						item->goal_anim_state = 2;
						break;
					}
				}

				if (item->ai_bits & GUARD || item->ai_bits & FOLLOW && (sas->reached_goal || iDistance > 0x400000))
					item->goal_anim_state = 2;
				else if (sas->mood != ESCAPE_MOOD) {
					if (Targetable(item, &info))
						item->goal_anim_state = 2;
					else if (sas->mood == BORED_MOOD || sas->mood == STALK_MOOD && !(item->ai_bits & FOLLOW) && info.distance < 0x400000)
						item->goal_anim_state = 2;
				}

				break;

			case 4:
				head = iAngle;
				sas->flags = 0;
				sas->maximum_turn = 0;

				if (item->ai_bits & GUARD) {
					head = AIGuard(sas);

					if (!(GetRandomControl() & 0xFF))
						item->goal_anim_state = 1;
				} else if (Targetable(item, &info) || sas->mood || !info.ahead || item->ai_bits & MODIFY || lara.vehicle)
					item->goal_anim_state = 1;

				break;

			case 11:
			case 13:

				if (item->goal_anim_state != 1 && item->goal_anim_state != 14 && (sas->mood == ESCAPE_MOOD || !Targetable(item, &info))) {
					if (item->current_anim_state == 11)
						item->goal_anim_state = 1;
					else
						item->goal_anim_state = 14;
				}

			case 5:
			case 6:

				if (info.ahead) {
					torso_y = info.angle;
					torso_x = info.x_angle;
				}

				if (sas->flags)
					sas->flags--;
				else {
					ShotLara(item, &info, &sas_fire, torso_y, mod_object_customization->damage_1);
					sas->flags = 5;
					item->fired_weapon = 3;
				}

				break;

			case 8:
			case 10:
			case 12:
				sas->flags = 0;

				if (info.ahead) {
					torso_y = info.angle;
					torso_x = info.x_angle;

					if (Targetable(item, &info)) {
						if (item->current_anim_state == 8)
							item->goal_anim_state = 5;
						else if (item->current_anim_state == 12)
							item->goal_anim_state = 13;
						else if (GetRandomControl() & 1)
							item->goal_anim_state = 11;
						else
							item->goal_anim_state = 15;
					} else
						item->goal_anim_state = 1;
				}

				break;

			case 9:
				sas->flags = 0;

				if (info.ahead) {
					torso_y = info.angle;
					torso_x = info.x_angle;

					if (Targetable(item, &info))
						item->goal_anim_state = 6;
					else
						item->goal_anim_state = 2;
				}

				break;

			case 15:
				torso_y = info.angle;
				torso_x = info.x_angle;
				break;

			case 16:

				if (info.ahead) {
					xrot = info.x_angle;
					yrot = info.angle;
					torso_y = info.angle;
					torso_x = info.x_angle;

					if (info.distance > 0x900000) {
						xrot = int16_t(phd_sqrt(info.distance) + xrot - 1024);
						torso_x = xrot;
					}
				} else {
					xrot = 0;
					yrot = 0;
				}

				if (item->frame_number == anims[item->anim_number].frame_base + 20) {
					if (!sas->enemy->speed) {
						xrot = xrot + (GetRandomControl() & 0x1FF) - 256;
						yrot = yrot + (GetRandomControl() & 0x1FF) - 256;
						torso_x = xrot;
						torso_y = yrot;
					}

					SasFireGrenade(item, xrot, yrot);

					if (Targetable(item, &info))
						item->goal_anim_state = 15;
				}

				break;

			case 17:

				if (!lara.blindTimer && !(GetRandomControl() & 0x7F))
					item->goal_anim_state = 4;

				break;
		}

		if (lara.blindTimer > 100 && item->current_anim_state != 17) {
			sas->maximum_turn = 0;
			item->anim_number = objects[SAS].anim_index + 28;
			item->frame_number = anims[item->anim_number].frame_base + (GetRandomControl() & 7);
			item->current_anim_state = 17;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);
	CreatureAnimation(item_number, angle, 0);
}
