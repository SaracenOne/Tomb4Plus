#include "../../../tomb4/pch.h"
#include "baddy.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/3dmath.h"
#include "../../control.h"
#include "../../sphere.h"
#include "../../effect2.h"
#include "../../../specific/function_stubs.h"
#include "../../lot.h"
#include "../../items.h"
#include "../../people.h"
#include "../../lara_states.h"
#include "../../effects.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"

static BITE_INFO baddy_fire = { 0, -16, 200, 11 };
static BITE_INFO baddy_blade = { 0, 0, 0, 15 };

void InitialiseBaddy(int16_t item_number) {
	ITEM_INFO* item;
	int16_t obj_num, flag;

	item = &items[item_number];
	InitialiseCreature(item_number);

	if (objects[BADDY_2].loaded)
		obj_num = BADDY_2;
	else
		obj_num = BADDY_1;

	if (item->object_number == BADDY_1) {
		item->meshswap_meshbits = 0x7FC010;
		item->mesh_bits = ~0x7E0000;
		item->item_flags[2] = 24;
	} else {
		item->meshswap_meshbits = 0x880;
		item->mesh_bits = -1;
		item->item_flags[2] = 0;
	}

	item->item_flags[1] = -1;
	flag = item->trigger_flags % 1000;

	if (flag > 9 && flag < 20) {
		item->item_flags[2] += 24;
		item->trigger_flags -= 10;
		flag -= 10;
	}

	if (!flag || (flag > 4 && flag < 7)) {
		item->anim_number = objects[obj_num].anim_index + BADDY_STAND_IDLE_ANIMATION;
		item->current_anim_state = BADDY_STATE_IDLE;
		item->goal_anim_state = BADDY_STATE_IDLE;
	} else if (flag == 1) {
		item->anim_number = objects[obj_num].anim_index + BADDY_STAND_TO_JUMP_RIGHT_ANIMATION;
		item->current_anim_state = BADDY_STATE_JUMP_RIGHT;
		item->goal_anim_state = BADDY_STATE_JUMP_RIGHT;
	} else if (flag == 2) {
		item->anim_number = objects[obj_num].anim_index + BADDY_STAND_TO_ROLL_LEFT_ANIMATION;
		item->current_anim_state = BADDY_STATE_ROLL_LEFT;
		item->goal_anim_state = BADDY_STATE_ROLL_LEFT;
	} else if (flag == 3) {
		item->anim_number = objects[obj_num].anim_index + BADDY_CROUCH_ANIMATION;
		item->current_anim_state = BADDY_STATE_CROUCH;
		item->goal_anim_state = BADDY_STATE_CROUCH;
	} else if (flag == 4) {
		item->anim_number = objects[obj_num].anim_index + BADDY_CLIMB_4_CLICKS_ANIMATION;
		item->current_anim_state = BADDY_STATE_CLIMB_4_STEPS;
		item->goal_anim_state = BADDY_STATE_CLIMB_4_STEPS;
		item->pos.x_pos += CLICK_SIZE * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
		item->pos.z_pos += CLICK_SIZE * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
	} else if (flag > 100) {
		item->anim_number = objects[obj_num].anim_index + BADDY_CROUCH_ANIMATION;
		item->current_anim_state = BADDY_STATE_CROUCH;
		item->goal_anim_state = BADDY_STATE_CROUCH;
		item->pos.x_pos += CLICK_SIZE * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
		item->pos.z_pos += CLICK_SIZE * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
		item->item_flags[3] = flag;
	}

	item->frame_number = anims[item->anim_number].frame_base;
}

void BaddyControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* target;
	ITEM_INFO* enemy;
	CREATURE_INFO* baddy;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	AI_INFO info;
	AI_INFO larainfo;
	int32_t x, y, z, Xoffset, Zoffset, nearheight, midheight, farheight, jump_ahead, long_jump_ahead;
	int32_t dx, dz, h1, h2, can_jump, can_roll, h, c;
	int16_t obj_num, angle, tilt, head, torso_x, torso_y, room_number, target_num, state;

	if (!CreatureActive(item_number))
		return;

	item = &items[item_number];
	baddy = (CREATURE_INFO*)item->data;

	if (objects[BADDY_2].loaded)
		obj_num = BADDY_2;
	else
		obj_num = BADDY_1;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, obj_num);

	angle = 0;
	tilt = 0;
	head = 0;
	torso_x = 0;
	torso_y = 0;

	if (item->trigger_flags % 1000) {
		baddy->LOT.is_jumping = true;
		baddy->maximum_turn = 0;

		if (item->trigger_flags % 1000 > 100) {
			item->item_flags[0] = -80;
			FindAITargetObject(baddy, AI_X1);
		}

		item->trigger_flags = 1000 * (item->trigger_flags / 1000);
	}

	Xoffset = 942 * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	Zoffset = 942 * phd_cos(item->pos.y_rot) >> W2V_SHIFT;

	room_number = item->room_number;
	x = item->pos.x_pos + Xoffset;
	y = item->pos.y_pos;
	z = item->pos.z_pos + Zoffset;
	floor = GetFloor(x, y, z, &room_number);
	nearheight = GetHeight(floor, x, y, z);

	room_number = item->room_number;
	x += Xoffset;
	z += Zoffset;
	floor = GetFloor(x, y, z, &room_number);
	midheight = GetHeight(floor, x, y, z);

	room_number = item->room_number;
	x += Xoffset;
	z += Zoffset;
	floor = GetFloor(x, y, z, &room_number);
	farheight = GetHeight(floor, x, y, z);

	if ((baddy->enemy && item->box_number == baddy->enemy->box_number) ||
	        y >= nearheight - (CLICK_SIZE + HALF_CLICK_SIZE) || y >= midheight + CLICK_SIZE || y <= midheight - CLICK_SIZE)
		jump_ahead = 0;
	else
		jump_ahead = 1;

	if ((baddy->enemy && item->box_number == baddy->enemy->box_number) ||
	        y >= nearheight - (CLICK_SIZE + HALF_CLICK_SIZE) || y >= midheight - (CLICK_SIZE + HALF_CLICK_SIZE) || y >= farheight + CLICK_SIZE || y <= farheight - CLICK_SIZE)
		long_jump_ahead = 0;
	else
		long_jump_ahead = 1;

	if (item->item_flags[1] != item->room_number && gfCurrentLevel != 5) {
		for (target_num = room[item->room_number].item_number; target_num != NO_ITEM; target_num = target->next_item) {
			target = &items[target_num];

			if (target->object_number == SMALLMEDI_ITEM || target->object_number == UZI_AMMO_ITEM) {
				if (SameZone(baddy, target) && target->status != ITEM_INVISIBLE) {
					baddy->enemy = target;
					break;
				}
			}
		}
	}

	item->item_flags[1] = item->room_number;

	if (item->fired_weapon) {
		pos.x = baddy_fire.x;
		pos.y = baddy_fire.y;
		pos.z = baddy_fire.z;
		GetJointAbsPosition(item, &pos, baddy_fire.mesh_num);
		TriggerDynamic(pos.x, pos.y, pos.z, 4 * item->fired_weapon + 8, 24, 16, 4);
		item->fired_weapon--;
	}

	if (item->hit_points <= 0) {
		item->hit_points = 0;
		baddy->LOT.is_jumping = false;
		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		switch (item->current_anim_state) {
			case BADDY_STATE_MONKEY_GRAB:
			case BADDY_STATE_MONKEY_IDLE:
			case BADDY_STATE_MONKEY_FORWARD:
				item->anim_number = objects[obj_num].anim_index + BADDY_MONKEY_TO_FREEFALL_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_MONKEY_TO_FREEFALL;
				item->speed = 0;
				break;

			case BADDY_STATE_DEATH:
				item->gravity_status = 1;
				baddy->LOT.is_jumping = true;

				if (item->pos.y_pos >= item->floor) {
					item->pos.y_pos = item->floor;
					item->fallspeed = 0;
					item->gravity_status = 0;
				}

				break;

			case BADDY_STATE_MONKEY_TO_FREEFALL:
				item->goal_anim_state = BADDY_STATE_FREEFALL;
				item->gravity_status = 0;
				break;
			case BADDY_STATE_FREEFALL:
				item->gravity_status = 1;

				if (item->pos.y_pos >= item->floor) {
					item->pos.y_pos = item->floor;
					item->fallspeed = 0;
					item->gravity_status = 0;
					item->goal_anim_state = BADDY_STATE_FREEFALL_LAND_DEATH;
				}

				break;
			case BADDY_STATE_FREEFALL_LAND_DEATH:
				item->pos.y_pos = item->floor;
				break;
			default:
				item->anim_number = objects[obj_num].anim_index + BADDY_STAND_DEATH_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_DEATH;
				baddy->LOT.is_jumping = true;

				if (item->trigger_flags > 999) {
					for (target_num = room[2].item_number; target_num != NO_ITEM; target_num = target->next_item) {
						target = &items[target_num];

						if (target->trigger_flags / 1000 == item->trigger_flags / 1000 + 1) {
							target->touch_bits = 0;

							if (EnableBaddieAI(target_num, 0))
								item->status = ITEM_ACTIVE;
							else
								item->status = ITEM_INVISIBLE;

							AddActiveItem(target_num);
							break;
						}
					}
				}
				break;
		}
	} else {
		if (item->ai_bits)
			GetAITarget(baddy);
		else if (!baddy->enemy)
			baddy->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (baddy->enemy == lara_item) {
			larainfo.angle = info.angle;
			larainfo.ahead = info.ahead;
			larainfo.distance = info.distance;
		} else {
			dx = lara_item->pos.x_pos - item->pos.x_pos;
			dz = lara_item->pos.z_pos - item->pos.z_pos;
			larainfo.angle = int16_t(phd_atan(dz, dx) - item->pos.y_rot);

			if (larainfo.angle > -FRONT_ARC && larainfo.angle < FRONT_ARC)
				larainfo.ahead = 1;
			else
				larainfo.ahead = 0;

			larainfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, true);

		if (lara.vehicle != NO_ITEM && info.bite)
			baddy->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, baddy->maximum_turn);
		enemy = baddy->enemy;

		if (item->hit_status || (larainfo.distance < 0x100000 || TargetVisible(item, &larainfo)) && abs(lara_item->pos.y_pos - item->pos.y_pos) < BLOCK_SIZE)
			baddy->alerted = true;

		baddy->enemy = enemy;

		if (item == lara.target && larainfo.distance > 0x3AE && larainfo.angle > -0x2800 && larainfo.angle < 0x2800) {
			room_number = item->room_number;
			x = item->pos.x_pos + (942 * phd_sin(item->pos.y_rot + 0x2000) >> W2V_SHIFT);
			z = item->pos.z_pos + (942 * phd_cos(item->pos.y_rot + 0x2000) >> W2V_SHIFT);
			floor = GetFloor(x, y, z, &room_number);
			h1 = GetHeight(floor, x, y, z);

			room_number = item->room_number;
			x = item->pos.x_pos + (942 * phd_sin(item->pos.y_rot + 0x3800) >> W2V_SHIFT);
			z = item->pos.z_pos + (942 * phd_cos(item->pos.y_rot + 0x3800) >> W2V_SHIFT);
			floor = GetFloor(x, y, z, &room_number);
			h2 = GetHeight(floor, x, y, z);

			if (abs(h2 - y) > CLICK_SIZE || h1 + HALF_BLOCK_SIZE >= y)
				can_jump = false;
			else
				can_jump = true;

			room_number = item->room_number;
			x = item->pos.x_pos + (942 * phd_sin(item->pos.y_rot - 0x2000) >> W2V_SHIFT);
			z = item->pos.z_pos + (942 * phd_cos(item->pos.y_rot - 0x2000) >> W2V_SHIFT);
			floor = GetFloor(x, y, z, &room_number);
			h1 = GetHeight(floor, x, y, z);

			room_number = item->room_number;
			x = item->pos.x_pos + (942 * phd_sin(item->pos.y_rot - 0x3800) >> W2V_SHIFT);
			z = item->pos.z_pos + (942 * phd_cos(item->pos.y_rot - 0x3800) >> W2V_SHIFT);
			floor = GetFloor(x, y, z, &room_number);
			h2 = GetHeight(floor, x, y, z);

			if (abs(h2 - y) > CLICK_SIZE || h1 + HALF_BLOCK_SIZE >= y)
				can_roll = false;
			else
				can_roll = true;
		} else {
			can_roll = false;
			can_jump = false;
		}

		switch (item->current_anim_state) {
			case BADDY_STATE_IDLE:
				baddy->LOT.is_jumping = false;
				baddy->LOT.is_monkeying = false;
				baddy->flags = 0;
				baddy->maximum_turn = 0;
				head = info.angle >> 1;

				if (info.ahead && item->ai_bits != GUARD) {
					torso_y = info.angle >> 1;
					torso_x = info.x_angle;
				}

				if (item->ai_bits & GUARD) {
					head = AIGuard(baddy);
					item->goal_anim_state = BADDY_STATE_IDLE;
				} else if (item->meshswap_meshbits == 0x880 && item == lara.target && larainfo.ahead && larainfo.distance > 0x718E4) {
					item->goal_anim_state = BADDY_STATE_DODGE_START;
				} else if (Targetable(item, &info) && item->item_flags[2] > 0) {
					if (item->meshswap_meshbits == 0x7FC010)
						item->goal_anim_state = 31;
					else if (item->meshswap_meshbits == 0x7E0880 || item->meshswap_meshbits == 0x880)
						item->goal_anim_state = BADDY_STATE_HOLSTER_SWORD;
					else
						item->goal_anim_state = BADDY_STATE_DRAW_GUN;
				} else if (item->ai_bits == MODIFY) {
					item->goal_anim_state = BADDY_STATE_IDLE;

					if (item->floor > item->pos.y_pos + (HALF_BLOCK_SIZE + CLICK_SIZE))
						item->ai_bits &= ~MODIFY;
				} else if (jump_ahead || long_jump_ahead) {
					baddy->maximum_turn = 0;
					item->anim_number = objects[obj_num].anim_index + BADDY_STAND_TO_JUMP_FORWARD_ANIMATION;
					item->frame_number = anims[item->anim_number].frame_base;
					item->current_anim_state = BADDY_STATE_JUMP_FORWARD_1_BLOCK;

					if (long_jump_ahead)
						item->goal_anim_state = BADDY_STATE_JUMP_FORWARD_2_BLOCKS;
					else
						item->goal_anim_state = BADDY_STATE_JUMP_FORWARD_1_BLOCK;

					baddy->LOT.is_jumping = true;
				} else if (enemy && (enemy->object_number == SMALLMEDI_ITEM || enemy->object_number == UZI_AMMO_ITEM) && info.distance < 0x40000) {
					item->goal_anim_state = BADDY_STATE_STAND_TO_CROUCH;
					item->required_anim_state = BADDY_STATE_CROUCH_PICKUP;
				} else if (item->meshswap_meshbits == 0x7FC010 && item->item_flags[2] < 1) {
					item->goal_anim_state = BADDY_STATE_HOLSTER_GUN;
				} else if (baddy->monkey_ahead) {
					floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
					h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
					c = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

					if (c == h - 1536) {
						if (item->meshswap_meshbits == 0x7FC800)
							item->goal_anim_state = BADDY_STATE_MONKEY_GRAB;
						else if (item->meshswap_meshbits == 0x7FC010)
							item->goal_anim_state = BADDY_STATE_HOLSTER_GUN;
						else
							item->goal_anim_state = BADDY_STATE_HOLSTER_SWORD;
					} else {
						item->goal_anim_state = BADDY_STATE_WALK;
					}
				} else if (can_roll) {
					baddy->maximum_turn = 0;
					item->goal_anim_state = BADDY_STATE_ROLL_LEFT;
				} else if (can_jump) {
					baddy->maximum_turn = 0;
					item->goal_anim_state = BADDY_STATE_JUMP_RIGHT;
				} else if (item->meshswap_meshbits == 0x7FC800) {
					item->goal_anim_state = BADDY_STATE_DRAW_SWORD;
				} else if (enemy && enemy->hit_points > 0 && info.distance < 0x718E4) {
					if (item->meshswap_meshbits == 0x7FC010)
						item->goal_anim_state = BADDY_STATE_HOLSTER_GUN;
					else if (info.distance >= 0x40000)
						item->goal_anim_state = BADDY_STATE_SWORD_HIT_FRONT;
					else if (GetRandomControl() & 1)
						item->goal_anim_state = BADDY_STATE_SWORD_HIT_LEFT;
					else
						item->goal_anim_state = BADDY_STATE_SWORD_HIT_RIGHT;
				} else
					item->goal_anim_state = BADDY_STATE_WALK;

				break;
			case BADDY_STATE_WALK:
				baddy->LOT.is_jumping = false;
				baddy->LOT.is_monkeying = false;
				baddy->maximum_turn = DEGREES_TO_ROTATION(7);
				baddy->flags = 0;

				if (larainfo.ahead)
					head = larainfo.angle;
				else if (info.ahead)
					head = info.angle;

				if (Targetable(item, &info) && item->item_flags[2] > 0) {
					item->goal_anim_state = BADDY_STATE_IDLE;
				} else if (jump_ahead || long_jump_ahead) {
					baddy->maximum_turn = 0;
					item->goal_anim_state = BADDY_STATE_IDLE;
				} else if (baddy->reached_goal || baddy->monkey_ahead) {
					item->goal_anim_state = BADDY_STATE_IDLE;
				} else if (item->item_flags[2] >= 1 || item->meshswap_meshbits == 0x7E0880 || item->meshswap_meshbits == 0x880) {
					if (info.ahead && info.distance < 0x40000)
						item->goal_anim_state = BADDY_STATE_IDLE;
					else if (info.bite && info.distance < 0x718E4)
						item->goal_anim_state = BADDY_STATE_IDLE;
					else if (info.bite && info.distance < 0x100000)
						item->goal_anim_state = BADDY_STATE_WALK_SWORD_HIT_RIGHT;
					else if (can_roll || can_jump)
						item->goal_anim_state = BADDY_STATE_IDLE;
					else if (baddy->mood == ATTACK_MOOD && !baddy->jump_ahead && info.distance > 0x100000)
						item->goal_anim_state = BADDY_STATE_RUN;
				} else
					item->goal_anim_state = BADDY_STATE_IDLE;

				break;
			case BADDY_STATE_RUN:
				if (info.ahead)
					head = info.angle;

				baddy->maximum_turn = DEGREES_TO_ROTATION(11);
				tilt = angle / 2;

				if (item->object_number == BADDY_2 && item->frame_number == anims[item->anim_number].frame_base + 11 && farheight == nearheight &&
				        abs(nearheight - y) < (CLICK_SIZE + HALF_CLICK_SIZE) && (info.angle > -4096 && info.angle < 4096 && info.distance < 0x900000 || midheight >= nearheight + 512)) {
					item->goal_anim_state = BADDY_STATE_SOMERSAULT;
					baddy->maximum_turn = 0;
				} else if (Targetable(item, &info) && item->item_flags[2] > 0)
					item->goal_anim_state = BADDY_STATE_IDLE;
				else if (jump_ahead || long_jump_ahead || baddy->monkey_ahead || item->ai_bits == GUARD)
					item->goal_anim_state = BADDY_STATE_IDLE;
				else if (info.distance < 0x5C0A4 || baddy->jump_ahead)
					item->goal_anim_state = BADDY_STATE_IDLE;
				else if (info.distance < 0x100000)
					item->goal_anim_state = BADDY_STATE_WALK;

				break;
			case BADDY_STATE_DODGE:
				baddy->maximum_turn = 0;
				CreatureYRot(&item->pos, info.angle, DEGREES_TO_ROTATION(11));

				if (larainfo.distance < 0x718E4 || item != lara.target)
					item->goal_anim_state = BADDY_STATE_DODGE_END;

				break;
			case BADDY_STATE_DRAW_GUN:
				if (item->frame_number == anims[item->anim_number].frame_base + 21)
					item->meshswap_meshbits = 0x7FC010;

				break;
			case BADDY_STATE_HOLSTER_GUN:
				if (item->frame_number == anims[item->anim_number].frame_base + 20)
					item->meshswap_meshbits = 0x7FC800;

				break;
			case BADDY_STATE_DRAW_SWORD:
				if (item->frame_number == anims[item->anim_number].frame_base + 12) {
					if (item->object_number == BADDY_1)
						item->meshswap_meshbits = 0x7E0880;
					else
						item->meshswap_meshbits = 0x880;
				}

				break;
			case BADDY_STATE_HOLSTER_SWORD:
				if (item->frame_number == anims[item->anim_number].frame_base + 22)
					item->meshswap_meshbits = 0x7FC800;

				break;
			case BADDY_STATE_FIRE:
				if (info.ahead) {
					torso_y = info.angle;
					torso_x = info.x_angle;
				}

				CreatureYRot(&item->pos, info.angle, DEGREES_TO_ROTATION(7));

				if (item->frame_number < anims[item->anim_number].frame_base + 13 && !((item->frame_number - anims[item->anim_number].frame_base) & 1)) {
					item->fired_weapon = 1;

					if (!(item->ai_bits & MODIFY))
						item->item_flags[2]--;

					if (!ShotLara(item, &info, &baddy_fire, torso_y, mod_object_customization->damage_1))
						item->goal_anim_state = BADDY_STATE_IDLE;
				}

				break;
			case BADDY_STATE_SWORD_HIT_RIGHT:
				if (info.distance < 0x40000)
					item->goal_anim_state = BADDY_STATE_SWORD_HIT_LEFT;
				[[fallthrough]];
			case BADDY_STATE_SWORD_HIT_FRONT:
			case BADDY_STATE_SWORD_HIT_LEFT:
			case BADDY_STATE_WALK_SWORD_HIT_RIGHT:
				if (info.ahead) {
					torso_y = info.angle;
					torso_x = info.x_angle;
				}

				baddy->maximum_turn = 0;

				if (item->current_anim_state != BADDY_STATE_SWORD_HIT_FRONT || item->frame_number < anims[item->anim_number].frame_base + 12) {
					if (abs(info.angle) < DEGREES_TO_ROTATION(7))
						item->pos.y_rot += info.angle;
					else if (info.angle < 0)
						item->pos.y_rot -= DEGREES_TO_ROTATION(7);
					else
						item->pos.y_rot += DEGREES_TO_ROTATION(7);
				}

				if (!baddy->flags && item->touch_bits & 0x1C000) {
					if (item->frame_number > anims[item->anim_number].frame_base + 13 && item->frame_number < anims[item->anim_number].frame_base + 21) {
						lara_item->hit_points -= mod_object_customization->damage_2;
						lara_item->hit_status = 1;
						CreatureEffectT(item, &baddy_blade, 10, item->pos.y_rot, DoBloodSplat);
						baddy->flags = 1;
					}
				}

				if (item->frame_number == anims[item->anim_number].frame_end - 1) {
					baddy->flags = 0;
				}

				break;
			case BADDY_STATE_MONKEY_IDLE:
				torso_x = 0;
				torso_y = 0;
				baddy->maximum_turn = 0;
				baddy->flags = 0;
				state = lara_item->current_anim_state;

				if (larainfo.ahead && larainfo.distance < 0x718E4
				        && (state > AS_DASHDIVE && state < AS_ALL4S || state == AS_HANGTURNL || state == AS_HANGTURNR)) {
					item->goal_anim_state = BADDY_STATE_MONKEY_PUSH_OFF;
				} else if (item->box_number == baddy->LOT.target_box || !baddy->monkey_ahead) {
					floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
					h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
					c = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

					if (c == h - 1536) {
						item->goal_anim_state = BADDY_STATE_MONKEY_FALL_LAND;
						baddy->LOT.is_jumping = false;
						baddy->LOT.is_monkeying = false;
					} else {
						item->goal_anim_state = BADDY_STATE_MONKEY_FORWARD;
					}
				} else {
					item->goal_anim_state = BADDY_STATE_MONKEY_FORWARD;
				}

				break;
			case BADDY_STATE_MONKEY_FORWARD:
				torso_x = 0;
				torso_y = 0;
				baddy->LOT.is_jumping = true;
				baddy->LOT.is_monkeying = true;
				baddy->flags = 0;
				baddy->maximum_turn = DEGREES_TO_ROTATION(7);

				if (item->box_number == baddy->LOT.target_box || !baddy->monkey_ahead) {
					floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
					h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
					c = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

					if (c == h - 1536)
						item->goal_anim_state = BADDY_STATE_MONKEY_IDLE;
				}

				state = lara_item->current_anim_state;

				if (larainfo.ahead && larainfo.distance < 0x718E4 &&
				        (state > AS_DASHDIVE && state < AS_ALL4S || state == AS_HANGTURNL || state == AS_HANGTURNR))
					item->goal_anim_state = BADDY_STATE_MONKEY_IDLE;

				break;

			case BADDY_STATE_MONKEY_PUSH_OFF:
				baddy->maximum_turn = DEGREES_TO_ROTATION(7);

				if (!baddy->flags && item->touch_bits) {
					lara_item->anim_number = LARA_ANIM_STOPHANG;
					lara_item->frame_number = anims[LARA_ANIM_STOPHANG].frame_base + 9;
					lara_item->current_anim_state = AS_UPJUMP;
					lara_item->goal_anim_state = AS_UPJUMP;
					lara_item->gravity_status = 1;
					lara_item->speed = 2;
					lara_item->fallspeed = 1;
					lara_item->pos.y_pos += 192;
					lara.gun_status = LG_NO_ARMS;
					baddy->flags = 1;
				}

				break;
			case BADDY_STATE_ROLL_LEFT:
			case BADDY_STATE_JUMP_RIGHT:
				baddy->alerted = 0;
				baddy->maximum_turn = 0;
				item->ai_bits |= GUARD;
				break;

			case BADDY_STATE_CROUCH:

				if (item->item_flags[0]) {
					if (info.distance < 0x718E4) {
						item->goal_anim_state = BADDY_STATE_CROUCH_TO_STAND;
						baddy->enemy = 0;
					}
				} else if (enemy && (enemy->object_number == SMALLMEDI_ITEM || enemy->object_number == UZI_AMMO_ITEM) && info.distance < 0x40000)
					item->goal_anim_state = BADDY_STATE_CROUCH_PICKUP;
				else if (baddy->alerted)
					item->goal_anim_state = BADDY_STATE_CROUCH_TO_STAND;

				break;
			case BADDY_STATE_CROUCH_PICKUP:
				CreatureYRot(&item->pos, info.angle, DEGREES_TO_ROTATION(11));

				if (item->frame_number == anims[item->anim_number].frame_base + 9 && baddy->enemy) {
					if (baddy->enemy->object_number != SMALLMEDI_ITEM && baddy->enemy->object_number != UZI_AMMO_ITEM)
						break;

					if (baddy->enemy->room_number == 255 || baddy->enemy->status == ITEM_INVISIBLE || baddy->enemy->flags & IFL_CLEARBODY) {
						baddy->enemy = 0;
						break;
					}

					if (baddy->enemy->object_number == SMALLMEDI_ITEM) {
						item->hit_points += objects[item->object_number].hit_points >> 1;

						if (item->hit_points > objects[item->object_number].hit_points)
							item->hit_points = objects[item->object_number].hit_points;
					} else
						item->item_flags[2] += 24;

					KillItem(int16_t(baddy->enemy - items));

					for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
						if (baddie_slots[i].item_num != -1 && baddie_slots[i].item_num != item_number && baddie_slots[i].enemy == baddy->enemy)
							baddie_slots[i].enemy = 0;
					}

					baddy->enemy = 0;
				}

				break;
			case BADDY_STATE_SOMERSAULT:
				if (item->anim_number == objects[obj_num].anim_index + BADDY_SOMERSAULT_END_ANIMATION)
					CreatureYRot(&item->pos, info.angle, DEGREES_TO_ROTATION(7));
				else if (item->anim_number == objects[obj_num].anim_index + BADDY_STAND_IDLE_ANIMATION)
					baddy->LOT.is_jumping = true;

				break;
			case BADDY_STATE_AIM:
				baddy->maximum_turn = 0;

				if (info.ahead) {
					torso_y = info.angle;
					torso_x = info.x_angle;
				}

				CreatureYRot(&item->pos, info.angle, 1274);

				if (Targetable(item, &info) && item->item_flags[2] >= 1)
					item->goal_anim_state = BADDY_STATE_FIRE;
				else
					item->goal_anim_state = BADDY_STATE_IDLE;

				break;
			case BADDY_STATE_JUMP_FORWARD_1_BLOCK:
			case BADDY_STATE_JUMP_FORWARD_2_BLOCKS:
				if (item->item_flags[0] < 0 && item->anim_number != objects[obj_num].anim_index + BADDY_STAND_TO_JUMP_FORWARD_ANIMATION)
					item->item_flags[0] += 2;

				break;
			case BADDY_STATE_BLIND:

				if (!lara.blindTimer && !(GetRandomControl() & 0x7F))
					item->goal_anim_state = BADDY_STATE_IDLE;

				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (gfLevelFlags & GF_TRAIN && item->pos.y_pos > -CLICK_SIZE) {
		item->item_flags[0] = 0;
		item->hit_points = 0;
		item->pos.x_pos += gfUVRotate << 5;
	} else if (item->item_flags[0] < 0) {
		item->pos.x_pos += item->item_flags[0];
	}

	state = item->current_anim_state;

	if (state >= BADDY_STATE_JUMP_FORWARD_2_BLOCKS
	        || state == BADDY_STATE_JUMP_FORWARD_1_BLOCK
	        || state == BADDY_STATE_MONKEY_FORWARD
	        || state == BADDY_STATE_DEATH
	        || state == BADDY_STATE_SOMERSAULT) {
		CreatureAnimation(item_number, angle, 0);
	} else if (lara.blindTimer > 100) {
		baddy->maximum_turn = 0;
		item->anim_number = objects[obj_num].anim_index + BADDY_BLIND_ANIMATION;
		item->frame_number = anims[item->anim_number].frame_base + (GetRandomControl() & 7);
		item->current_anim_state = BADDY_STATE_BLIND;
	} else {
		switch (CreatureVault(item_number, angle, 2, 260)) {
			case -4:
				baddy->maximum_turn = 0;
				item->anim_number = objects[obj_num].anim_index + BADDY_JUMP_OFF_4_CLICKS_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_JUMP_OFF_4_STEPS;
				break;

			case -3:
				baddy->maximum_turn = 0;
				item->anim_number = objects[obj_num].anim_index + BADDY_JUMP_OFF_3_CLICKS_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_JUMP_OFF_3_STEPS;
				break;

			case 2:
				baddy->maximum_turn = 0;
				item->anim_number = objects[obj_num].anim_index + BADDY_CLIMB_2_CLICKS_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_CLIMB_2_STEPS;
				break;

			case 3:
				baddy->maximum_turn = 0;
				item->anim_number = objects[obj_num].anim_index + BADDY_CLIMB_3_CLICKS_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_CLIMB_3_STEPS;
				break;

			case 4:
				baddy->maximum_turn = 0;
				item->anim_number = objects[obj_num].anim_index + BADDY_CLIMB_4_CLICKS_ANIMATION;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = BADDY_STATE_CLIMB_4_STEPS;
				break;
		}
	}
}
