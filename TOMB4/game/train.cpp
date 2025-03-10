#include "../tomb4/pch.h"
#include "train.h"
#include "../specific/3dmath.h"
#include "../specific/output.h"
#include "../specific/specificfx.h"
#include "sound.h"
#include "control.h"
#include "items.h"
#include "camera.h"
#include "draw.h"
#include "lara.h"
#include "gameflow.h"
#include "objects.h"

static int16_t dels_handy_train_map[128] = {
	//36: ARCHITECTURE6
	//37: ARCHITECTURE7
	//38: ARCHITECTURE8
	//39: ARCHITECTURE9
	36, 36, 36, 37, 38, 39, 36, 37, 38, 38, 38, 39, 37,
	38, 39, 36, 36, 36, 36, 36, 37, 39, 36, 36, 37, 38,
	38, 39, 36, 36, 37, 39, 36, 36, 36, 37, 38, 39, 36,
	37, 38, 38, 38, 39, 37, 38, 39, 36, 36, 36, 36, 36,
	37, 39, 36, 36, 37, 38, 38, 39, 36, 36, 37, 39, 38,
	37, 36, 39, 37, 36, 36, 36, 39, 38, 38, 38, 38, 37,
	36, 39, 38, 38, 37, 39, 38, 38, 38, 38, 38, 38, 37,
	36, 36, 36, 39, 38, 38, 37, 36, 39, 37, 36, 36, 36,
	39, 38, 38, 38, 38, 37, 36, 39, 38, 38, 37, 39, 38,
	38, 38, 38, 38, 38, 37, 36, 36, 36, 39, 38
};

static TRAIN_STATIC dels_handy_train_map2[64] = {
	{ROCK2, (BLOCK_SIZE * 4)}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)}, {NO_ITEM, 0},
	{ROCK3, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0},
	{ROCK0, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)},
	{ROCK3, -(BLOCK_SIZE * 2)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK2, (BLOCK_SIZE * 4)}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)}, {NO_ITEM, 0},
	{ROCK3, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0},
	{ROCK0, -(BLOCK_SIZE * 3)}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)}, {NO_ITEM, 0}, {NO_ITEM, 0},
	{ROCK0, (BLOCK_SIZE * 4)},
	{ROCK3, -(BLOCK_SIZE * 2)}, {NO_ITEM, 0}, {NO_ITEM, 0}, {NO_ITEM, 0}
};

int32_t trainmappos;

void DrawTrainObjects() {
	TRAIN_STATIC* p;
	int16_t* obj;
	int32_t x, x2;

	trainmappos = (trainmappos + (gfUVRotate << 5)) % 0x60000;
	obj= &dels_handy_train_map[96 - ((trainmappos / 6144 - lara_item->pos.x_pos / 6144) & 0x1F)];
	x = trainmappos % 6144 - 24576;
	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos - lara_item->pos.x_pos % 6144, CLICK_SIZE, 47168);

	for (int i = 0; i < 8; i++) {
		phd_PutPolygons_train(meshes[static_objects[obj[0]].mesh_number], x);
		obj++;
		x += 6144;
	}

	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateAbs(x + lara_item->pos.x_pos - lara_item->pos.x_pos % 6144, CLICK_SIZE, 47168);
	phd_PutPolygons_train(meshes[static_objects[obj[0]].mesh_number], 0);
	phd_PopMatrix();

	obj = &dels_handy_train_map[32 - ((trainmappos / 6144 - lara_item->pos.x_pos / 6144 + 8) & 0x1F)];
	x = trainmappos % 6144 - 24576;
	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos - lara_item->pos.x_pos % 6144, CLICK_SIZE, 58304);
	phd_RotY(DEGREES_TO_ROTATION(180));
	x2 = x + 0xC000;

	for (int i = 0; i < 8; i++) {
		phd_PutPolygons_train(meshes[static_objects[obj[0]].mesh_number], -x);
		obj++;
		x += 6144;
	}

	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateAbs(x2 + lara_item->pos.x_pos - lara_item->pos.x_pos % 6144, CLICK_SIZE, 58304);
	phd_RotY(32760);
	phd_PutPolygons_train(meshes[static_objects[obj[0]].mesh_number], 0);
	phd_PopMatrix();

	p = &dels_handy_train_map2[32 - ((trainmappos / 6144 - lara_item->pos.x_pos / 6144 + 8) & 0x1F)];
	x = trainmappos % 6144 - 24576;
	phd_PushMatrix();

	for (int i = 0; i < 8; i++) {
		if (p->type != NO_ITEM) {
			phd_TranslateAbs(lara_item->pos.x_pos - lara_item->pos.x_pos % 6144, CLICK_SIZE, p->zoff + 52224);
			phd_PutPolygons_train(meshes[static_objects[p->type].mesh_number], x);
		}

		p++;
		x += 6144;
	}

	if (p->type != NO_ITEM) {
		phd_TranslateAbs(x + lara_item->pos.x_pos - lara_item->pos.x_pos % 6144, CLICK_SIZE, p->zoff + 52224);
		phd_PutPolygons_train(meshes[static_objects[p->type].mesh_number], 0);
	}

	phd_PopMatrix();
}

void DrawTrainFloor() {
	int32_t x = lara_item->pos.x_pos;
	lara_item->pos.x_pos = camera.pos.x;
	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos & -BLOCK_SIZE, 0, 52224);
	DrawTrainStrips();
	camera.bounce = -12;
	phd_PopMatrix();
	DrawTrainObjects();
	lara_item->pos.x_pos = x;
}

void InitialiseTrainJeep(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;

	item = &items[item_number];
	item->item_flags[0] = -80;

	for (int i = 0; i < level_items; i++) {	//find your baddy
		item2 = &items[i];

		if (item != item2 && item2->trigger_flags == item->trigger_flags) {
			item->item_flags[1] = i;
			item2->item_flags[0] = -80;
			item2->pos.y_pos = item->pos.y_pos - BLOCK_SIZE;
			return;
		}
	}
}

void TrainJeepControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	int16_t room_number;

	item = &items[item_number];
	item2 = &items[item->item_flags[1]];

	if (item->item_flags[0] == -80) {
		if (item->item_flags[2] < 0x4000)
			item->item_flags[2] += (QUARTER_CLICK_SIZE / 2);
	} else if (item->item_flags[2] > BLOCK_SIZE)
		item->item_flags[2] -= HALF_BLOCK_SIZE;

	SoundEffect(SFX_JEEP_MOVE, &item->pos, (item->item_flags[2] << 9) + (0x1000000 | SFX_SETPITCH));
	item->pos.x_pos += item->item_flags[0];
	AnimateItem(item);
	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (item2->item_flags[0] > -40 || item2->hit_points < 1)
		item->item_flags[0] += 3;

	if (item->item_flags[0] > 400)
		KillItem(item_number);
}
