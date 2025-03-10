#include "../../../tomb4/pch.h"

#include "tightrope.h"

#include "../../../global/types.h"
#include "../../control.h"
#include "../../lara.h"
#include "../../lara_states.h"
#include "../../../specific/input.h"
#include "../../collide.h"
#include "../../../specific/function_stubs.h"
#include "../../delstuff.h"

#include "../../trng/trng.h"

// Imported from Troye's Tomb5 project

LARA_TIGHTROPE_EXTRASTATE lara_tightrope_extrastate;

static void GetTighRopeFallOff(int32_t chance) {
	if (lara_item->hit_points <= 0 || lara_item->hit_status) {
		lara_item->anim_number = ANIM_TROPEFALLOFF_L;
		lara_item->frame_number = anims[ANIM_TROPEFALLOFF_L].frame_base;
		lara_item->current_anim_state = AS_TROPEFALL_L;
		lara_item->goal_anim_state = AS_TROPEFALL_L;
	}

	if (!lara_tightrope_extrastate.TightRopeFall && !(GetRandomControl() & chance)) {
		if (GetRandomControl() & 0xF)
			lara_tightrope_extrastate.TightRopeFall = 1;
		else
			lara_tightrope_extrastate.TightRopeFall = 2;
	}
}

void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll) {
	// Added to better match TRNG
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;

	if (input & IN_LOOK)
		LookUpDown();

	GetTighRopeFallOff(127);

	if (lara_item->current_anim_state == AS_TROPEFALL_L)
		return;

	if (lara_tightrope_extrastate.TightRopeFall) {
		if (GetRandomControl() & 1)
			item->goal_anim_state = AS_TROPEFALL_R;
		else
			item->goal_anim_state = AS_TROPEFALL_L;
	} else if (input & IN_FORWARD)
		item->goal_anim_state = AS_TROPEWALK;
	else if (input & IN_ROLL || input & IN_BACK) {
		if (item->anim_number == ANIM_TRPOSE) {
			item->current_anim_state = AS_TROPETURN;
			item->anim_number = ANIM_TROPETURN;
			item->frame_number = anims[ANIM_TROPETURN].frame_base;
			GetTighRopeFallOff(1);
		}
	}
}

void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll) {
	// Added to better match TRNG
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;

	if (item->trigger_flags > 0) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "Trigger flags are not yet supported for tightropes.");
	}

	FLOOR_INFO* floor;
	int16_t room_number;

	if (lara_tightrope_extrastate.TightRopeOnCount)
		lara_tightrope_extrastate.TightRopeOnCount--;
	else if (lara_tightrope_extrastate.TightRopeOff) {
		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) == item->pos.y_pos) {
			lara_tightrope_extrastate.TightRopeOff = 0;
			item->goal_anim_state = AS_TROPEGETOFF;
		}
	} else {
		GetTighRopeFallOff(127);
	}

	if (lara_item->current_anim_state != AS_TROPEFALL_L) {
		if (input & IN_LOOK)
			LookUpDown();

		if (item->goal_anim_state != AS_TROPEGETOFF &&
		        (lara_tightrope_extrastate.TightRopeFall || (input & (IN_BACK | IN_ROLL) || !(input & IN_FORWARD)) && !lara_tightrope_extrastate.TightRopeOnCount && !lara_tightrope_extrastate.TightRopeOff))
			item->goal_anim_state = AS_TROPEPOSE;
	}
}

void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll) {
	// Added to better match TRNG
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;

	PHD_VECTOR pos;
	int32_t UndoInput, WrongInput, UndoAnim, UndoFrame;

	if (item->anim_number == ANIM_TROPEFALLOFF_L || item->anim_number == ANIM_TROPEFALLOFF_R) {
		if (item->frame_number == anims[item->anim_number].frame_end) {
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;
			GetLaraJointPos(&pos, LMX_FOOT_R);
			item->pos.x_pos = pos.x;
			item->pos.y_pos = pos.y + 75;
			item->pos.z_pos = pos.z;
			item->goal_anim_state = AS_FASTFALL;
			item->current_anim_state = AS_FASTFALL;
			item->anim_number = ANIM_FASTFALL;
			item->frame_number = anims[ANIM_FASTFALL].frame_base;
			item->fallspeed = 81;
			lara.gun_status = LG_NO_ARMS;
		}

		return;
	}

	if (lara_tightrope_extrastate.TightRopeOnCount)
		lara_tightrope_extrastate.TightRopeOnCount--;

	if (item->anim_number == ANIM_TROPEFALL_L) {
		UndoInput = IN_RIGHT;
		WrongInput = IN_LEFT;
		UndoAnim = ANIM_TROPEUNDO_L;
	} else if (item->anim_number == ANIM_TROPEFALL_R) {
		UndoInput = IN_LEFT;
		WrongInput = IN_RIGHT;
		UndoAnim = ANIM_TROPEUNDO_R;
	} else
		return;

	UndoFrame = anims[item->anim_number].frame_end + anims[UndoAnim].frame_base - item->frame_number;

	if (input & UndoInput && !lara_tightrope_extrastate.TightRopeOnCount) {
		item->anim_number = (int16_t)UndoAnim;
		item->frame_number = (int16_t)UndoFrame;
		item->current_anim_state = AS_TROPEUNDOFALL;
		item->goal_anim_state = AS_TROPEPOSE;
		lara_tightrope_extrastate.TightRopeFall--;
	} else if (input & WrongInput && lara_tightrope_extrastate.TightRopeOnCount < 10)
		lara_tightrope_extrastate.TightRopeOnCount += (GetRandomControl() & 3) + 2;
}

static int16_t TightRopeBounds[12] = { -256, 256, 0, 0, -256, 256, -1820, 1820, -5460, 5460, -1820, 1820 };
static PHD_VECTOR TightRopePos = { 0, 0, 0 };

void InitialiseTightRope(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (!item->pos.y_rot)
		item->pos.z_pos -= CLICK_SIZE;
	else if (item->pos.y_rot == 0x4000)
		item->pos.x_pos -= CLICK_SIZE;
	else if (item->pos.y_rot == -0x4000)
		item->pos.x_pos += CLICK_SIZE;
	else if (item->pos.y_rot == -0x8000)
		item->pos.z_pos += CLICK_SIZE;
}

void TightRopeCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_num];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && !l->gravity_status &&
	        lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_num) {
		item->pos.y_rot += 0x8000;

		if (TestLaraPosition(TightRopeBounds, item, l)) {
			if (MoveLaraPosition(&TightRopePos, item, l)) {
				l->current_anim_state = AS_TROPEGETON;
				l->anim_number = ANIM_TROPEGETON;
				l->frame_number = anims[ANIM_TROPEGETON].frame_base;
				lara.IsMoving = 0;
				lara.head_x_rot = 0;
				lara.head_y_rot = 0;
				lara.torso_y_rot = 0;
				lara.torso_z_rot = 0;
				lara_tightrope_extrastate.TightRopeOnCount = 60;
				lara_tightrope_extrastate.TightRopeOff = 0;
				lara_tightrope_extrastate.TightRopeFall = 0;
			} else
				lara.GeneralPtr = item_num;
		} else if (lara.IsMoving && lara.GeneralPtr == item_num)
			lara.IsMoving = 0;

		item->pos.y_rot += 0x8000;
	} else if (l->current_anim_state == AS_TROPEWALK && l->goal_anim_state != AS_TROPEGETOFF && !lara_tightrope_extrastate.TightRopeOff &&
	           item->pos.y_rot == l->pos.y_rot && ((abs(item->pos.x_pos - l->pos.x_pos) + abs(item->pos.z_pos - l->pos.z_pos)) < (HALF_BLOCK_SIZE + HALF_CLICK_SIZE)))
		lara_tightrope_extrastate.TightRopeOff = 1;
}

void SetupLaraTightropeExtraState() {
	lara_tightrope_extrastate.TightRopeFall = 0;
	lara_tightrope_extrastate.TightRopeOff = 0;
	lara_tightrope_extrastate.TightRopeOnCount = 0;
}