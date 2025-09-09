#include "../tomb4/pch.h"
#include "gameflow.h"
#include "moveblok.h"
#include "control.h"
#include "collide.h"
#include "objects.h"
#include "lara_states.h"
#include "sound.h"
#include "delstuff.h"
#include "items.h"
#include "draw.h"
#include "sphere.h"
#include "deltapak.h"
#include "../specific/function_stubs.h"
#include "tomb4fx.h"
#include "../specific/3dmath.h"
#include "../specific/output.h"
#include "box.h"
#include "../specific/input.h"
#include "lara.h"

#include "trng/trng_extra_state.h"

#include "../tomb4/mod_config.h"
#include "../specific/file.h"
#include "../tomb4/tomb4plus/t4plus_items.h"

static int16_t MovingBlockBounds[] = {
	0,
	0,
	-CLICK_SIZE,
	0,
	0,
	0,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static PHD_VECTOR MovingBlockPos = { 0, 0, 0 };

static void ClearMovableBlockSplitters(int32_t x, int32_t y, int32_t z, int16_t room_number) {
	FLOOR_INFO* floor;
	int16_t room_num, height;

	floor = GetFloor(x, y, z, &room_number);
	boxes[floor->box].overlap_index &= ~BLOCKED; // T4Plus - fix mask.
	height = boxes[floor->box].height;
	room_num = room_number;
	floor = GetFloor(x + BLOCK_SIZE, y, z, &room_number);

	if (floor->box != 0x7FF) {
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & BLOCKABLE && boxes[floor->box].overlap_index & BLOCKED) {
			ClearMovableBlockSplitters(x + BLOCK_SIZE, y, z, room_number);
		}
	}

	room_number = room_num;
	floor = GetFloor(x - BLOCK_SIZE, y, z, &room_number);

	if (floor->box != 0x7FF) {
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & BLOCKABLE && boxes[floor->box].overlap_index & BLOCKED) {
			ClearMovableBlockSplitters(x - BLOCK_SIZE, y, z, room_number);
		}
	}

	room_number = room_num;
	floor = GetFloor(x, y, z + BLOCK_SIZE, &room_number);

	if (floor->box != 0x7FF) {
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & BLOCKABLE && boxes[floor->box].overlap_index & BLOCKED) {
			ClearMovableBlockSplitters(x, y, z + BLOCK_SIZE, room_number);
		}
	}

	room_number = room_num;
	floor = GetFloor(x, y, z - BLOCK_SIZE, &room_number);

	if (floor->box != 0x7FF) {
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & BLOCKABLE && boxes[floor->box].overlap_index & BLOCKED) {
			ClearMovableBlockSplitters(x, y, z - BLOCK_SIZE, room_number);
		}
	}
}

int32_t GetMoveableBlockHeight(int16_t item_number) {
	MOD_GLOBAL_INFO* global_info = get_game_mod_global_info();
	MOD_LEVEL_MISC_INFO* misc_info = get_game_mod_level_misc_info(gfCurrentLevel);

	ITEM_INFO *item = T4PlusGetItemInfoForID(item_number);

	int32_t climbable_block_height = 0;
	if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
		climbable_block_height = item->trigger_flags & 0xf;
	} else if (misc_info->enable_standing_pushables) {
		climbable_block_height = (item->trigger_flags & 0xf00) >> 8;
	}

	return climbable_block_height;
}

void InitialiseMovingBlock(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	ClearMovableBlockSplitters(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
	MOD_LEVEL_MISC_INFO *misc_info = get_game_mod_level_misc_info(gfCurrentLevel);

	// TRNG
	int32_t climbable_block_height = GetMoveableBlockHeight(item_number);

	if (climbable_block_height) {
		if (item->status == ITEM_INACTIVE) {
			AlterFloorHeight(item, -climbable_block_height * CLICK_SIZE);
		}
	}
}

static int32_t TestBlockPush(ITEM_INFO* item, int32_t height, uint16_t quadrant, bool can_push_over_ledges) {
	ITEM_INFO** itemlist;
	ITEM_INFO* collided;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	int32_t x, y, z, rx, rz;
	int16_t room_number;

	// TRNG
	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();

	if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
		if (item->trigger_flags & 0x100) { // TRNG: Pushing disabled
			return 0;
		}

		if (quadrant == EAST || quadrant == WEST) {
			if (item->trigger_flags & 0x400) { // TRNG: East and West direction disabled
				return 0;
			}
		} else if (quadrant == NORTH || quadrant == SOUTH) {
			if (item->trigger_flags & 0x200) { // TRNG: North and South direction disabled
				return 0;
			}
		}
	}

	x = item->pos.x_pos;
	y = item->pos.y_pos;
	z = item->pos.z_pos;
	itemlist = (ITEM_INFO**)&tsv_buffer[0];

	switch (quadrant) {
		case NORTH:
			z += BLOCK_SIZE;
			break;

		case EAST:
			x += BLOCK_SIZE;
			break;

		case SOUTH:
			z -= BLOCK_SIZE;
			break;

		case WEST:
			x -= BLOCK_SIZE;
			break;
	}

	room_number = item->room_number;
	floor = GetFloor(x, y - CLICK_SIZE, z, &room_number);
	r = &room[room_number];
	rx = (x - r->x) >> WALL_SHIFT;
	rz = (z - r->z) >> WALL_SHIFT;

	if (r->floor[rx * r->x_size + rz].stopper)
		return 0;

	if (can_push_over_ledges) {
		if (GetHeight(floor, x, y - CLICK_SIZE, z) < y)
			return 0;
	} else {
		if (GetHeight(floor, x, y - CLICK_SIZE, z) != y)
			return 0;
	}

	GetHeight(floor, x, y, z);

	if (height_type != WALL)
		return 0;

	y -= height - 100;
	floor = GetFloor(x, y, z, &room_number);

	if (GetCeiling(floor, x, y, z) > y)
		return 0;

	rx = item->pos.x_pos;
	rz = item->pos.z_pos;
	item->pos.x_pos = x;
	item->pos.z_pos = z;
	GetCollidedObjects(item, CLICK_SIZE, 1, itemlist, 0, 0);
	item->pos.x_pos = rx;
	item->pos.z_pos = rz;

	if (itemlist[0]) {
		for (int32_t i = 0; itemlist[0] != 0; i++, itemlist++) {
			collided = itemlist[0];

			if (collided->object_number == TWOBLOCK_PLATFORM || collided->object_number == HAMMER)
				return 1;
		}

		return 0;
	}

	return 1;
}

static int32_t TestBlockPull(ITEM_INFO* item, int32_t height, uint16_t quadrant) {
	ITEM_INFO** itemlist;
	ITEM_INFO* collided;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	int32_t x, y, z, destx, destz, rx, rz, ignore;
	int16_t room_number;

	// TRNG
	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();

	if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
		if (item->trigger_flags & 0x80) { // TRNG: Pulling disabled
			return 0;
		}

		if (quadrant == EAST || quadrant == WEST) {
			if (item->trigger_flags & 0x400) { // TRNG: East and West direction disabled
				return 0;
			}
		} else if (quadrant == NORTH || quadrant == SOUTH) {
			if (item->trigger_flags & 0x200) { // TRNG: North and South direction disabled
				return 0;
			}
		}
	}

	itemlist = (ITEM_INFO**)&tsv_buffer[0];
	destx = 0;
	destz = 0;

	switch (quadrant) {
		case NORTH:
			destz = -BLOCK_SIZE;
			break;

		case EAST:
			destx = -BLOCK_SIZE;
			break;

		case SOUTH:
			destz = BLOCK_SIZE;
			break;

		case WEST:
			destx = BLOCK_SIZE;
			break;
	}

	x = item->pos.x_pos + destx;
	y = item->pos.y_pos;
	z = item->pos.z_pos + destz;
	room_number = item->room_number;
	floor = GetFloor(x, y - CLICK_SIZE, z, &room_number);
	r = &room[room_number];
	rx = (x - r->x) >> WALL_SHIFT;
	rz = (z - r->z) >> WALL_SHIFT;

	if (r->floor[rx * r->x_size + rz].stopper)
		return 0;

	if (GetHeight(floor, x, y - CLICK_SIZE, z) != y)
		return 0;

	floor = GetFloor(x, y - height, z, &room_number);

	if (floor->ceiling << 8 > y - height)
		return 0;

	rx = item->pos.x_pos;
	rz = item->pos.z_pos;
	item->pos.x_pos = x;
	item->pos.z_pos = z;
	GetCollidedObjects(item, CLICK_SIZE, 1, itemlist, 0, 0);
	item->pos.x_pos = rx;
	item->pos.z_pos = rz;

	if (itemlist[0]) {
		ignore = 0;

		for (int32_t i = 0; itemlist[0] != 0; i++, itemlist++) {
			collided = itemlist[0];

			if (collided->object_number == TWOBLOCK_PLATFORM || collided->object_number == HAMMER) {
				ignore = 1;
				break;
			}
		}

		if (!ignore)
			return 0;
	}

	x += destx;
	z += destz;
	room_number = item->room_number;
	floor = GetFloor(x, y - CLICK_SIZE, z, &room_number);

	if (GetHeight(floor, x, y - CLICK_SIZE, z) != y)
		return 0;

	floor = GetFloor(x, y - ((HALF_BLOCK_SIZE + CLICK_SIZE) - 6), z, &room_number);

	if (floor->ceiling << 8 > y - ((HALF_BLOCK_SIZE + CLICK_SIZE) - 6))
		return 0;

	x = lara_item->pos.x_pos + destx;
	y = lara_item->pos.y_pos;
	z = lara_item->pos.z_pos + destz;
	room_number = lara_item->room_number;
	GetFloor(x, y, z, &room_number);
	r = &room[room_number];
	rx = (x - r->x) >> WALL_SHIFT;
	rz = (z - r->z) >> WALL_SHIFT;

	if (r->floor[rx * r->x_size + rz].stopper)
		return 0;

	rx = lara_item->pos.x_pos;
	rz = lara_item->pos.z_pos;
	lara_item->pos.x_pos = x;
	lara_item->pos.z_pos = z;
	GetCollidedObjects(lara_item, CLICK_SIZE, 1, itemlist, 0, 0);
	lara_item->pos.x_pos = rx;
	lara_item->pos.z_pos = rz;

	if (itemlist[0]) {
		for (int32_t i = 0; itemlist[0] != 0; i++, itemlist++) {
			collided = itemlist[0];

			if (collided == item || collided->object_number == TWOBLOCK_PLATFORM || collided->object_number == HAMMER)
				return 1;
		}

		return 0;
	}

	return 1;
}

void MovableBlock(int16_t item_number) {
	ITEM_INFO* item;
	PHD_VECTOR pos;
	int32_t offset;
	uint16_t quadrant;
	int16_t frame, base;
	static int8_t sfx = 0;

	item = &items[item_number];

	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
	MOD_LEVEL_MISC_INFO *misc_info = get_game_mod_level_misc_info(gfCurrentLevel);
	// TRNG
	int32_t climbable_block_height = GetMoveableBlockHeight(item_number);

	// TRNG
	if (global_info->trng_pushables_have_gravity) {
		int16_t room_number = item->room_number;
		FLOOR_INFO *floor_info = GetFloor(item->pos.x_pos, item->pos.y_pos - 128, item->pos.z_pos, &room_number);
		int32_t height = GetHeight(floor_info, item->pos.x_pos, item->pos.y_pos - 128, item->pos.z_pos);

		if (item->pos.y_pos < height) {
			item->gravity_status = 1;
			ApplyItemGravity(item);

			floor_info = GetFloor(item->pos.x_pos, item->pos.y_pos - 128, item->pos.z_pos, &room_number);
			height = GetHeight(floor_info, item->pos.x_pos, item->pos.y_pos - 128, item->pos.z_pos);
			if (item->room_number != room_number)
				ItemNewRoom(item_number, room_number);
		} else if (item->gravity_status) {
			item->gravity_status = 0;
			item->pos.y_pos = height;
			SoundEffect(SFX_BOULDER_FALL, &item->pos, SFX_DEFAULT);

			// If the object has landed and Lara is not performing animations, complete the sequence.
			if (lara_item->anim_number != LARA_ANIM_PULL && lara_item->anim_number != LARA_ANIM_PUSH && lara_item->anim_number != 417 && lara_item->anim_number != 418) {
				floor_info = GetFloor(item->pos.x_pos, item->pos.y_pos - 128, item->pos.z_pos, &room_number);
				height = GetHeight(floor_info, item->pos.x_pos, item->pos.y_pos - 128, item->pos.z_pos);

				TestTriggers(trigger_index, true, item->flags & IFL_CODEBITS);
				RemoveActiveItem(item_number);
				item->status = ITEM_INACTIVE;

				if (climbable_block_height > 0) {
					if (item->room_number != room_number)
						ItemNewRoom(item_number, room_number);

					AlterFloorHeight(item, -climbable_block_height * CLICK_SIZE);
				}
			}
		}
	}

	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	quadrant = uint16_t(lara_item->pos.y_rot + 0x2000) / 0x4000;

	switch (lara_item->anim_number) {
		case LARA_ANIM_PUSH:
			frame = lara_item->frame_number;
			base = anims[LARA_ANIM_PUSH].frame_base;

			if ((frame < base + 30 || frame > base + 67) && (frame < base + 78 || frame > base + 125) && (frame < base + 140 || frame > base + 160)) {
				if (sfx) {
					SoundEffect(SFX_PUSH_BLOCK_END, &item->pos, SFX_ALWAYS);
					sfx = 0;
				}
			} else {
				SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, SFX_ALWAYS);
				sfx = 1;
			}

			GetLaraJointPos(&pos, LMX_HAND_L);

			switch (quadrant) {
				case NORTH:
					offset = pos.z + *(int32_t*)&item->item_flags[2] - *(int32_t*)&lara_item->item_flags[2];

					if (abs(item->pos.z_pos - offset) < HALF_BLOCK_SIZE && item->pos.z_pos < offset)
						item->pos.z_pos = offset;

					break;

				case EAST:
					offset = pos.x + *(int32_t*)item->item_flags - *(int32_t*)lara_item->item_flags;

					if (abs(item->pos.x_pos - offset) < HALF_BLOCK_SIZE && item->pos.x_pos < offset)
						item->pos.x_pos = offset;

					break;

				case SOUTH:
					offset = pos.z + *(int32_t*)&item->item_flags[2] - *(int32_t*)&lara_item->item_flags[2];

					if (abs(item->pos.z_pos - offset) < HALF_BLOCK_SIZE && item->pos.z_pos > offset)
						item->pos.z_pos = offset;

					break;

				case WEST:
					offset = pos.x + *(int32_t*)item->item_flags - *(int32_t*)lara_item->item_flags;

					if (abs(item->pos.x_pos - offset) < HALF_BLOCK_SIZE && item->pos.x_pos > offset)
						item->pos.x_pos = offset;

					break;
			}


			if (lara_item->frame_number == anims[lara_item->anim_number].frame_end - 1) {
				// T4Plus: Update Room
				UpdateItemRoom(item_number, -CLICK_SIZE);

				if (input & IN_ACTION) {
					// TRNG
					MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
					bool can_push_over_ledges = false;
					if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
						if (!(item->trigger_flags & 0x100)) { // TRNG: Pushing disabled
							can_push_over_ledges = item->trigger_flags & 0x20;
						}
					}

					if (!TestBlockPush(item, BLOCK_SIZE, quadrant, can_push_over_ledges))
						lara_item->goal_anim_state = 2;
				} else {
					lara_item->goal_anim_state = 2;
				}
			}

			break;

		case LARA_ANIM_PULL:
			frame = lara_item->frame_number;
			base = anims[LARA_ANIM_PULL].frame_base;

			if ((frame < base + 40 || frame > base + 122) && (frame < base + 130 || frame > base + 170)) {
				if (sfx) {
					SoundEffect(SFX_PUSH_BLOCK_END, &item->pos, SFX_ALWAYS);
					sfx = 0;
				}
			} else {
				SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, SFX_ALWAYS);
				sfx = 1;
			}

			GetLaraJointPos(&pos, LMX_HAND_L);

			switch (quadrant) {
				case NORTH:
					offset = pos.z + *(int32_t*)&item->item_flags[2] - *(int32_t*)&lara_item->item_flags[2];

					if (abs(item->pos.z_pos - offset) < HALF_BLOCK_SIZE && item->pos.z_pos > offset)
						item->pos.z_pos = offset;

					break;

				case EAST:
					offset = pos.x + *(int32_t*)item->item_flags - *(int32_t*)lara_item->item_flags;

					if (abs(item->pos.x_pos - offset) < HALF_BLOCK_SIZE && item->pos.x_pos > offset)
						item->pos.x_pos = offset;

					break;

				case SOUTH:
					offset = pos.z + *(int32_t*)&item->item_flags[2] - *(int32_t*)&lara_item->item_flags[2];

					if (abs(item->pos.z_pos - offset) < HALF_BLOCK_SIZE && item->pos.z_pos < offset)
						item->pos.z_pos = offset;

					break;

				case WEST:
					offset = pos.x + *(int32_t*)item->item_flags - *(int32_t*)lara_item->item_flags;

					if (abs(item->pos.x_pos - offset) < HALF_BLOCK_SIZE && item->pos.x_pos < offset)
						item->pos.x_pos = offset;

					break;
			}

			if (lara_item->frame_number == anims[lara_item->anim_number].frame_end - 1) {
				// T4Plus: Update Room
				UpdateItemRoom(item_number, -CLICK_SIZE);

				if (input & IN_ACTION) {
					if (!TestBlockPull(item, BLOCK_SIZE, quadrant))
						lara_item->goal_anim_state = 2;
				} else {
					lara_item->goal_anim_state = 2;
				}
			}

			break;

		case 417:
		case 418:
			frame = lara_item->frame_number;

			if (frame == anims[417].frame_base || frame == anims[418].frame_base) {
				item->pos.x_pos = (item->pos.x_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				item->pos.z_pos = (item->pos.z_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
			}

			if (frame == anims[lara_item->anim_number].frame_end) {
				if (item->gravity_status == 0 || !global_info->trng_pushables_have_gravity) {
					int16_t room_number = item->room_number;
					GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos - CLICK_SIZE, item->pos.z_pos, &room_number),
					          item->pos.x_pos, item->pos.y_pos - 256, item->pos.z_pos);
					if (item->room_number != room_number)
						ItemNewRoom(item_number, room_number);

					TestTriggers(trigger_index, true, item->flags & IFL_CODEBITS);
					RemoveActiveItem(item_number);
					item->status = ITEM_INACTIVE;

					if (climbable_block_height > 0) {
						AlterFloorHeight(item, -climbable_block_height * CLICK_SIZE);
					}
				}
			}

			break;
	}
}

void MovableBlockCollision(int16_t item_number, ITEM_INFO* laraitem, COLL_INFO* coll) {
	ITEM_INFO* item;
	PHD_VECTOR pos;
	int16_t* bounds;
	int16_t room_number, yrot, quadrant;

	item = &items[item_number];

	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
	MOD_LEVEL_MISC_INFO* misc_info = get_game_mod_level_misc_info(gfCurrentLevel);

	// TRNG
	int32_t climbable_block_height = 0;
	if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
		climbable_block_height = item->trigger_flags & 0xf;
		// TREP
	} else if (misc_info->enable_standing_pushables) {
		climbable_block_height = (item->trigger_flags & 0xf00) >> 8;
	}

	room_number = item->room_number;

	// TODO: currently, climable blocks are not compatible with raising blocks.
	if (climbable_block_height == 0) {
		if (!get_game_mod_global_info()->trng_advanced_block_raising_behaviour) {
			item->pos.y_pos = GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos - CLICK_SIZE, item->pos.z_pos, &room_number),
			                            item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
		}
	}

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (input & IN_ACTION && laraitem->current_anim_state == AS_STOP && laraitem->anim_number == LARA_ANIM_BREATH && !laraitem->gravity_status &&
	        lara.gun_status == LG_NO_ARMS && item->status == ITEM_INACTIVE && item->trigger_flags >= 0 || (lara.IsMoving && lara.GeneralPtr == item_number)) {
		room_number = laraitem->room_number;
		GetFloor(item->pos.x_pos, item->pos.y_pos - CLICK_SIZE, item->pos.z_pos, &room_number);

		if (room_number == item->room_number) {
			// TRNG - disable movement in certain directions
			if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
				quadrant = (uint16_t)(laraitem->pos.y_rot + (BLOCK_SIZE * 8)) >> W2V_SHIFT;
				if (quadrant == EAST || quadrant == WEST) {
					if (item->trigger_flags & 0x400) { // TRNG: East and West direction disabled
						return;
					}
				} else if (quadrant == NORTH || quadrant == SOUTH) {
					if (item->trigger_flags & 0x200) { // TRNG: North and South direction disabled
						return;
					}
				}
			}

			bounds = GetBoundsAccurate(item);
			MovingBlockBounds[0] = bounds[0] - 100;
			MovingBlockBounds[1] = bounds[1] + 100;
			MovingBlockBounds[4] = bounds[4] - 200;
			MovingBlockBounds[5] = 0;
			yrot = item->pos.y_rot;
			item->pos.y_rot = (laraitem->pos.y_rot + 0x2000) & 0xC000;

			if (TestLaraPosition(MovingBlockBounds, item, laraitem)) {
				if (climbable_block_height != 0 && item->status == ITEM_INACTIVE) {
					// NGLE: remove the block collision immediately
					AlterFloorHeight(item, climbable_block_height * CLICK_SIZE);

					item->status = ITEM_ACTIVE;
				}

				if ((uint16_t(yrot + 0x2000) / 0x4000) + ((uint16_t)item->pos.y_rot / 0x4000) & 1)
					if (climbable_block_height == 0)
						MovingBlockPos.z = bounds[0] - 35;
					else
						MovingBlockPos.z = bounds[0] - 105;
				else if (climbable_block_height == 0)
					MovingBlockPos.z = bounds[4] - 35;
				else
					MovingBlockPos.z = bounds[0] - 105;

				if (MoveLaraPosition(&MovingBlockPos, item, laraitem)) {
					laraitem->anim_number = LARA_ANIM_PPREADY;
					laraitem->frame_number = anims[LARA_ANIM_PPREADY].frame_base;
					laraitem->current_anim_state = AS_PPREADY;
					laraitem->goal_anim_state = AS_PPREADY;
					lara.IsMoving = 0;
					lara.gun_status = LG_HANDS_BUSY;
					lara.CornerX = item;

					// NGLE: restore it once we can grab it
					if (climbable_block_height != 0 && item->status == ITEM_ACTIVE) {
						AlterFloorHeight(item, -climbable_block_height * CLICK_SIZE);

						item->status = ITEM_INACTIVE;
					}
				} else {
					lara.GeneralPtr = item_number;
				}
			} else if (lara.IsMoving && lara.GeneralPtr == item_number) {
				lara.IsMoving = 0;
				lara.gun_status = LG_NO_ARMS;
			}

			item->pos.y_rot = yrot;
		}
	} else if (laraitem->current_anim_state == AS_PPREADY && laraitem->frame_number == anims[LARA_ANIM_PPREADY].frame_base + 19 && lara.CornerX == item) {
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		quadrant = (uint16_t)(laraitem->pos.y_rot + (BLOCK_SIZE * 8)) >> W2V_SHIFT;

		if (input & IN_FORWARD) {
			// TRNG
			MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
			bool can_push_over_ledges = false;
			if (global_info->trng_pushable_extended_ocb && item->trigger_flags & 0x40) {
				if (!(item->trigger_flags & 0x100)) { // TRNG: Pushing disabled
					can_push_over_ledges = item->trigger_flags & 0x20;
				}
			}

			if (!TestBlockPush(item, BLOCK_SIZE, quadrant, can_push_over_ledges))
				return;

			laraitem->goal_anim_state = AS_PUSHBLOCK;

			if (climbable_block_height > 0) {
				// NGLE: Reset the floorstate and position back to normal
				AlterFloorHeight(item, climbable_block_height * CLICK_SIZE);
			}
		} else if (input & IN_BACK) {
			if (!TestBlockPull(item, BLOCK_SIZE, quadrant))
				return;

			laraitem->goal_anim_state = AS_PULLBLOCK;

			if (climbable_block_height > 0) {
				// NGLE: Reset the floorstate and position back to normal
				AlterFloorHeight(item, climbable_block_height * CLICK_SIZE);
			}
		} else {
			return;
		}

		AddActiveItem(item_number);
		item->status = ITEM_ACTIVE;
		lara.head_x_rot = 0;
		lara.head_y_rot = 0;
		lara.torso_x_rot = 0;
		lara.torso_y_rot = 0;
		GetLaraJointPos(&pos, LMX_HAND_L);
		*(int32_t*)&laraitem->item_flags[0] = pos.x;
		*(int32_t*)&laraitem->item_flags[2] = pos.z;
		*(int32_t*)&item->item_flags[0] = item->pos.x_pos;
		*(int32_t*)&item->item_flags[2] = item->pos.z_pos;
	} else {
		if (climbable_block_height == 0) {
			ObjectCollision(item_number, laraitem, coll);
		}
	}
}

void InitialisePlanetEffect(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	char* pifl;
	uint8_t others[4];

	item = &items[item_number];
	item->mesh_bits = 0;

	for (int32_t i = 0; i < level_items; i++) { //get the pushable we are linked to
		item2 = &items[i];

		if (item2->object_number >= PUSHABLE_OBJECT1 && item2->object_number <= PUSHABLE_OBJECT5 && item2->trigger_flags == item->trigger_flags) {
			item->item_flags[0] = i;
			break;
		}
	}

	if (item->trigger_flags == 1) { //get other planet effects
		for (int32_t i = 0, j = 0; i < level_items; i++) {
			item2 = &items[i];

			if (item2->object_number == PLANET_EFFECT && item_number != i)
				others[j++] = i;
		}

		pifl = (char*)&item->item_flags[2];

		for (int32_t i = 0; i < 4; i++) {
			for (int32_t j = 0; j < 4; j++) {
				item2 = &items[others[j]];

				if (item2->trigger_flags == i + 2) {
					*pifl++ = others[j];
					break;
				}
			}
		}
	}
}

void ControlPlanetEffect(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	char* pifl;
	int32_t b, g;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->item_flags[0] > 0) {
		items[item->item_flags[0]].trigger_flags = -items[item->item_flags[0]].trigger_flags;	//disable pushable :D
		item->item_flags[0] = NO_ITEM;
	}

	item->mesh_bits = 255;
	AnimateItem(item);

	if (item->trigger_flags == 1) {
		if ((items[(item->item_flags[2]) & 0xff].flags & IFL_CODEBITS) == IFL_CODEBITS &&
		        (items[(item->item_flags[2] >> 8) & 0xff].flags & IFL_CODEBITS) == IFL_CODEBITS &&
		        (items[(item->item_flags[3]) & 0xff].flags & IFL_CODEBITS) == IFL_CODEBITS &&
		        (items[(item->item_flags[3] >> 8) & 0xff].flags & IFL_CODEBITS) == IFL_CODEBITS) {
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;
			GetJointAbsPosition(item, &pos, 0);

			item2 = find_an_item_with_object_type(ANIMATING4);
			pos2.x = 0;
			pos2.y = 0;
			pos2.z = 0;
			GetJointAbsPosition(item2, &pos2, 0);

			b = (GetRandomControl() & 0x1F) + 224;
			g = b - (GetRandomControl() & 0x3F);
			TriggerLightningGlow(pos.x, pos.y, pos.z, RGBA(0, g, b, (GetRandomControl() & 0x1F) + (QUARTER_CLICK_SIZE - (QUARTER_CLICK_SIZE / 4))));
			TriggerLightningGlow(pos2.x, pos2.y, pos2.z, RGBA(0, g, b, (GetRandomControl() & 0x1F) + QUARTER_CLICK_SIZE));

			if (!(GlobalCounter & 3))
				TriggerLightning(&pos, &pos2, (GetRandomControl() & 0x1F) + 32, RGBA(0, g, b, 24), 1, 32, 5);

			pifl = (char*)&item->item_flags[2];

			for (int32_t i = 0; i < 4; i++) {
				pos2.x = 0;
				pos2.y = 0;
				pos2.z = 0;
				GetJointAbsPosition(&items[pifl[i]], &pos2, 0);

				if (!(GlobalCounter & 3))
					TriggerLightning(&pos2, &pos, (GetRandomControl() & 0x1F) + 32, RGBA(0, g, b, 24), 1, 32, 5);

				TriggerLightningGlow(pos.x, pos.y, pos.z, RGBA(0, g, b, (GetRandomControl() & 0x1F) + 48));
				SoundEffect(SFX_ELEC_ARCING_LOOP, (PHD_3DPOS*)&pos, SFX_DEFAULT);
				pos = pos2;
			}

			TriggerLightningGlow(pos2.x, pos2.y, pos2.z, RGBA(0, g, b, (GetRandomControl() & 0x1F) + 48));
		}
	}
}

void DrawPlanetEffect(ITEM_INFO* item) {
	OBJECT_INFO* obj;
	int16_t** meshpp;
	int32_t* bone;
	int16_t* frm[2];
	int16_t* rot;
	int32_t poppush;

	// T4Plus: Animation safety check
	if (item->anim_number < 0 || item->anim_number >= num_anims) {
		return;
	}

	if (!item->mesh_bits)
		return;

	GetFrames(item, frm, &poppush);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	CalculateObjectLighting(item, frm[0]);

	obj = &objects[item->object_number];
	meshpp = &meshes[obj->mesh_index];
	bone = &bones[obj->bone_index];
	phd_TranslateRel(frm[0][6], frm[0][7], frm[0][8]);
	rot = frm[0] + 9;
	gar_RotYXZsuperpack(&rot, 0);
	phd_PutPolygons(*meshpp, -1);
	phd_PutPolygons(*meshpp, -1);
	meshpp += 2;

	for (int32_t i = 0; i < obj->nmeshes - 1; i++, bone += 4, meshpp += 2) {
		poppush = bone[0];

		//These look inverted..
		if (poppush & 1)
			phd_PopMatrix();

		if (poppush & 2)
			phd_PushMatrix();

		phd_TranslateRel(bone[1], bone[2], bone[3]);
		gar_RotYXZsuperpack(&rot, 0);
		phd_PutPolygons(*meshpp, -1);
	}

	phd_PopMatrix();
}
