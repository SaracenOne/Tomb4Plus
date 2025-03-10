#include "../../../tomb4/pch.h"
#include "scarabs.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../effects.h"
#include "../../../specific/3dmath.h"
#include "../../control.h"
#include "../../../specific/output.h"
#include "../../effect2.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"

SCARAB_STRUCT Scarabs[MAX_SCARABS];

static int32_t next_scarab = 0;

int32_t GetFreeScarab() {
	SCARAB_STRUCT* fx;
	int32_t lp, free;

	free = next_scarab;
	lp = 0;
	fx = &Scarabs[free];

	while (fx->On) {
		if (free == (MAX_SCARABS - 1)) {
			free = 0;
			fx = Scarabs;
		} else {
			free++;
			fx++;
		}

		lp++;

		if (lp >= MAX_SCARABS)
			return -1;
	}

	next_scarab = (free + 1) & 0x7F;
	return free;
}

void ClearScarabs() {
	for (int i = 0; i < MAX_SCARABS; i++)
		Scarabs[i].On = 0;

	next_scarab = 0;
	flipeffect = -1;
}

void TriggerScarab(int16_t item_number) {
	SCARAB_STRUCT* fx;
	ITEM_INFO* item;
	int16_t fx_num;

	item = &items[item_number];

	if (item->trigger_flags && (!item->item_flags[2] || !(GetRandomControl() & 0xF))) {
		item->trigger_flags--;

		if (item->item_flags[2] && GetRandomControl() & 1)
			item->item_flags[2]--;

		fx_num = (int16_t)GetFreeScarab();

		if (fx_num != -1) {
			fx = &Scarabs[fx_num];
			fx->pos.x_pos = item->pos.x_pos;
			fx->pos.y_pos = item->pos.y_pos;
			fx->pos.z_pos = item->pos.z_pos;
			fx->room_number = item->room_number;

			if (item->item_flags[0]) {
				fx->pos.y_rot = int16_t(GetRandomControl() << 1);
				fx->fallspeed = -16 - (GetRandomControl() & 0x1F);
			} else {
				fx->fallspeed = 0;
				fx->pos.y_rot = (GetRandomControl() & 0x3FFF) + item->pos.y_rot - 8192;
			}

			fx->pos.x_rot = 0;
			fx->pos.z_rot = 0;
			fx->On = 1;
			fx->flags = 0;
			fx->speed = (GetRandomControl() & 0x1F) + 1;
		}
	}
}

void UpdateScarabs() {
	SCARAB_STRUCT* fx;
	FLOOR_INFO* floor;
	int32_t h, dx, dy, dz, oldx, oldy, oldz;
	int16_t angle;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, LITTLE_BEETLE);

	for (int i = 0; i < MAX_SCARABS; i++) {
		fx = &Scarabs[i];

		if (fx->On) {
			oldx = fx->pos.x_pos;
			oldy = fx->pos.y_pos;
			oldz = fx->pos.z_pos;
			fx->pos.x_pos += fx->speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT;
			fx->pos.y_pos += fx->fallspeed;
			fx->pos.z_pos += fx->speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT;
			fx->fallspeed += 6;
			dx = lara_item->pos.x_pos - fx->pos.x_pos;
			dy = lara_item->pos.y_pos - fx->pos.y_pos;
			dz = lara_item->pos.z_pos - fx->pos.z_pos;
			angle = (int16_t)phd_atan(dz, dx) - fx->pos.y_rot;

			if (abs(dz) < 85 && abs(dy) < 85 && abs(dx) < 85) {
				lara_item->hit_points -= mod_object_customization->damage_1;
				lara_item->hit_status = 1;
			}

			if (fx->flags) {
				if (abs(dx) + abs(dz) > 0x400) {
					if (fx->speed < (i & 0x1F) + 24)
						fx->speed++;

					if (abs(angle) < 0x1000)
						fx->pos.y_rot += int16_t((wibble - i) << 3);
					else if (angle < 0)
						fx->pos.y_rot -= 0x400;
					else
						fx->pos.y_rot += 0x400;
				} else {
					fx->pos.y_rot += fx->speed & 1 ? HALF_BLOCK_SIZE : -HALF_BLOCK_SIZE;
					fx->speed = 48 - (lara.LitTorch << 6) - (abs(angle) >> 7);

					if (fx->speed < -16)
						fx->speed = i & 0xF;
				}
			}

			floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &fx->room_number);
			h = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

			if (h < fx->pos.y_pos - 1280 || h == NO_HEIGHT) {
				fx->pos.y_rot += angle > 0 ? 0x4000 : -0x4000;
				fx->pos.x_pos = oldx;
				fx->pos.y_pos = oldy;
				fx->pos.z_pos = oldz;
				fx->fallspeed = 0;
			} else if (fx->pos.y_pos > h) {
				fx->pos.y_pos = h;
				fx->fallspeed = 0;
				fx->flags = 1;
			}

			if (fx->fallspeed < 500)
				fx->pos.x_rot = -0x40 * fx->fallspeed;
			else {
				fx->On = 0;
				next_scarab = 0;
			}
		}
	}
}

void DrawScarabs() {
	SCARAB_STRUCT* fx;
	int16_t** meshpp;

	meshpp = &meshes[objects[LITTLE_BEETLE].mesh_index + (wibble >> 2 & 2)];

	for (int i = 0; i < MAX_SCARABS; i++) {
		fx = &Scarabs[i];

		if (fx->On) {
			phd_PushMatrix();
			phd_TranslateAbs(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
			phd_RotYXZ(fx->pos.y_rot, fx->pos.x_rot, fx->pos.z_rot);
			phd_PutPolygons_train(*meshpp, 0);
			phd_PopMatrix();
		}
	}
}

void InitialiseScarabGenerator(int16_t item_number) {
	ITEM_INFO* item;
	int16_t flag;

	item = &items[item_number];
	flag = item->trigger_flags / 1000;
	item->pos.x_rot = 0x2000;
	item->item_flags[0] = flag & 1;
	item->item_flags[1] = flag & 2;
	item->item_flags[2] = flag & 4;
	item->trigger_flags %= 1000;

	if (!item->item_flags[0]) {
		if (item->pos.y_rot > 4096 && item->pos.y_rot < 28672)
			item->pos.x_pos -= HALF_BLOCK_SIZE;
		else if (item->pos.y_rot < -4096 && item->pos.y_rot > -28672)
			item->pos.x_pos += HALF_BLOCK_SIZE;

		if (item->pos.y_rot > -8192 && item->pos.y_rot < 8192)
			item->pos.z_pos -= HALF_BLOCK_SIZE;
		else if (item->pos.y_rot < -(BLOCK_SIZE * 20) || item->pos.y_rot >(BLOCK_SIZE * 20))
			item->pos.z_pos += HALF_BLOCK_SIZE;
	}
}
