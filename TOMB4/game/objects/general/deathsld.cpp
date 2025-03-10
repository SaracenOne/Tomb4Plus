#include "../../../tomb4/pch.h"
#include "deathsld.h"
#include "../../../specific/function_stubs.h"
#include "../../lara_states.h"
#include "../../collide.h"
#include "../../laramisc.h"
#include "../../items.h"
#include "../../control.h"
#include "../../sound.h"
#include "../../../specific/3dmath.h"
#include "../../../specific/input.h"
#include "../../lara.h"
#include "../../objects.h"

static int16_t DeathSlideBounds[] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	-100,
	100,
	CLICK_SIZE,
	HALF_BLOCK_SIZE,
	0,
	0,
	-DEGREES_TO_ROTATION(25),
	DEGREES_TO_ROTATION(25),
	0,
	0
};
static PHD_VECTOR DeathSlidePosition = { 0, 0, 371 };

void InitialiseDeathSlide(int16_t item_number) {
	ITEM_INFO* item;
	GAME_VECTOR* old;

	item = &items[item_number];
	old = (GAME_VECTOR*)game_malloc(sizeof(GAME_VECTOR));
	item->data = old;
	old->x = item->pos.x_pos;
	old->y = item->pos.y_pos;
	old->z = item->pos.z_pos;
	old->room_number = item->room_number;
}

void DeathSlideCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	if (input & IN_ACTION && !l->gravity_status && lara.gun_status == LG_NO_ARMS && l->current_anim_state == AS_STOP) {
		item = &items[item_number];

		if (item->status == ITEM_INACTIVE) {
			if (TestLaraPosition(DeathSlideBounds, item, l)) {
				AlignLaraPosition(&DeathSlidePosition, item, l);
				lara.gun_status = LG_HANDS_BUSY;
				l->goal_anim_state = AS_DEATHSLIDE;

				do AnimateLara(l);
				while (l->current_anim_state != AS_NULL);

				if (!item->active)
					AddActiveItem(item_number);

				item->status = ITEM_ACTIVE;
				item->flags |= IFL_INVISIBLE;
			}
		}
	}
}

void ControlDeathSlide(int16_t item_number) {
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	GAME_VECTOR* old;
	int32_t x, y, z, h, c;
	int16_t room_number;

	item = &items[item_number];

	if (item->status != ITEM_ACTIVE)
		return;

	if (item->flags & IFL_INVISIBLE) {
		if (item->current_anim_state == 1) {
			AnimateItem(item);
			return;
		}

		AnimateItem(item);

		if (item->fallspeed < 100)
			item->fallspeed += 5;

		item->pos.x_pos += item->fallspeed * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
		item->pos.y_pos += item->fallspeed >> 2;
		item->pos.z_pos += item->fallspeed * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
		room_number = item->room_number;
		GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (room_number != item->room_number)
			ItemNewRoom(item_number, room_number);

		if (lara_item->current_anim_state == AS_DEATHSLIDE) {
			lara_item->pos.x_pos = item->pos.x_pos;
			lara_item->pos.y_pos = item->pos.y_pos;
			lara_item->pos.z_pos = item->pos.z_pos;
		}

		x = item->pos.x_pos + (phd_sin(item->pos.y_rot) >> 4);
		y = item->pos.y_pos + QUARTER_CLICK_SIZE;
		z = item->pos.z_pos + (phd_cos(item->pos.y_rot) >> 4);
		floor = GetFloor(x, y, z, &room_number);
		h = GetHeight(floor, x, y, z);
		c = GetCeiling(floor, x, y, z);

		if (h <= y + CLICK_SIZE || c >= y - CLICK_SIZE) {
			if (lara_item->current_anim_state == AS_DEATHSLIDE) {
				lara_item->goal_anim_state = 3;
				AnimateLara(lara_item);
				lara_item->gravity_status = 1;
				lara_item->speed = item->fallspeed;
				lara_item->fallspeed = item->fallspeed >> 2;
			}

			SoundEffect(SFX_VONCROY_KNIFE_SWISH, &item->pos, SFX_DEFAULT);
			RemoveActiveItem(item_number);
			item->status = ITEM_INACTIVE;
			item->flags -= IFL_INVISIBLE;
		} else
			SoundEffect(SFX_TRAIN_DOOR_CLOSE, &item->pos, SFX_DEFAULT);
	} else {
		old = (GAME_VECTOR*)item->data;
		item->pos.x_pos = old->x;
		item->pos.y_pos = old->y;
		item->pos.z_pos = old->z;

		if (old->room_number != item->room_number)
			ItemNewRoom(item_number, old->room_number);

		item->status = ITEM_INACTIVE;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
		item->anim_number = objects[item->object_number].anim_index;
		item->frame_number = anims[item->anim_number].frame_base;
		RemoveActiveItem(item_number);
	}
}
