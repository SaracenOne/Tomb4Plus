#include "../../../tomb4/pch.h"
#include "draggable_body.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../control.h"
#include "../../lara_states.h"
#include "../../switch.h"
#include "../../items.h"
#include "../../collide.h"
#include "../../sphere.h"
#include "../../box.h"
#include "../../../specific/3dmath.h"
#include "../../../specific/input.h"
#include "../../lara.h"
#include "../../gameflow.h"

static int16_t DragSASBounds[12] = {
	-CLICK_SIZE,
	CLICK_SIZE,
	-100,
	100,
	-HALF_BLOCK_SIZE,
	-460,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	0,
	0
};

static PHD_VECTOR DragSASPos = { 0, 0, -460 };

void DragSASCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t x, z;

	item = &items[item_number];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && lara.gun_status == LG_NO_ARMS &&
	        !l->gravity_status && !(item->flags & IFL_CODEBITS) || lara.IsMoving && lara.GeneralPtr == item_number) {
		if (TestLaraPosition(DragSASBounds, item, l)) {
			if (MoveLaraPosition(&DragSASPos, item, l)) {
				l->anim_number = ANIM_DRAGSAS;
				l->frame_number = anims[ANIM_DRAGSAS].frame_base;
				l->current_anim_state = AS_CONTROLLED;
				l->pos.y_rot = item->pos.y_rot;
				lara.IsMoving = 0;
				lara.head_x_rot = 0;
				lara.head_y_rot = 0;
				lara.torso_x_rot = 0;
				lara.torso_y_rot = 0;
				lara.gun_status = LG_HANDS_BUSY;
				item->flags |= IFL_CODEBITS;
				item->status = ITEM_ACTIVE;
				AddActiveItem(item_number);
			} else
				lara.GeneralPtr = item_number;
		}
	} else {
		if (item->status == ITEM_ACTIVE) {
			if (item->frame_number == anims[item->anim_number].frame_end) {
				x = (2048 * phd_sin(l->pos.y_rot)) >> W2V_SHIFT;
				z = (2048 * phd_cos(l->pos.y_rot)) >> W2V_SHIFT;
				TestTriggersAtXYZ(l->pos.x_pos - x, l->pos.y_pos, l->pos.z_pos - z, l->room_number, 1, 0);
				RemoveActiveItem(item_number);
				item->status = ITEM_INACTIVE;
			}
		}

		ObjectCollision(item_number, l, coll);
	}
}