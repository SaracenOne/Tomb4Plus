#include "../../../tomb4/pch.h"
#include "../../../global/types.h"
#include "../../control.h"
#include "../../lara.h"
#include "../../lara_states.h"
#include "../../../specific/input.h"
#include "../../collide.h"
#include "../../delstuff.h"
#include "../../../specific/3dmath.h"

// Imported from Troye's Tomb5 project

void lara_as_parallelbars(ITEM_INFO* item, COLL_INFO* coll) {
	// Added to better match TRNG
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;

	if (!(input & IN_ACTION))
		item->goal_anim_state = AS_PBLEAP;
}

void lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll) {
	ITEM_INFO* pitem;
	int32_t Dist;

	int item_num = lara.GeneralPtr;
	pitem = &items[item_num];

	item->gravity_status = 1;

	if (item->frame_number == anims[item->anim_number].frame_base) {
		if (item->pos.y_rot == pitem->pos.y_rot)
			Dist = pitem->trigger_flags / 100 - 2;
		else
			Dist = pitem->trigger_flags % 100 - 2;

		item->fallspeed = -(int16_t(20 * Dist + 64));
		item->speed = int16_t(20 * Dist + 58);
	}

	if (item->frame_number == anims[item->anim_number].frame_end) {
		item->pos.x_pos += 700 * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
		item->pos.y_pos -= 361;
		item->pos.z_pos += 700 * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
		item->anim_number = ANIM_GRABLOOP;
		item->frame_number = anims[ANIM_GRABLOOP].frame_base;
		item->goal_anim_state = AS_REACH;
		item->current_anim_state = AS_REACH;
	}
}

static int16_t ParallelBarsBounds[12] = { -640, 640, 704, 832, -96, 96, -1820, 1820, -5460, 5460, -1820, 1820 };
static int16_t PoleBounds[12] = { -256, 256, 0, 0, -512, 512, -1820, 1820, -5460, 5460, -1820, 1820 };
static PHD_VECTOR PolePos = { 0, 0, -208 };
static PHD_VECTOR PolePosR = { 0, 0, 0 };

void ParallelBarsCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	int16_t pass, pass1;

	item = &items[item_num];

	if (!(input & IN_ACTION) || l->current_anim_state != AS_REACH || l->anim_number != ANIM_GRABLOOP) {
		if (l->current_anim_state != AS_PBSPIN)
			ObjectCollision(item_num, l, coll);

		return;
	}

	pass = (int16_t)TestLaraPosition(ParallelBarsBounds, item, l);

	if (!pass) {
		item->pos.y_rot += 0x8000;
		pass1 = (int16_t)TestLaraPosition(ParallelBarsBounds, item, l);
		item->pos.y_rot += 0x8000;

		if (!pass1) {
			ObjectCollision(item_num, l, coll);
			return;
		}
	}

	l->current_anim_state = AS_CONTROLLED;
	l->anim_number = ANIM_PB_GRAB;
	l->frame_number = anims[ANIM_PB_GRAB].frame_base;
	l->fallspeed = 0;
	l->gravity_status = 0;
	lara.head_x_rot = 0;
	lara.head_y_rot = 0;
	lara.torso_x_rot = 0;
	lara.torso_y_rot = 0;

	if (pass)
		l->pos.y_rot = item->pos.y_rot;
	else
		l->pos.y_rot = item->pos.y_rot + 0x8000;

	pos.x = 0;
	pos.y = -HALF_CLICK_SIZE;
	pos.z = HALF_BLOCK_SIZE;
	GetLaraJointPos(&pos, LMX_HAND_L);

	pos2.x = 0;
	pos2.y = -HALF_CLICK_SIZE;
	pos2.z = HALF_BLOCK_SIZE;
	GetLaraJointPos(&pos2, LMX_HAND_R);

	if (l->pos.y_rot & 0x4000)
		l->pos.x_pos += item->pos.x_pos - ((pos.x + pos2.x) >> 1);
	else
		l->pos.z_pos += item->pos.z_pos - ((pos.z + pos2.z) >> 1);

	l->pos.y_pos += item->pos.y_pos - ((pos.y + pos2.y) >> 1);

	lara.GeneralPtr = item_num;
}