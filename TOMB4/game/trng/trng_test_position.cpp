#include "../../tomb4/pch.h"
#include "trng_script_parser.h"
#include "../collide.h"
#include "trng.h"
#include "../lara.h"
#include "../../specific/3dmath.h"
#include "../control.h"
#include "../lara_states.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"

int32_t NGTestLaraDistance(PHD_VECTOR* target, ITEM_INFO* item, ITEM_INFO* l) {
	PHD_VECTOR pos;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	pos.x = l->pos.x_pos - item->pos.x_pos;
	pos.y = l->pos.y_pos - item->pos.y_pos;
	pos.z = l->pos.z_pos - item->pos.z_pos;
	int32_t x = int32_t(pos.x * mMXPtr[M00] + pos.y * mMXPtr[M10] + pos.z * mMXPtr[M20]);
	int32_t y = int32_t(pos.x * mMXPtr[M01] + pos.y * mMXPtr[M11] + pos.z * mMXPtr[M21]);
	int32_t z = int32_t(pos.x * mMXPtr[M02] + pos.y * mMXPtr[M12] + pos.z * mMXPtr[M22]);
	phd_PopMatrix();

	return x == target->x && y == target->y && z == target->z;
}

void NGStopLaraMovement(bool set_stop_animation_when_goal_reached) {
	if (set_stop_animation_when_goal_reached) {
		lara.IsMoving = 0;
		NGSetItemAnimation(lara.item_number, ANIM_STOP, true, true, true, false);
	}

	ng_animation_target_item = -1;
	ng_animation_target_test_position = -1;
}

PHD_VECTOR NGCalculatePositionForTestPosition(NG_TEST_POSITION* test_position) {
	PHD_VECTOR TestBoundsPosition = {
		(test_position->x_distance_min + test_position->x_distance_max) / 2,
		(test_position->y_distance_min + test_position->y_distance_max) / 2,
		(test_position->z_distance_min + test_position->z_distance_max) / 2
	};

	return TestBoundsPosition;
}

bool NGTestLaraPosition(NG_TEST_POSITION *test_position, ITEM_INFO *item, ITEM_INFO *l) {
	int16_t TestBoundsBounds[] = {
		test_position->x_distance_min,
		test_position->x_distance_max,
		test_position->y_distance_min,
		test_position->y_distance_max,
		test_position->z_distance_min,
		test_position->z_distance_max,
		test_position->v_orient_diff_min,
		test_position->v_orient_diff_max,
		test_position->h_orient_diff_min,
		test_position->h_orient_diff_max,
		test_position->r_orient_diff_min,
		test_position->r_orient_diff_max
	};

	if (item->object_number == test_position->moveable_slot) {
		if (test_position->flags == 0 || test_position->flags == 0xffff) {
			if (TestLaraPosition(TestBoundsBounds, item, l)) {
				return true;
			}
		} else {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGTestLaraPosition: flags are currently unsupported!");
		}
	}

	return false;
}

bool NGMoveLara(bool set_stop_animation_when_goal_reached) {
	if (ng_animation_target_item >= 0) {
		NG_TEST_POSITION* test_position = &current_test_positions[ng_animation_target_test_position];
		ITEM_INFO *item = T4PlusGetItemInfoForID(ng_animation_target_item);
		if (item) {
			if (NGTestLaraPosition(test_position, item, lara_item)) {
				PHD_VECTOR target_position = NGCalculatePositionForTestPosition(test_position);

				lara.GeneralPtr = 0;

				if (MoveLaraPosition(&target_position, item, lara_item)) {
					NGStopLaraMovement(set_stop_animation_when_goal_reached);

					return true;
				}
			} else {
				NGStopLaraMovement(set_stop_animation_when_goal_reached);

				return true;
			}
		}
	}

	return false;
}