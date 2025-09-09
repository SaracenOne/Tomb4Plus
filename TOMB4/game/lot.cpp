#include "../tomb4/pch.h"
#include "lot.h"
#include "../specific/function_stubs.h"
#include "objects.h"
#include "box.h"
#include "camera.h"
#include "lara.h"
#include "control.h"

CREATURE_INFO* baddie_slots;

static int32_t slots_used = 0;

void InitialiseLOTarray(bool allocmem) {
	CREATURE_INFO* creature;

	if (allocmem) {
		baddie_slots = (CREATURE_INFO*)game_malloc(MAXIMUM_BADDIES * sizeof(CREATURE_INFO));
	}

	for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
		creature = &baddie_slots[i];
		creature->item_num = NO_ITEM;

		if (allocmem) {
			creature->LOT.node = (BOX_NODE*)game_malloc(sizeof(BOX_NODE) * num_boxes);
		}
	}

	slots_used = 0;
}

void DisableBaddieAI(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* creature;

	item = &items[item_number];
	creature = (CREATURE_INFO*)item->data;
	item->data = 0;

	if (creature) {
		creature->item_num = NO_ITEM;
		slots_used--;
	}
}

void ClearLOT(LOT_INFO* lot) {
	BOX_NODE* node;

	lot->tail = NO_BOX;
	lot->head = NO_BOX;
	lot->search_number = 0;
	lot->target_box = NO_BOX;
	lot->required_box = NO_BOX;
	node = lot->node;

	for (int32_t i = 0; i < num_boxes; i++) {
		node->next_expansion = NO_BOX;
		node->exit_box = NO_BOX;
		node->search_number = 0;
		node++;
	}
}

void CreateZone(ITEM_INFO* item) {
	CREATURE_INFO* creature;
	ROOM_INFO* r;
	BOX_NODE* node;
	int16_t* zone;
	int16_t* flip;
	int16_t zone_number, flip_number;

	creature = (CREATURE_INFO*)item->data;
	r = &room[item->room_number];
	item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((item->pos.x_pos - r->x) >> WALL_SHIFT)].box;

	if (creature->LOT.fly) {
		creature->LOT.zone_count = 0;
		node = creature->LOT.node;

		for (int32_t i = 0; i < num_boxes; i++) {
			node->box_number = i;
			node++;
			creature->LOT.zone_count++;
		}
	} else {
		zone = ground_zone[creature->LOT.zone][0];
		flip = ground_zone[creature->LOT.zone][1];
		zone_number = zone[item->box_number];
		flip_number = flip[item->box_number];
		creature->LOT.zone_count = 0;
		node = creature->LOT.node;

		for (int32_t i = 0; i < num_boxes; i++) {
			if (*zone == zone_number || *flip == flip_number) {
				node->box_number = i;
				node++;
				creature->LOT.zone_count++;
			}

			zone++;
			flip++;
		}
	}
}

void InitialiseSlot(int16_t item_number, int32_t slot) {
	ITEM_INFO* item;
	CREATURE_INFO* creature;

	creature = &baddie_slots[slot];
	item = &items[item_number];
	item->data = creature;
	creature->item_num = item_number;
	creature->mood = BORED_MOOD;
	creature->joint_rotation[0] = 0;
	creature->joint_rotation[1] = 0;
	creature->joint_rotation[2] = 0;
	creature->joint_rotation[3] = 0;
	creature->alerted = false;
	creature->head_left = false;
	creature->head_right = false;
	creature->reached_goal = false;
	creature->hurt_by_lara = false;
	creature->patrol2 = false;
	creature->jump_ahead = false;
	creature->monkey_ahead = false;
	creature->LOT.can_jump = false;
	creature->LOT.can_monkey = false;
	creature->LOT.is_jumping = false;
	creature->LOT.is_monkeying = false;
	creature->maximum_turn = DEGREES_TO_ROTATION(1);
	creature->flags = 0;
	creature->enemy = NULL;
	creature->LOT.step = CLICK_SIZE;
	creature->LOT.drop = -HALF_BLOCK_SIZE;
	creature->LOT.block_mask = BLOCKED;
	creature->LOT.fly = 0;
	creature->LOT.zone = BASIC_ZONE;

	switch (item->object_number) {
		case ENEMY_JEEP:
		case VON_CROY:
		case BADDY_1:
		case BADDY_2:
			creature->LOT.step = BLOCK_SIZE;
			creature->LOT.drop = -BLOCK_SIZE;
			creature->LOT.can_jump = true;
			creature->LOT.can_monkey = true;
			creature->LOT.zone = HUMAN_ZONE;
			break;

		case GUIDE:
		case MUMMY:
		case SPHINX:
		case HORSEMAN:
		case SCORPION:
		case TROOPS:
		case KNIGHTS_TEMPLAR:
		case MUTANT:
		case WILD_BOAR:
		case DEMIGOD1:
		case DEMIGOD2:
		case DEMIGOD3:
		case DOG:
			creature->LOT.step = CLICK_SIZE;
			creature->LOT.drop = -HALF_BLOCK_SIZE;
			creature->LOT.zone = BASIC_ZONE;
			break;

		case SKELETON:
		case SETHA:
			creature->LOT.step = CLICK_SIZE;
			creature->LOT.drop = -HALF_BLOCK_SIZE;
			creature->LOT.can_jump = true;
			creature->LOT.zone = SKELLY_ZONE;
			break;

		case CROCODILE:
			creature->LOT.step = (BLOCK_SIZE * 20);
			creature->LOT.drop = -(BLOCK_SIZE * 20);
			creature->LOT.fly = QUARTER_CLICK_SIZE / 2;
			creature->LOT.zone = CROC_ZONE;
			break;

		case BAT:
			creature->LOT.step = (BLOCK_SIZE * 20);
			creature->LOT.drop = -(BLOCK_SIZE * 20);
			creature->LOT.fly = QUARTER_CLICK_SIZE / 4;
			creature->LOT.zone = FLYER_ZONE;
			break;

		case HARPY:
		case BIG_BEETLE:
		case HAMMERHEAD:
			creature->LOT.step = (BLOCK_SIZE * 20);
			creature->LOT.drop = -(BLOCK_SIZE * 20);
			creature->LOT.fly = QUARTER_CLICK_SIZE / 2;
			creature->LOT.zone = FLYER_ZONE;
			break;
	}

	ClearLOT(&creature->LOT);

	if (item_number != lara.item_number)
		CreateZone(item);

	slots_used++;
}

int32_t EnableBaddieAI(int16_t item_number, int32_t Always) {
	ITEM_INFO* item;
	CREATURE_INFO* creature;
	int32_t x, y, z, slot, worstslot, dist, worstdist;

	item = &items[item_number];

	if (item->data)
		return 1;

	if (slots_used < MAXIMUM_BADDIES) {
		for (slot = 0; slot < MAXIMUM_BADDIES; slot++) {
			creature = &baddie_slots[slot];

			if (creature->item_num == NO_ITEM) {
				InitialiseSlot(item_number, slot);
				return 1;
			}
		}
	}

	if (Always)
		worstdist = 0;
	else {
		x = (item->pos.x_pos - camera.pos.x) >> 8;
		y = (item->pos.y_pos - camera.pos.y) >> 8;
		z = (item->pos.z_pos - camera.pos.z) >> 8;
		worstdist = SQUARE(x) + SQUARE(y) + SQUARE(z);
	}

	worstslot = -1;

	for (slot = 0; slot < MAXIMUM_BADDIES; slot++) {
		creature = &baddie_slots[slot];
		item = &items[creature->item_num];
		x = (item->pos.x_pos - camera.pos.x) >> 8;
		y = (item->pos.y_pos - camera.pos.y) >> 8;
		z = (item->pos.z_pos - camera.pos.z) >> 8;
		dist = SQUARE(x) + SQUARE(y) + SQUARE(z);

		if (dist > worstdist) {
			worstslot = slot;
			worstdist = dist;
		}
	}

	if (worstslot >= 0) {
		items[baddie_slots[worstslot].item_num].status = ITEM_INVISIBLE;
		DisableBaddieAI(baddie_slots[worstslot].item_num);
		InitialiseSlot(item_number, worstslot);
		return 1;
	}

	return 0;
}
