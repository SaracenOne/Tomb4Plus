#include "../../../tomb4/pch.h"
#include "locusts.h"
#include "../../objects.h"
#include "../../control.h"
#include "../../../specific/3dmath.h"
#include "../../effects.h"
#include "../../../specific/function_stubs.h"
#include "../../sphere.h"
#include "../../items.h"
#include "../../../specific/output.h"
#include "../../draw.h"
#include "../../tomb4fx.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"
#include "../../../tomb4/tomb4plus/t4plus_objects.h"

LOCUST_STRUCT Locusts[MAX_LOCUSTS];

static int32_t next_locust = 0;

int32_t GetFreeLocust() {
	LOCUST_STRUCT* fx;

	fx = &Locusts[next_locust];

	for (int32_t free = next_locust, i = 0; i < MAX_LOCUSTS; i++) {
		if (fx->On) {
			if (free == 63) {
				fx = Locusts;
				free = 0;
			} else {
				free++;
				fx++;
			}
		} else {
			next_locust = (free + 1) & 0x3F;
			return free;
		}
	}

	return NO_ITEM;
}

void TriggerLocust(ITEM_INFO* item) {
	LOCUST_STRUCT* fx;
	PHD_VECTOR vec;
	PHD_VECTOR vec2;
	int32_t fx_number;
	int16_t angles[2];

	fx_number = GetFreeLocust();

	if (fx_number == NO_ITEM)
		return;

	fx = &Locusts[fx_number];

	if (item->object_number == FISH) {
		vec.x = item->pos.x_pos;
		vec.y = item->pos.y_pos;
		vec.z = item->pos.z_pos;
		*(int32_t*)angles = item->pos.y_rot + 0x8000;
	} else {
		vec2.x = 0;
		vec2.y = -96;
		vec2.z = 144;
		GetJointAbsPosition(item, &vec2, 9);
		vec.x = 0;
		vec.y = -128;
		vec.z = 288;
		GetJointAbsPosition(item, &vec, 9);
		phd_GetVectorAngles(vec.x - vec2.x, vec.y - vec2.y, vec.z - vec2.z, angles);
	}

	fx->room_number = item->room_number;
	fx->pos.x_pos = vec.x;
	fx->pos.y_pos = vec.y;
	fx->pos.z_pos = vec.z;
	fx->pos.x_rot = (GetRandomControl() & 0x3FF) + angles[1] - HALF_BLOCK_SIZE;
	fx->pos.y_rot = (GetRandomControl() & 0x7FF) + angles[0] - BLOCK_SIZE;
	fx->On = 1;
	fx->flags = 0;
	fx->speed = (GetRandomControl() & 0x1F) + 16;
	fx->LaraTarget = GetRandomControl() & 0x1FF;
	fx->Counter = 20 * ((GetRandomControl() & 7) + 15);
}

void InitialiseLocustEmitter(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (!item->pos.y_rot)
		item->pos.z_pos += HALF_BLOCK_SIZE;
	else if (item->pos.y_rot == 0x4000)
		item->pos.x_pos += HALF_BLOCK_SIZE;
	else if (item->pos.y_rot == -0x8000)
		item->pos.z_pos -= HALF_BLOCK_SIZE;
	else if (item->pos.y_rot == -0x4000)
		item->pos.x_pos -= HALF_BLOCK_SIZE;
}

void ControlLocustEmitter(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->trigger_flags) {
		TriggerLocust(item);
		item->trigger_flags--;
	} else
		KillItem(item_number);
}

void DrawLocusts() {
	LOCUST_STRUCT* fx;
	int16_t** meshpp;

	for (int32_t i = 0; i < MAX_LOCUSTS; i++) {
		fx = &Locusts[i];

		if (fx->On) {
			meshpp = &meshes[objects[AHMET_MIP].mesh_index + 2 * (-GlobalCounter & 3)];
			phd_PushMatrix();
			phd_TranslateAbs(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
			phd_RotYXZ(fx->pos.y_rot, fx->pos.x_rot, fx->pos.z_rot);
			phd_PutPolygons_train(*meshpp, 0);
			phd_PopMatrix();
		}
	}
}

void UpdateLocusts() {
	LOCUST_STRUCT* fx;
	int16_t* lb;
	int32_t bounds[6];
	int32_t speed, ox, oy, oz, closestdist, closestnum;
	int16_t angles[2];
	int16_t max_turn;

	lb = GetBoundsAccurate(lara_item);
	bounds[0] = lb[0] - (lb[0] >> 2) + lara_item->pos.x_pos;
	bounds[1] = lb[1] - (lb[1] >> 2) + lara_item->pos.x_pos;
	bounds[2] = lb[2] - (lb[2] >> 2) + lara_item->pos.y_pos;
	bounds[3] = lb[3] - (lb[3] >> 2) + lara_item->pos.y_pos;
	bounds[4] = lb[4] - (lb[4] >> 2) + lara_item->pos.z_pos;
	bounds[5] = lb[5] - (lb[5] >> 2) + lara_item->pos.z_pos;
	closestdist = 0xFFFFFFF;
	closestnum = -1;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, FISH);

	for (int32_t i = 0; i < MAX_LOCUSTS; i++) {
		fx = &Locusts[i];

		if (fx->On) {
			if ((lara.burn || lara_item->hit_points <= 0) && fx->Counter > 90 && !(GetRandomControl() & 7))
				fx->Counter = 90;

			fx->Counter--;

			if (!fx->Counter) {
				fx->On = 0;
				continue;
			}

			if (!(GetRandomControl() & 7)) {
				fx->LaraTarget = (GetRandomControl() % (HALF_BLOCK_SIZE + HALF_CLICK_SIZE)) + HALF_CLICK_SIZE;
				fx->XTarget = (GetRandomControl() & 0x7F) - QUARTER_CLICK_SIZE;
				fx->ZTarget = (GetRandomControl() & 0x7F) - QUARTER_CLICK_SIZE;
			}

			phd_GetVectorAngles(
			    lara_item->pos.x_pos + (fx->XTarget << 3) - fx->pos.x_pos,
			    lara_item->pos.y_pos - fx->LaraTarget - fx->pos.y_pos,
			    lara_item->pos.z_pos + (fx->ZTarget << 3) - fx->pos.z_pos,
			    angles);

			ox = SQUARE(lara_item->pos.x_pos - fx->pos.x_pos);
			oz = SQUARE(lara_item->pos.z_pos - fx->pos.z_pos);

			if (ox + oz < closestdist) {
				closestdist = ox + oz;
				closestnum = i;
			}

			ox = phd_sqrt(ox + oz) >> 3;

			if (ox > 128)
				ox = 128;
			else if (ox < 48)
				ox = 48;

			if (fx->speed < ox)
				fx->speed++;
			else if (fx->speed > ox)
				fx->speed--;

			if (fx->Counter > 90) {
				max_turn = fx->speed << 7;
				oy = (uint16_t)angles[0] - (uint16_t)fx->pos.y_rot;

				if (abs(oy) > 0x8000)
					oy = (uint16_t)fx->pos.y_rot - (uint16_t)angles[0];

				ox = (uint16_t)angles[1] - (uint16_t)fx->pos.x_rot;

				if (abs(ox) > 0x8000)
					ox = (uint16_t)fx->pos.x_rot - (uint16_t)angles[0];

				ox >>= 3;
				oy >>= 3;

				if (oy > max_turn)
					oy = max_turn;
				else if (oy < -max_turn)
					oy = -max_turn;
				if (ox > max_turn)
					ox = max_turn;
				else if (ox < -max_turn)
					ox = -max_turn;

				fx->pos.y_rot += (int16_t)oy;
				fx->pos.x_rot += (int16_t)ox;
			}

			ox = fx->pos.x_pos;
			oy = fx->pos.y_pos;
			oz = fx->pos.z_pos;
			speed = fx->speed * phd_cos(fx->pos.x_rot) >> W2V_SHIFT;
			fx->pos.x_pos += speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT;
			fx->pos.y_pos += fx->speed * phd_sin(-fx->pos.x_rot) >> W2V_SHIFT;
			fx->pos.z_pos += speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT;

			if (!(i & 1)) {
				if (fx->pos.x_pos > bounds[0] && fx->pos.x_pos < bounds[1] && fx->pos.y_pos > bounds[2] &&
				        fx->pos.y_pos < bounds[3] && fx->pos.z_pos > bounds[4] && fx->pos.z_pos < bounds[5]) {
					TriggerBlood(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, GetRandomControl() << 1, 2);

					if (lara_item->hit_points > 0)
						lara_item->hit_points -= mod_object_customization->damage_1;
				}
			}
		}
	}

	if (closestnum != -1) {
		fx = &Locusts[closestnum];
		SoundEffect(SFX_LOCUSTS_LOOP, &fx->pos, SFX_DEFAULT);
	}
}