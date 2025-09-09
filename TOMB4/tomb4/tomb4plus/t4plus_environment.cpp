#include "../../tomb4/pch.h"

#include "../../global/types.h"
#include "t4plus_environment.h"

#include "../mod_config.h"
#include "../../game/gameflow.h"

bool T4PlusIsRoomSwamp(ROOM_INFO* r) {
	int32_t swamp_flag = get_game_mod_level_environment_info(gfCurrentLevel)->room_swamp_flag;

	if (swamp_flag) {
		if (r->flags & swamp_flag) {
			return true;
		}
	}

	return false;
}

bool T4PlusIsRoomCold(ROOM_INFO* r) {
	int32_t cold_flag = get_game_mod_level_environment_info(gfCurrentLevel)->room_cold_flag;

	if (cold_flag) {
		if (r->flags & cold_flag) {
			return true;
		}
	}

	return false;
}

bool T4PlusIsRoomDamage(ROOM_INFO *r) {
	int32_t damage_flag = get_game_mod_level_environment_info(gfCurrentLevel)->room_damage_flag;

	if (damage_flag) {
		if (r->flags & damage_flag) {
			return true;
		}
	}

	return false;
}

bool T4PlusDoesRoomCauseColdBreath(ROOM_INFO *r) {
	if (get_game_mod_level_gfx_info(gfCurrentLevel)->cold_breath != COLD_BREATH_DISABLED) {
		if (get_game_mod_level_gfx_info(gfCurrentLevel)->cold_breath == COLD_BREATH_ENABLED_OUTSIDE_AND_IN_COLD_ROOMS) {
			if (r->flags & ROOM_OUTSIDE) {
				return true;
			}
		}

		int32_t cold_flag = get_game_mod_level_environment_info(gfCurrentLevel)->room_cold_flag;

		if (cold_flag) {
			if (r->flags & cold_flag) {
				return true;
			}
		}
	}

	return false;
}
