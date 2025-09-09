#include "../tomb4/pch.h"
#include "box.h"
#include "tomb4fx.h"
#include "items.h"
#include "lot.h"
#include "objects.h"
#include "control.h"
#include "draw.h"
#include "../specific/3dmath.h"
#include "lara_states.h"
#include "../specific/function_stubs.h"
#include "sphere.h"
#include "camera.h"
#include "effect2.h"
#include "lara.h"
#include "../specific/file.h"
#include "../tomb4/tomb4plus/t4plus_environment.h"

BOX_INFO* boxes;
uint16_t* overlap;
int16_t* ground_zone[5][2];
int32_t num_boxes;

void CreatureDie(int16_t item_number, bool explode) {
	ITEM_INFO* item;
	ITEM_INFO* pickup;
	int16_t pickup_number, room_number;

	item = &items[item_number];
	item->hit_points = INFINITE_HEALTH;
	item->collidable = 0;

	if (explode) {
		if (objects[item->object_number].HitEffect == 1)
			ExplodingDeath2(item_number, -1, CLICK_SIZE + 2);
		else
			ExplodingDeath2(item_number, -1, CLICK_SIZE);

		KillItem(item_number);
	} else
		RemoveActiveItem(item_number);

	DisableBaddieAI(item_number);
	item->flags |= IFL_INVISIBLE | IFL_CLEARBODY;

	//inlined DropBaddyPickups..

	pickup_number = item->carried_item;

	while (pickup_number != NO_ITEM) {
		pickup = &items[pickup_number];

		if (item->object_number == TROOPS && item->trigger_flags == 1) {
			pickup->pos.x_pos = ((item->pos.x_pos + ((BLOCK_SIZE * phd_sin(item->pos.y_rot)) >> W2V_SHIFT)) & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
			pickup->pos.z_pos = ((item->pos.z_pos + ((BLOCK_SIZE * phd_cos(item->pos.y_rot)) >> W2V_SHIFT)) & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
		} else {
			pickup->pos.x_pos = (item->pos.x_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
			pickup->pos.z_pos = (item->pos.z_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
		}

		room_number = item->room_number;
		pickup->pos.y_pos = GetHeight(GetFloor(pickup->pos.x_pos, item->pos.y_pos, pickup->pos.z_pos, &room_number),
		                              pickup->pos.x_pos, item->pos.y_pos, pickup->pos.z_pos);
		pickup->pos.y_pos -= GetBoundsAccurate(pickup)[3];
		ItemNewRoom(pickup_number, item->room_number);
		pickup->flags |= IFL_TRIGGERED;
		pickup_number = pickup->carried_item;
	}
}

void InitialiseCreature(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	item->collidable = 1;
	item->data = 0;
}

int32_t CreatureActive(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->flags & IFL_CLEARBODY)
		return 0;

	if (item->status == ITEM_INVISIBLE) {
		if (EnableBaddieAI(item_number, 0))
			item->status = ITEM_ACTIVE;
		else
			return 0;
	}

	return 1;
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info) {
	CREATURE_INFO* creature;
	OBJECT_INFO* obj;
	ITEM_INFO* enemy;
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	int16_t* zone;
	int32_t x, y, z;
	int16_t pivot, ang, state;

	creature = (CREATURE_INFO*)item->data;

	if (!creature)
		return;

	obj = &objects[item->object_number];

	if (item->poisoned) {
		if (!obj->undead && !(wibble & 0x3F) && item->hit_points > 1) {
			item->hit_points--;
		}
	}

	enemy = creature->enemy;

	if (!enemy) {
		enemy = lara_item;
		creature->enemy = lara_item;
	}

	zone = ground_zone[creature->LOT.zone][flip_status];
	r = &room[item->room_number];
	floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((item->pos.x_pos - r->x) >> WALL_SHIFT)];
	item->box_number = floor->box;
	info->zone_number = zone[item->box_number];

	r = &room[enemy->room_number];
	floor = &r->floor[((enemy->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((enemy->pos.x_pos - r->x) >> WALL_SHIFT)];
	enemy->box_number = floor->box;
	info->enemy_zone = zone[enemy->box_number];

	if (boxes[enemy->box_number].overlap_index & creature->LOT.block_mask ||
		creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | BLOCKED_SEARCH)) {
		info->enemy_zone |= BLOCKED;
	}

	pivot = obj->pivot_length;

	if (enemy == lara_item) {
		ang = lara.move_angle;
	} else {
		ang = enemy->pos.y_rot;
	}

	x = enemy->pos.x_pos + (14 * enemy->speed * phd_sin(ang) >> W2V_SHIFT) - (pivot * phd_sin(item->pos.y_rot) >> W2V_SHIFT) - item->pos.x_pos;
	y = item->pos.y_pos - enemy->pos.y_pos;
	z = enemy->pos.z_pos + (14 * enemy->speed * phd_cos(ang) >> W2V_SHIFT) - (pivot * phd_cos(item->pos.y_rot) >> W2V_SHIFT) - item->pos.z_pos;

	ang = (int16_t)phd_atan(z, x);

	if (z > 32000 || z < -32000 || x > 32000 || x < -32000) {
		info->distance = 0x7FFFFFFF;
	} else if (creature->enemy) {
		info->distance = SQUARE(x) + SQUARE(z);
	} else {
		info->distance = 0x7FFFFFFF;
	}

	info->angle = ang - item->pos.y_rot;
	info->enemy_facing = ang - enemy->pos.y_rot + 0x8000;

	x = abs(x);
	z = abs(z);

	if (enemy == lara_item) {
		state = lara_item->current_anim_state;

		if (state == AS_DUCK || state == AS_DUCKROLL || state == AS_ALL4S || state == AS_CRAWL ||
			state == AS_ALL4TURNL || state == AS_ALL4TURNR || state == AS_DUCKROTL || state == AS_DUCKROTR) {
			y -= (CLICK_SIZE + HALF_CLICK_SIZE);
		}
	}

	if (x > z)
		info->x_angle = (int16_t)phd_atan(x + (z >> 1), y);
	else
		info->x_angle = (int16_t)phd_atan(z + (x >> 1), y);

	if (info->angle > -FRONT_ARC && info->angle < FRONT_ARC) {
		info->ahead = true;
	} else {
		info->ahead = false;
	}

	if (info->ahead && enemy->hit_points > 0 && abs(enemy->pos.y_pos - item->pos.y_pos) <= HALF_BLOCK_SIZE) {
		info->bite = true;
	} else {
		info->bite = false;
	}
}

int32_t SearchLOT(LOT_INFO* LOT, int32_t expansion) {
	BOX_NODE* node;
	BOX_NODE* expand;
	BOX_INFO* box;
	int16_t* zone;
	int32_t index, done, box_number, overlap_flags, change;
	int16_t search_zone;

	zone = ground_zone[LOT->zone][flip_status];
	search_zone = zone[LOT->head];

	for (int32_t i = 0; i < expansion; i++) {
		if (LOT->head == NO_BOX) {
			LOT->tail = NO_BOX;
			return 0;
		}

		box = &boxes[LOT->head];
		node = &LOT->node[LOT->head];
		index = box->overlap_index & OVERLAP_INDEX;
		done = 0;

		do {
			box_number = overlap[index++];
			overlap_flags = box_number & ~BOX_NUMBER;

			if (box_number & BOX_END_BIT)
				done = 1;

			box_number &= BOX_NUMBER;

			if (LOT->fly == NO_FLYING && search_zone != zone[box_number]) {	//zone isn't in search area + can't fly
				continue;
			}

			change = boxes[box_number].height - box->height;

			if ((change > LOT->step || change < LOT->drop) && (!(overlap_flags & MONKEY_BIT) || !LOT->can_monkey)) {	//can't traverse block + can't monkey
				continue;
			}

			if (overlap_flags & JUMP_BIT && !LOT->can_jump) {	//can't jump
				continue;
			}

			expand = &LOT->node[box_number];

			if ((node->search_number & SEARCH_NUMBER) < (expand->search_number & SEARCH_NUMBER)) {
				continue;
			}

			if (node->search_number & BLOCKED_SEARCH) {
				if ((node->search_number & SEARCH_NUMBER) == (expand->search_number & SEARCH_NUMBER)) {
					continue;
				}

				expand->search_number = node->search_number;
			} else {
				if ((node->search_number & SEARCH_NUMBER) == (expand->search_number & SEARCH_NUMBER) && !(expand->search_number & BLOCKED_SEARCH)) {
					continue;
				}

				if (boxes[box_number].overlap_index & LOT->block_mask) {
					expand->search_number = node->search_number | BLOCKED_SEARCH;
				} else {
					expand->search_number = node->search_number;
					expand->exit_box = LOT->head;
				}
			}

			if (expand->next_expansion == NO_BOX && box_number != LOT->tail) {
				LOT->node[LOT->tail].next_expansion = (int16_t)box_number;
				LOT->tail = (int16_t)box_number;
			}

		} while (!done);

		LOT->head = node->next_expansion;
		node->next_expansion = NO_BOX;
	}

	return 1;
}

int32_t UpdateLOT(LOT_INFO* LOT, int32_t expansion) {
	BOX_NODE* expand;

	if (LOT->required_box != NO_BOX && LOT->required_box != LOT->target_box) {
		LOT->target_box = LOT->required_box;
		expand = &LOT->node[LOT->required_box];

		if (expand->next_expansion == NO_BOX && LOT->tail != LOT->required_box) {
			expand->next_expansion = LOT->head;

			if (LOT->head == NO_BOX) {
				LOT->tail = LOT->target_box;
			}

			LOT->head = LOT->target_box;
		}

		LOT->search_number++;
		expand->search_number = LOT->search_number;
		expand->exit_box = NO_BOX;
	}

	return SearchLOT(LOT, expansion);
}

void TargetBox(LOT_INFO* LOT, int16_t box_number) {
	BOX_INFO* box;

	box = &boxes[box_number & BOX_NUMBER];
	LOT->target.x = (((uint32_t)box->bottom - (uint32_t)box->top - 1) >> 5) * GetRandomControl() + ((uint32_t)box->top << WALL_SHIFT) + HALF_BLOCK_SIZE;
	LOT->target.z = (((uint32_t)box->right - (uint32_t)box->left - 1) >> 5) * GetRandomControl() + ((uint32_t)box->left << WALL_SHIFT) + HALF_BLOCK_SIZE;
	LOT->required_box = box_number & BOX_NUMBER;

	if (LOT->fly) {
		LOT->target.y = box->height - (CLICK_SIZE + HALF_CLICK_SIZE);
	} else {
		LOT->target.y = box->height;
	}
}

int32_t EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int16_t box_number) {
	BOX_INFO* box;
	int32_t x, z;

	box = &boxes[box_number];
	x = (((uint32_t)box->bottom + (uint32_t)box->top) << 9) - enemy->pos.x_pos;
	z = (((uint32_t)box->left + (uint32_t)box->right) << 9) - enemy->pos.z_pos;

	if (x > -(BLOCK_SIZE * 5) && x < (BLOCK_SIZE * 5) && z > -(BLOCK_SIZE * 5) && z < (BLOCK_SIZE * 5))
		return 0;

	return z > 0 == item->pos.z_pos > enemy->pos.z_pos || x > 0 == item->pos.x_pos > enemy->pos.x_pos;
}

int32_t ValidBox(ITEM_INFO* item, int16_t zone_number, int16_t box_number) {
	CREATURE_INFO* creature;
	BOX_INFO* box;

	creature = (CREATURE_INFO*)item->data;

	if (!creature->LOT.fly && ground_zone[creature->LOT.zone][flip_status][box_number] != zone_number) {
		return 0;
	}

	box = &boxes[box_number];

	if (creature->LOT.block_mask & box->overlap_index) {
		return 0;
	}

	if (item->pos.z_pos > box->left << WALL_SHIFT && item->pos.z_pos < box->right << WALL_SHIFT &&
		item->pos.x_pos > box->top << WALL_SHIFT && item->pos.x_pos < box->bottom << WALL_SHIFT) {
		return 0;
	}

	return 1;
}

int32_t StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, int16_t box_number) {
	BOX_INFO* box;
	int32_t x, z, xrange, zrange, enemy_quad, box_quad, baddie_quad;

	if (!enemy) {
		return 0;
	}

	box = &boxes[box_number];
	x = (((uint32_t)box->bottom + (uint32_t)box->top) << 9) - enemy->pos.x_pos;
	z = (((uint32_t)box->left + (uint32_t)box->right) << 9) - enemy->pos.z_pos;
	xrange = ((uint32_t)box->bottom - (uint32_t)box->top + 3) << WALL_SHIFT;	//3 is the # of blocks
	zrange = ((uint32_t)box->right - (uint32_t)box->left + 3) << WALL_SHIFT;

	if (x > xrange || x < -xrange || z > zrange || z < -zrange)
		return 0;

	enemy_quad = (enemy->pos.y_rot >> W2V_SHIFT) + 2;
	box_quad = z <= 0 ? (x <= 0 ? 0 : 3) : (x > 0) + 1;

	if (enemy_quad == box_quad)
		return 0;

	baddie_quad = item->pos.z_pos <= enemy->pos.z_pos ? (item->pos.x_pos <= enemy->pos.x_pos ? 0 : 3) : (item->pos.x_pos > enemy->pos.x_pos) + 1;
	return enemy_quad != baddie_quad || abs(enemy_quad - box_quad) != 2;
}

target_type CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT) {
	int32_t box_left, box_right, box_top, box_bottom;

	UpdateLOT(LOT, MAX_EXPANSION);
	target->x = item->pos.x_pos;
	target->y = item->pos.y_pos;
	target->z = item->pos.z_pos;
	int32_t box_number = item->box_number;

	if (box_number == NO_BOX)
		return NO_TARGET;

	BOX_INFO* box = &boxes[box_number];
	int32_t left = box->left << WALL_SHIFT;
	int32_t right = (box->right << WALL_SHIFT) - 1;
	int32_t top = box->top << WALL_SHIFT;
	int32_t bottom = (box->bottom << WALL_SHIFT) - 1;
	
	static uint16_t loops = 0;
	int32_t prime_free = BOX_CLIP_ALL;

	do {
		loops++;
		box = &boxes[box_number];

		if (LOT->fly) {
			if (target->y > box->height - BLOCK_SIZE)
				target->y = box->height - BLOCK_SIZE;
		}
		else if (target->y > box->height) {
			target->y = box->height;
		}

		box_left = box->left << WALL_SHIFT;
		box_right = (box->right << WALL_SHIFT) - 1;
		box_top = box->top << WALL_SHIFT;
		box_bottom = (box->bottom << WALL_SHIFT) - 1;

		if (item->pos.z_pos >= box_left && item->pos.z_pos <= box_right && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom) {
			left = box->left << WALL_SHIFT;
			right = (box->right << WALL_SHIFT) - 1;
			top = box->top << WALL_SHIFT;
			bottom = (box->bottom << WALL_SHIFT) - 1;
		} else {
			if (item->pos.z_pos < box_left && prime_free != BOX_CLIP_RIGHT) {
				if (prime_free & BOX_CLIP_LEFT && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom) {
					if (target->z < box_left + HALF_BLOCK_SIZE)
						target->z = box_left + HALF_BLOCK_SIZE;

					if (prime_free & BOX_CLIP_SECONDARY)
						return SECONDARY_TARGET;

					if (box_top > top)
						top = box_top;

					if (box_bottom < bottom)
						bottom = box_bottom;

					prime_free = BOX_CLIP_LEFT;
				} else if (prime_free != BOX_CLIP_LEFT) {
					target->z = right - HALF_BLOCK_SIZE;

					if (prime_free != BOX_CLIP_ALL)
						return SECONDARY_TARGET;

					prime_free = (BOX_CLIP_ALL | BOX_CLIP_SECONDARY);
				}
			} else if (item->pos.z_pos > box_right && prime_free != BOX_CLIP_LEFT) {
				if (prime_free & BOX_CLIP_RIGHT && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom) {
					if (target->z > box_right - HALF_BLOCK_SIZE)
						target->z = box_right - HALF_BLOCK_SIZE;

					if (prime_free & BOX_CLIP_SECONDARY)
						return SECONDARY_TARGET;

					if (box_top > top)
						top = box_top;

					if (box_bottom < bottom)
						bottom = box_bottom;

					prime_free = BOX_CLIP_RIGHT;
				} else if (prime_free != BOX_CLIP_RIGHT) {
					target->z = left + HALF_BLOCK_SIZE;

					if (prime_free != BOX_CLIP_ALL)
						return SECONDARY_TARGET;

					prime_free = (BOX_CLIP_ALL | BOX_CLIP_SECONDARY);
				}
			}

			if (item->pos.x_pos < box_top && prime_free != BOX_CLIP_BOTTOM) {
				if (prime_free & BOX_CLIP_TOP && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right) {
					if (target->x < box_top + HALF_BLOCK_SIZE)
						target->x = box_top + HALF_BLOCK_SIZE;

					if (prime_free & BOX_CLIP_SECONDARY)
						return SECONDARY_TARGET;

					if (box_left > left)
						left = box_left;

					if (box_right < right)
						right = box_right;

					prime_free = BOX_CLIP_TOP;
				} else if (prime_free != BOX_CLIP_TOP) {
					target->x = bottom - HALF_BLOCK_SIZE;

					if (prime_free != BOX_CLIP_ALL)
						return SECONDARY_TARGET;

					prime_free = (BOX_CLIP_ALL | BOX_CLIP_SECONDARY);
				}
			} else if (item->pos.x_pos > box_bottom && prime_free != BOX_CLIP_TOP) {
				if (prime_free & BOX_CLIP_BOTTOM && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right) {
					if (target->x > box_bottom - HALF_BLOCK_SIZE)
						target->x = box_bottom - HALF_BLOCK_SIZE;

					if (prime_free & BOX_CLIP_SECONDARY)
						return SECONDARY_TARGET;

					if (box_left > left)
						left = box_left;

					if (box_right < right)
						right = box_right;

					prime_free = BOX_CLIP_BOTTOM;
				} else if (prime_free != BOX_CLIP_BOTTOM) {
					target->x = top + HALF_BLOCK_SIZE;

					if (prime_free != BOX_CLIP_ALL)
						return SECONDARY_TARGET;

					prime_free = (BOX_CLIP_ALL | BOX_CLIP_SECONDARY);
				}
			}
		}

		if (box_number == LOT->target_box) {
			if (prime_free & (BOX_CLIP_LEFT | BOX_CLIP_RIGHT))
				target->z = LOT->target.z;
			else if (!(prime_free & BOX_CLIP_SECONDARY)) {
				if (target->z < box_left + HALF_BLOCK_SIZE)
					target->z = box_left + HALF_BLOCK_SIZE;
				else if (target->z > box_right - HALF_BLOCK_SIZE)
					target->z = box_right - HALF_BLOCK_SIZE;
			}

			if (prime_free & (BOX_CLIP_TOP | BOX_CLIP_BOTTOM))
				target->x = LOT->target.x;
			else if (!(prime_free & BOX_CLIP_SECONDARY)) {
				if (target->x < box_top + HALF_BLOCK_SIZE)
					target->x = box_top + HALF_BLOCK_SIZE;
				else if (target->x > box_bottom - HALF_BLOCK_SIZE)
					target->x = box_bottom - HALF_BLOCK_SIZE;
			}

			target->y = LOT->target.y;
			return PRIME_TARGET;
		}

		box_number = LOT->node[box_number].exit_box;

		if (box_number != NO_BOX && (boxes[box_number].overlap_index & LOT->block_mask)) {
			break;
		}
	} while (box_number != NO_BOX);

	if (!(prime_free & BOX_CLIP_SECONDARY)) {
		if (target->z < box_left + HALF_BLOCK_SIZE)
			target->z = box_left + HALF_BLOCK_SIZE;
		else if (target->z > box_right - HALF_BLOCK_SIZE)
			target->z = box_right - HALF_BLOCK_SIZE; // Was set to 521, but that might have been a typo.
	}

	if (!(prime_free & BOX_CLIP_SECONDARY)) {	//wut
		if (target->x < box_top + HALF_BLOCK_SIZE)
			target->x = box_top + HALF_BLOCK_SIZE;
		else if (target->x > box_bottom - HALF_BLOCK_SIZE)
			target->x = box_bottom - HALF_BLOCK_SIZE;
	}

	if (LOT->fly)
		target->y = box->height - 320;
	else
		target->y = box->height;

	return NO_TARGET;
}

void CreatureMood(ITEM_INFO* item, AI_INFO* info, bool violent) {
	CREATURE_INFO* creature;
	ITEM_INFO* enemy;
	LOT_INFO* LOT;
	static target_type type;
	int16_t index, box_no;

	creature = (CREATURE_INFO*)item->data;

	if (!creature) {
		return;
	}

	enemy = creature->enemy;
	LOT = &creature->LOT;

	switch (creature->mood) {
		case BORED_MOOD:
			box_no = LOT->node[(creature->LOT.zone_count * GetRandomControl()) >> 15].box_number;

			if (ValidBox(item, info->zone_number, box_no)) {
				if (StalkBox(item, enemy, box_no) && enemy->hit_points > 0 && creature->enemy) {
					TargetBox(&creature->LOT, box_no);
					creature->mood = BORED_MOOD;
				} else if (creature->LOT.required_box == NO_BOX) {
					TargetBox(&creature->LOT, box_no);
				}
			}

			break;

		case ATTACK_MOOD:
			if (GetRandomControl() < objects[item->object_number].aggression) {
				creature->LOT.target.x = enemy->pos.x_pos;
				creature->LOT.target.y = enemy->pos.y_pos;
				creature->LOT.target.z = enemy->pos.z_pos;
				creature->LOT.required_box = enemy->box_number;

				if (creature->LOT.fly && lara.water_status == LW_ABOVE_WATER) {
					creature->LOT.target.y += GetBestFrame(enemy)[2];
				}
			}

			break;

		case ESCAPE_MOOD:
			box_no = LOT->node[(creature->LOT.zone_count * GetRandomControl()) >> 15].box_number;

			if (ValidBox(item, info->zone_number, box_no) && creature->LOT.required_box == NO_BOX) {
				if (EscapeBox(item, enemy, box_no)) {
					TargetBox(&creature->LOT, box_no);
				} else if (info->zone_number == info->enemy_zone && StalkBox(item, enemy, box_no) && !violent) {
					TargetBox(&creature->LOT, box_no);
					creature->mood = STALK_MOOD;
				}
			}

			break;

		case STALK_MOOD:
			if (creature->LOT.required_box == NO_BOX || !StalkBox(item, enemy, creature->LOT.required_box)) {
				box_no = LOT->node[(creature->LOT.zone_count * GetRandomControl()) >> 15].box_number;

				if (ValidBox(item, info->zone_number, box_no)) {
					if (StalkBox(item, enemy, box_no)) {
						TargetBox(&creature->LOT, box_no);
					} else if (creature->LOT.required_box == NO_BOX) {
						TargetBox(&creature->LOT, box_no);

						if (info->zone_number != info->enemy_zone) {
							creature->mood = BORED_MOOD;
						}
					}
				}
			}

			break;
	}

	if (creature->LOT.target_box == NO_BOX) {
		TargetBox(&creature->LOT, item->box_number);
	}

	type = CalculateTarget(&creature->target, item, &creature->LOT);
	creature->jump_ahead = false;
	creature->monkey_ahead = false;

	if (LOT->node[item->box_number].exit_box != NO_BOX) {
		index = boxes[item->box_number].overlap_index & OVERLAP_INDEX;

		do box_no = overlap[index++];
		while (box_no != NO_BOX && !(box_no & BOX_END_BIT) && (box_no & BOX_NUMBER) != LOT->node[item->box_number].exit_box);

		if ((box_no & BOX_NUMBER) == LOT->node[item->box_number].exit_box) {
			if (box_no & JUMP_BIT) {
				creature->jump_ahead = true;
			}

			if (box_no & MONKEY_BIT) {
				creature->monkey_ahead = true;
			}
		}
	}
}

void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, bool violent) {
	CREATURE_INFO* creature;
	ITEM_INFO* enemy;
	LOT_INFO* LOT;
	mood_type mood;

	creature = (CREATURE_INFO*)item->data;

	if (!creature)
		return;

	LOT = &creature->LOT;
	enemy = creature->enemy;

	if (creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | BLOCKED_SEARCH))
		creature->LOT.required_box = NO_BOX;

	if (creature->mood != ATTACK_MOOD && creature->LOT.required_box != NO_BOX && !ValidBox(item, info->zone_number, creature->LOT.target_box)) {
		if (info->zone_number == info->enemy_zone)
			creature->mood = BORED_MOOD;

		creature->LOT.required_box = NO_BOX;
	}

	mood = creature->mood;

	if (enemy) {
		if (enemy->hit_points <= 0 && enemy == lara_item)
			creature->mood = BORED_MOOD;
		else if (violent) {
			switch (creature->mood) {
				case BORED_MOOD:
				case STALK_MOOD:

					if (info->zone_number == info->enemy_zone)
						creature->mood = ATTACK_MOOD;
					else if (item->hit_status)
						creature->mood = ESCAPE_MOOD;

					break;

				case ATTACK_MOOD:

					if (info->zone_number != info->enemy_zone)
						creature->mood = BORED_MOOD;

					break;

				case ESCAPE_MOOD:

					if (info->zone_number == info->enemy_zone)
						creature->mood = ATTACK_MOOD;

					break;
			}
		} else {
			switch (creature->mood) {
				case BORED_MOOD:
				case STALK_MOOD:

					if (creature->alerted && info->zone_number != info->enemy_zone)
						creature->mood = info->distance > 3072 ? STALK_MOOD : BORED_MOOD;
					else if (info->zone_number == info->enemy_zone) {
						if (info->distance < 0x900000 || mood == STALK_MOOD && LOT->required_box == NO_BOX)
							creature->mood = ATTACK_MOOD;
						else
							creature->mood = STALK_MOOD;
					}

					break;

				case ATTACK_MOOD:

					if (item->hit_status && GetRandomControl() < (BLOCK_SIZE * 2) || info->zone_number != info->enemy_zone)
						creature->mood = STALK_MOOD;
					else if (info->zone_number != info->enemy_zone && info->distance > 6144)
						creature->mood = BORED_MOOD;

					break;

				case ESCAPE_MOOD:

					if (info->zone_number == info->enemy_zone && GetRandomControl() < CLICK_SIZE)
						creature->mood = STALK_MOOD;

					break;
			}
		}
	} else
		creature->mood = BORED_MOOD;

	if (mood != creature->mood) {
		if (mood == ATTACK_MOOD) {
			TargetBox(LOT, LOT->target_box);
			LOT = &creature->LOT;
		}

		LOT->required_box = NO_BOX;
	}
}

int32_t CreatureCreature(int16_t item_number) {
	ITEM_INFO* item;
	int32_t x, z, dx, dz, dist;
	int16_t yrot, rad, item_num;

	item = &items[item_number];
	x = item->pos.x_pos;
	z = item->pos.z_pos;
	yrot = item->pos.y_rot;
	rad = objects[item->object_number].radius;

	for (item_num = room[item->room_number].item_number; item_num != NO_ITEM; item_num = item->next_item) {
		item = &items[item_num];

		if (item_num != item_number && item != lara_item && item->status == ITEM_ACTIVE && item->hit_points > 0) {
			dx = abs(item->pos.x_pos - x);
			dz = abs(item->pos.z_pos - z);
			dist = dx > dz ? dx + (dz >> 1) : dz + (dx >> 1);

			if (dist < rad + objects[item->object_number].radius)
				return int16_t(phd_atan(item->pos.z_pos - z, item->pos.x_pos - x) - yrot);
		}
	}

	return 0;
}

int32_t BadFloor(int32_t x, int32_t y, int32_t z, int32_t box_height, int32_t next_height, int16_t room_number, LOT_INFO* LOT) {
	FLOOR_INFO* floor;
	BOX_INFO* box;

	floor = GetFloor(x, y, z, &room_number);

	if (floor->box == NO_BOX)
		return 1;

	if (LOT->is_jumping)
		return 0;

	box = &boxes[floor->box];

	if (box->overlap_index & LOT->block_mask)
		return 1;

	if (box_height - box->height > LOT->step || box_height - box->height < LOT->drop)
		return 1;

	if (box_height - box->height < -LOT->step && box->height > next_height)
		return 1;

	if (LOT->fly && y > box->height + LOT->fly)
		return 1;

	return 0;
}

int32_t CreatureAnimation(int16_t item_number, int16_t angle, int16_t tilt) {
	ITEM_INFO* item;
	CREATURE_INFO* creature;
	LOT_INFO* LOT;
	FLOOR_INFO* floor;
	PHD_VECTOR oldPos;
	int16_t* zone;
	int16_t* bounds;
	int32_t box_height, y, height, next_box, next_height, x, z, wx, wz, xShift, zShift, dy;
	int16_t room_number, rad;

	item = &items[item_number];
	creature = (CREATURE_INFO*)item->data;

	if (!creature)
		return 0;

	LOT = &creature->LOT;
	oldPos.x = item->pos.x_pos;
	oldPos.y = item->pos.y_pos;
	oldPos.z = item->pos.z_pos;
	height = boxes[item->box_number].height;
	zone = ground_zone[LOT->zone][flip_status];
	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED) {
		CreatureDie(item_number, 0);
		return 0;
	}

	bounds = GetBoundsAccurate(item);
	y = item->pos.y_pos + bounds[2];
	room_number = item->room_number;
	GetFloor(oldPos.x, y, oldPos.z, &room_number);
	floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
	box_height = boxes[floor->box].height;
	next_box = LOT->node[floor->box].exit_box;

	if (next_box == NO_BOX)
		next_height = boxes[floor->box].height;
	else
		next_height = boxes[next_box].height;

	if (floor->box == NO_BOX || !LOT->is_jumping &&
	        (zone[item->box_number] != zone[floor->box] || height - box_height > LOT->step || height - box_height < LOT->drop)) {
		wx = item->pos.x_pos >> WALL_SHIFT;
		wz = item->pos.z_pos >> WALL_SHIFT;

		if (wx < oldPos.x >> WALL_SHIFT) {
			item->pos.x_pos = oldPos.x & ~(BLOCK_SIZE - 1);
		} else if (wx > oldPos.x >> WALL_SHIFT) { // T4Plus: converted from else to else if to better replicate OG behaviour.
			item->pos.x_pos = oldPos.x | (BLOCK_SIZE - 1);
		}

		if (wz < oldPos.z >> WALL_SHIFT) {
			item->pos.z_pos = oldPos.z & ~(BLOCK_SIZE - 1);
		} else if (wz > oldPos.z >> WALL_SHIFT) { // T4Plus: converted from else to else if to better replicate OG behaviour.
			item->pos.z_pos = oldPos.z | (BLOCK_SIZE - 1);
		}

		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		box_height = boxes[floor->box].height;
		next_box = LOT->node[floor->box].exit_box;

		if (next_box == NO_BOX) {
			next_height = boxes[floor->box].height;
		} else {
			next_height = boxes[next_box].height;
		}
	}

	x = item->pos.x_pos;
	z = item->pos.z_pos;
	wx = x & (BLOCK_SIZE - 1);
	wz = z & (BLOCK_SIZE - 1);
	rad = objects[item->object_number].radius;
	xShift = 0;
	zShift = 0;

	if (wz < rad) {
		if (BadFloor(x, y, z - rad, box_height, next_height, room_number, LOT)) {
			zShift = rad - wz;
		}

		if (wx < rad) {
			if (!BadFloor(x - rad, y, z, box_height, next_height, room_number, LOT)) {
				if (!zShift && BadFloor(x - rad, y, z - rad, box_height, next_height, room_number, LOT)) {
					if (item->pos.y_rot > -0x6000 && item->pos.y_rot < 0x2000) {
						zShift = rad - wz;
					} else {
						xShift = rad - wx;
					}
				}
			} else
				xShift = rad - wx;
		} else if (wx > BLOCK_SIZE - rad) {
			if (!BadFloor(x + rad, y, z, box_height, next_height, room_number, LOT)) {
				if (!zShift && BadFloor(x + rad, y, z - rad, box_height, next_height, room_number, LOT)) {
					if (item->pos.y_rot > -0x2000 && item->pos.y_rot < 0x6000) {
						zShift = rad - wz;
					} else {
						xShift = BLOCK_SIZE - rad - wx;
					}
				}
			} else
				xShift = BLOCK_SIZE - rad - wx;
		}
	} else if (wz > BLOCK_SIZE - rad) {
		if (BadFloor(x, y, z + rad, box_height, next_height, room_number, LOT))
			zShift = BLOCK_SIZE - rad - wz;

		if (wx < rad) {
			if (!BadFloor(x - rad, y, z, box_height, next_height, room_number, LOT)) {
				if (!zShift && BadFloor(x - rad, y, z + rad, box_height, next_height, room_number, LOT)) {
					if (item->pos.y_rot > -0x2000 && item->pos.y_rot < 0x6000) {
						xShift = rad - wx;
					} else {
						zShift = BLOCK_SIZE - rad - wz;
					}
				}
			} else
				xShift = rad - wx;

		} else if (wx > BLOCK_SIZE - rad) {
			if (!BadFloor(x + rad, y, z, box_height, next_height, room_number, LOT)) {
				if (!zShift && BadFloor(x + rad, y, z + rad, box_height, next_height, room_number, LOT)) {
					if (item->pos.y_rot > -0x6000 && item->pos.y_rot < 0x2000) {
						xShift = BLOCK_SIZE - rad - wx;
					} else {
						zShift = BLOCK_SIZE - rad - wz;
					}
				}
			} else
				xShift = BLOCK_SIZE - rad - wx;
		}
	} else if (wx < rad) {
		if (BadFloor(x - rad, y, z, box_height, next_height, room_number, LOT)) {
			xShift = rad - wx;
		}
	} else if (wx > BLOCK_SIZE - rad) {
		if (BadFloor(x + rad, y, z, box_height, next_height, room_number, LOT)) {
			xShift = BLOCK_SIZE - rad - wx;
		}
	}

	item->pos.x_pos += xShift;
	item->pos.z_pos += zShift;

	if (xShift || zShift) {
		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		item->pos.y_rot += angle;

		if (tilt) {
			CreatureTilt(item, 2 * tilt);
		}
	}

	if (item->speed && item->hit_points > 0) {
		angle = (int16_t)CreatureCreature(item_number);

		if (angle) {
			if (abs(angle) < NAVIGATION_AVOIDENCE_TURN) {
				item->pos.y_rot -= angle;
			} else if (angle > 0) {
				item->pos.y_rot -= NAVIGATION_AVOIDENCE_TURN;
			} else {
				item->pos.y_rot += NAVIGATION_AVOIDENCE_TURN;
			}

			return 1;
		}
	}

	if (LOT->fly && item->hit_points > 0) {
		dy = creature->target.y - item->pos.y_pos;

		if (dy > LOT->fly) {
			dy = LOT->fly;
		} else if (dy < -LOT->fly) {
			dy = -LOT->fly;
		}

		height = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);

		if (item->pos.y_pos + dy > height) {
			if (item->pos.y_pos > height) {
				dy = -LOT->fly;
				item->pos.x_pos = oldPos.x;
				item->pos.z_pos = oldPos.z;
			} else {
				dy = 0;
				item->pos.y_pos = height;
			}
		} else if (objects[item->object_number].water_creature) {
			height = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos);

			if (item->pos.y_pos + bounds[2] + dy < height) {
				if (item->pos.y_pos + bounds[2] < height) {
					dy = LOT->fly;
					item->pos.x_pos = oldPos.x;
					item->pos.z_pos = oldPos.z;
				} else {
					dy = 0;
				}
			}
		} else {
			GetFloor(item->pos.x_pos, y + CLICK_SIZE, item->pos.z_pos, &room_number);

			if (room[room_number].flags & ROOM_UNDERWATER || T4PlusIsRoomSwamp(&room[room_number])) {
				dy = -LOT->fly;
			}
		}

		item->pos.y_pos += dy;
		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		item->floor = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);

		if (item->speed) {
			angle = (int16_t)phd_atan(item->speed, -dy);

			if (angle < -DEGREES_TO_ROTATION(20)) {
				angle = -DEGREES_TO_ROTATION(20);
			} else if (angle > DEGREES_TO_ROTATION(20)) {
				angle = DEGREES_TO_ROTATION(20);
			}
		} else {
			angle = 0;
		}

		if (angle < item->pos.x_rot - DEGREES_TO_ROTATION(1)) {
			item->pos.x_rot -= DEGREES_TO_ROTATION(1);
		} else  if (angle > item->pos.x_rot + DEGREES_TO_ROTATION(1)) {
			item->pos.x_rot += DEGREES_TO_ROTATION(1);
		} else {
			item->pos.x_rot = angle;
		}
	} else if (LOT->is_jumping) {
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (LOT->is_monkeying) {
			item->pos.y_pos = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos) - bounds[2];
		} else if (item->pos.y_pos > item->floor) {
			if (item->pos.y_pos > item->floor + CLICK_SIZE) {
				item->pos.x_pos = oldPos.x;
				item->pos.z_pos = oldPos.z;
				item->pos.y_pos = oldPos.y;
			} else {
				item->pos.y_pos = item->floor;
			}
		}
	} else {
		floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
		height = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos);

		if (item->pos.y_pos + bounds[2] < height) {
			item->pos.x_pos = oldPos.x;
			item->pos.z_pos = oldPos.z;
			item->pos.y_pos = oldPos.y;
		}

		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (item->pos.y_pos > item->floor) {
			item->pos.y_pos = item->floor;
		} else if (item->floor - item->pos.y_pos > QUARTER_CLICK_SIZE) {
			item->pos.y_pos += QUARTER_CLICK_SIZE;
		} else if (item->pos.y_pos < item->floor) {
			item->pos.y_pos = item->floor;
		}
	}

	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos - 32, item->pos.z_pos, &room_number);

	if (item->room_number != room_number) {
		ItemNewRoom(item_number, room_number);
	}

	return 1;
}

int16_t CreatureTurn(ITEM_INFO* item, int16_t maximum_turn) {
	CREATURE_INFO* creature;
	ROOM_INFO* r;
	int32_t x, z, feelxplus, feelzplus, feelxminus, feelzminus, feelxmid, feelzmid, feelplus, feelminus, feelmid;
	int16_t angle;

	creature = (CREATURE_INFO*)item->data;

	if (!creature || !maximum_turn)
		return 0;

	x = item->pos.x_pos;
	z = item->pos.z_pos;
	r = &room[item->room_number];

	feelxplus = x + (HALF_BLOCK_SIZE * phd_sin(item->pos.y_rot + DEGREES_TO_ROTATION(45)) >> W2V_SHIFT);
	feelzplus = z + (HALF_BLOCK_SIZE * phd_cos(item->pos.y_rot + DEGREES_TO_ROTATION(45)) >> W2V_SHIFT);
	feelplus = r->floor[((feelzplus - r->z) >> WALL_SHIFT) + r->x_size * ((feelxplus - r->x) >> WALL_SHIFT)].stopper;

	feelxminus = x + (HALF_BLOCK_SIZE * phd_sin(item->pos.y_rot - DEGREES_TO_ROTATION(45)) >> W2V_SHIFT);
	feelzminus = z + (HALF_BLOCK_SIZE * phd_cos(item->pos.y_rot - DEGREES_TO_ROTATION(45)) >> W2V_SHIFT);
	feelminus = r->floor[((feelzminus - r->z) >> WALL_SHIFT) + r->x_size * ((feelxminus - r->x) >> WALL_SHIFT)].stopper;

	feelxmid = x + (HALF_BLOCK_SIZE * phd_sin(item->pos.y_rot) >> W2V_SHIFT);
	feelzmid = z + (HALF_BLOCK_SIZE * phd_cos(item->pos.y_rot) >> W2V_SHIFT);
	feelmid = r->floor[((feelzmid - r->z) >> WALL_SHIFT) + r->x_size * ((feelxmid - r->x) >> WALL_SHIFT)].stopper;

	if (feelminus && feelmid) {
		creature->target.x = feelxplus;
		creature->target.z = feelzplus;
	} else if (feelplus && feelmid) {
		creature->target.x = feelxminus;
		creature->target.z = feelzminus;
	} else if (feelplus || feelminus) {
		creature->target.x = feelxmid;
		creature->target.z = feelzmid;
	}

	x = creature->target.x - item->pos.x_pos;
	z = creature->target.z - item->pos.z_pos;
	angle = int16_t(phd_atan(z, x) - item->pos.y_rot);

	if (angle > FRONT_ARC || angle < -FRONT_ARC) {
		if (SQUARE(x) + SQUARE(z) < SQUARE((item->speed << W2V_SHIFT) / maximum_turn))
			maximum_turn >>= 1;
	}

	if (angle > maximum_turn)
		angle = maximum_turn;
	else if (angle < -maximum_turn)
		angle = -maximum_turn;

	item->pos.y_rot += angle;
	return angle;
}

void CreatureTilt(ITEM_INFO* item, int16_t angle) {
	angle = (angle << 2) - item->pos.z_rot;

	if (angle < -DEGREES_TO_ROTATION(3))
		item->pos.z_rot -= DEGREES_TO_ROTATION(3);
	else if (angle > DEGREES_TO_ROTATION(3))
		item->pos.z_rot += DEGREES_TO_ROTATION(3);
}

void CreatureJoint(ITEM_INFO* item, int16_t joint, int16_t required) {
	CREATURE_INFO* creature;
	int16_t change;

	creature = (CREATURE_INFO*)item->data;

	if (!creature)
		return;

	change = required - creature->joint_rotation[joint];

	if (change > DEGREES_TO_ROTATION(3)) {
		change = DEGREES_TO_ROTATION(3);
	} else if (change < -DEGREES_TO_ROTATION(3)) {
		change = -DEGREES_TO_ROTATION(3);
	}

	creature->joint_rotation[joint] += change;

	if (creature->joint_rotation[joint] > 0x3000) {
		creature->joint_rotation[joint] = 0x3000;
	} else if (creature->joint_rotation[joint] < -0x3000) {
		creature->joint_rotation[joint] = -0x3000;
	}
}

void CreatureFloat(int16_t item_number) {
	ITEM_INFO* item;
	int32_t water_level;
	int16_t room_number;

	item = &items[item_number];
	item->hit_points = INFINITE_HEALTH;
	item->pos.x_rot = 0;
	water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (item->pos.y_pos > water_level) {
		item->pos.y_pos -= 32;
	}

	if (item->pos.y_pos < water_level) {
		item->pos.y_pos = water_level;
	}

	AnimateItem(item);
	room_number = item->room_number;
	item->floor = GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number),
	                        item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (item->pos.y_pos <= water_level && item->frame_number == anims[item->anim_number].frame_base) {
		item->status = ITEM_DEACTIVATED;
		item->collidable = 0;
		item->pos.y_pos = water_level;
		DisableBaddieAI(item_number);
		RemoveActiveItem(item_number);
		item->after_death = 1;
	}
}

void CreatureUnderwater(ITEM_INFO* item, int32_t depth) {
	int32_t water_level, floorheight;
	int16_t room_number;

	water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (item->pos.y_pos < water_level + depth) {
		room_number = item->room_number;
		floorheight = GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number),
		                        item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
		item->pos.y_pos = water_level + depth;

		if (item->pos.y_pos > floorheight)
			item->pos.y_pos = floorheight;

		if (item->pos.x_rot > DEGREES_TO_ROTATION(2))
			item->pos.x_rot -= DEGREES_TO_ROTATION(2);
		else if (item->pos.x_rot > 0)
			item->pos.x_rot = 0;
	}
}

int16_t CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, int16_t(*generate)(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot, int16_t room_number)) {
	PHD_VECTOR pos;

	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;
	GetJointAbsPosition(item, &pos, bite->mesh_num);
	return generate(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number);
}

int16_t CreatureEffectT(ITEM_INFO* item, BITE_INFO* bite, int16_t damage, int16_t angle,
                        int16_t(*generate)(int32_t x, int32_t y, int32_t z, int16_t damage, int16_t angle, int16_t room_number)) {
	PHD_VECTOR pos;

	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;
	GetJointAbsPosition(item, &pos, bite->mesh_num);
	return generate(pos.x, pos.y, pos.z, damage, angle, item->room_number);
}

int32_t CreatureVault(int16_t item_number, int16_t angle, int32_t vault, int32_t shift) {
	ITEM_INFO* item;
	int32_t x, y, z, x_floor, z_floor;
	int16_t room_number;

	item = &items[item_number];
	x = item->pos.x_pos >> WALL_SHIFT;
	y = item->pos.y_pos;
	z = item->pos.z_pos >> WALL_SHIFT;
	room_number = item->room_number;
	CreatureAnimation(item_number, angle, 0);

	if (item->floor > y + (BLOCK_SIZE + HALF_CLICK_SIZE)) {
		vault = 0;
	} else if (item->floor > y + ((CLICK_SIZE * 3) + HALF_CLICK_SIZE)) {
		vault = -4;
	} else if (item->floor > y + (HALF_BLOCK_SIZE + HALF_CLICK_SIZE) && item->object_number == VON_CROY) {
		vault = -3;
	} else if (item->floor > y + ((CLICK_SIZE) + HALF_CLICK_SIZE) && item->object_number == VON_CROY) {
		vault = -2;
	} else {
		if (item->pos.y_pos > y - ((CLICK_SIZE * 1) + HALF_CLICK_SIZE)) {
			return 0;
		}
		
		if (item->pos.y_pos > y - (HALF_BLOCK_SIZE + HALF_CLICK_SIZE)) {
			vault = 2;
		} else if (item->pos.y_pos > y - ((CLICK_SIZE * 3) + HALF_CLICK_SIZE)) {
			vault = 3;
		} else if (item->pos.y_pos > y - (BLOCK_SIZE + HALF_CLICK_SIZE)) {
			vault = 4;
		}
	}

	x_floor = item->pos.x_pos >> WALL_SHIFT;
	z_floor = item->pos.z_pos >> WALL_SHIFT;

	if (z == z_floor) {
		if (x == x_floor) {
			return 0;
		}

		if (x >= x_floor) {
			item->pos.y_rot = -0x4000;
			item->pos.x_pos = shift + (x << 10);
		} else {
			item->pos.y_rot = 0x4000;
			item->pos.x_pos = (x_floor << 10) - shift;
		}
	} else if (x == x_floor) {
		if (z < z_floor) {
			item->pos.y_rot = 0;
			item->pos.z_pos = (z_floor << 10) - shift;
		} else {
			item->pos.y_rot = -0x8000;
			item->pos.z_pos = shift + (z << 10);
		}
	}

	item->floor = y;
	item->pos.y_pos = y;

	if (vault)
		ItemNewRoom(item_number, room_number);

	return vault;
}

void CreatureKill(ITEM_INFO* item, int16_t kill_anim, int16_t kill_state, int16_t lara_anim) {
	item->anim_number = objects[item->object_number].anim_index + kill_anim;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = kill_state;
	lara_item->anim_number = lara_anim;
	lara_item->frame_number = anims[lara_item->anim_number].frame_base;
	lara_item->current_anim_state = 8;
	lara_item->goal_anim_state = 8;
	lara_item->pos.x_pos = item->pos.x_pos;
	lara_item->pos.y_pos = item->pos.y_pos;
	lara_item->pos.z_pos = item->pos.z_pos;
	lara_item->pos.x_rot = item->pos.x_rot;
	lara_item->pos.y_rot = item->pos.y_rot;
	lara_item->pos.z_rot = item->pos.z_rot;
	lara_item->fallspeed = 0;
	lara_item->gravity_status = 0;
	lara_item->speed = 0;

	if (lara_item->room_number != item->room_number) {
		ItemNewRoom(lara.item_number, item->room_number);
	}

	AnimateItem(lara_item);
	lara.gun_status = LG_HANDS_BUSY;
	lara.gun_type = WEAPON_NONE;
	lara.hit_direction = -1;
	lara.air = -1;
	camera.pos.room_number = lara_item->room_number;
	ForcedFixedCamera.x = item->pos.x_pos + (((BLOCK_SIZE * 2) * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
	ForcedFixedCamera.y = item->pos.y_pos - BLOCK_SIZE;
	ForcedFixedCamera.z = item->pos.z_pos + (((BLOCK_SIZE * 2) * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
	ForcedFixedCamera.room_number = item->room_number;
	UseForcedFixedCamera = 1;
}

void AlertAllGuards(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* creature;

	item = &items[item_number];

	for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
		creature = &baddie_slots[i];

		if (creature->item_num != NO_ITEM) {
			if (items[creature->item_num].object_number == item->object_number) {
				creature->alerted = true;
			}
		}
	}
}

void AlertNearbyGuards(ITEM_INFO* item) {
	ITEM_INFO* target;
	CREATURE_INFO* creature;
	int32_t dx, dy, dz, dist;

	for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
		creature = &baddie_slots[i];

		if (creature->item_num == NO_ITEM) {
			continue;
		}

		target = &items[creature->item_num];

		if (target->room_number == item->room_number) {
			creature->alerted = true;
			continue;
		}

		dx = (target->pos.x_pos - item->pos.x_pos) >> 6;
		dy = (target->pos.y_pos - item->pos.y_pos) >> 6;
		dz = (target->pos.z_pos - item->pos.z_pos) >> 6;
		dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

		if (dist < 8000)
			creature->alerted = true;
	}
}

int16_t AIGuard(CREATURE_INFO* creature) {
	int32_t rnd;

	if (items[creature->item_num].ai_bits & MODIFY)
		return 0;

	rnd = GetRandomControl();

	if (rnd < CLICK_SIZE) {
		creature->head_left = true;
		creature->head_right = true;
	} else if (rnd < (CLICK_SIZE + HALF_CLICK_SIZE)) {
		creature->head_left = true;
		creature->head_right = false;
	} else if (rnd < HALF_BLOCK_SIZE) {
		creature->head_left = false;
		creature->head_right = true;
	}

	if (creature->head_left && creature->head_right)
		return 0;

	if (creature->head_left)
		return -0x4000;

	if (creature->head_right)
		return 0x4000;

	return 0;
}

void FindAITargetObject(CREATURE_INFO* creature, int16_t obj_num) {
	ITEM_INFO* item;
	ITEM_INFO* enemy;
	AIOBJECT* aiObj;
	ROOM_INFO* r;
	int16_t* zone;
	int16_t zone_number, ai_zone;

	item = &items[creature->item_num];

	for (int32_t i = 0; i < nAIObjects; i++) {
		aiObj = &AIObjects[i];

		if (aiObj->object_number != obj_num || aiObj->trigger_flags != item->item_flags[3] || aiObj->room_number == 255)
			continue;

		zone = ground_zone[creature->LOT.zone][flip_status];

		r = &room[item->room_number];
		item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((item->pos.x_pos - r->x) >> WALL_SHIFT)].box;
		zone_number = zone[item->box_number];

		r = &room[aiObj->room_number];
		aiObj->box_number = r->floor[((aiObj->z - r->z) >> WALL_SHIFT) + r->x_size * ((aiObj->x - r->x) >> WALL_SHIFT)].box;
		ai_zone = zone[aiObj->box_number];

		if (zone_number != ai_zone)
			continue;

		creature->enemy = &creature->ai_target;
		enemy = creature->enemy;
		enemy->object_number = aiObj->object_number;
		enemy->room_number = aiObj->room_number;
		enemy->pos.x_pos = aiObj->x;
		enemy->pos.y_pos = aiObj->y;
		enemy->pos.z_pos = aiObj->z;
		enemy->pos.y_rot = aiObj->y_rot;
		enemy->flags = aiObj->flags;
		enemy->trigger_flags = aiObj->trigger_flags;
		enemy->box_number = aiObj->box_number;

		if (!(enemy->flags & 0x20)) {
			enemy->pos.x_pos += CLICK_SIZE * phd_sin(enemy->pos.y_rot) >> W2V_SHIFT;
			enemy->pos.z_pos += CLICK_SIZE * phd_cos(enemy->pos.y_rot) >> W2V_SHIFT;
		}

		break;
	}
}

void GetAITarget(CREATURE_INFO* creature) {
	ITEM_INFO* item;
	ITEM_INFO* enemy;
	int16_t enemy_object;
	int8_t ai_bits;

	enemy = creature->enemy;

	if (enemy) {
		enemy_object = enemy->object_number;
	} else {
		enemy_object = NO_ITEM;
	}

	item = &items[creature->item_num];
	ai_bits = item->ai_bits;

	if (ai_bits & GUARD) {
		if (creature->alerted) {
			item->ai_bits &= ~GUARD;

			if (ai_bits & AMBUSH) {
				item->ai_bits |= MODIFY;
			}
		}
	} else if (ai_bits & PATROL1) {
		if (creature->alerted || creature->hurt_by_lara) {
			item->ai_bits &= ~PATROL1;

			if (ai_bits & AMBUSH) {
				item->ai_bits |= MODIFY;
			}
		} else if (!creature->patrol2 && enemy_object != AI_PATROL1) {
			FindAITargetObject(creature, AI_PATROL1);
		} else if (creature->patrol2 && enemy_object != AI_PATROL2) {
			FindAITargetObject(creature, AI_PATROL2);
		} else if (abs(enemy->pos.x_pos - item->pos.x_pos) < NAVIGATION_GOAL_RADIUS
		           && abs(enemy->pos.y_pos - item->pos.y_pos) < NAVIGATION_GOAL_RADIUS
		           && abs(enemy->pos.z_pos - item->pos.z_pos) < NAVIGATION_GOAL_RADIUS) {
			GetHeight(GetFloor(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, &enemy->room_number),
			          enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos);
			TestTriggers(trigger_index, true, 0);
			creature->patrol2 = ~creature->patrol2;
		}
	} else if (ai_bits & AMBUSH) {
		if (enemy_object != AI_AMBUSH) {
			FindAITargetObject(creature, AI_AMBUSH);
		} else if (abs(enemy->pos.x_pos - item->pos.x_pos) < NAVIGATION_GOAL_RADIUS
				&& abs(enemy->pos.y_pos - item->pos.y_pos) < NAVIGATION_GOAL_RADIUS
				&& abs(enemy->pos.z_pos - item->pos.z_pos) < NAVIGATION_GOAL_RADIUS) {

			GetHeight(GetFloor(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, &enemy->room_number),
			          enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos);
			TestTriggers(trigger_index, true, 0);
			creature->reached_goal = true;
			creature->enemy = lara_item;
			item->ai_bits &= ~AMBUSH;

			if (item->ai_bits != MODIFY) {
				item->ai_bits |= GUARD;
				creature->alerted = false;
			}
		}
	} else if (ai_bits & FOLLOW) {
		if (creature->hurt_by_lara) {
			creature->enemy = lara_item;
			creature->alerted = true;
			item->ai_bits &= ~FOLLOW;
		} else if (item->hit_status) {
			item->ai_bits &= ~FOLLOW;
		} else if (enemy_object != AI_FOLLOW) {
			FindAITargetObject(creature, AI_FOLLOW);
		} else if (abs(enemy->pos.x_pos - item->pos.x_pos) < NAVIGATION_GOAL_RADIUS
				&& abs(enemy->pos.y_pos - item->pos.y_pos) < NAVIGATION_GOAL_RADIUS
				&& abs(enemy->pos.z_pos - item->pos.z_pos) < NAVIGATION_GOAL_RADIUS) {
			creature->reached_goal = true;
			item->ai_bits &= ~FOLLOW;
		}
	}
}

int16_t SameZone(CREATURE_INFO* creature, ITEM_INFO* target_item) {
	ITEM_INFO* item;
	ROOM_INFO* r;
	int16_t* zone;

	zone = ground_zone[creature->LOT.zone][flip_status];
	item = &items[creature->item_num];

	r = &room[item->room_number];
	item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((item->pos.x_pos - r->x) >> WALL_SHIFT)].box;

	r = &room[target_item->room_number];
	target_item->box_number = r->floor[((target_item->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((target_item->pos.x_pos - r->x) >> WALL_SHIFT)].box;

	return zone[item->box_number] == zone[target_item->box_number];
}

void CreatureYRot(PHD_3DPOS* srcpos, int16_t angle, int16_t angadd) {
	if (angle > angadd)
		srcpos->y_rot += angadd;
	else if (angle < -angadd)
		srcpos->y_rot -= angadd;
	else
		srcpos->y_rot += angle;
}

int32_t MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int32_t velocity, int16_t angdif, int32_t angadd) {
	int32_t x, y, z, dist;

	x = destpos->x_pos - srcpos->x_pos;
	y = destpos->y_pos - srcpos->y_pos;
	z = destpos->z_pos - srcpos->z_pos;
	dist = phd_sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

	if (velocity < dist) {
		srcpos->x_pos += velocity * x / dist;
		srcpos->y_pos += velocity * y / dist;
		srcpos->z_pos += velocity * z / dist;
	} else {
		srcpos->x_pos = destpos->x_pos;
		srcpos->y_pos = destpos->y_pos;
		srcpos->z_pos = destpos->z_pos;
	}

	if (angdif > angadd)
		srcpos->y_rot += (int16_t)angadd;
	else if (angdif < -angadd)
		srcpos->y_rot -= (int16_t)angadd;
	else
		srcpos->y_rot = destpos->y_rot;

	return srcpos->x_pos == destpos->x_pos && srcpos->y_pos == destpos->y_pos &&
	       srcpos->z_pos == destpos->z_pos && srcpos->y_rot == destpos->y_rot;
}
