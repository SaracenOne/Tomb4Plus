#include "../../../tomb4/pch.h"
#include "skeleton.h"
#include "../../items.h"
#include "../../control.h"
#include "../../../specific/function_stubs.h"
#include "../../objects.h"
#include "../../effect2.h"
#include "../../../specific/3dmath.h"
#include "../../box.h"
#include "../../people.h"
#include "../../tomb4fx.h"
#include "../../lot.h"
#include "../../effects.h"
#include "../../sound.h"
#include "../../sphere.h"
#include "../../debris.h"
#include "../../lara.h"
#include "../../savegame.h"
#include "../../gameflow.h"

#include "../../trng/trng.h"
#include "../../../tomb4/mod_config.h"

static BITE_INFO skelly_hit = { 180, 0, 0, 16 };

void TriggerRiseEffect(ITEM_INFO* item) {
	FX_INFO* fx;
	FLOOR_INFO* floor;
	SPARKS* sptr;
	int16_t fx_number, room_number;

	fx_number = CreateEffect(item->room_number);

	if (fx_number == NO_ITEM)
		return;

	fx = &effects[fx_number];

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	fx->pos.x_pos = (GetRandomControl() & 0xFF) + item->pos.x_pos - 128;
	fx->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) - 48;
	fx->pos.z_pos = (GetRandomControl() & 0xFF) + item->pos.z_pos - 128;
	fx->room_number = item->room_number;
	fx->pos.y_rot = int16_t(GetRandomControl() << 1);
	fx->speed = int16_t(GetRandomControl() >> 11);
	fx->fallspeed = -int16_t(GetRandomControl() >> 10);
	fx->object_number = BODY_PART;
	fx->frame_number = objects[AHMET_MIP].mesh_index;
	fx->shade = 0x4210;
	fx->flag2 = 1537;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dR = 100;
	sptr->dG = 60;
	sptr->dB = 30;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 4;
	sptr->Life = (GetRandomControl() & 7) + 16;
	sptr->sLife = sptr->Life;
	sptr->x = fx->pos.x_pos;
	sptr->y = fx->pos.y_pos;
	sptr->z = fx->pos.z_pos;
	sptr->Xvel = phd_sin(fx->pos.y_rot) >> 2;
	sptr->Yvel = 0;
	sptr->Zvel = phd_cos(fx->pos.y_rot) >> 2;
	sptr->TransType = 2;
	sptr->Friction = 68;
	sptr->Flags = SF_ROTATE | SF_DEF | SF_SCALE;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -16 - (GetRandomControl() & 0xF);
	else
		sptr->RotAdd = (GetRandomControl() & 0xF) + 16;

	sptr->Gravity = -4 - (GetRandomControl() & 3);
	sptr->Scalar = 3;
	sptr->MaxYvel = -4 - (GetRandomControl() & 3);
	sptr->Size = (GetRandomControl() & 0xF) + 8;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size << 2;
}

void InitialiseSkeleton(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);

	if (!item->trigger_flags) {
		item->anim_number = objects[SKELETON].anim_index;
		item->current_anim_state = SKELETON_STATE_UNDERGROUND;
		item->goal_anim_state = SKELETON_STATE_UNDERGROUND;
	} else if (item->trigger_flags == 1) {
		item->anim_number = objects[SKELETON].anim_index + 37;
		item->current_anim_state = SKELETON_STATE_JUMP_RIGHT;
		item->goal_anim_state = SKELETON_STATE_JUMP_RIGHT;
	} else if (item->trigger_flags == 2) {
		item->anim_number = objects[SKELETON].anim_index + 34;
		item->current_anim_state = SKELETON_STATE_JUMP_LEFT;
		item->goal_anim_state = SKELETON_STATE_JUMP_LEFT;
	} else if (item->trigger_flags == 3) {
		item->anim_number = objects[SKELETON].anim_index + 46;
		item->current_anim_state = SKELETON_STATE_LAYING_DOWN;
		item->goal_anim_state = SKELETON_STATE_LAYING_DOWN;
		item->status += ITEM_ACTIVE;
	}

	item->frame_number = anims[item->anim_number].frame_base;
}

void SkeletonControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* enemy;
	CREATURE_INFO* skelly;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	MESH_INFO* mesh;
	AI_INFO info;
	AI_INFO larainfo;
	PHD_VECTOR pos;
	int32_t x, y, z, Xoffset, Zoffset, nearheight, midheight, farheight, jump_ahead, long_jump_ahead;
	int32_t dx, dz, jump_left, jump_right, h1, h2, h;
	int16_t angle, room_number, state;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, SKELETON);

	angle = 0;
	item = &items[item_number];
	skelly = (CREATURE_INFO*)item->data;

	Xoffset = 870 * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	Zoffset = 870 * phd_cos(item->pos.y_rot) >> W2V_SHIFT;

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

	jump_ahead = (item->box_number != lara_item->box_number || !(item->mesh_bits & 0x200)) &&
	             y < nearheight - (CLICK_SIZE + HALF_CLICK_SIZE) && y < midheight + CLICK_SIZE && y > midheight - CLICK_SIZE;

	long_jump_ahead = (item->box_number != lara_item->box_number || !(item->mesh_bits & 0x200)) &&
	                  y < nearheight - (CLICK_SIZE + HALF_CLICK_SIZE) && y < midheight - (CLICK_SIZE + HALF_CLICK_SIZE) && y < farheight + CLICK_SIZE && y > farheight - CLICK_SIZE;

	if (item->ai_bits)
		GetAITarget(skelly);
	else
		skelly->enemy = lara_item;

	CreatureAIInfo(item, &info);
	state = item->current_anim_state;

	if (item->hit_status && lara.gun_type == WEAPON_SHOTGUN && info.distance < 0xC40000 &&
	        state != 7 && state != 17 && state != 12 && state != 13 && state != 25) {
		if (info.angle >= 0x3000 || info.angle <= -0x3000) {
			item->anim_number = objects[SKELETON].anim_index + 33;
			item->current_anim_state = 13;
			item->pos.y_rot += info.angle + 0x8000;
		} else {
			item->anim_number = objects[SKELETON].anim_index + 17;
			item->current_anim_state = 12;
			item->pos.y_rot += info.angle;
		}

		item->frame_number = anims[item->anim_number].frame_base;
		skelly->LOT.is_jumping = true;
		item->hit_points = 25;
		CreatureAnimation(item_number, angle, 0);
		return;
	}

	if (skelly->enemy == lara_item) {
		larainfo.distance = info.distance;
		larainfo.angle = info.angle;
	} else {
		dx = lara_item->pos.x_pos - item->pos.x_pos;
		dz = lara_item->pos.z_pos - item->pos.z_pos;
		larainfo.angle = int16_t(phd_atan(dz, dx) - item->pos.y_rot);
		larainfo.distance = SQUARE(dx) + SQUARE(dz);
	}

	// T4Plus: initialize this
	larainfo.ahead = larainfo.angle > -FRONT_ARC && larainfo.angle < FRONT_ARC;
	GetCreatureMood(item, &info, true);

	if (!(item->mesh_bits & 0x200))
		skelly->mood = ESCAPE_MOOD;

	CreatureMood(item, &info, true);
	angle = CreatureTurn(item, skelly->maximum_turn);
	enemy = skelly->enemy;
	skelly->enemy = lara_item;

	if (item->hit_status || larainfo.distance < 0x100000 || TargetVisible(item, &larainfo))
		skelly->alerted = true;

	skelly->enemy = enemy;

	if (item == lara.target && larainfo.distance > 870 && larainfo.angle > -0x2800 && larainfo.angle < 0x2800) {
		room_number = item->room_number;
		x = item->pos.x_pos + (870 * phd_sin(item->pos.y_rot + 0x2000) >> W2V_SHIFT);
		z = item->pos.z_pos + (870 * phd_cos(item->pos.y_rot + 0x2000) >> W2V_SHIFT);
		floor = GetFloor(x, y, z, &room_number);
		h1 = GetHeight(floor, x, y, z);

		room_number = item->room_number;
		x = item->pos.x_pos + (870 * phd_sin(item->pos.y_rot + 0x3800) >> W2V_SHIFT);
		z = item->pos.z_pos + (870 * phd_cos(item->pos.y_rot + 0x3800) >> W2V_SHIFT);
		floor = GetFloor(x, y, z, &room_number);
		h2 = GetHeight(floor, x, y, z);
		jump_right = abs(h2 - y) <= CLICK_SIZE && h1 + HALF_BLOCK_SIZE < y;

		room_number = item->room_number;
		x = item->pos.x_pos + (870 * phd_sin(item->pos.y_rot - 0x2000) >> W2V_SHIFT);
		z = item->pos.z_pos + (870 * phd_cos(item->pos.y_rot - 0x2000) >> W2V_SHIFT);
		floor = GetFloor(x, y, z, &room_number);
		h1 = GetHeight(floor, x, y, z);

		room_number = item->room_number;
		x = item->pos.x_pos + (870 * phd_sin(item->pos.y_rot - 0x3800) >> W2V_SHIFT);
		z = item->pos.z_pos + (870 * phd_cos(item->pos.y_rot - 0x3800) >> W2V_SHIFT);
		floor = GetFloor(x, y, z, &room_number);
		h2 = GetHeight(floor, x, y, z);
		jump_left = abs(h2 - y) <= CLICK_SIZE && h1 + HALF_BLOCK_SIZE < y;
	} else {
		jump_left = 0;
		jump_right = 0;
	}

	switch (item->current_anim_state) {
		case SKELETON_STATE_UNDERGROUND:
			if (item->frame_number - anims[item->anim_number].frame_base < 32) {
				TriggerRiseEffect(item);
			}
			break;
		case SKELETON_STATE_WAIT:
			if (!(GetRandomControl() & 0xF)) {
				item->goal_anim_state = SKELETON_STATE_IDLE;
			}
			break;
		case SKELETON_STATE_IDLE:
			skelly->LOT.is_jumping = false;
			skelly->flags = 0;

			if (skelly->mood == BORED_MOOD) {
				skelly->maximum_turn = 0;
			} else {
				skelly->maximum_turn = DEGREES_TO_ROTATION(2);
			}

			if (!(item->ai_bits & GUARD) && (GetRandomControl() & 0x1F || info.distance <= 0x100000 && skelly->mood == ATTACK_MOOD)) {
				if (item->ai_bits & PATROL1) {
					item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
				} else if (jump_ahead || long_jump_ahead) {
					skelly->maximum_turn = 0;
					item->anim_number = objects[SKELETON].anim_index + 40;
					item->frame_number = anims[item->anim_number].frame_base;
					item->current_anim_state = 21;

					if (long_jump_ahead) {
						item->goal_anim_state = SKELETON_STATE_JUMP_FORWARD_2_BLOCKS;
					} else {
						item->goal_anim_state = SKELETON_STATE_JUMP_FORWARD_1_BLOCK;
					}

					skelly->LOT.is_jumping = true;
				} else if (jump_left) {
					item->anim_number = objects[SKELETON].anim_index + 34;
					item->frame_number = anims[item->anim_number].frame_base;
					item->current_anim_state = SKELETON_STATE_JUMP_LEFT;
					item->goal_anim_state = SKELETON_STATE_JUMP_LEFT;
				} else if (jump_right) {
					item->anim_number = objects[SKELETON].anim_index + 37;
					item->frame_number = anims[item->anim_number].frame_base;
					item->current_anim_state = SKELETON_STATE_JUMP_RIGHT;
					item->goal_anim_state = SKELETON_STATE_JUMP_RIGHT;
				} else if (skelly->mood == ESCAPE_MOOD) {
					if (lara.target == item || !info.ahead || item->hit_status || !(item->mesh_bits & 0x200)) {
						item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
					} else {
						item->goal_anim_state = SKELETON_STATE_IDLE;
					}
				} else if (skelly->mood == BORED_MOOD || item->ai_bits & FOLLOW && (skelly->reached_goal || larainfo.distance > 0x400000)) {
					if (item->required_anim_state) {
						item->goal_anim_state = item->required_anim_state;
					} else if (!(GetRandomControl() & 0x3F)) {
						item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
					}
				} else if (lara.target == item && larainfo.ahead && larainfo.distance < 0x400000 && GetRandomControl() & 1 &&
					(lara.gun_type == WEAPON_SHOTGUN || !(GetRandomControl() & 0xF)) && item->mesh_bits == -1) {
						item->goal_anim_state = SKELETON_STATE_USE_SHIELD;
				}
				else if (info.bite && info.distance < 0x718E4) {
					if (!(GetRandomControl() & 3) || lara_item->hit_points <= 0) {
						item->goal_anim_state = SKELETON_STATE_ATTACK_3;
					} else if (GetRandomControl() & 1) {
						item->goal_anim_state = SKELETON_STATE_ATTACK_1;
					} else {
						item->goal_anim_state = SKELETON_STATE_ATTACK_2;
					}
				} else if (item->hit_status || item->required_anim_state) {
					if (GetRandomControl() & 1) {
						item->goal_anim_state = SKELETON_STATE_DODGE_LEFT;
					} else {
						item->goal_anim_state = SKELETON_STATE_DODGE_RIGHT;
					}
					item->required_anim_state = item->goal_anim_state;
				} else {
					item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
				}
			} else if (!(GetRandomControl() & 0x3F)) {
				if (GetRandomControl() & 1)
					item->goal_anim_state = SKELETON_STATE_LOOK_LEFT;
				else
					item->goal_anim_state = SKELETON_STATE_LOOK_RIGHT;
			}
			break;

		case SKELETON_STATE_USE_SHIELD:
			if (item->hit_status) {
				if (item->mesh_bits != -1 || !larainfo.ahead || lara.gun_type != WEAPON_SHOTGUN)
					item->goal_anim_state = SKELETON_STATE_IDLE;
				else if (GetRandomControl() & 3)
					item->goal_anim_state = SKELETON_STATE_BLOCK_ATTACK;
				else
					ExplodeItemNode(item, 11, 1, -24);
			} else if (lara.target != item || item->mesh_bits != -1 || lara.gun_type != WEAPON_SHOTGUN || !(GetRandomControl() & 0x7F)) {
				item->goal_anim_state = SKELETON_STATE_IDLE;
			}
			break;
		case SKELETON_STATE_ATTACK_1:
		case SKELETON_STATE_ATTACK_2:
		case SKELETON_STATE_WALK_ATTACK:
			skelly->maximum_turn = 0;

			if (abs(info.angle) < DEGREES_TO_ROTATION(6)) {
				item->pos.y_rot += info.angle;
			} else if (info.angle < 0) {
				item->pos.y_rot -= DEGREES_TO_ROTATION(6);
			} else {
				item->pos.y_rot += DEGREES_TO_ROTATION(6);
			}

			if (item->frame_number > anims[item->anim_number].frame_base + 15) {
				r = &room[item->room_number];
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				GetJointAbsPosition(item, &pos, 16);

				floor = &r->floor[((pos.z - r->z) >> 10) + r->x_size * ((pos.x - r->x) >> 10)];

				if (floor->stopper) {
					for (int32_t i = 0; i < r->num_meshes; i++) {
						mesh = &r->mesh[i];

						MOD_LEVEL_STATIC_INFO* static_info = &get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number];
						if (mesh->z >> WALL_SHIFT == pos.z >> WALL_SHIFT && mesh->x >> WALL_SHIFT == pos.x >> WALL_SHIFT && static_info->creatures_can_shatter) {
							ShatterObject(0, mesh, -64, lara_item->room_number, 0);
							if (static_info->shatter_sound_id >= 0) {
								SoundEffect(static_info->shatter_sound_id, &item->pos, SFX_DEFAULT);
							}
							mesh->Flags &= ~1;
							floor->stopper = 0;
							GetHeight(floor, pos.x, pos.y, pos.z);
							TestTriggers(trigger_index, true, 0);
						}
					}
				}

				if (!skelly->flags && item->touch_bits & 0x18000) {
					lara_item->hit_points -= mod_object_customization->damage_1;
					lara_item->hit_status = 1;
					CreatureEffectT(item, &skelly_hit, 10, item->pos.y_rot, DoBloodSplat);
					SoundEffect(SFX_LARA_THUD, &item->pos, SFX_DEFAULT);
					skelly->flags = 1;
				}
			}
			break;
		case SKELETON_STATE_ATTACK_3:
			skelly->maximum_turn = 0;

			if (abs(info.angle) < DEGREES_TO_ROTATION(6)) {
				item->pos.y_rot += info.angle;
			} else if (info.angle < 0) {
				item->pos.y_rot -= DEGREES_TO_ROTATION(6);
			} else {
				item->pos.y_rot += DEGREES_TO_ROTATION(6);
			}

			if (!skelly->flags && item->touch_bits & 0x18000) {
				lara_item->hit_points -= mod_object_customization->damage_1;
				lara_item->hit_status = 1;
				CreatureEffectT(item, &skelly_hit, 15, -1, DoBloodSplat);
				SoundEffect(SFX_LARA_THUD, &item->pos, SFX_DEFAULT);
				skelly->flags = 1;
			}

			if (!(GetRandomControl() & 0x3F) || lara_item->hit_points <= 0) {
				item->goal_anim_state = SKELETON_STATE_STUCK_SWORD;
			}
			break;
		case SKELETON_STATE_STUCK_SWORD:
			skelly->maximum_turn = 0;
			break;
		case SKELETON_STATE_RECOIL_FRONT:
		case SKELETON_STATE_RECOIL_BACK:
			if (item->frame_number < anims[item->anim_number].frame_base + 20) {
				item->hit_points = 25;
				skelly->maximum_turn = 0;
				break;
			}

		case SKELETON_STATE_LAYING_DOWN:
			item->hit_points = 25;
			skelly->LOT.is_jumping = false;
			skelly->maximum_turn = 0;

			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (h > item->pos.y_pos + BLOCK_SIZE) {
				skelly->maximum_turn = 0;
				item->anim_number = objects[SKELETON].anim_index + 47;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = 24;
				item->gravity_status = 1;
			} else if (!(GetRandomControl() & 0x1F)) {
				item->goal_anim_state = SKELETON_STATE_STAND_UP;
			}
			break;
		case SKELETON_STATE_WALK_FORWARD:
			skelly->LOT.is_jumping = false;
			skelly->flags = 0;

			if (skelly->mood == BORED_MOOD)
				skelly->maximum_turn = DEGREES_TO_ROTATION(2);
			else
				skelly->maximum_turn = DEGREES_TO_ROTATION(6);

			if (item->ai_bits & PATROL1) {
				item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
			} else if (item->hit_status) {
				item->goal_anim_state = SKELETON_STATE_IDLE;

				if (GetRandomControl() & 1) {
					item->required_anim_state = SKELETON_STATE_DODGE_LEFT;
				} else {
					item->required_anim_state = SKELETON_STATE_DODGE_RIGHT;
				}
			} else if (jump_left || jump_right) {
				item->goal_anim_state = SKELETON_STATE_IDLE;
			} else if (skelly->mood == ESCAPE_MOOD) {
				item->goal_anim_state = SKELETON_STATE_RUN_FORWARD;
			} else if (skelly->mood == BORED_MOOD) {
				if (!(GetRandomControl() & 0x3F)) {
					item->goal_anim_state = SKELETON_STATE_IDLE;
				}
			} else if (info.distance < 0x718E4) {
				item->goal_anim_state = SKELETON_STATE_IDLE;
			} else if (info.bite && info.distance < 0x100000) {
				item->goal_anim_state = SKELETON_STATE_WALK_ATTACK;
			} else if (jump_ahead || long_jump_ahead) {
				skelly->maximum_turn = 0;
				item->goal_anim_state = SKELETON_STATE_IDLE;
			} else if (!info.ahead || info.distance > 0x400000)
				item->goal_anim_state = SKELETON_STATE_RUN_FORWARD;
			break;
		case SKELETON_STATE_RUN_FORWARD:
			skelly->maximum_turn = DEGREES_TO_ROTATION(7);
			skelly->LOT.is_jumping = false;

			if (item->ai_bits & GUARD || jump_ahead || long_jump_ahead) {
				if (item->mesh_bits & 0x200) {
					skelly->maximum_turn = 0;
					item->goal_anim_state = SKELETON_STATE_IDLE;
				} else {
					skelly->LOT.is_jumping = true;
					floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
					h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

					if (h > item->pos.y_pos + BLOCK_SIZE) {
						skelly->maximum_turn = 0;
						item->anim_number = objects[SKELETON].anim_index + 44;
						item->frame_number = anims[item->anim_number].frame_base;
						item->current_anim_state = 23;
						skelly->LOT.is_jumping = false;
						item->gravity_status = 1;
					}
				}
			} else if (skelly->mood == ESCAPE_MOOD) {
				if (lara.target != item && info.ahead && item->mesh_bits & 0x200)
					item->goal_anim_state = SKELETON_STATE_IDLE;
			} else if (item->ai_bits & FOLLOW && (skelly->reached_goal || larainfo.distance > 0x400000)) {
				item->goal_anim_state = SKELETON_STATE_IDLE;
			} else if (skelly->mood == BORED_MOOD) {
				item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
			} else if (info.ahead && info.distance < 0x400000) {
				item->goal_anim_state = SKELETON_STATE_WALK_FORWARD;
			}
			break;
		case SKELETON_STATE_JUMP_LEFT:
		case SKELETON_STATE_JUMP_RIGHT:
			skelly->alerted = 0;
			skelly->maximum_turn = 0;
			item->ai_bits |= GUARD;
			break;
		case SKELETON_STATE_JUMP_FORWARD_1_BLOCK:
			if (item->anim_number == objects[SKELETON].anim_index + 43) {
				room_number = item->room_number;
				floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
				h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

				if (h > item->pos.y_pos + 1280) {
					skelly->maximum_turn = 0;
					item->anim_number = objects[SKELETON].anim_index + 44;
					item->frame_number = anims[item->anim_number].frame_base;
					item->current_anim_state = 23;
					skelly->LOT.is_jumping = false;
					item->gravity_status = 1;
				}
			}
			break;
		case SKELETON_STATE_JUMP_CONTINUE:
		case SKELETON_STATE_JUMP_START:
			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (h <= item->pos.y_pos && item->active) {
				ExplodingDeath2(item_number, -1, 929);
				KillItem(item_number);
				DisableBaddieAI(item_number);
				savegame.Level.Kills++;
			}

			break;
	}

	CreatureAnimation(item_number, angle, 0);
}
