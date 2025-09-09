#include "../../../tomb4/pch.h"
#include "guide.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../sphere.h"
#include "../../tomb4fx.h"
#include "../../sound.h"
#include "../../effect2.h"
#include "../../../specific/3dmath.h"
#include "../../switch.h"
#include "../../items.h"
#include "../../lot.h"
#include "../../effects.h"
#include "../../lara.h"
#include "../../control.h"

static BITE_INFO guide_hit = { 0, 20, 200, 18 };
static BITE_INFO guide_lighter = { 30, 80, 50, 15 };

void InitialiseGuide(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[GUIDE].anim_index + GUIDE_STAND_ANIMATION;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;

	if (!objects[WRAITH1].loaded)
		item->meshswap_meshbits = 0x40000;
	else {
		item->meshswap_meshbits = 0;
		item->item_flags[1] = 2;
	}
}

void GuideControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* target;
	ITEM_INFO* enemy;
	CREATURE_INFO* guide;
	PHD_VECTOR pos;
	AI_INFO info;
	int32_t rnd, r, g, b, x, y, z, iDistance, dist, bestdist;
	int16_t tilt, head, torso_x, torso_y, iAngle, xAngle, angle, dy, frame;
	bool iAhead, got_torch;

	if (!CreatureActive(item_number)) {
		return;
	}

	item = &items[item_number];

	guide = (CREATURE_INFO*)item->data;
	got_torch = false; //JUST grabbed the torch
	tilt = 0;
	head = 0;
	torso_x = 0;
	torso_y = 0;

	if (item->item_flags[1] == 2) {
		rnd = GetRandomControl();
		r = 255 - ((rnd >> 4) & 0x1F);
		g = 192 - ((rnd >> 6) & 0x1F);
		b = rnd & 0x3F;
		pos.x = guide_hit.x;
		pos.y = guide_hit.y;
		pos.z = guide_hit.z;
		GetJointAbsPosition(item, &pos, guide_hit.mesh_num);
		AddFire(pos.x, pos.y, pos.z, 0, item->room_number, 0);
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);
		TriggerFireFlame(pos.x, pos.y - 40, pos.z, -1, 7);
		TriggerDynamic(pos.x, pos.y, pos.z, 15, r, g, b);

		if (item->anim_number == objects[GUIDE].anim_index + GUIDE_READ_INSCRIPTION_ANIMATION) {
			if (item->frame_number > anims[item->anim_number].frame_base + 32 && item->frame_number < anims[item->anim_number].frame_base + 42) {
				x = (rnd & 0x3F) + pos.x - 32;
				y = ((rnd >> 3) & 0x3F) + pos.y - 128;
				z = pos.z + ((rnd >> 6) & 0x3F) - 32;
				TriggerFireFlame(x, y, z, -1, 1);
			}
		}
	}

	item->ai_bits = FOLLOW;
	GetAITarget(guide);
	x = lara_item->pos.x_pos - item->pos.x_pos;
	z = lara_item->pos.z_pos - item->pos.z_pos;
	iAngle = int16_t(phd_atan(z, x) - item->pos.y_rot);

	if (iAngle > -FRONT_ARC && iAngle < FRONT_ARC) {
		iAhead = true;
	} else {
		iAhead = false;
	}

	if (z > 32000 || z < -32000 || x > 32000 || x < -32000) {
		iDistance = 0x7FFFFFFF;
	} else {
		iDistance = SQUARE(x) + SQUARE(z);
	}

	x = abs(x);
	z = abs(z);

	if (x > z) {
		xAngle = (int16_t)phd_atan(x + (z >> 1), item->pos.y_pos - lara_item->pos.y_pos);
	} else {
		xAngle = (int16_t)phd_atan(z + (x >> 1), item->pos.y_pos - lara_item->pos.y_pos);
	}

	target = 0;
	bestdist = 0x7FFFFFFF;

	if (!objects[WRAITH1].loaded && (item->current_anim_state <= GUIDE_STATE_RUN || item->current_anim_state == GUIDE_STATE_ATTACK_LOW)) {
		for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
			if (baddie_slots[i].item_num != NO_ITEM && baddie_slots[i].item_num != item_number) {
				ITEM_INFO* candidate = &items[baddie_slots[i].item_num];

				if (candidate->object_number != GUIDE && abs(candidate->pos.y_pos - item->pos.y_pos) <= HALF_BLOCK_SIZE) {
					x = candidate->pos.x_pos - item->pos.x_pos;
					z = candidate->pos.z_pos - item->pos.z_pos;

					if (z > 32000 || z < -32000 || x > 32000 || x < -32000) {
						dist = 0x7FFFFFFF;
					} else {
						dist = SQUARE(x) + SQUARE(z);
					}

					if (dist < bestdist
						&& dist < 0x400000
						&& (abs(item->pos.y_pos - candidate->pos.y_pos) < 256 || iDistance < 0x400000 || candidate->object_number == DOG)) {
						target = candidate;
						bestdist = dist;
					}
				}
			}
		}
	}

	enemy = guide->enemy;

	if (target) {
		guide->enemy = target;
	}

	CreatureAIInfo(item, &info);
	GetCreatureMood(item, &info, true);
	CreatureMood(item, &info, true);

	angle = CreatureTurn(item, guide->maximum_turn);

	if (target) {
		guide->enemy = enemy;
		enemy = target;
	}

	switch (item->current_anim_state) {
		case GUIDE_STATE_STOP:
			guide->LOT.is_jumping = false;
			guide->flags = 0;
			guide->maximum_turn = 0;
			head = info.angle >> 1;

			if (iAhead) {
				torso_x = xAngle >> 1;
				torso_y = iAngle >> 1;
				head = iAngle >> 1;
			} else if (info.ahead) {
				torso_x = info.x_angle >> 1;
				torso_y = info.angle >> 1;
				head = info.angle >> 1;
			}

			if (objects[WRAITH1].loaded) {
				if (item->item_flags[3] == 5 || item->item_flags[3] == 6) {
					if (item->item_flags[3] == 5)
						item->goal_anim_state = GUIDE_STATE_WALK;

					break;
				}
			}

			if (item->required_anim_state) {
				item->goal_anim_state = item->required_anim_state;
			} else if (lara.location < item->item_flags[3] && item->item_flags[1] == 2) {
				item->goal_anim_state = GUIDE_STATE_STOP;
			} else if (!guide->reached_goal || target) {
				if (item->meshswap_meshbits == 0x40000) {
					item->goal_anim_state = GUIDE_STATE_STAND_TO_WALK;
				} else if (target && info.distance < 0x100000) {
					if (info.bite) {
						item->goal_anim_state = GUIDE_STATE_ATTACK_LOW;
					}
				} else if (enemy != lara_item || info.distance > 0x400000) {
					item->goal_anim_state = GUIDE_STATE_WALK;
				}
			} else if (!enemy->flags) {
				guide->reached_goal = false;
				guide->enemy = NULL;
				item->ai_bits = FOLLOW;
				item->item_flags[3]++;
			} else if (info.distance > 0x4000) {
				guide->maximum_turn = 0;

				if (info.ahead) {
					item->required_anim_state = GUIDE_STATE_CORRECT_POSITION_FRONT;
				} else {
					item->required_anim_state = GUIDE_STATE_CORRECT_POSITION_BACK;
				}
			} else {
				switch (enemy->flags) {
					case 2:
						item->goal_anim_state = GUIDE_STATE_READ_INSCRIPTION_SHORT;
						item->required_anim_state = GUIDE_STATE_READ_INSCRIPTION_SHORT;
						break;
					case 32:
						item->goal_anim_state = GUIDE_STATE_GRAB_TORCH;
						item->required_anim_state = GUIDE_STATE_GRAB_TORCH;
						break;
					case 40:
						if (iDistance < 0x400000) {
							item->goal_anim_state = GUIDE_STATE_READ_INSCRIPTION_SLOW_SCARY;
							item->required_anim_state = GUIDE_STATE_READ_INSCRIPTION_SLOW_SCARY;
						}
						break;
					case 16:
						if (iDistance < 0x400000) {
							item->goal_anim_state = GUIDE_STATE_CROUCH;
							item->required_anim_state = GUIDE_STATE_CROUCH;
						}
						break;
					case 4:
						if (iDistance < 0x400000) {
							item->goal_anim_state = GUIDE_STATE_CROUCH;
							item->required_anim_state = GUIDE_STATE_CROUCH;
						}
						break;
					case 62:
						item->status = ITEM_INVISIBLE;
						RemoveActiveItem(item_number);
						DisableBaddieAI(item_number);
						break;
				}
			}

			break;

		case GUIDE_STATE_WALK:
			guide->LOT.is_jumping = false;
			guide->maximum_turn = DEGREES_TO_ROTATION(7);

			if (iAhead) {
				head = iAngle;
			} else if (info.ahead) {
				head = info.angle;
			}

			if (objects[WRAITH1].loaded && item->item_flags[3] == 5) {
				item->item_flags[3] = 6;
				item->goal_anim_state = GUIDE_STATE_STOP;
			} else if (item->item_flags[1] == 1) {
				item->goal_anim_state = GUIDE_STATE_STOP;
				item->required_anim_state = GUIDE_STATE_USE_LIGHTER;
			} else if (guide->reached_goal) {
				if (!enemy->flags) {
					guide->reached_goal = false;
					guide->enemy = NULL;
					item->ai_bits = FOLLOW;
					item->item_flags[3]++;
				} else {
					item->goal_anim_state = GUIDE_STATE_STOP;
				}
			} else if (lara.location < item->item_flags[3]) {
				item->goal_anim_state = GUIDE_STATE_STOP;
			} else if (!target || info.distance >= 0x200000 && (item->meshswap_meshbits & 0x40000 || info.distance >= 0x900000)) {
				if (enemy == lara_item) {
					if (info.distance < 0x400000) {
						item->goal_anim_state = GUIDE_STATE_STOP;
					} else if (info.distance > 0x1000000) {
						item->goal_anim_state = GUIDE_STATE_RUN;
					}
				} else if (lara.location > item->item_flags[3] && iDistance > 0x400000) {
					item->goal_anim_state = GUIDE_STATE_RUN;
				}
			} else {
				item->goal_anim_state = GUIDE_STATE_STOP;
			}

			break;

		case GUIDE_STATE_RUN:
			if (info.ahead) {
				head = info.angle;
			}

			guide->maximum_turn = DEGREES_TO_ROTATION(11);
			tilt = angle / 2;

			if (info.distance < 0x400000 || lara.location < item->item_flags[3]) {
				item->goal_anim_state = GUIDE_STATE_STOP;
			} else if (guide->reached_goal) {
				if (!enemy->flags) {
					guide->reached_goal = false;
					guide->enemy = NULL;
					item->ai_bits = FOLLOW;
					item->item_flags[3]++;
				} else {
					item->goal_anim_state = GUIDE_STATE_STOP;
				}
			} else if (target && !(item->meshswap_meshbits & 0x40000) && info.distance < 0x900000) {
				item->goal_anim_state = GUIDE_STATE_STOP;
			}

			break;
		case GUIDE_STATE_USE_LIGHTER:
			rnd = GetRandomControl();
			pos.x = guide_lighter.x;
			pos.y = guide_lighter.y;
			pos.z = guide_lighter.z;
			GetJointAbsPosition(item, &pos, guide_lighter.mesh_num);
			frame = item->frame_number - anims[item->anim_number].frame_base;

			if (frame == 32) {
				item->meshswap_meshbits |= 0x8000;
			} else if (frame == 216) {
				item->meshswap_meshbits &= ~0x8000;
			} else if (frame > 79 && frame < 84) {
				r = rnd & 0x1F;
				g = 96 - ((rnd >> 6) & 0x1F);
				b = 128 - ((rnd >> 4) & 0x1F);
				TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);
				TriggerFlareSparks(pos.x, pos.y, pos.z, -1, -1, 0, 1);
			} else if (frame > 83 && frame < 94) {
				r = 192 - ((rnd >> 4) & 0x1F);
				g = 128 - ((rnd >> 6) & 0x1F);
				b = rnd & 0x1F;
				TriggerDynamic(pos.x - (QUARTER_CLICK_SIZE / 2), pos.y - QUARTER_CLICK_SIZE, pos.z - (QUARTER_CLICK_SIZE / 2), 10, r, g, b);

				x = (rnd & 0x3F) + pos.x - QUARTER_CLICK_SIZE;
				y = ((rnd >> 5) & 0x3F) + pos.y - (QUARTER_CLICK_SIZE + (QUARTER_CLICK_SIZE / 2));
				z = ((rnd >> 10) & 0x3F) + pos.z - QUARTER_CLICK_SIZE;
				TriggerFireFlame(x, y, z, -1, 7);
			} else if (frame > 159 && frame < 164) {
				r = rnd & 0x1F;
				g = 96 - ((rnd >> 6) & 0x1F);
				b = 128 - ((rnd >> 4) & 0x1F);
				TriggerFlareSparks(pos.x, pos.y, pos.z, -1, -1, 0, 1);
				TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);
			} else if (frame > 163 && frame < 181) {
				x = (rnd & 0x3F) + pos.x - QUARTER_CLICK_SIZE;
				y = ((rnd >> 5) & 0x3F) + pos.y - (QUARTER_CLICK_SIZE + (QUARTER_CLICK_SIZE / 2));
				z = ((rnd >> 10) & 0x3F) + pos.z - QUARTER_CLICK_SIZE;
				TriggerFireFlame(x, y, z, -1, 7);

				r = 192 - ((rnd >> 4) & 0x1F);
				g = 128 - ((rnd >> 6) & 0x1F);
				b = rnd & 0x1F;
				TriggerDynamic(pos.x - 32, pos.y - QUARTER_CLICK_SIZE, pos.z - (QUARTER_CLICK_SIZE / 2), 10, r, g, b);
				item->item_flags[1] = 2;
			}

			break;
		case GUIDE_STATE_TURN_LEFT:
			guide->maximum_turn = 0;

			if (iAngle < -256) {
				item->pos.y_rot -= 399;
			}

			break;
		case GUIDE_STATE_ATTACK_LOW:
			if (info.ahead) {
				torso_x = info.x_angle >> 1;
				torso_y = info.angle >> 1;
				head = info.angle >> 1;
			}

			guide->maximum_turn = 0;

			if (abs(info.angle) < DEGREES_TO_ROTATION(7)) {
				item->pos.y_rot += info.angle;
			} else if (info.angle < 0) {
				item->pos.y_rot -= DEGREES_TO_ROTATION(7);
			} else {
				item->pos.y_rot += DEGREES_TO_ROTATION(7);
			}

			if (guide->flags || !enemy) {
				break;
			}

			if (item->frame_number > anims[item->anim_number].frame_base + 15 && item->frame_number < anims[item->anim_number].frame_base + 26) {
				x = abs(enemy->pos.x_pos - item->pos.x_pos);
				y = abs(enemy->pos.y_pos - item->pos.y_pos);
				z = abs(enemy->pos.z_pos - item->pos.z_pos);

				if (x < HALF_BLOCK_SIZE && y <= HALF_BLOCK_SIZE && z < HALF_BLOCK_SIZE) {
					enemy->hit_points -= 20;

					if (enemy->hit_points <= 0)
						item->ai_bits = FOLLOW;

					enemy->hit_status = 1;
					guide->flags = 1;
					CreatureEffectT(item, &guide_hit, 8, -1, DoBloodSplat);
				}
			}

			break;
		case GUIDE_STATE_TURN_RIGHT:
			guide->maximum_turn = 0;

			if (iAngle > 256) {
				item->pos.y_rot += 399;
			}

			break;
		case GUIDE_STATE_CROUCH:
		case GUIDE_STATE_CROUCH_TORCH_ACTIVATE:
			if (enemy) {
				dy = enemy->pos.y_rot - item->pos.y_rot;

				if (dy > DEGREES_TO_ROTATION(2)) {
					item->pos.y_rot += DEGREES_TO_ROTATION(2);
				} else if (dy < -DEGREES_TO_ROTATION(2)) {
					item->pos.y_rot -= DEGREES_TO_ROTATION(2);
				}
			}

			if (item->required_anim_state == GUIDE_STATE_CROUCH_TORCH_ACTIVATE) {
				item->goal_anim_state = GUIDE_STATE_CROUCH_TORCH_ACTIVATE;
			} else if (item->anim_number != objects[GUIDE].anim_index + GUIDE_STAND_TO_CROUCH_ANIMATION && item->frame_number == anims[item->anim_number].frame_end - 20) {
				item->goal_anim_state = GUIDE_STATE_STOP;
				TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, 0);
				guide->reached_goal = false;
				guide->enemy = NULL;
				item->ai_bits = FOLLOW;
				item->item_flags[3]++;
			}

			break;

		case GUIDE_STATE_GRAB_TORCH:
			if (item->frame_number == anims[item->anim_number].frame_base) {
				got_torch = true;
				item->pos.x_pos = enemy->pos.x_pos;
				item->pos.y_pos = enemy->pos.y_pos;
				item->pos.z_pos = enemy->pos.z_pos;
				item->pos.x_rot = enemy->pos.x_rot;
				item->pos.y_rot = enemy->pos.y_rot;
				item->pos.z_rot = enemy->pos.z_rot;
			} else if (item->frame_number == anims[item->anim_number].frame_base + 35) {
				item->meshswap_meshbits &= ~0x40000;

				ITEM_INFO *candidate = NULL;
				int16_t candidate_num = 0;
				for (candidate_num = room[item->room_number].item_number; candidate_num != NO_ITEM; candidate_num = candidate->next_item) {
					candidate = &items[candidate_num];

					if (candidate->object_number >= ANIMATING1 && candidate->object_number <= ANIMATING15 &&
					        !((item->pos.z_pos ^ candidate->pos.z_pos) & ~(BLOCK_SIZE - 1)) && !((item->pos.x_pos ^ candidate->pos.x_pos) & ~(BLOCK_SIZE - 1))) {
						candidate->mesh_bits = 0xFFFFFFFD;
						break;
					}
				}
			}

			item->item_flags[1] = 1;

			if (got_torch) {
				guide->reached_goal = false;
				guide->enemy = NULL;
				item->ai_bits = FOLLOW;
				item->item_flags[3]++;
			}

			break;
		case GUIDE_STATE_READ_INSCRIPTION_SHORT:
			if (item->frame_number == anims[item->anim_number].frame_base) {
				item->pos.x_pos = enemy->pos.x_pos;
				item->pos.y_pos = enemy->pos.y_pos;
				item->pos.z_pos = enemy->pos.z_pos;
			} else if (item->frame_number == anims[item->anim_number].frame_base + 42) {
				TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, 0);
				item->pos.y_rot = enemy->pos.y_rot;
				guide->reached_goal = false;
				guide->enemy = NULL;
				item->ai_bits = FOLLOW;
				item->item_flags[3]++;
			} else if (item->frame_number < anims[item->anim_number].frame_base + 42) {
				dy = enemy->pos.y_rot - item->pos.y_rot;

				if (dy > DEGREES_TO_ROTATION(2)) {
					item->pos.y_rot += DEGREES_TO_ROTATION(2);
				} else if (dy < -DEGREES_TO_ROTATION(2)) {
					item->pos.y_rot -= DEGREES_TO_ROTATION(2);
				}
			}

			break;
		case GUIDE_STATE_READ_INSCRIPTION_SLOW_SCARY:
			if (item->frame_number < anims[item->anim_number].frame_base + 20) {
				dy = enemy->pos.y_rot - item->pos.y_rot;

				if (dy > DEGREES_TO_ROTATION(2)) {
					item->pos.y_rot += DEGREES_TO_ROTATION(2);
				} else if (dy < -DEGREES_TO_ROTATION(2)) {
					item->pos.y_rot -= DEGREES_TO_ROTATION(2);
				}
			} else if (item->frame_number == anims[item->anim_number].frame_base + 20) {
				item->goal_anim_state = GUIDE_STATE_STOP;
				TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, 0);
				guide->reached_goal = false;
				guide->enemy = NULL;
				item->ai_bits = FOLLOW;
				item->item_flags[3]++;
			} else if (item->frame_number == anims[item->anim_number].frame_base + 70 && item->room_number == 70) {
				item->required_anim_state = GUIDE_STATE_RUN;
				item->meshswap_meshbits |= 0x200000;
				SoundEffect(SFX_GUIDE_SCARE, &item->pos, SFX_DEFAULT);
			}

			break;
		case GUIDE_STATE_STAND_TO_WALK:
			guide->LOT.is_jumping = false;
			guide->maximum_turn = DEGREES_TO_ROTATION(7);

			if (iAhead) {
				head = iAngle;
			} else if (info.ahead) {
				head = info.angle;
			}

			if (guide->reached_goal) {
				if (!enemy->flags) {
					guide->reached_goal = false;
					guide->enemy = NULL;
					item->ai_bits = FOLLOW;
					item->item_flags[3]++;
					break;
				}

				if (enemy->flags == 42) {
					TestTriggersAtXYZ(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, enemy->room_number, 1, 0);
					guide->reached_goal = false;
					guide->enemy = NULL;
					item->ai_bits = FOLLOW;
					item->item_flags[3]++;
				} else if (item->trigger_flags <= 999) {
					item->goal_anim_state = GUIDE_STATE_STOP;
				} else {
					KillItem(item_number);
					DisableBaddieAI(item_number);
					item->flags |= IFL_INVISIBLE;
				}
			}

			break;

		case GUIDE_STATE_CORRECT_POSITION_FRONT:
		case GUIDE_STATE_CORRECT_POSITION_BACK:
			guide->maximum_turn = 0;
			MoveCreature3DPos(&item->pos, &enemy->pos, 15, enemy->pos.y_rot - item->pos.y_rot, DEGREES_TO_ROTATION(10));
			break;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);
	CreatureJoint(item, 3, torso_x);
	CreatureAnimation(item_number, angle, 0);
}
