#include "../../../tomb4/pch.h"
#include "enemy_jeep.h"
#include "../../../specific/function_stubs.h"
#include "../../objects.h"
#include "../../lara_states.h"
#include "../../collide.h"
#include "../../control.h"
#include "../../../specific/3dmath.h"
#include "../../newinv.h"
#include "../../../specific/specificfx.h"
#include "../../effect2.h"
#include "../../lara1gun.h"
#include "../../tomb4fx.h"
#include "../../items.h"
#include "../../sound.h"
#include "../../../specific/audio.h"
#include "../../laraflar.h"
#include "../../lot.h"
#include "../../lara.h"
#include "../../sphere.h"
#include "../../effects.h"
#include "../../traps.h"
#include "../../debris.h"
#include "../../box.h"
#include "../../switch.h"
#include "../../camera.h"
#include "../../draw.h"
#include "../../../specific/input.h"
#include "../../laramisc.h"
#include "../../../specific/file.h"
#include "../../gameflow.h"

#include "../../../tomb4/mod_config.h"
#include "../../../tomb4/tomb4plus/t4plus_objects.h"

void JeepFireGrenade(ITEM_INFO* item) {
	ITEM_INFO* grenade;
	int16_t item_number;

	item_number = CreateItem();

	if (item_number != NO_ITEM) {
		grenade = &items[item_number];
		grenade->shade = -0x3DF0;
		grenade->object_number = GRENADE;
		grenade->room_number = item->room_number;
		InitialiseItem(item_number);
		grenade->pos.x_rot = item->pos.x_rot;
		grenade->pos.z_rot = 0;
		grenade->pos.y_rot= item->pos.y_rot + 0x8000;
		grenade->pos.x_pos = item->pos.x_pos + (BLOCK_SIZE * phd_sin(grenade->pos.y_rot) >> W2V_SHIFT);
		grenade->pos.y_pos = item->pos.y_pos - (BLOCK_SIZE - CLICK_SIZE);
		grenade->pos.z_pos = item->pos.z_pos + (BLOCK_SIZE * phd_cos(grenade->pos.y_rot) >> W2V_SHIFT);
		SmokeCountL = 32;
		SmokeWeapon = 5;

		for (int i = 0; i < 5; i++)
			TriggerGunSmoke(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 0, 0, 0, 1, 5, 32);

		if (GetRandomControl() & 3)
			grenade->item_flags[0] = 1;
		else
			grenade->item_flags[0] = 2;

		grenade->speed = 32;
		grenade->fallspeed = -32 * phd_sin(grenade->pos.x_rot) >> W2V_SHIFT;
		grenade->current_anim_state = grenade->pos.x_rot;
		grenade->goal_anim_state = grenade->pos.y_rot;
		grenade->required_anim_state = 0;
		grenade->hit_points = 120;
		AddActiveItem(item_number);
	}
}

void InitialiseEnemyJeep(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[item->object_number].anim_index + 14;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 0;
	item->goal_anim_state = 0;
	item->mesh_bits = 0xFFFDBFFF;
	item->status -= ITEM_INVISIBLE;
}

void EnemyJeepControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* jeep;
	FLOOR_INFO* floor;
	AIOBJECT* aiobj;
	AI_INFO info;
	PHD_VECTOR pos;
	int32_t Xoffset, Zoffset, x, y, z, h1, h2, _h1, _h2, iAngle, iDist;
	int16_t room_number, xrot, zrot;

	if (!CreatureActive(item_number))
		return;

	item = &items[item_number];
	jeep = (CREATURE_INFO*)item->data;
	Xoffset = 682 * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	Zoffset = 682 * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
	x = item->pos.x_pos - Zoffset;
	y = item->pos.y_pos;
	z = item->pos.z_pos + Xoffset;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	h1 = GetHeight(floor, x, y, z);

	if (abs(y - h1) > (HALF_BLOCK_SIZE + CLICK_SIZE)) {
		item->pos.x_pos += Zoffset >> 6;
		item->pos.z_pos -= Xoffset >> 6;
		item->pos.y_rot += DEGREES_TO_ROTATION(2);
		h1 = y;
	}

	x = item->pos.x_pos + Zoffset;
	z = item->pos.z_pos - Xoffset;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	h2 = GetHeight(floor, x, y, z);

	if (abs(y - h2) > (HALF_BLOCK_SIZE + CLICK_SIZE)) {
		item->pos.x_pos -= Zoffset >> 6;
		item->pos.z_pos += Xoffset >> 6;
		item->pos.y_rot -= DEGREES_TO_ROTATION(2);
		h2 = y;
	}

	zrot = (int16_t)phd_atan(1364, h2 - h1);

	x = item->pos.x_pos + Xoffset;
	z = item->pos.z_pos + Zoffset;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	h1 = GetHeight(floor, x, y, z);
	_h1 = h1;

	if (abs(y - h1) > (HALF_BLOCK_SIZE + CLICK_SIZE))
		h1 = y;

	x = item->pos.x_pos - Xoffset;
	z = item->pos.z_pos - Zoffset;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	h2 = GetHeight(floor, x, y, z);
	_h2 = h2;

	if (abs(y - h2) > (HALF_BLOCK_SIZE + CLICK_SIZE))
		h2 = y;

	xrot = (int16_t)phd_atan(1364, h2 - h1);
	CreatureAIInfo(item, &info);
	jeep->enemy = &jeep->ai_target;

	if (jeep->enemy == lara_item) {
		iAngle = info.angle;
		iDist = info.distance;
	} else {
		x = lara_item->pos.x_pos - item->pos.x_pos;
		z = lara_item->pos.z_pos - item->pos.z_pos;

		iAngle = phd_atan(z, x) - item->pos.y_rot;

		if (x > 32000 || x < -32000 || z > 32000 || z < -32000)
			iDist = 0x7FFFFFFF;
		else
			iDist = SQUARE(x) + SQUARE(z);
	}

	switch (item->current_anim_state) {
		case 0:
		case 2:
			item->item_flags[0] -= 128;
			item->mesh_bits = 0xFFFE7FFF;

			if (item->item_flags[0] < 0)
				item->item_flags[0] = 0;

			pos.x = 0;
			pos.y = -(HALF_CLICK_SIZE + (QUARTER_CLICK_SIZE / 4));
			pos.z = -BLOCK_SIZE;
			GetJointAbsPosition(item, &pos, 11);
			TriggerDynamic(pos.x, pos.y, pos.z, 10, 64, 0, 0);

			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else if (info.distance > 0x100000 || lara.location >= item->item_flags[3])
				item->goal_anim_state = 1;

			break;

		case 1:
			jeep->maximum_turn = item->item_flags[0] >> 4;
			item->item_flags[0] += 37;		//34 in debug exe
			item->mesh_bits = 0xFFFDBFFF;

			if (item->item_flags[0] > 0x2200)
				item->item_flags[0] = 0x2200;

			if (info.angle > 0x100)
				item->goal_anim_state = 4;
			else if (info.angle < -0x100)
				item->goal_anim_state = 3;

			break;

		case 3:
		case 4:
			item->item_flags[0] += 18;		//17 in debug exe

			if (item->item_flags[0] > 0x2200)
				item->item_flags[0] = 0x2200;

			item->goal_anim_state = 1;
			break;

		case 5:

			if (item->item_flags[0] < 0x4A0)
				item->item_flags[0] = 0x4A0;

			break;
	}

	if (_h1 > item->floor + HALF_BLOCK_SIZE) {
		jeep->LOT.is_jumping = 1;

		if (item->item_flags[1] > 0) {
			xrot = item->item_flags[1];
			item->item_flags[1] -= 8;

			if (item->item_flags[1] < 0)
				jeep->LOT.is_jumping = 0;

			item->pos.y_pos += item->item_flags[1] >> 6;
		} else {
			item->item_flags[1] = xrot << 1;
			jeep->LOT.is_jumping = 1;
		}

		if (jeep->LOT.is_jumping) {
			jeep->maximum_turn = 0;
			item->goal_anim_state = 1;
		}
	} else if (_h2 > item->floor + HALF_BLOCK_SIZE && item->current_anim_state != 5) {
		item->item_flags[1] = 0;
		item->anim_number = objects[item->object_number].anim_index + 8;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = 5;
		item->goal_anim_state = 1;
	}

	if (info.distance < 0x240000 || item->item_flags[3] == -2)
		jeep->reached_goal = 1;

	if (jeep->reached_goal) {
		TestTriggersAtXYZ(jeep->enemy->pos.x_pos, jeep->enemy->pos.y_pos, jeep->enemy->pos.z_pos, jeep->enemy->room_number, 1, 0);

		if (lara.location < item->item_flags[3] && item->current_anim_state != 2 && item->goal_anim_state != 2) {
			item->anim_number = objects[item->object_number].anim_index + 1;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 2;
			item->goal_anim_state = 2;

			if (jeep->enemy->flags & 4) {
				item->pos.x_pos = jeep->enemy->pos.x_pos;
				item->pos.y_pos = jeep->enemy->pos.y_pos;
				item->pos.z_pos = jeep->enemy->pos.z_pos;
				item->pos.x_rot = jeep->enemy->pos.x_rot;
				item->pos.y_rot = jeep->enemy->pos.y_rot;
				item->pos.z_rot = jeep->enemy->pos.z_rot;

				if (item->room_number != jeep->enemy->room_number)
					ItemNewRoom(item_number, jeep->enemy->room_number);
			}
		}

		if (iDist > 0x400000 && iDist < 0x6400000 && !item->item_flags[2] && (iAngle < -20480 || iAngle > 20480)) {
			JeepFireGrenade(item);
			item->item_flags[2] = 150;
		}

		if (jeep->enemy->flags == 62) {
			item->status = ITEM_INVISIBLE;
			RemoveActiveItem(item_number);
			DisableBaddieAI(item_number);
		}

		if (lara.location >= item->item_flags[3] || !(jeep->enemy->flags & 4)) {
			jeep->reached_goal = 0;
			item->item_flags[3]++;
			jeep->enemy = 0;

			for (int i = 0; i < nAIObjects; i++) {
				aiobj = &AIObjects[i];

				if (aiobj->trigger_flags == item->item_flags[3] && aiobj->room_number != 255) {
					jeep->enemy = &jeep->ai_target;
					jeep->enemy->object_number = aiobj->object_number;
					jeep->enemy->room_number = aiobj->room_number;
					jeep->enemy->pos.x_pos = aiobj->x;
					jeep->enemy->pos.y_pos = aiobj->y;
					jeep->enemy->pos.z_pos = aiobj->z;
					jeep->enemy->pos.y_rot = aiobj->y_rot;
					jeep->enemy->flags = aiobj->flags;
					jeep->enemy->trigger_flags = aiobj->trigger_flags;
					jeep->enemy->box_number = aiobj->box_number;

					if (!(jeep->enemy->flags & 0x20)) {
						jeep->enemy->pos.x_pos += CLICK_SIZE * phd_sin(jeep->enemy->pos.y_rot) >> W2V_SHIFT;
						jeep->enemy->pos.z_pos += CLICK_SIZE * phd_cos(jeep->enemy->pos.y_rot) >> W2V_SHIFT;
					}

					break;
				}
			}
		}
	}

	item->item_flags[2]--;

	if (item->item_flags[2] < 0)
		item->item_flags[2] = 0;

	if (abs(xrot - item->pos.x_rot) < CLICK_SIZE)
		item->pos.x_rot = xrot;
	else if (xrot > item->pos.x_rot)
		item->pos.x_rot += CLICK_SIZE;
	else if (xrot < item->pos.x_rot)
		item->pos.x_rot -= CLICK_SIZE;

	if (abs(zrot - item->pos.z_rot) < CLICK_SIZE)
		item->pos.z_rot = zrot;
	else if (zrot > item->pos.z_rot)
		item->pos.z_rot += CLICK_SIZE;
	else if (zrot < item->pos.z_rot)
		item->pos.z_rot -= CLICK_SIZE;

	item->item_flags[0] -= xrot >> 9;
	item->item_flags[0] -= 2;

	if (item->item_flags[0] < 0)
		item->item_flags[0] = 0;

	x = item->item_flags[0] * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	z = item->item_flags[0] * phd_cos(item->pos.y_rot) >> W2V_SHIFT;

	for (int i = 0; i < 4; i++)
		jeep->joint_rotation[i] -= item->item_flags[0];

	item->pos.x_pos += x >> 6;
	item->pos.z_pos += z >> 6;

	if (!jeep->reached_goal)
		CreatureYRot(&item->pos, info.angle, item->item_flags[0] >> 4);

	jeep->maximum_turn = 0;
	AnimateItem(item);
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (item->pos.y_pos < item->floor)
		item->gravity_status = 1;
	else {
		item->fallspeed = 0;
		item->pos.y_pos = item->floor;
		item->gravity_status = 0;
	}

	SoundEffect(SFX_JEEP_MOVE, &item->pos, (item->item_flags[0] << 10) + (SFX_SETPITCH | 0x1000000));
}
