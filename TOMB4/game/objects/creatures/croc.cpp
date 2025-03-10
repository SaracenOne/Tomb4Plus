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

static BITE_INFO croc_bite = { 0, -100, 500, 9 };

void InitialiseCroc(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);

	if (room[item->room_number].flags & ROOM_UNDERWATER) {
		item->anim_number = objects[CROCODILE].anim_index + 12;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = 8;
		item->goal_anim_state = 8;
	} else {
		item->anim_number = objects[CROCODILE].anim_index;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
	}
}

void CrocControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* croc;
	FLOOR_INFO* floor;
	AI_INFO info;
	int32_t s, c, x, z, h, h2;
	int16_t room_number, angle, rot, roll;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, CROCODILE);

	item = &items[item_number];
	croc = (CREATURE_INFO*)item->data;
	angle = 0;
	rot = 0;
	s = (BLOCK_SIZE * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	c = (BLOCK_SIZE * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
	x = item->pos.x_pos + s;
	z = item->pos.z_pos + c;
	room_number = item->room_number;
	floor = GetFloor(x, item->pos.y_pos, z, &room_number);
	h = GetHeight(floor, x, item->pos.y_pos, z);

	if (abs(item->pos.y_pos - h) > 512)
		h = item->pos.y_pos;

	x = item->pos.x_pos - s;
	z = item->pos.z_pos - c;
	room_number = item->room_number;
	floor = GetFloor(x, item->pos.y_pos, z, &room_number);
	h2 = GetHeight(floor, x, item->pos.y_pos, z);

	if (abs(item->pos.y_pos - h2) > 512)
		h2 = item->pos.y_pos;

	roll = (int16_t)phd_atan((BLOCK_SIZE * 2), h2 - h);

	if (item->hit_points <= 0) {
		item->hit_points = 0;

		if (item->current_anim_state != 7 && item->current_anim_state != 10) {
			if (room[item->room_number].flags & ROOM_UNDERWATER) {
				item->anim_number = objects[CROCODILE].anim_index + 16;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = 10;
				item->goal_anim_state = 10;
				item->hit_points = INFINITE_HEALTH;
			} else {
				item->anim_number = objects[CROCODILE].anim_index + 11;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = 7;
				item->goal_anim_state = 7;
			}
		}

		if (room[item->room_number].flags & ROOM_UNDERWATER)
			CreatureFloat(item_number);
	} else {
		if (item->ai_bits)
			GetAITarget(croc);
		else if (croc->hurt_by_lara)
			croc->enemy = lara_item;

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, croc->maximum_turn);

		if (item->hit_status || info.distance < 0x240000 || (TargetVisible(item, &info) && info.distance < 0x1900000)) {
			if (!croc->alerted)
				croc->alerted = 1;

			AlertAllGuards(item_number);
		}

		rot = angle << 2;

		switch (item->current_anim_state) {
			case 1:
				croc->maximum_turn = 0;

				if (item->ai_bits & GUARD) {
					rot = item->item_flags[0];
					item->goal_anim_state = 1;
					item->item_flags[0] += item->item_flags[1];

					if (!(GetRandomControl() & 0x1F)) {
						if (GetRandomControl() & 1)
							item->item_flags[1] = 0;
						else {
							if (GetRandomControl() & 1)
								item->item_flags[1] = 12;
							else
								item->item_flags[1] = -12;
						}
					}

					if (item->item_flags[0] > BLOCK_SIZE)
						item->item_flags[0] = BLOCK_SIZE;
					else if (item->item_flags[0] < -BLOCK_SIZE)
						item->item_flags[0] = -BLOCK_SIZE;
				} else if (info.bite && info.distance < 0x90000)
					item->goal_anim_state = 5;
				else if (info.ahead && info.distance < 0x100000)
					item->goal_anim_state = 3;
				else
					item->goal_anim_state = 2;

				break;

			case 2:
				croc->maximum_turn = DEGREES_TO_ROTATION(3);

				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.bite && info.distance < 0x90000)
					item->goal_anim_state = 1;
				else if (info.ahead && info.distance < 0x100000)
					item->goal_anim_state = 3;

				break;

			case 3:
				croc->maximum_turn = DEGREES_TO_ROTATION(3);
				croc->LOT.step = CLICK_SIZE;
				croc->LOT.drop = -CLICK_SIZE;

				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.bite && info.distance < 0x90000)
					item->goal_anim_state = 1;
				else if (!info.ahead || info.distance > 0x240000)
					item->goal_anim_state = 2;

				break;

			case 5:

				if (item->frame_number == anims[item->anim_number].frame_base)
					item->required_anim_state = 0;

				if (info.bite && item->touch_bits & 0x300) {
					if (!item->required_anim_state) {
						CreatureEffectT(item, &croc_bite, 10, -1, DoBloodSplat);
						lara_item->hit_points -= mod_object_customization->damage_1;
						lara_item->hit_status = 1;
						item->required_anim_state = 1;
					}
				} else
					item->goal_anim_state = 1;

				break;

			case 8:
				croc->maximum_turn = DEGREES_TO_ROTATION(3);
				croc->LOT.step = (BLOCK_SIZE * 20);
				croc->LOT.drop = -(BLOCK_SIZE * 20);

				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.bite && item->touch_bits & 0x300)
					item->goal_anim_state = 9;

				break;

			case 9:

				if (item->frame_number == anims[item->anim_number].frame_base)
					item->required_anim_state = 0;

				if (info.bite && item->touch_bits & 0x300) {
					if (!item->required_anim_state) {
						CreatureEffectT(item, &croc_bite, 10, -1, DoBloodSplat);
						lara_item->hit_points -= mod_object_customization->damage_1;
						lara_item->hit_status = 1;
						item->required_anim_state = 8;
					}
				} else
					item->goal_anim_state = 8;

				break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, rot);
	CreatureJoint(item, 1, rot);
	CreatureJoint(item, 2, -rot);
	CreatureJoint(item, 3, -rot);

	if (item->current_anim_state < 8) {
		if (abs(roll - item->pos.x_rot) < 0x100)
			item->pos.x_rot = roll;
		else if (roll > item->pos.x_rot)
			item->pos.x_rot += 0x100;
		else if (roll < item->pos.x_rot)
			item->pos.x_rot -= 0x100;
	}

	CreatureAnimation(item_number, angle, 0);

	if (item->current_anim_state == 8)
		s = (0x400 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	else
		s = (0x200 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;

	c = (0x400 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
	x = item->pos.x_pos + s;
	z = item->pos.z_pos + c;
	room_number = item->room_number;
	GetFloor(x, item->pos.y_pos, z, &room_number);

	if (room[item->room_number].flags & ROOM_UNDERWATER) {
		if (room[room_number].flags & 1) {
			if (item->current_anim_state == 2) {
				item->required_anim_state = 3;
				item->goal_anim_state = 3;
			} else if (item->current_anim_state == 3) {
				item->required_anim_state = 8;
				item->goal_anim_state = 8;
			} else if (item->anim_number != objects[CROCODILE].anim_index + 17) {
				croc->LOT.step = (BLOCK_SIZE * 20);
				croc->LOT.drop = -(BLOCK_SIZE * 20);
				croc->LOT.fly = 16;
				CreatureUnderwater(item, CLICK_SIZE);
			}
		} else {
			item->required_anim_state = 3;
			item->goal_anim_state = 3;
			croc->LOT.step = CLICK_SIZE;
			croc->LOT.drop = -CLICK_SIZE;
			croc->LOT.fly = 0;
			CreatureUnderwater(item, 0);
		}
	} else
		croc->LOT.fly = 0;
}