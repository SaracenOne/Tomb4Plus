#include "../../tomb4/pch.h"
#include "trng_env_condition.h"
#include "trng_script_parser.h"
#include "trng.h"
#include "../lara.h"
#include "../control.h"
#include "trng_test_position.h"
#include "trng_triggergroup.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"

int32_t NGAbsDiffO(int16_t first, int16_t second) {
	if (first > second) {
		int16_t swap_value = first;
		first = second;
		second = swap_value;
	}

	if (first < -0x4000 && second > 0x4000) {
		uint16_t word_value = (uint16_t)first;
		return abs(word_value - second);
	}

	return abs(first - second);
}

uint16_t NGGetAlignedOrient(uint16_t orient, bool test_force_horthogonal, int32_t *gap) {
	int32_t min_diff;

	if (!test_force_horthogonal) {
		min_diff = NGAbsDiffO(orient, 0x2000);
		if (min_diff <= 0x1000) {
			*gap = min_diff;
			return 0x2000;
		}

		min_diff = NGAbsDiffO(orient, 0x6000);
		if (min_diff <= 0x1000) {
			*gap = min_diff;
			return 0x6000;
		}


		min_diff = NGAbsDiffO(orient, (int16_t)0xA000);
		if (min_diff <= 0x1000) {
			*gap = min_diff;
			return 0xA000;
		}

		min_diff = NGAbsDiffO(orient, (int16_t)0xE000);
		if (min_diff <= 0x1000) {
			*gap = min_diff;
			return 0xE000;
		}
	}

	min_diff = NGAbsDiffO(orient, 0x0000);
	if (min_diff <= 0x2000) {
		*gap = min_diff;
		return 0x0000;
	}

	min_diff = NGAbsDiffO(orient, 0x4000);
	if (min_diff <= 0x2000) {
		*gap = min_diff;
		return 0x4000;
	}

	min_diff = NGAbsDiffO(orient, (int16_t)0x8000);
	if (min_diff <= 0x2000) {
		*gap = min_diff;
		return 0x8000;
	}

	min_diff = NGAbsDiffO(orient, (int16_t)0xc000);
	if (min_diff <= 0x2000) {
		*gap = min_diff;
		return 0xc000;
	}

	NGLog(NG_LOG_TYPE_ERROR, "No aligned orient found for source orient=0x%X", orient);

	*gap = 0x7000;
	return 0x0000;
}

int32_t NGProportionDistance(int32_t increment, int32_t distance) {
	return (int32_t)((float)increment * ((float)distance / float(BLOCK_SIZE)));
}

void NGCalculateIncrement(int16_t orientation, int32_t* inc_x_out, int32_t* inc_z_out, int32_t distance) {
	int32_t inc_x, inc_z;
	int32_t indice;

	if (distance == 0) {
		*inc_x_out = 0;
		*inc_z_out = 0;
		return;
	}

	indice = orientation >> 3;
	indice &= 0x1FFE;

	inc_x = rcossin_tbl[indice] << 12;
	inc_z = rcossin_tbl[indice + 1] << 12;
	inc_x = inc_x >> W2V_SHIFT;
	inc_z = inc_z >> W2V_SHIFT;
	if (distance != BLOCK_SIZE) {
		inc_x = NGProportionDistance(inc_x, distance);
		inc_z = NGProportionDistance(inc_z, distance);
	}

	*inc_x_out = inc_x;
	*inc_z_out = inc_z;
}


TestEnvConditionTripletResult TestEnvConditionTriplet(NG_MULTI_ENV_TRIPLET* triplet, bool set_alignment_variables, int32_t item_index) {
	TestEnvConditionTripletResult result;

	if (triplet->env_condition == 0xffff) {
		result.is_valid = true;
		result.seek_item = -1;
		result.test_position_id = -1;

		return result;
	}

	if (triplet->env_condition & ~(0xe07f + ENV_NON_TRUE + ENV_POS_HORTOGONAL)) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TestEnvConditionTriplet: unsupported mask features!", triplet->env_condition);

		result.is_valid = false;
		result.seek_item = -1;
		result.test_position_id = -1;

		return result;
	}

	int32_t difference;
	uint16_t aligned_orient = NGGetAlignedOrient(lara_item->pos.y_rot, true, &difference);
	bool check_forward_strip = triplet->env_condition & ENV_POS_STRIP_1;
	bool check_middle_strip = triplet->env_condition & ENV_POS_STRIP_2;
	bool check_back_strip = triplet->env_condition & ENV_POS_STRIP_3;
	bool check_hortogonal = triplet->env_condition & ENV_POS_HORTOGONAL;

	if (check_forward_strip || check_middle_strip || check_back_strip) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TestEnvConditionTriplet: sector strip detection not yet supported!", triplet->env_condition);

		result.is_valid = false;
		result.seek_item = -1;
		result.test_position_id = -1;

		return result;
	}

	if (check_hortogonal) {
		// TODO: Store old env position.
		if (difference > 0xA00) {
			result.is_valid = false;
			result.seek_item = -1;
			result.test_position_id = -1;

			return result;
		}
	}

	ITEM_INFO* current_item = lara_item;
	if (item_index >= 0) {
		if (item_index >= ITEM_COUNT) {
			NGLog(NG_LOG_TYPE_ERROR, "Item out of range!");
		}
		current_item = T4PlusGetItemInfoForID(item_index);
	}

	uint16_t orientation = current_item->pos.y_rot;
	uint32_t coord_x = current_item->pos.x_pos;
	uint32_t coord_y = current_item->pos.y_pos;
	uint32_t coord_z = current_item->pos.z_pos;

	uint32_t env_condition_switch_value = triplet->env_condition & 0x7f;
	switch (env_condition_switch_value) {
		case 0: {
			result.is_valid = true;
			break;
		}
		case ENV_NO_BLOCK_IN_FRONT:
		case ENV_NO_BLOCK_AT_RIGHT:
		case ENV_NO_BLOCK_AT_LEFT:
		case ENV_NO_BLOCK_BACK: {
			int16_t distance_test = (HALF_BLOCK_SIZE + CLICK_SIZE);
			if (triplet->distance_for_env != -1) {
				distance_test = triplet->distance_for_env;
			} else {
				distance_test = (HALF_BLOCK_SIZE + CLICK_SIZE);
			}

			int16_t room_num = lara_item->room_number;
			FLOOR_INFO *floor_info = GetFloor(coord_x, coord_y, coord_z, &room_num);
			int32_t height = GetHeight(floor_info, coord_x, coord_y, coord_z);

			switch (env_condition_switch_value) {
				case ENV_NO_BLOCK_AT_RIGHT:
					orientation += 0x4000;
					break;
				case ENV_NO_BLOCK_AT_LEFT:
					orientation -= 0x4000;
					break;
				case ENV_NO_BLOCK_BACK:
					orientation += 0x8000;
					break;
			}

			int32_t inc_x;
			int32_t inc_z;
			NGCalculateIncrement(orientation, &inc_x, &inc_z, BLOCK_SIZE);

			coord_x += inc_x;
			coord_z += inc_z;

			floor_info = GetFloor(coord_x, coord_y, coord_z, &room_num);
			height = GetHeight(floor_info, coord_x, coord_y, coord_z);

			if (height == NO_HEIGHT) {
				result.is_valid = false;
				break;
			}

			if ((lara_item->pos.y_pos - height) >= distance_test) {
				result.is_valid = false;
				break;
			}

			int32_t ceiling = GetCeiling(floor_info, coord_x, coord_y, coord_z);
			if ((lara_item->pos.y_pos - distance_test) < ceiling) {
				result.is_valid = false;
				break;
			}

			result.is_valid = true;
			break;
		}
		case ENV_HOLE_FLOOR_IN_FRONT:
		case ENV_HOLE_FLOOR_AT_RIGHT:
		case ENV_HOLE_FLOOR_AT_LEFT:
		case ENV_HOLE_FLOOR_BACK: {
			int16_t distance_test = (HALF_BLOCK_SIZE + CLICK_SIZE);
			if (triplet->distance_for_env != -1) {
				distance_test = triplet->distance_for_env;
			}

			switch (env_condition_switch_value) {
				case ENV_HOLE_FLOOR_AT_RIGHT:
					orientation += 0x4000;
					break;
				case ENV_HOLE_FLOOR_AT_LEFT:
					orientation -= 0x4000;
					break;
				case ENV_HOLE_FLOOR_BACK:
					orientation += 0x8000;
					break;
			}

			int32_t inc_x;
			int32_t inc_z;
			NGCalculateIncrement(orientation, &inc_x, &inc_z, BLOCK_SIZE);

			coord_x += inc_x;
			coord_z += inc_z;

			int16_t room_num = lara_item->room_number;

			FLOOR_INFO* floor_info = GetFloor(coord_x, coord_y, coord_z, &room_num);
			int32_t height = GetHeight(floor_info, coord_x, coord_y, coord_z);

			if (height == NO_HEIGHT) {
				result.is_valid = false;
				break;
			}

			if ((height - lara_item->pos.y_pos) < distance_test) {
				result.is_valid = false;
				break;
			}

			int32_t ceiling = GetCeiling(floor_info, coord_x, coord_y, coord_z);
			if ((lara_item->pos.y_pos - distance_test) < ceiling) {
				result.is_valid = false;
				break;
			}

			result.is_valid = true;
			break;
		}
		case ENV_MULT_CONDITION: {
			NG_MULTI_ENV_CONDITION* multi_env_cond = &current_multi_env_conditions[triplet->distance_for_env];
			// Not sure if this is correct.
			if (multi_env_cond->env_condition_triplet_count > 0) {
				result.is_valid = true;
			}

			for (int32_t i = 0; i < multi_env_cond->env_condition_triplet_count; i++) {
				TestEnvConditionTripletResult sub_result = TestEnvConditionTriplet(&multi_env_cond->env_condition_triplet_array[i], set_alignment_variables, item_index);

				if (!sub_result.is_valid) {
					result.is_valid = false;
					break;
				} else {
					if (set_alignment_variables) {
						// This is probably incorrect. Investigate behaviour...
						if (result.seek_item == -1) {
							result.seek_item = sub_result.seek_item;
						}
						if (result.test_position_id == -1) {
							result.test_position_id = sub_result.test_position_id;
						}
					}
				}
			}
			break;
		}
		case ENV_DISTANCE_CEILING: {
			FLOOR_INFO *floor = GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &lara_item->room_number);
			int32_t ceiling = GetCeiling(floor, lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

			int32_t distance_to_ceiling = -(ceiling - lara_item->pos.y_pos);

			if (distance_to_ceiling >= triplet->distance_for_env) {
				result.is_valid = true;
			} else {
				result.is_valid = false;
			}

			break;
		}
		case ENV_MULT_OR_CONDITION: {
			result.is_valid = false;

			NG_MULTI_ENV_CONDITION* multi_env_cond = &current_multi_env_conditions[triplet->distance_for_env];
			for (int32_t i = 0; i < multi_env_cond->env_condition_triplet_count; i++) {
				TestEnvConditionTripletResult sub_result = TestEnvConditionTriplet(&multi_env_cond->env_condition_triplet_array[i], set_alignment_variables, item_index);

				if (sub_result.is_valid) {
					result.is_valid = true;
				} else {
					if (set_alignment_variables) {
						// This is probably incorrect. Investigate behaviour...
						if (result.seek_item == -1) {
							result.seek_item = sub_result.seek_item;
						}
						if (result.test_position_id == -1) {
							result.test_position_id = sub_result.test_position_id;
						}
					}
				}
			}
			break;
		}
		case ENV_FRAME_NUMBER: {
			if ((lara_item->frame_number - anims[lara_item->anim_number].frame_base) == triplet->distance_for_env) {
				result.is_valid = true;
			} else {
				result.is_valid = false;
			}
			break;
		}
		case ENV_FREE_HANDS: {
			if ((lara.gun_status == LG_NO_ARMS || lara.gun_status == LG_FLARE))
				result.is_valid = true;
			else
				result.is_valid = false;

			break;
		}
		case ENV_ROOM_IS: {
			if (lara_item->room_number == triplet->distance_for_env) {
				result.is_valid = true;
			} else {
				result.is_valid = false;
			}

			break;
		}
		case ENV_SUPPORT_IN_FRONT_WALL:
		case ENV_SUPPORT_IN_RIGHT_WALL:
		case ENV_SUPPORT_IN_LEFT_WALL:
		case ENV_SUPPORT_IN_BACK_WALL:
		case ENV_WALL_HOLE_IN_FRONT: {
			int16_t current_distance;
			if (triplet->distance_for_env != -1) {
				current_distance = triplet->distance_for_env;
			} else {
				current_distance = 0x2174;
			}

			int16_t orientation = aligned_orient;
			switch (triplet->env_condition) {
				case ENV_SUPPORT_IN_RIGHT_WALL:
					orientation += 0x4000;
					break;
				case ENV_SUPPORT_IN_LEFT_WALL:
					orientation -= 0x4000;
					break;
				case ENV_SUPPORT_IN_BACK_WALL:
					orientation += 0x8000;
					break;
			}

			switch (orientation) {
				case 0x8000:
					coord_z -= BLOCK_SIZE;
					break;
				case 0x0000:
					coord_z += BLOCK_SIZE;
					break;
				case 0xC000:
					coord_x -= BLOCK_SIZE;
					break;
				case 0x4000:
					coord_x += BLOCK_SIZE;
					break;
			}

			int16_t room_num = lara_item->room_number;
			int32_t current_lara_y = lara_item->pos.y_pos;

			int32_t min_height = (current_distance & 0x000f) << 8;
			int32_t max_Height = (current_distance & 0x00f0) << 4;
			int32_t space_height = (current_distance & 0x0f00);
			int32_t space_height_max = (current_distance & 0xf000) >> 4;

			min_height = current_lara_y - min_height;
			max_Height = current_lara_y - max_Height;

			FLOOR_INFO *floor_info = GetFloor(coord_x, min_height, coord_z, &room_num);
			int32_t height = GetHeight(floor_info, coord_x, min_height, coord_z);

			if (height == NO_HEIGHT) {
				result.is_valid = false;
				break;
			}

			if (min_height < (height - 0x80) && room_num == lara_item->room_number) {
				room_num = lara_item->room_number;
				floor_info = GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &room_num);

				if (floor_info->sky_room == -1)
					result.is_valid = false;
				break;

				room_num = floor_info->sky_room;
				floor_info = GetFloor(coord_x, min_height, coord_z, &room_num);
				height = GetHeight(floor_info, coord_x, min_height, coord_z);

				if (height == NO_HEIGHT) {
					result.is_valid = false;
					break;
				}
			}

			if (height > (min_height + 0xe0)) {
				result.is_valid = false;
				break;
			}

			if (height > (min_height + 0xe0)) {
				result.is_valid = false;
				break;
			}


			int32_t ceiling = GetCeiling(floor_info, coord_x, min_height, coord_z);
			int32_t current_hole_height = height - ceiling;

			if (triplet->env_condition == ENV_WALL_HOLE_IN_FRONT) {
				if (current_hole_height < space_height ||
				        current_hole_height > space_height_max) {
					result.is_valid = false;
					break;
				}
			} else {
				if (current_hole_height < space_height) {
					result.is_valid = false;
					break;
				}
			}

			room_num = lara_item->room_number;
			floor_info = GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &room_num);
			ceiling = GetCeiling(floor_info, lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

			if ((height - ceiling) >= space_height) {
				result.is_valid = true;
			} else {
				result.is_valid = false;
			}

			break;
		}
		case ENV_ITEM_TEST_POSITION: {
			ITEM_INFO *item = NULL;
			int16_t item_number;

			result.is_valid = false;
			for (item_number = room[lara_item->room_number].item_number; item_number != NO_ITEM; item_number = item->next_item) {
				item = T4PlusGetItemInfoForID(item_number);
				int32_t test_position_id = triplet->distance_for_env;

				NG_TEST_POSITION &test_position = current_test_positions[test_position_id];

				if (NGTestLaraPosition(&test_position, item, lara_item) == true) {
					result.is_valid = true;
					result.seek_item = item_number;
					result.test_position_id = test_position_id;
				}
			}
			break;
		}
		case ENV_CONDITION_TRIGGER_GROUP: {
			result.is_valid = NGTriggerGroupFunction(triplet->distance_for_env, TRIGGER_GROUP_EXECUTION_MULTIPLE);

			break;
		}
		case ENV_LARA_IN_MICRO_STRIP: {
			result.is_valid = false;

			int32_t min_distance = triplet->distance_for_env & 0xff;
			int32_t max_distance = triplet->distance_for_env >> 8;
			max_distance &= 0xff;
			coord_x = current_item->pos.x_pos & 0x3ff;
			coord_z = current_item->pos.z_pos & 0x3ff;
			int32_t GapX = coord_x >> 5;
			int32_t GapZ = coord_z >> 5;

			orientation = NGGetAlignedOrient(current_item->pos.y_rot, true, &difference);
			int32_t strip = 0;

			switch (orientation) {
				case 0x0000:
					strip = ((QUARTER_CLICK_SIZE / 2) - 1) - GapZ;
					break;
				case 0x8000:
					strip = GapZ;
					break;
				case 0x4000:
					strip = ((QUARTER_CLICK_SIZE / 2) - 1) - GapX;
					break;
				case 0xC000:
					strip = GapX;
					break;
			}
			if (strip >= min_distance && strip <= max_distance)
				result.is_valid = true;

			break;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TestEnvConditionTriplet: Unimplemented environment condition: %u!", env_condition_switch_value);
			result.is_valid = false;

			break;
		}
	}

	if (triplet->env_condition & ENV_NON_TRUE) {
		result.is_valid = !result.is_valid;
	}

	return result;
}

bool TestMultiEnvCondition(int32_t multi_env_condition_id, bool evaluate_as_or, int32_t item_index) {
	bool is_valid;
	if (evaluate_as_or)
		is_valid = false;
	else
		is_valid = true;

	NG_MULTI_ENV_CONDITION* multi_env_cond = &current_multi_env_conditions[multi_env_condition_id];
	for (int32_t i = 0; i < multi_env_cond->env_condition_triplet_count; i++) {
		TestEnvConditionTripletResult sub_result = TestEnvConditionTriplet(&multi_env_cond->env_condition_triplet_array[i], false, item_index);

		if (evaluate_as_or) {
			if (sub_result.is_valid) {
				is_valid = true;
				break;
			}
		} else {
			if (!sub_result.is_valid) {
				is_valid = false;
				break;
			}
		}
	}
	return is_valid;
}