#include "../tomb4/pch.h"
#include "laraclmb.h"
#include "lara.h"
#include "lara_states.h"
#include "laramisc.h"
#include "control.h"
#include "camera.h"
#include "../specific/input.h"

#include "trng/trng.h"
#include "../specific/3dmath.h"

static int16_t LeftIntRightExtTab[4] = { BLOCK_SIZE * 2, CLICK_SIZE, HALF_BLOCK_SIZE, BLOCK_SIZE };
static int16_t LeftExtRightIntTab[4] = { HALF_BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE * 2, CLICK_SIZE };

void lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll) {
	lara.IsClimbing = 1;
	coll->enable_spaz = false;
	coll->enable_baddie_push = false;
	camera.target_elevation = -DEGREES_TO_ROTATION(20);

	if (input & IN_LOOK)
		LookUpDown();

	if (input & IN_LEFT || input & IN_LSTEP) {
		item->goal_anim_state = AS_CLIMBLEFT;
		lara.move_angle = item->pos.y_rot - 0x4000;
	} else if (input & IN_RIGHT || input & IN_RSTEP) {
		item->goal_anim_state = AS_CLIMBRIGHT;
		lara.move_angle = item->pos.y_rot + 0x4000;
	} else if (input & IN_JUMP) {
		if (item->anim_number == ANIM_CLIMBSTNC) {
			item->goal_anim_state = AS_BACKJUMP;
			lara.gun_status = LG_NO_ARMS;
			lara.move_angle = item->pos.y_rot + 0x8000;
		}
	}
}

void lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll) {
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;
	camera.target_angle = -DEGREES_TO_ROTATION(30);
	camera.target_elevation = -DEGREES_TO_ROTATION(15);

	if (!(input & (IN_LEFT | IN_LSTEP)))
		item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t shift, res;

	if (!LaraCheckForLetGo(item, coll)) {
		lara.move_angle = item->pos.y_rot - 0x4000;
		res = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift);
		LaraDoClimbLeftRight(item, coll, res, shift);
	}
}

void lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll) {
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;
	camera.target_angle = DEGREES_TO_ROTATION(30);
	camera.target_elevation = -DEGREES_TO_ROTATION(15);

	if (!(input & (IN_RIGHT | IN_RSTEP)))
		item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t shift, res;

	if (!LaraCheckForLetGo(item, coll)) {
		lara.move_angle = item->pos.y_rot + 0x4000;
		res = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift);
		LaraDoClimbLeftRight(item, coll, res, shift);
	}
}

void lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll) {
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;
	camera.target_elevation = DEGREES_TO_ROTATION(30);
}

void lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll) {
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;
	camera.target_elevation = -DEGREES_TO_ROTATION(45);
}

void lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll) {
	coll->enable_baddie_push = false;
	coll->enable_spaz = false;
	camera.flags = 1;
	camera.target_angle = -DEGREES_TO_ROTATION(45);
}

void lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t result_r, result_l, shift_r, shift_l, ledge_r, ledge_l;

	if (LaraCheckForLetGo(item, coll) || item->anim_number != ANIM_CLIMBSTNC)
		return;

	if (input & IN_FORWARD) {
		if (item->goal_anim_state == AS_NULL)
			return;

		item->goal_anim_state = AS_CLIMBSTNC;
		result_r = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &shift_r, &ledge_r);
		result_l = LaraTestClimbUpPos(item, coll->radius, coll->radius - 120, &shift_l, &ledge_l);

		if (!result_r || !result_l)
			return;

		if (result_r < 0 || result_l < 0) {
			if (abs(ledge_l - ledge_r) <= 120) {
				item->goal_anim_state = AS_NULL;
				item->pos.y_pos += (ledge_l + ledge_r) / 2 - CLICK_SIZE;
				return;
			}
		}

		if (shift_r) {
			if (shift_l) {
				if (shift_r < 0 != shift_l < 0)
					return;

				if (shift_r < 0 && shift_r < shift_l)
					shift_l = shift_r;
				else if (shift_r > 0 && shift_r > shift_l)
					shift_l = shift_r;
			} else
				shift_l = shift_r;
		}

		item->goal_anim_state = AS_CLIMBING;
		item->pos.y_pos += shift_l;
	} else if (input & IN_BACK) {
		if (item->goal_anim_state == AS_HANG)
			return;

		item->goal_anim_state = AS_CLIMBSTNC;
		item->pos.y_pos += CLICK_SIZE;
		result_r = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift_r);
		result_l = LaraTestClimbPos(item, coll->radius, coll->radius - 120, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift_l);
		item->pos.y_pos -= CLICK_SIZE;

		if (!result_r || !result_l)
			return;

		if (shift_r && shift_l) {
			if (shift_r < 0 != shift_l < 0)
				return;

			if ((shift_r > 0 && shift_r > shift_l) || (shift_r < 0 && shift_r < shift_l))
				shift_l = shift_r;
		}

		if (result_l == 1 && result_r == 1) {
			item->goal_anim_state = AS_CLIMBDOWN;
			item->pos.y_pos += shift_l;
		} else
			item->goal_anim_state = AS_HANG;
	}
}

void lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t frame, yshift, result_r, result_l, shift_r, shift_l, ledge_r, ledge_l;

	if (LaraCheckForLetGo(item, coll))
		return;

	if (item->anim_number != ANIM_CLIMBING)
		return;

	frame = item->frame_number - anims[ANIM_CLIMBING].frame_base;

	if (!frame)
		yshift = 0;
	else if (frame == 28 || frame == 29)
		yshift = -CLICK_SIZE;
	else if (frame == 57)
		yshift = -HALF_BLOCK_SIZE;
	else
		return;

	item->pos.y_pos += yshift - CLICK_SIZE;
	result_r = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &shift_r, &ledge_r);
	result_l = LaraTestClimbUpPos(item, coll->radius, -(coll->radius + 120), &shift_l, &ledge_l);
	item->pos.y_pos += CLICK_SIZE;

	if (result_r && result_l && input & IN_FORWARD) {
		if (result_r < 0 || result_l < 0) {
			item->goal_anim_state = AS_CLIMBSTNC;
			AnimateLara(item);

			if (abs(ledge_r - ledge_l) <= 120) {
				item->goal_anim_state = AS_NULL;
				item->pos.y_pos += (ledge_r + ledge_l) / 2 - CLICK_SIZE;
			}
		} else {
			item->goal_anim_state = AS_CLIMBING;
			item->pos.y_pos -= yshift;
		}
	} else {
		item->goal_anim_state = AS_CLIMBSTNC;

		if (yshift)
			AnimateLara(item);
	}
}

void lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t frame, yshift, result_r, result_l, shift_r, shift_l;

	if (LaraCheckForLetGo(item, coll))
		return;

	if (item->anim_number != ANIM_CLIMBDOWN)
		return;

	frame = item->frame_number - anims[ANIM_CLIMBDOWN].frame_base;

	if (!frame)
		yshift = 0;
	else if (frame == 28 || frame == 29)
		yshift = CLICK_SIZE;
	else if (frame == 57)
		yshift = HALF_BLOCK_SIZE;
	else
		return;

	item->pos.y_pos += yshift + CLICK_SIZE;
	result_r = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift_r);
	result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift_l);
	item->pos.y_pos -= CLICK_SIZE;

	if (result_r && result_l && input & IN_BACK) {
		if (shift_r && shift_l && shift_r < 0 != shift_l < 0) {
			item->goal_anim_state = AS_CLIMBSTNC;
			AnimateLara(item);
		} else if (result_r == -1 || result_l == -1) {
			item->anim_number = ANIM_CLIMBSTNC;
			item->frame_number = anims[ANIM_CLIMBSTNC].frame_base;
			item->current_anim_state = AS_CLIMBSTNC;
			item->goal_anim_state = AS_HANG;
			AnimateLara(item);
		} else {
			item->goal_anim_state = AS_CLIMBDOWN;
			item->pos.y_pos -= yshift;
		}
	} else {
		item->goal_anim_state = AS_CLIMBSTNC;

		if (yshift)
			AnimateLara(item);
	}
}

int16_t GetClimbTrigger(int32_t x, int32_t y, int32_t z, int16_t room_number) {
	FLOOR_INFO* floor;
	int16_t* data;

	floor = GetFloor(x, y, z, &room_number);
	GetHeight(floor, x, y, z);

	if (!trigger_index)
		return 0;

	data = trigger_index;

	if ((*data & 0x1F) == LAVA_TYPE) {
		if (*data & 0x8000)
			return 0;

		data++;
	}

	if ((*data & 0x1F) == CLIMB_TYPE)
		return *data;

	return 0;
}

int32_t LaraTestClimb(int32_t x, int32_t y, int32_t z, int32_t xfront, int32_t zfront, int32_t item_height, int16_t item_room, int32_t* shift) {
	FLOOR_INFO* floor;
	int32_t hang, h, c;
	int16_t room_number;

	*shift = 0;
	hang = 1;

	if (!lara.climb_status)
		return 0;

	room_number = item_room;
	floor = GetFloor(x, y - HALF_CLICK_SIZE, z, &room_number);
	h = GetHeight(floor, x, y, z);

	if (h == NO_HEIGHT)
		return 0;

	h -= HALF_CLICK_SIZE + y + item_height;

	if (h < -70)
		return 0;

	if (h < 0)
		*shift = h;

	c = GetCeiling(floor, x, y, z) - y;

	if (c > 70)
		return 0;

	if (c > 0) {
		if (*shift)
			return 0;

		*shift = c;
	}

	if (item_height + h < 900)
		hang = 0;

	floor = GetFloor(xfront + x, y, zfront + z, &room_number);
	h = GetHeight(floor, xfront + x, y, zfront + z);

	if (h != NO_HEIGHT)
		h -= y;

	if (h > 70) {
		c = GetCeiling(floor, xfront + x, y, zfront + z) - y;

		if (c >= HALF_BLOCK_SIZE)
			return 1;

		if (c > 442) {
			if (*shift > 0) {
				if (hang)
					return -1;
				else
					return 0;
			}

			*shift = c - HALF_BLOCK_SIZE;
			return 1;
		}

		if (c > 0) {
			if (hang)
				return -1;
			else
				return 0;
		}

		if (c <= -70 || !hang || *shift > 0)
			return 0;

		if (*shift > c)
			*shift = c;

		return -1;
	}

	if (h > 0) {
		if (*shift < 0)
			return 0;

		if (h > *shift)
			*shift = h;
	}

	room_number = item_room;
	GetFloor(x, y + item_height, z, &room_number);
	floor = GetFloor(xfront + x, y + item_height, zfront + z, &room_number);
	c = GetCeiling(floor, xfront + x, y + item_height, zfront + z);

	if (c == NO_HEIGHT)
		return 1;

	c -= y;

	if (c <= h || c >= HALF_BLOCK_SIZE)
		return 1;

	if (c <= 442) {
		if (hang)
			return -1;
		else
			return 0;
	}

	if (*shift > 0) {
		if (hang)
			return -1;
		else
			return 0;
	}

	*shift = c - HALF_BLOCK_SIZE;
	return 1;
}

int32_t LaraTestClimbPos(ITEM_INFO* item, int32_t front, int32_t right, int32_t origin, int32_t height, int32_t* shift) {
	int32_t angle, x, z, xfront, zfront;

	xfront = 0;
	zfront = 0;
	angle = uint16_t(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle) {
		case NORTH:
			x = right + item->pos.x_pos;
			z = front + item->pos.z_pos;
			zfront = CLICK_SIZE;
			break;

		case EAST:
			x = front + item->pos.x_pos;
			z = item->pos.z_pos - right;
			xfront = CLICK_SIZE;
			break;

		case SOUTH:
			x = item->pos.x_pos - right;
			z = item->pos.z_pos - front;
			zfront = -CLICK_SIZE;
			break;

		default:
			x = item->pos.x_pos - front;
			z = right + item->pos.z_pos;
			xfront = -CLICK_SIZE;
			break;
	}

	return LaraTestClimb(x, origin + item->pos.y_pos, z, xfront, zfront, height, item->room_number, shift);
}

int32_t LaraTestClimbUpPos(ITEM_INFO* item, int32_t front, int32_t right, int32_t* shift, int32_t* ledge) {
	FLOOR_INFO* floor;
	int32_t angle, x, y, z, xfront, zfront, h, c;
	int16_t room_number;

	xfront = 0;
	zfront = 0;
	y = item->pos.y_pos - (HALF_BLOCK_SIZE + CLICK_SIZE);
	angle = uint16_t(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle) {
		case NORTH:
			x = right + item->pos.x_pos;
			z = front + item->pos.z_pos;
			zfront = 4;
			break;

		case EAST:
			x = front + item->pos.x_pos;
			z = item->pos.z_pos - right;
			xfront = 4;
			break;

		case SOUTH:
			x = item->pos.x_pos - right;
			z = item->pos.z_pos - front;
			zfront = -4;
			break;

		default:
			x = item->pos.x_pos - front;
			z = right + item->pos.z_pos;
			xfront = -4;
			break;
	}

	*shift = 0;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	c = CLICK_SIZE - y + GetCeiling(floor, x, y, z);

	if (c > 70)
		return 0;

	if (c > 0)
		*shift = c;

	floor = GetFloor(x + xfront, y, z + zfront, &room_number);
	h = GetHeight(floor, x + xfront, y, z + zfront);

	if (h == NO_HEIGHT) {
		*ledge = NO_HEIGHT;
		return 1;
	}

	h -= y;
	*ledge = h;

	if (h > HALF_CLICK_SIZE) {
		c = GetCeiling(floor, x+xfront, y, z+zfront) - y;

		if (c >= HALF_BLOCK_SIZE)
			return 1;

		if (h - c > ((HALF_BLOCK_SIZE + CLICK_SIZE) - 6)) {
			*shift = h;
			return -1;
		}
	} else {
		if (h > 0 && h > *shift)
			*shift = h;

		room_number = item->room_number;
		GetFloor(x, y + HALF_BLOCK_SIZE, z, &room_number);
		floor = GetFloor(x + xfront, y + HALF_BLOCK_SIZE, z + zfront, &room_number);
		c = GetCeiling(floor, x + xfront, y + HALF_BLOCK_SIZE, z + zfront) - y;

		if (c <= h || c >= HALF_BLOCK_SIZE)
			return 1;
	}

	return 0;
}

int32_t LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll) {
	FLOOR_INFO* floor;
	int16_t room_number;

	item->fallspeed = 0;
	item->gravity_status = 0;
	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	coll->trigger_index = trigger_index;

	if (!(input & IN_ACTION) || item->hit_points <= 0) {
		lara.torso_x_rot = 0;
		lara.torso_y_rot = 0;
		lara.head_x_rot = 0;
		lara.head_y_rot = 0;
		item->anim_number = ANIM_FALLDOWN;
		item->frame_number = anims[ANIM_FALLDOWN].frame_base;
		item->current_anim_state = AS_FORWARDJUMP;
		item->goal_anim_state = AS_FORWARDJUMP;
		item->speed = 2;
		item->gravity_status = 1;
		item->fallspeed = 1;
		lara.gun_status = LG_NO_ARMS;
		return 1;
	}

	return 0;
}

int32_t LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t flag, oldX, oldZ, x, z, shift;
	int16_t oldY, angle;

	flag = 0;

	if (item->anim_number != 171)
		return 0;

	oldX = item->pos.x_pos;
	oldY = item->pos.y_rot;
	oldZ = item->pos.z_pos;
	angle = uint16_t(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle) {
		case NORTH:
		case SOUTH:
			x = (oldX & ~0x3FF) - (oldZ & 0x3FF) + BLOCK_SIZE;
			z = (oldZ & ~0x3FF) - (oldX & 0x3FF) + BLOCK_SIZE;
			break;

		default:
			x = (oldX & ~0x3FF) + (oldZ & 0x3FF);
			z = (oldZ & ~0x3FF) + (oldX & 0x3FF);
			break;
	}

	if (GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & LeftIntRightExtTab[angle]) {
		item->pos.x_pos = x;
		item->pos.z_pos = z;
		lara.CornerX = (void*)(size_t(x) & 0xffffffff);
		lara.CornerZ = (void*)(size_t(z) & 0xffffffff);
		item->pos.y_rot -= 0x4000;
		lara.move_angle = item->pos.y_rot;
		flag = LaraTestClimbPos(item, coll->radius, -120 - coll->radius, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift);
		item->item_flags[3] = (int16_t)flag;

		if (flag)
			flag = -1;
	}

	if (!flag) {
		item->pos.x_pos = oldX;
		item->pos.y_rot = oldY;
		item->pos.z_pos = oldZ;
		lara.move_angle = oldY;

		switch (angle) {
			case NORTH:
				x= (item->pos.x_pos ^ (((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF)) - BLOCK_SIZE;
				z = (((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF) ^ (item->pos.z_pos + BLOCK_SIZE);
				break;

			case SOUTH:
				x = (((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF) ^ (item->pos.x_pos + BLOCK_SIZE);
				z = (((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF) ^ (item->pos.z_pos - BLOCK_SIZE);
				break;

			case WEST:
				x = (item->pos.x_pos & ~0x3FFu) - (item->pos.z_pos & 0x3FF);
				z = (item->pos.z_pos & ~0x3FFu) - (item->pos.x_pos & 0x3FF);
				break;

			default:
				x = ((item->pos.x_pos + BLOCK_SIZE) & ~0x3FF) - (item->pos.z_pos & 0x3FF) + BLOCK_SIZE;
				z = ((item->pos.z_pos + BLOCK_SIZE) & ~0x3FF) - (item->pos.x_pos & 0x3FF) + BLOCK_SIZE;
				break;
		}

		if (GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & LeftExtRightIntTab[angle]) {
			item->pos.x_pos = x;
			item->pos.z_pos = z;
			lara.CornerX = (void*)(size_t(x) & 0xffffffff);
			lara.CornerX = (void*)(size_t(z) & 0xffffffff);
			item->pos.y_rot += 0x4000;
			lara.move_angle = item->pos.y_rot;
			flag = LaraTestClimbPos(item, coll->radius, -120 - coll->radius, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift);
			item->item_flags[3] = (int16_t)flag;

			if (flag)
				flag = 1;
		}
	}

	item->pos.x_pos = oldX;
	item->pos.y_rot = oldY;
	item->pos.z_pos = oldZ;
	lara.move_angle = oldY;
	return flag;
}

int32_t LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll) {
	int32_t flag, oldX, oldZ, x, z, shift;
	int16_t oldY, angle;

	flag = 0;

	if (item->anim_number != 170)
		return 0;

	oldX = item->pos.x_pos;
	oldY = item->pos.y_rot;
	oldZ = item->pos.z_pos;
	angle = uint16_t(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle) {
		case NORTH:
		case SOUTH:
			x = (oldX & ~0x3FF) + (oldZ & 0x3FF);
			z = (oldZ & ~0x3FF) + (oldX & 0x3FF);
			break;

		default:
			x = (oldX & ~0x3FF) - (oldZ & 0x3FF) + BLOCK_SIZE;
			z = (oldZ & ~0x3FF) - (oldX & 0x3FF) + BLOCK_SIZE;
			break;
	}

	if (GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & LeftExtRightIntTab[angle]) {
		item->pos.x_pos = x;
		item->pos.z_pos = z;
		lara.CornerX = (void*)(size_t(x) & 0xffffffff);
		lara.CornerX = (void*)(size_t(z) & 0xffffffff);
		item->pos.y_rot += 0x4000;
		lara.move_angle = item->pos.y_rot;
		flag = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift);

		if (flag)
			flag = -1;
	}

	if (!flag) {
		item->pos.x_pos = oldX;
		item->pos.y_rot = oldY;
		item->pos.z_pos = oldZ;
		lara.move_angle = oldY;

		switch (angle) {
			case NORTH:
				x = ((item->pos.x_pos + BLOCK_SIZE) & ~0x3FF) - (item->pos.z_pos & 0x3FF) + BLOCK_SIZE;
				z = ((item->pos.z_pos + BLOCK_SIZE) & ~0x3FF) - (item->pos.x_pos & 0x3FF) + BLOCK_SIZE;
				break;

			case SOUTH:
				x = ((item->pos.x_pos - BLOCK_SIZE) & ~0x3FF) - (item->pos.z_pos & 0x3FF) + BLOCK_SIZE;
				z = ((item->pos.z_pos - BLOCK_SIZE) & ~0x3FF) - (item->pos.x_pos & 0x3FF) + BLOCK_SIZE;
				break;

			case WEST:
				x = (item->pos.x_pos ^ ((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF) - BLOCK_SIZE;
				z = ((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF ^ (item->pos.z_pos + BLOCK_SIZE);
				break;

			default:
				x = (((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF) ^ (item->pos.x_pos + BLOCK_SIZE);
				z = (item->pos.z_pos ^ (((uint16_t)item->pos.z_pos ^ (uint16_t)item->pos.x_pos) & 0x3FF)) - BLOCK_SIZE;
				break;
		}

		if (GetClimbTrigger(x, item->pos.y_pos, z, item->room_number) & LeftIntRightExtTab[angle]) {
			item->pos.x_pos = x;
			item->pos.z_pos = z;
			lara.CornerX = (void*)(size_t(x) & 0xffffffff);
			lara.CornerZ = (void*)(size_t(z) & 0xffffffff);
			item->pos.y_rot -= 0x4000;
			lara.move_angle = item->pos.y_rot;
			flag = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, &shift);

			if (flag)
				flag = 1;
		}
	}

	item->pos.x_pos = oldX;
	item->pos.y_rot = oldY;
	item->pos.z_pos = oldZ;
	lara.move_angle = oldY;
	return flag;
}

void LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int32_t result, int32_t shift) {
	int32_t flag;

	if (result == 1) {
		if (input & IN_LEFT)
			item->goal_anim_state = AS_CLIMBLEFT;
		else if (input & IN_RIGHT)
			item->goal_anim_state = AS_CLIMBRIGHT;
		else
			item->goal_anim_state = AS_CLIMBSTNC;

		item->pos.y_pos += shift;
		return;
	} else if (!result) {
		item->pos.x_pos = coll->old.x;
		item->pos.z_pos = coll->old.z;
		item->current_anim_state = AS_CLIMBSTNC;
		item->goal_anim_state = AS_CLIMBSTNC;

		if (coll->old_anim_state != AS_CLIMBSTNC) {
			item->anim_number = ANIM_CLIMBSTNC;
			item->frame_number = anims[ANIM_CLIMBSTNC].frame_base;
			return;
		}

		if (input & IN_LEFT) {
			flag = LaraClimbLeftCornerTest(item, coll);

			if (flag) {
				if (flag > 0) {
					item->anim_number = ANIM_EXTCLIMBL;
					item->frame_number = anims[ANIM_EXTCLIMBL].frame_base;
					item->current_anim_state = AS_CORNEREXTL;
					item->goal_anim_state = AS_CORNEREXTL;
				} else {
					item->anim_number = ANIM_INTCLIMBL;
					item->frame_number = anims[ANIM_INTCLIMBL].frame_base;
					item->current_anim_state = AS_CORNERINTL;
					item->goal_anim_state = AS_CORNERINTL;
				}

				return;
			}
		} else if (input & IN_RIGHT) {
			flag = LaraClimbRightCornerTest(item, coll);

			if (flag) {
				if (flag > 0) {
					item->anim_number = ANIM_EXTCLIMBR;
					item->frame_number = anims[ANIM_EXTCLIMBR].frame_base;
					item->current_anim_state = AS_CORNEREXTR;
					item->goal_anim_state = AS_CORNEREXTR;
				} else {
					item->anim_number = ANIM_INTCLIMBR;
					item->frame_number = anims[ANIM_INTCLIMBR].frame_base;
					item->current_anim_state = AS_CORNERINTR;
					item->goal_anim_state = AS_CORNERINTR;
				}

				return;
			}
		}

		item->frame_number = coll->old_frame_number;
		item->anim_number = coll->old_anim_number;
		AnimateLara(item);
	} else {
		item->goal_anim_state = AS_HANG;

		do AnimateItem(item);
		while (item->current_anim_state != AS_HANG);

		item->pos.x_pos = coll->old.x;
		item->pos.z_pos = coll->old.z;
	}
}
