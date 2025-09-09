#include "../tomb4/pch.h"
#include "gameflow.h"
#include "laraswim.h"
#include "lara_states.h"
#include "lara.h"
#include "laramisc.h"
#include "collide.h"
#include "larafire.h"
#include "control.h"
#include "../specific/3dmath.h"
#include "camera.h"
#include "../specific/input.h"
#include "effect2.h"

#include "trng/trng.h"
#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_environment.h"

void lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll) {
	if (input & IN_FORWARD)
		item->pos.x_rot -= DEGREES_TO_ROTATION(3);
	else if (input & IN_BACK)
		item->pos.x_rot += DEGREES_TO_ROTATION(3);

	if (input & IN_LEFT) {
		lara.turn_rate -= 613;

		if (lara.turn_rate < -DEGREES_TO_ROTATION(6))
			lara.turn_rate = -DEGREES_TO_ROTATION(6);
	} else if (input & IN_RIGHT) {
		lara.turn_rate += 613;

		if (lara.turn_rate > DEGREES_TO_ROTATION(6))
			lara.turn_rate = DEGREES_TO_ROTATION(6);
	}

	if (input & IN_ACTION)
		TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 31, 255, 255, 255);

	if (input & IN_ROLL)
		lara.turn_rate = -DEGREES_TO_ROTATION(12);

	if (input & IN_JUMP) {
		item->fallspeed += 16;

		if (item->fallspeed > 400)
			item->fallspeed = 400;
	} else {
		if (item->fallspeed >= 8)
			item->fallspeed -= item->fallspeed >> 3;
		else
			item->fallspeed = 0;
	}
}

void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll) {
	if (item->hit_points <= 0) {
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	if (input & IN_ROLL) {
		item->current_anim_state = AS_WATERROLL;
		item->anim_number = LARA_ANIM_WATERROLL;
		item->frame_number = anims[LARA_ANIM_WATERROLL].frame_base;
	} else {
		SwimTurn(item);
		item->fallspeed += 8;

		if (item->fallspeed > 200) {
			item->fallspeed = 200;
		}

		if (!(input & IN_JUMP)) {
			item->goal_anim_state = AS_GLIDE;
		}
	}
}

void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll) {
	coll->bad_pos = -NO_HEIGHT;
	coll->bad_neg = -400;
	coll->bad_ceiling = 400;
	coll->old.x = item->pos.x_pos;
	coll->old.y = item->pos.y_pos;
	coll->old.z = item->pos.z_pos;
	coll->radius = lara.water_status == LW_FLYCHEAT ? 100 : 300;
	coll->trigger_index = 0;
	coll->slopes_are_walls = 0;
	coll->slopes_are_pits = 0;
	coll->lava_is_pit = 0;
	coll->enable_spaz = false;
#ifdef FLYCHEAT_NOCLIP
	// T4Plus: noclip
	if (lara.water_status != LW_FLYCHEAT) {
		coll->enable_baddie_push = true;
	} else {
		coll->enable_baddie_push = false;
	}
#else
	coll->enable_baddie_push = true;
#endif

	if (input & IN_LOOK && lara.look) {
		LookLeftRight();
	} else {
		ResetLook();
	}

	lara.look = 1;
	lara_control_routines[item->current_anim_state](item, coll);

	if (item->pos.z_rot < -DEGREES_TO_ROTATION(2)) {
		item->pos.z_rot += DEGREES_TO_ROTATION(2);
	} else if (item->pos.z_rot > DEGREES_TO_ROTATION(2)) {
		item->pos.z_rot -= DEGREES_TO_ROTATION(2);
	} else {
		item->pos.z_rot = 0;
	}

	if (item->pos.x_rot < -DEGREES_TO_ROTATION(85)) {
		item->pos.x_rot = -DEGREES_TO_ROTATION(85);
	} else if (item->pos.x_rot > DEGREES_TO_ROTATION(85)) {
		item->pos.x_rot = DEGREES_TO_ROTATION(85);
	}

	if (item->pos.z_rot < -DEGREES_TO_ROTATION(22)) {
		item->pos.z_rot = -DEGREES_TO_ROTATION(22);
	} else if (item->pos.z_rot > DEGREES_TO_ROTATION(22)) {
		item->pos.z_rot = DEGREES_TO_ROTATION(22);
	}

	if (lara.turn_rate < -LARA_TURN_DECREMENT) {
		lara.turn_rate += LARA_TURN_DECREMENT;
	} else if (lara.turn_rate > LARA_TURN_DECREMENT) {
		lara.turn_rate -= LARA_TURN_DECREMENT;
	} else {
		lara.turn_rate = 0;
	}

	item->pos.y_rot += lara.turn_rate;

	if (lara.current_active && lara.water_status != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);
	item->pos.x_pos += (((phd_sin(item->pos.y_rot) * item->fallspeed) >> W2V_SHIFT + 2) * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
	item->pos.y_pos -= (phd_sin(item->pos.x_rot) * item->fallspeed) >> W2V_SHIFT + 2;
	item->pos.z_pos += (((phd_cos(item->pos.y_rot) * item->fallspeed) >> W2V_SHIFT + 2) * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
	LaraBaddieCollision(item, coll);

	if (lara.vehicle == NO_ITEM) {
		lara_collision_routines[item->current_anim_state](item, coll);
	}

	UpdateLaraRoom(item, 0);
	LaraGun();

	TestTriggers(coll->trigger_index, false, 0);

	if (lara.water_status == LW_FLYCHEAT) {
		item->anim_number = LARA_ANIM_FASTFALL;
		item->frame_number = anims[LARA_ANIM_FASTFALL].frame_base + 5;
	}
}

void lara_col_swim(ITEM_INFO* item, COLL_INFO* coll) {
	LaraSwimCollision(item, coll);
}

void lara_col_glide(ITEM_INFO* item, COLL_INFO* coll) {
	LaraSwimCollision(item, coll);
}

void lara_col_tread(ITEM_INFO* item, COLL_INFO* coll) {
	LaraSwimCollision(item, coll);
}

void lara_col_dive(ITEM_INFO* item, COLL_INFO* coll) {
	LaraSwimCollision(item, coll);
}

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll) {
	LaraSwimCollision(item, coll);
}

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll) {
	if (lara.water_status == LW_FLYCHEAT) {
		lara_as_swimcheat(item, coll);
	} else if (lara.water_status != LW_ABOVE_WATER) {
		if (item->hit_points <= 0) {
			item->goal_anim_state = AS_UWDEATH;
			return;
		}

		if (input & IN_ROLL) {
			item->current_anim_state = AS_WATERROLL;
			item->anim_number = LARA_ANIM_WATERROLL;
			item->frame_number = anims[LARA_ANIM_WATERROLL].frame_base;
		} else {
			SwimTurn(item);

			if (input & IN_JUMP)
				item->goal_anim_state = AS_SWIM;

			item->fallspeed -= 6;

			if (item->fallspeed < 0) {
				item->fallspeed = 0;
			}

			if (item->fallspeed <= 133) {
				item->goal_anim_state = AS_TREAD;
			}
		}
	}
}

void lara_as_tread(ITEM_INFO* item, COLL_INFO* coll) {
	if (item->hit_points <= 0) {
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	if (input & IN_ROLL) {
		item->current_anim_state = AS_WATERROLL;
		item->anim_number = LARA_ANIM_WATERROLL;
		item->frame_number = anims[LARA_ANIM_WATERROLL].frame_base;
		return;
	}

	if (input & IN_LOOK)
		LookUpDown();

	SwimTurn(item);

	if (input & IN_JUMP)
		item->goal_anim_state = AS_SWIM;

	item->fallspeed -= 6;

	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (lara.gun_status == LG_HANDS_BUSY)
		lara.gun_status = LG_NO_ARMS;
}

void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll) {
	if (input & IN_FORWARD)
		item->pos.x_rot -= DEGREES_TO_ROTATION(1);
}

void lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll) {
	lara.look = 0;
	item->fallspeed -= 8;

	if (item->fallspeed <= 0)
		item->fallspeed = 0;

	if (item->pos.x_rot >= -DEGREES_TO_ROTATION(2) && item->pos.x_rot <= DEGREES_TO_ROTATION(2))
		item->pos.x_rot = 0;
	else if (item->pos.x_rot < 0)
		item->pos.x_rot += DEGREES_TO_ROTATION(2);
	else
		item->pos.x_rot -= DEGREES_TO_ROTATION(2);
}

void lara_as_waterroll(ITEM_INFO* item, COLL_INFO* coll) {
	if (lara.water_status == LW_FLYCHEAT)
		item->current_anim_state = AS_GLIDE;
	else
		item->fallspeed = 0;
}

void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t wh;

	item->hit_points = -1;
	lara.air = -1;
	lara.gun_status = LG_HANDS_BUSY;
	wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
		item->pos.y_pos -= 5;

	LaraSwimCollision(item, coll);
}

int32_t GetWaterDepth(int32_t x, int32_t y, int32_t z, int16_t room_number) {
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	int32_t x_floor, y_floor, h;
	int16_t door;

	r = &room[room_number];

	do {
		x_floor = (z - r->z) >> WALL_SHIFT;
		y_floor = (x - r->x) >> WALL_SHIFT;

		if (x_floor <= 0) {
			x_floor = 0;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		} else if (x_floor >= r->x_size - 1) {
			x_floor = r->x_size - 1;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		} else if (y_floor < 0)
			y_floor = 0;
		else if (y_floor >= r->y_size)
			y_floor = r->y_size - 1;

		floor = &r->floor[x_floor + y_floor * r->x_size];
		door = GetDoor(floor);

		if (door != 255) {
			room_number = door;
			r = &room[door];
		}

	} while (door != 255);

	if (r->flags & ROOM_UNDERWATER || T4PlusIsRoomSwamp(r)) {
		while (floor->sky_room != 255) {
			r = &room[floor->sky_room];

			if (!(r->flags & ROOM_UNDERWATER || T4PlusIsRoomSwamp(r))) {
				h = GetMinimumCeiling(floor, x, z);
				floor = GetFloor(x, y, z, &room_number);
				return GetHeight(floor, x, y, z) - h;
			}

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];
		}

		return 0x7FFF;
	} else {
		while (floor->pit_room != 255) {
			r = &room[floor->pit_room];

			if (r->flags & ROOM_UNDERWATER || T4PlusIsRoomSwamp(r)) {
				h = GetMaximumFloor(floor, x, z);
				floor = GetFloor(x, y, z, &room_number);
				return GetHeight(floor, x, y, z) - h;
			}

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];
		}

		return NO_HEIGHT;
	}
}

void SwimTurn(ITEM_INFO* item) {
	if (input & IN_FORWARD)
		item->pos.x_rot -= DEGREES_TO_ROTATION(2);
	else if (input & IN_BACK)
		item->pos.x_rot += DEGREES_TO_ROTATION(2);

	if (input & IN_LEFT) {
		lara.turn_rate -= LARA_TURN_INCREMENT;

		if (lara.turn_rate < -DEGREES_TO_ROTATION(6))
			lara.turn_rate = -DEGREES_TO_ROTATION(6);

		item->pos.z_rot -= DEGREES_TO_ROTATION(3);
	} else if (input & IN_RIGHT) {
		lara.turn_rate += LARA_TURN_INCREMENT;

		if (lara.turn_rate > DEGREES_TO_ROTATION(6))
			lara.turn_rate = DEGREES_TO_ROTATION(6);

		item->pos.z_rot += DEGREES_TO_ROTATION(3);
	}
}

void LaraTestWaterDepth(ITEM_INFO* item, COLL_INFO* coll) {
	FLOOR_INFO* floor;
	int32_t wd;
	int16_t room_number;

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	wd = GetWaterDepth(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);

	if (wd == NO_HEIGHT) {
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;
		item->fallspeed = 0;
	} else if (wd <= HALF_BLOCK_SIZE) {
		item->anim_number = LARA_ANIM_SWIM2QSTND;
		item->frame_number = anims[LARA_ANIM_SWIM2QSTND].frame_base;
		item->current_anim_state = AS_WATEROUT;
		item->goal_anim_state = AS_STOP;
		item->pos.x_rot = 0;
		item->pos.z_rot = 0;
		item->gravity_status = 0;
		item->speed = 0;
		item->fallspeed = 0;
		lara.water_status = LW_WADE;
		item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	}
}

void LaraSwimCollisionTR4(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t y;

	if (item->pos.x_rot < -0x4000 || item->pos.x_rot > 0x4000)
		lara.move_angle = item->pos.y_rot + 0x8000;
	else
		lara.move_angle = item->pos.y_rot;

	coll->facing = lara.move_angle;
	y = ((HALF_BLOCK_SIZE + CLICK_SIZE) - 6) * phd_sin(item->pos.x_rot) >> W2V_SHIFT;
	y = abs(y);

	if (y < 200)
		y = 200;

	coll->bad_neg = -QUARTER_CLICK_SIZE;
	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + y / 2, item->pos.z_pos, item->room_number, y);
	ShiftItem(item, coll);

	switch (coll->coll_type) {
		case CT_FRONT:

			if (item->pos.x_rot > DEGREES_TO_ROTATION(45))
				item->pos.x_rot += DEGREES_TO_ROTATION(2);
			else if (item->pos.x_rot < -DEGREES_TO_ROTATION(45))
				item->pos.x_rot -= DEGREES_TO_ROTATION(2);
			else
				item->fallspeed = 0;

			break;

		case CT_TOP:

			if (item->pos.x_rot < -DEGREES_TO_ROTATION(45))
				item->pos.x_rot -= DEGREES_TO_ROTATION(2);

			break;

		case CT_TOP_FRONT:
			item->fallspeed = 0;
			break;

		case CT_LEFT:
			item->pos.y_rot += DEGREES_TO_ROTATION(5);
			break;

		case CT_RIGHT:
			item->pos.y_rot -= DEGREES_TO_ROTATION(5);
			break;

		case CT_CLAMP:
			item->pos.x_pos = coll->old.x;
			item->pos.y_pos = coll->old.y;
			item->pos.z_pos = coll->old.z;
			item->fallspeed = 0;
			return;
	}

	if (coll->mid_floor < 0 && coll->mid_floor != NO_HEIGHT) {
		item->pos.x_rot += DEGREES_TO_ROTATION(2);
		item->pos.y_pos += coll->mid_floor;
	}

	if (lara.water_status != LW_FLYCHEAT)
		LaraTestWaterDepth(item, coll);
}

void LaraSwimCollisionTR5(ITEM_INFO* item, COLL_INFO* coll) {
	COLL_INFO coll2, coll3;
	int32_t ox, oy, oz, height;
	int16_t oxr, oyr, hit;

	hit = 0;
	ox = item->pos.x_pos;
	oy = item->pos.y_pos;
	oz = item->pos.z_pos;
	oxr = item->pos.x_rot;
	oyr = item->pos.y_rot;

	if (oxr < -0x4000 || oxr > 0x4000) {
		lara.move_angle = item->pos.y_rot - 0x8000;
		coll->facing = item->pos.y_rot - 0x8000;
	} else {
		lara.move_angle = item->pos.y_rot;
		coll->facing = item->pos.y_rot;
	}

	height = ((HALF_BLOCK_SIZE + CLICK_SIZE) - 6) * phd_sin(item->pos.x_rot) >> W2V_SHIFT;

	if (height < 0)
		height = -height;

	// Divesuit check would go here.
	if (height < 200)
		height = 200;

	coll->bad_neg = -QUARTER_CLICK_SIZE;
	memcpy(&coll2, coll, sizeof(COLL_INFO));
	memcpy(&coll3, coll, sizeof(COLL_INFO));

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);

	coll2.facing += 0x2000;
	GetCollisionInfo(&coll2, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);

	coll3.facing -= 0x2000;
	GetCollisionInfo(&coll3, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);

	ShiftItem(item, coll);

	switch (coll->coll_type) {
		case CT_FRONT:

			if (item->pos.x_rot > DEGREES_TO_ROTATION(25)) {
				item->pos.x_rot += DEGREES_TO_ROTATION(1);
				hit = 1;
			} else if (item->pos.x_rot < -DEGREES_TO_ROTATION(25)) {
				item->pos.x_rot -= DEGREES_TO_ROTATION(1);
				hit = 1;
			} else if (item->pos.x_rot > DEGREES_TO_ROTATION(5))
				item->pos.x_rot += HALF_DEGREES_TO_ROTATION(1);
			else if (item->pos.x_rot < -DEGREES_TO_ROTATION(5))
				item->pos.x_rot -= HALF_DEGREES_TO_ROTATION(1);
			else if (item->pos.x_rot > 0)
				item->pos.x_rot += 45;
			else if (item->pos.x_rot < 0)
				item->pos.x_rot -= 45;
			else {
				hit = 1;
				item->fallspeed = 0;
			}

			if (coll2.coll_type == CT_LEFT)
				item->pos.y_rot += DEGREES_TO_ROTATION(2);
			else if (coll2.coll_type == CT_RIGHT)
				item->pos.y_rot -= DEGREES_TO_ROTATION(2);
			else if (coll3.coll_type == CT_LEFT)
				item->pos.y_rot += DEGREES_TO_ROTATION(2);
			else if (coll3.coll_type == CT_RIGHT)
				item->pos.y_rot -= DEGREES_TO_ROTATION(2);

			break;

		case CT_TOP:

			if (item->pos.x_rot >= -DEGREES_TO_ROTATION(45)) {
				hit = 1;
				item->pos.x_rot -= DEGREES_TO_ROTATION(1);
			}

			break;

		case CT_TOP_FRONT:
			item->fallspeed = 0;
			hit = 1;
			break;

		case CT_LEFT:
			item->pos.y_rot += DEGREES_TO_ROTATION(2);
			hit = 1;
			break;

		case CT_RIGHT:
			item->pos.y_rot -= DEGREES_TO_ROTATION(2);
			hit = 1;
			break;

		case CT_CLAMP:
			hit = 2;
			item->pos.x_pos = coll->old.x;
			item->pos.y_pos = coll->old.y;
			item->pos.z_pos = coll->old.z;
			item->fallspeed = 0;
			break;
	}

	if (coll->mid_floor < 0 && coll->mid_floor != NO_HEIGHT) {
		hit = 1;
		item->pos.x_rot += DEGREES_TO_ROTATION(1);
		item->pos.y_pos += coll->mid_floor;
	}

	// Another divesuit check would go here.

	if (hit != 2 && lara.water_status != LW_FLYCHEAT)
		LaraTestWaterDepth(item, coll);
}

void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll) {
	if (get_game_mod_level_lara_info(gfCurrentLevel)->use_tr5_swimming_collision) {
		LaraSwimCollisionTR5(item, coll);
	} else {
		LaraSwimCollisionTR4(item, coll);
	}
}

void LaraWaterCurrent(COLL_INFO* coll) {
	int32_t angle, speed, sinkval, shifter, absvel;

	if (lara.current_active) {
		sinkval = lara.current_active - 1;
		speed = camera.fixed[sinkval].data;
		angle = ((mGetAngle(camera.fixed[sinkval].x, camera.fixed[sinkval].z, lara_item->pos.x_pos, lara_item->pos.z_pos) - 0x4000) >> 4) & 0xFFF;
		lara.current_xvel += int16_t((((speed * rcossin_tbl[2 * angle]) >> 2) - lara.current_xvel) >> 4);
		lara.current_zvel += int16_t((((speed * rcossin_tbl[2 * angle + 1]) >> 2) - lara.current_zvel) >> 4);
		lara_item->pos.y_pos += (camera.fixed[sinkval].y - lara_item->pos.y_pos) >> 4;
	} else {
		absvel = abs(lara.current_xvel);

		if (absvel > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else
			shifter = 2;

		lara.current_xvel -= lara.current_xvel >> shifter;

		if (abs(lara.current_xvel) < 4)
			lara.current_xvel = 0;

		absvel = abs(lara.current_zvel);

		if (absvel > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else
			shifter = 2;

		lara.current_zvel -= lara.current_zvel >> shifter;

		if (abs(lara.current_zvel) < 4)
			lara.current_zvel = 0;

		if (!lara.current_xvel && !lara.current_zvel)
			return;
	}

	lara_item->pos.x_pos += lara.current_xvel >> 8;
	lara_item->pos.z_pos += lara.current_zvel >> 8;
	lara.current_active = 0;
	coll->facing = (int16_t)phd_atan(lara_item->pos.z_pos - coll->old.z, lara_item->pos.x_pos - coll->old.x);
	GetCollisionInfo(coll, lara_item->pos.x_pos, lara_item->pos.y_pos + 200, lara_item->pos.z_pos, lara_item->room_number, 400);

	switch (coll->coll_type) {
		case CT_FRONT:

			if (lara_item->pos.x_rot > DEGREES_TO_ROTATION(35))
				lara_item->pos.x_rot += DEGREES_TO_ROTATION(2);
			else if (lara_item->pos.x_rot < -DEGREES_TO_ROTATION(35))
				lara_item->pos.x_rot -= DEGREES_TO_ROTATION(2);
			else
				lara_item->fallspeed = 0;

			break;

		case CT_TOP:
			lara_item->pos.x_rot -= DEGREES_TO_ROTATION(2);
			break;

		case CT_TOP_FRONT:
			lara_item->fallspeed = 0;
			break;

		case CT_LEFT:
			lara_item->pos.y_rot += DEGREES_TO_ROTATION(5);
			break;

		case CT_RIGHT:
			lara_item->pos.y_rot -= DEGREES_TO_ROTATION(5);
			break;
	}

	if (coll->mid_floor < 0 && coll->mid_floor != NO_HEIGHT)
		lara_item->pos.y_pos += coll->mid_floor;

	ShiftItem(lara_item, coll);
	coll->old.x = lara_item->pos.x_pos;
	coll->old.y = lara_item->pos.y_pos;
	coll->old.z = lara_item->pos.z_pos;
}
