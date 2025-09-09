#include "../tomb4/pch.h"
#include "larafire.h"
#include "lara_states.h"
#include "objects.h"
#include "lara2gun.h"
#include "lara1gun.h"
#include "laraflar.h"
#include "delstuff.h"
#include "draw.h"
#include "../specific/3dmath.h"
#include "control.h"
#include "../specific/function_stubs.h"
#include "sphere.h"
#include "effect2.h"
#include "sound.h"
#include "effects.h"
#include "items.h"
#include "flmtorch.h"
#include "camera.h"
#include "../specific/input.h"
#include "lara.h"
#include "savegame.h"
#include "lot.h"
#include "gameflow.h"
#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_objects.h"

WEAPON_INFO weapons[9] = {
	{
		0, 0, 0, 0,							//WEAPON_NONE
		0, 0, 0, 0,
		0, 0, 0, 0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	{
		-10920, 10920, -10920, 10920,		//WEAPON_PISTOLS
		-30940, 10920, -14560, 14560,
		-10920, 30940, -14560, 14560,
		1820,
		1456,
		650,
		8192,
		1,
		9,
		3,
		0,
		SFX_LARA_FIRE
	},

	{
		-10920, 10920, -10920, 10920,		//WEAPON_REVOLVER
		-1820, 1820, -14560, 14560,
		0, 0, 0, 0,
		1820,
		728,
		650,
		8192,
		21,
		16,
		3,
		0,
		SFX_DESERT_EAGLE_FIRE
	},

	{
		-10920, 10920, -10920, 10920,		//WEAPON_UZI
		-30940, 10920, -14560, 14560,
		-10920, 30940, -14560, 14560,
		1820,
		1456,
		650,
		8192,
		1,
		3,
		3,
		0,
		SFX_LARA_UZI_FIRE
	},

	{
		-10920, 10920, -10010, 10010,		//WEAPON_SHOTGUN
		-14560, 14560, -11830, 11830,
		-14560, 14560, -11830, 11830,
		1820,
		0,
		500,
		8192,
		3,
		9,
		3,
		10,
		SFX_LARA_SHOTGUN
	},

	{
		-10920, 10920, -10010, 10010,		//WEAPON_GRENADE
		-14560, 14560, -11830, 11830,
		-14560, 14560, -11830, 11830,
		1820,
		1456,
		500,
		8192,
		20,
		0,
		2,
		10,
		0
	},

	{
		-10920, 10920, -10010, 10010,		//WEAPON_CROSSBOW
		-14560, 14560, -11830, 11830,
		-14560, 14560, -11830, 11830,
		1820,
		1456,
		500,
		8192,
		5,
		0,
		2,
		10,
		0
	},

	{
		0, 0, 0, 0,							//WEAPON_FLARE
		0, 0, 0, 0,
		0, 0, 0, 0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	{
		-5460, 5460, -10010, 10010,			//WEAPON_TORCH
		-5460, 5460, -10010, 10010,
		-5460, 5460, -10010, 10010,
		1820,
		1456,
		400,
		8192,
		3,
		0,
		2,
		0,
		SFX_LARA_UZI_FIRE
	},
};

static int16_t HoldStates[] = {
	AS_WALK,
	AS_STOP,
	AS_POSE,
	AS_TURN_R,
	AS_TURN_L,
	AS_BACK,
	AS_FASTTURN,
	AS_STEPLEFT,
	AS_STEPRIGHT,
	AS_WADE,
	AS_DUCK,
	AS_DUCKROTL,
	AS_DUCKROTR,
	-1
};

static PHD_3DPOS bum_view;
static GAME_VECTOR bum_vdest;
static GAME_VECTOR bum_vsrc;
static ITEM_INFO* TargetList[TARGET_LIST_SIZE];
static ITEM_INFO* LastTargets[TARGET_LIST_SIZE];

static int32_t CheckForHoldingState(int32_t state) {
	int16_t* holds;

	holds = HoldStates;

	while (*holds >= 0) {
		if (state == *holds)
			return 1;

		holds++;
	}

	return 0;
}

void InitialiseNewWeapon() {
	lara.right_arm.frame_number = 0;
	lara.left_arm.frame_number = 0;
	lara.left_arm.z_rot = 0;
	lara.left_arm.y_rot = 0;
	lara.left_arm.x_rot = 0;
	lara.right_arm.z_rot = 0;
	lara.right_arm.y_rot = 0;
	lara.right_arm.x_rot = 0;
	lara.target = 0;
	lara.right_arm.lock = 0;
	lara.left_arm.lock = 0;
	lara.right_arm.flash_gun = 0;
	lara.left_arm.flash_gun = 0;

	switch (lara.gun_type) {
		case WEAPON_PISTOLS:
		case WEAPON_UZI:
			lara.left_arm.frame_base = objects[T4PlusGetPistolsAnimSlotID()].frame_base;
			lara.right_arm.frame_base = objects[T4PlusGetPistolsAnimSlotID()].frame_base;

			if (lara.gun_status != LG_NO_ARMS)
				draw_pistol_meshes(lara.gun_type);

			break;

		case WEAPON_REVOLVER:
		case WEAPON_SHOTGUN:
		case WEAPON_GRENADE:
			lara.left_arm.frame_base = objects[WeaponObject(lara.gun_type)].frame_base;
			lara.right_arm.frame_base = objects[WeaponObject(lara.gun_type)].frame_base;

			if (lara.gun_status != LG_NO_ARMS)
				draw_shotgun_meshes(lara.gun_type);

			break;

		case WEAPON_FLARE:
			lara.left_arm.frame_base = objects[FLARE_ANIM].frame_base;
			lara.right_arm.frame_base = objects[FLARE_ANIM].frame_base;

			if (lara.gun_status != LG_NO_ARMS)
				draw_flare_meshes();

			break;

		default:
			lara.left_arm.frame_base = anims[lara_item->anim_number].frame_ptr;
			lara.right_arm.frame_base = anims[lara_item->anim_number].frame_ptr;
			break;
	}
}

static void find_target_point(ITEM_INFO* item, GAME_VECTOR* target) {
	int32_t x, y, z, c, s;
	int16_t* bounds;

	bounds = GetBestFrame(item);
	x = (bounds[0] + bounds[1]) >> 1;
	y = (bounds[2] + (bounds[3] - bounds[2]) / 3);
	z = (bounds[4] + bounds[5]) >> 1;
	s = phd_sin(item->pos.y_rot);
	c = phd_cos(item->pos.y_rot);
	target->x = item->pos.x_pos + ((x * c + z * s) >> W2V_SHIFT);
	target->y = item->pos.y_pos + y;
	target->z = item->pos.z_pos + ((z * c - x * s) >> W2V_SHIFT);
	target->room_number = item->room_number;
}

void LaraTargetInfo(WEAPON_INFO* winfo) {
	GAME_VECTOR src, target;
	int16_t ang[2];

	if (!lara.target) {
		lara.right_arm.lock = 0;
		lara.left_arm.lock = 0;
		lara.target_angles[1] = 0;
		lara.target_angles[0] = 0;
		return;
	}

	src.x = 0;
	src.y = 0;
	src.z = 0;
	GetLaraJointPos((PHD_VECTOR*)&src, LMX_HAND_R);
	src.x = lara_item->pos.x_pos;
	src.z = lara_item->pos.z_pos;
	src.room_number = lara_item->room_number;
	find_target_point(lara.target, &target);
	phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
	ang[0] -= lara_item->pos.y_rot;
	ang[1] -= lara_item->pos.x_rot;

	if (LOS(&src, &target)) {
		if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] &&
		        ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3]) {
			lara.left_arm.lock = 1;
			lara.right_arm.lock = 1;
			lara.target_angles[0] = ang[0];
			lara.target_angles[1] = ang[1];
			return;
		}

		if (lara.left_arm.lock) {
			if (ang[0] < winfo->left_angles[0] || ang[0] > winfo->left_angles[1] ||
			        ang[1] < winfo->left_angles[2] || ang[1] > winfo->left_angles[3])
				lara.left_arm.lock = 0;
		}

		if (lara.right_arm.lock) {
			if (ang[0] < winfo->right_angles[0] || ang[0] > winfo->right_angles[1] ||
			        ang[1] < winfo->left_angles[2] || ang[1] > winfo->left_angles[3]) {
				lara.right_arm.lock = 0;
				lara.target_angles[0] = ang[0];
				lara.target_angles[1] = ang[1];
				return;
			}
		}
	} else {
		lara.right_arm.lock = 0;
		lara.left_arm.lock = 0;
	}

	lara.target_angles[0] = ang[0];
	lara.target_angles[1] = ang[1];
}

int16_t* get_current_ammo_pointer(int32_t weapon_type) {
	int16_t* ammo;

	switch (weapon_type) {
		case WEAPON_REVOLVER:
			ammo = &lara.num_revolver_ammo;
			break;

		case WEAPON_UZI:
			ammo = &lara.num_uzi_ammo;
			break;

		case WEAPON_SHOTGUN:

			if (lara.shotgun_type_carried & W_AMMO1)
				ammo = &lara.num_shotgun_ammo1;
			else
				ammo = &lara.num_shotgun_ammo2;

			break;

		case WEAPON_GRENADE:

			if (lara.grenade_type_carried & W_AMMO1)
				ammo = &lara.num_grenade_ammo1;
			else if (lara.grenade_type_carried & W_AMMO2)
				ammo = &lara.num_grenade_ammo2;
			else
				ammo = &lara.num_grenade_ammo3;

			break;

		case WEAPON_CROSSBOW:

			if (lara.crossbow_type_carried & W_AMMO1)
				ammo = &lara.num_crossbow_ammo1;
			else if (lara.crossbow_type_carried & W_AMMO2)
				ammo = &lara.num_crossbow_ammo2;
			else
				ammo = &lara.num_crossbow_ammo3;

			break;

		default:
			ammo = &lara.num_pistols_ammo;
			break;
	}

	return ammo;
}

int32_t FireWeapon(int32_t weapon_type, ITEM_INFO* target, ITEM_INFO* src, int16_t* angles) {
	WEAPON_INFO* winfo;
	SPHERE* sptr;
	int16_t* ammo;
	int32_t r, nSpheres, bestdist, best;
	int16_t room_number;

	bum_view.x_pos = 0;
	bum_view.y_pos = 0;
	bum_view.z_pos = 0;
	GetLaraJointPos((PHD_VECTOR*)&bum_view, LMX_HAND_R);
	ammo = get_current_ammo_pointer(weapon_type);

	if (!*ammo)
		return 0;

	if (*ammo != -1)
		--*ammo;

	winfo = &weapons[weapon_type];

	bum_view.x_pos = src->pos.x_pos;
	bum_view.z_pos = src->pos.z_pos;
	bum_view.x_rot = int16_t(winfo->shot_accuracy * (GetRandomControl() - 0x4000) / 0x10000 + angles[1]);
	bum_view.y_rot = int16_t(winfo->shot_accuracy * (GetRandomControl() - 0x4000) / 0x10000 + angles[0]);
	bum_view.z_rot = 0;

	// TODO: implement this properly!
	if (!camera_frozen) {
		phd_GenerateW2V(&bum_view);
	}

	nSpheres = GetSpheres(target, Slist, 0);
	best = -1;
	bestdist = 0x7FFFFFFF;

	for (int32_t i = 0; i < nSpheres; i++) {
		sptr = &Slist[i];
		r = sptr->r;

		if (abs(sptr->x) < r && abs(sptr->y) < r && sptr->z > r && SQUARE(sptr->x) + SQUARE(sptr->y) <= SQUARE(r)) {
			if (sptr->z - r < bestdist) {
				bestdist = sptr->z - r;
				best = i;
			}
		}
	}

	lara.has_fired = 1;
	bum_vsrc.x = bum_view.x_pos;
	bum_vsrc.y = bum_view.y_pos;
	bum_vsrc.z = bum_view.z_pos;
	room_number = src->room_number;
	GetFloor(bum_view.x_pos, bum_view.y_pos, bum_view.z_pos, &room_number);
	bum_vsrc.room_number = room_number;

	if (best < 0) {
		bum_vdest.x = bum_vsrc.x + int32_t(0x5000 * mMXPtr[M20]);
		bum_vdest.y = bum_vsrc.y + int32_t(0x5000 * mMXPtr[M21]);
		bum_vdest.z = bum_vsrc.z + int32_t(0x5000 * mMXPtr[M22]);
		GetTargetOnLOS(&bum_vsrc, &bum_vdest, 0, 1);
		return -1;
	} else {
		savegame.Game.AmmoHits++;
		bum_vdest.x = bum_vsrc.x + int32_t(bestdist * mMXPtr[M20]);
		bum_vdest.y = bum_vsrc.y + int32_t(bestdist * mMXPtr[M21]);
		bum_vdest.z = bum_vsrc.z + int32_t(bestdist * mMXPtr[M22]);

		if (!GetTargetOnLOS(&bum_vsrc, &bum_vdest, 0, 1))
			HitTarget(target, &bum_vdest, winfo->damage, 0);

		return 1;
	}
}

void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm) {
	int16_t speed, x, y;

	speed = winfo->aim_speed;

	if (arm->lock) {
		y = lara.target_angles[0];
		x = lara.target_angles[1];
	} else {
		x = 0;
		y = 0;
	}

	if (arm->y_rot >= y - speed && arm->y_rot <= speed + y)
		arm->y_rot = y;
	else if (arm->y_rot < y)
		arm->y_rot += speed;
	else
		arm->y_rot -= speed;

	if (arm->x_rot >= x - speed && arm->x_rot <= speed + x)
		arm->x_rot = x;
	else if (arm->x_rot < x)
		arm->x_rot += speed;
	else
		arm->x_rot -= speed;

	arm->z_rot = 0;
}

void LaraGetNewTarget(WEAPON_INFO* winfo) {
	ITEM_INFO* item;
	ITEM_INFO* bestitem;
	CREATURE_INFO* creature;
	GAME_VECTOR src, target;
	int32_t x, y, z, slot, dist, maxdist, maxdist2, bestdist;
	int16_t ang[2];
	int16_t bestyrot, targets, match;

	if (BinocularRange) {
		lara.target = 0;
		return;
	}

	bestitem = 0;
	src.x = lara_item->pos.x_pos;
	src.y = lara_item->pos.y_pos - 650;
	src.z = lara_item->pos.z_pos;
	src.room_number = lara_item->room_number;
	bestyrot = 0x7FFF;
	bestdist = 0x7FFFFFFF;
	maxdist = winfo->target_dist;
	maxdist2 = SQUARE(maxdist);
	creature = baddie_slots;
	targets = 0;

	for (slot = 0; slot < MAXIMUM_BADDIES; slot++, creature++) {
		if (creature->item_num != NO_ITEM) {
			item = &items[creature->item_num];

			if (item->hit_points > 0) {
				x = item->pos.x_pos - src.x;
				y = item->pos.y_pos - src.y;
				z = item->pos.z_pos - src.z;

				if (abs(x) <= maxdist && abs(y) <= maxdist && abs(z) <= maxdist) {
					dist = SQUARE(x) + SQUARE(y) + SQUARE(z);

					if (dist < maxdist2) {
						find_target_point(item, &target);

						if (LOS(&src, &target)) {
							phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
							ang[0] -= (lara.torso_y_rot + lara_item->pos.y_rot);
							ang[1] -= (lara.torso_x_rot + lara_item->pos.x_rot);

							if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] &&
							        ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3]) {
								TargetList[targets] = item;
								targets++;

								if (abs(ang[0]) < bestyrot + DEGREES_TO_ROTATION(15) && dist < bestdist) {
									bestdist = dist;
									bestyrot = abs(ang[0]);
									bestitem = item;
								}
							}
						}
					}
				}
			}
		}
	}

	TargetList[targets] = 0;

	if (TargetList[0]) {
		for (slot = 0; slot < TARGET_LIST_SIZE; slot++) {
			if (!TargetList[slot])
				lara.target = 0;

			if (TargetList[slot] == lara.target)
				break;
		}

		if (savegame.AutoTarget || input & IN_TARGET) {
			if (!lara.target) {
				lara.target = bestitem;
				LastTargets[0] = 0;
			} else if (input & IN_TARGET) {
				lara.target = 0;

				for (match = 0; match < TARGET_LIST_SIZE; match++) {
					if (!TargetList[match])
						break;

					for (slot = 0; slot < TARGET_LIST_SIZE; slot++) {
						if (!LastTargets[slot]) {
							slot = TARGET_LIST_SIZE;
							break;
						}

						if (LastTargets[slot] == TargetList[match])
							break;
					}

					if (slot == TARGET_LIST_SIZE) {
						lara.target = TargetList[match];
						break;
					}
				}

				if (!lara.target) {
					lara.target = bestitem;
					LastTargets[0] = 0;
				}
			}
		}
	} else
		lara.target = 0;

	if (lara.target != LastTargets[0]) {
		for (slot = 7; slot > 0; slot--)
			LastTargets[slot] = LastTargets[slot - 1];

		LastTargets[0] = lara.target;
	}

	LaraTargetInfo(winfo);
}

void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitpos, int32_t damage, int32_t grenade) {
	OBJECT_INFO* obj;

	obj = &objects[item->object_number];
	item->hit_status = 1;

	if (item->data && item != lara_item)
		((CREATURE_INFO*)item->data)->hurt_by_lara = 1;

	if (hitpos && obj->HitEffect) {
		switch (obj->HitEffect) {
			case 1:

				if (item->object_number == BADDY_2 && (item->current_anim_state == 8 || GetRandomControl() & 1) &&
				        (lara.gun_type == WEAPON_PISTOLS || lara.gun_type == WEAPON_SHOTGUN || lara.gun_type == WEAPON_UZI)) {
					SoundEffect(SFX_BAD_SWORD_RICO, &item->pos, SFX_DEFAULT);
					TriggerRicochetSpark(hitpos, lara_item->pos.y_rot, 3, 0);
					return;
				}

				DoBloodSplat(hitpos->x, hitpos->y, hitpos->z, (GetRandomControl() & 3) + 3, item->pos.y_rot, item->room_number);
				break;

			case 2:
				TriggerRicochetSpark(hitpos, lara_item->pos.y_rot, 3, -5);
				break;

			case 3:
				TriggerRicochetSpark(hitpos, lara_item->pos.y_rot, 3, 0);
				break;
		}
	}

	if (!obj->undead || grenade || item->hit_points == -0x4000) {
		if (item->hit_points > 0 && item->hit_points <= damage)
			savegame.Level.Kills++;

		item->hit_points -= (int16_t)damage;
	}
}

int32_t WeaponObject(int32_t weapon_type) {
	switch (weapon_type) {
		case WEAPON_REVOLVER:
			return T4PlusGetRevolverAnimSlotID();

		case WEAPON_UZI:
			return T4PlusGetUziAnimSlotID();

		case WEAPON_SHOTGUN:
			return T4PlusGetShotgunAnimSlotID();

		case WEAPON_GRENADE:
			return T4PlusGetGrenadeGunAnimSlotID();

		case WEAPON_CROSSBOW:
			return T4PlusGetCrossbowAnimSlotID();

		default:
			return T4PlusGetPistolsAnimSlotID();
	}
}

int32_t WeaponObjectMesh(int32_t weapon_type) {
	switch (weapon_type) {
		case WEAPON_REVOLVER:
			if (lara.sixshooter_type_carried & W_LASERSIGHT)
				return T4PlusGetLaraRevolverLaserSlotID();
			else
				return T4PlusGetRevolverAnimSlotID();

		case WEAPON_UZI:
			return T4PlusGetUziAnimSlotID();

		case WEAPON_SHOTGUN:
			return T4PlusGetShotgunAnimSlotID();

		case WEAPON_GRENADE:
			return T4PlusGetGrenadeGunAnimSlotID();

		case WEAPON_CROSSBOW:

			if (lara.crossbow_type_carried & W_LASERSIGHT)
				return T4PlusGetLaraCrossbowLaserSlotID();
			else
				return T4PlusGetCrossbowAnimSlotID();

		default:
			return T4PlusGetPistolsAnimSlotID();
	}
}

void DoProperDetection(int16_t item_number, int32_t x, int32_t y, int32_t z, int32_t xv, int32_t yv, int32_t zv) {
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	int32_t ceiling, height, oldtype, oldonobj, oldheight, bs, yang, xs;
	int16_t room_number;

	item = &items[item_number];

	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	oldheight = GetHeight(floor, x, y, z);

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->pos.y_pos >= height) {
		bs = 0;
		oldtype = height_type;

		if ((oldtype == BIG_SLOPE || oldtype == DIAGONAL) && oldheight < height) {
			yang = (uint16_t)item->pos.y_rot;

			if (tiltyoff < 0 && yang >= 0x8000 || tiltyoff > 0 && yang <= 0x8000 ||
			        tiltxoff < 0 && yang >= 0x4000 && yang <= 0xC000 || tiltxoff > 0 && (yang <= 0x4000 || yang >= 0xC000))
				bs = 1;
		}

		if (y > height + 32 && !bs && ((item->pos.x_pos ^ x) & ~0x3FF || (item->pos.z_pos ^ z) & ~0x3FF)) {
			xs = (item->pos.x_pos ^ x) & ~0x3FF && (item->pos.z_pos ^ z) & ~0x3FF ? abs(x - item->pos.x_pos) < abs(z - item->pos.z_pos) : 1;
			item->pos.y_rot = (item->pos.x_pos ^ x) & ~0x3FF && xs ? -item->pos.y_rot : -0x8000 - item->pos.y_rot;
			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
			item->speed >>= 1;
		} else if (oldtype != BIG_SLOPE && oldtype != DIAGONAL) {
			if (item->fallspeed > 0) {
				if (item->fallspeed > 16) {
					if (item->object_number == GRENADE)
						item->fallspeed = (item->fallspeed >> 1) - item->fallspeed;
					else {
						item->fallspeed = -(item->fallspeed >> 2);

						if (item->fallspeed < -100)
							item->fallspeed = -100;
					}
				} else {
					item->fallspeed = 0;

					if (item->object_number == GRENADE) {
						item->speed--;
						item->required_anim_state = 1;
						item->pos.x_rot = 0;
					} else
						item->speed -= 3;

					if (item->speed < 0)
						item->speed = 0;
				}
			}

			item->pos.y_pos = height;
		} else {
			item->speed -= item->speed >> 2;

			if (tiltyoff < 0 && abs(tiltyoff) - abs(tiltxoff) >= 2) {
				if ((uint16_t)item->pos.y_rot > 0x8000) {
					item->pos.y_rot = -1 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed -= int16_t(2 * tiltyoff);

						if ((uint16_t)item->pos.y_rot > 0x4000 && (uint16_t)item->pos.y_rot < 0xC000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0x4000)
								item->pos.y_rot = 0x4000;
						} else if ((uint16_t)item->pos.y_rot < 0x4000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0x4000)
								item->pos.y_rot = 0x4000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltyoff > 0 && abs(tiltyoff) - abs(tiltxoff) >= 2) {
				if ((uint16_t)item->pos.y_rot < 0x8000) {
					item->pos.y_rot = -1 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed += int16_t(2 * tiltyoff);

						if ((uint16_t)item->pos.y_rot > 0xC000 || (uint16_t)item->pos.y_rot < 16384) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0xC000)
								item->pos.y_rot = -0x4000;
						} else if ((uint16_t)item->pos.y_rot < 0xC000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0xC000)
								item->pos.y_rot = -0x4000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltxoff < 0 && abs(tiltxoff) - abs(tiltyoff) >= 2) {
				if ((uint16_t)item->pos.y_rot > 0x4000 && (uint16_t)item->pos.y_rot < 0xC000) {
					item->pos.y_rot = 0x8000 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed -= int16_t(2 * tiltxoff);

						if ((uint16_t)item->pos.y_rot < 0x8000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot > 61440)
								item->pos.y_rot = 0;
						} else {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot < 4096)
								item->pos.y_rot = 0;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltxoff > 0 && abs(tiltxoff) - abs(tiltyoff) >= 2) {
				if ((uint16_t)item->pos.y_rot > 0xC000 || (uint16_t)item->pos.y_rot < 0x4000) {
					item->pos.y_rot = (0x8000 - 1) - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed += int16_t(2 * tiltxoff);

						if ((uint16_t)item->pos.y_rot > 0x8000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0x8000)
								item->pos.y_rot = -0x8000;
						} else if ((uint16_t)item->pos.y_rot < 0x8000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0x8000)
								item->pos.y_rot = -0x8000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltyoff < 0 && tiltxoff < 0) {
				if ((uint16_t)item->pos.y_rot > 24576 && (uint16_t)item->pos.y_rot < 57344) {
					item->pos.y_rot = -16385 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed -= int16_t(tiltxoff + tiltyoff);

						if ((uint16_t)item->pos.y_rot > 0x2000 && (uint16_t)item->pos.y_rot < 0xa000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0x2000)
								item->pos.y_rot = 0x2000;
						} else if (item->pos.y_rot != 0x2000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0x2000)
								item->pos.y_rot = 0x2000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltyoff < 0 && tiltxoff > 0) {
				if ((uint16_t)item->pos.y_rot > 0xa000 || (uint16_t)item->pos.y_rot < 0x2000) {
					item->pos.y_rot = (0x4000 - 1) - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed += int16_t(tiltxoff - tiltyoff);

						if ((uint16_t)item->pos.y_rot < 0xe000 && (uint16_t)item->pos.y_rot > 0x6000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0x6000)
								item->pos.y_rot = 0x6000;
						} else if (item->pos.y_rot != 0x6000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0x6000)
								item->pos.y_rot = 0x6000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltyoff > 0 && tiltxoff > 0) {
				if ((uint16_t)item->pos.y_rot > 0xe000 || (uint16_t)item->pos.y_rot < 0x6000) {
					item->pos.y_rot = -16385 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed += int16_t(tiltxoff + tiltyoff);

						if ((uint16_t)item->pos.y_rot < 0x2000 || (uint16_t)item->pos.y_rot > 0xa000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0xa000)
								item->pos.y_rot = -0x6000;
						} else if (item->pos.y_rot != -0x6000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0xa000)
								item->pos.y_rot = -0x6000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			} else if (tiltyoff > 0 && tiltxoff < 0) {
				if ((uint16_t)item->pos.y_rot > 0x2000 && (uint16_t)item->pos.y_rot < 0xa000) {
					item->pos.y_rot = (0x4000 - 1) - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				} else {
					if (item->speed < 32) {
						item->speed += int16_t(tiltyoff - tiltxoff);

						if ((uint16_t)item->pos.y_rot < 0x6000 || (uint16_t)item->pos.y_rot > 0xe000) {
							item->pos.y_rot -= 0x1000;

							if ((uint16_t)item->pos.y_rot < 0xe000)
								item->pos.y_rot = -0x2000;
						} else if (item->pos.y_rot != -0x2000) {
							item->pos.y_rot += 0x1000;

							if ((uint16_t)item->pos.y_rot > 0xe000)
								item->pos.y_rot = -0x2000;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			}

			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
		}
	} else {
		if (yv >= 0) {
			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
			height = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);

			oldonobj = OnObject;

			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (item->pos.y_pos >= height && oldonobj) {
				if (item->fallspeed > 0) {
					if (item->fallspeed > 16) {
						if (item->object_number == GRENADE)
							item->fallspeed = (item->fallspeed >> 1) - item->fallspeed;
						else {
							item->fallspeed = -(item->fallspeed >> 2);

							if (item->fallspeed < -100)
								item->fallspeed = -100;
						}
					} else {
						item->fallspeed = 0;

						if (item->object_number == GRENADE) {
							item->speed--;
							item->required_anim_state = 1;
							item->pos.x_rot = 0;
						} else
							item->speed -= 3;

						if (item->speed < 0)
							item->speed = 0;
					}
				}

				item->pos.y_pos = height;
			}
		}

		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (item->pos.y_pos < ceiling) {
			if (y < ceiling && ((item->pos.x_pos ^ x) & ~0x3FF || (item->pos.z_pos ^ z) & ~0x3FF)) {
				item->pos.y_rot = (item->pos.x_pos ^ x) & ~0x3FF ? -item->pos.y_rot : -0x8000 - item->pos.y_rot;
				item->pos.x_pos = x;
				item->pos.y_pos = y;
				item->pos.z_pos = z;

				if (item->object_number == GRENADE)
					item->speed -= item->speed >> 3;
				else
					item->speed >>= 1;
			} else
				item->pos.y_pos = ceiling;

			if (item->fallspeed < 0)
				item->fallspeed = -item->fallspeed;
		}
	}

	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);
}

void LaraGun() {
	int16_t state;

	if (lara.left_arm.flash_gun > 0)
		lara.left_arm.flash_gun--;

	if (lara.right_arm.flash_gun > 0)
		lara.right_arm.flash_gun--;

	MOD_LEVEL_CAMERA_INFO *mod_camera_info = get_game_mod_level_camera_info(gfCurrentLevel);

	if (lara.gun_type == WEAPON_TORCH) {
		DoFlameTorch();
		return;
	}

	if (lara_item->hit_points <= 0)
		lara.gun_status = LG_NO_ARMS;
	else if (lara.gun_status == LG_NO_ARMS) {
		if (input & IN_DRAW)
			lara.request_gun_type = lara.last_gun_type;
		else if (input & IN_FLARE && !(gfLevelFlags & GF_YOUNGLARA)) {
			if (lara_item->current_anim_state == AS_DUCK && lara_item->anim_number != LARA_ANIM_DUCKBREATHE)
				return;

			if (lara.gun_type == WEAPON_FLARE) {
				if (!lara.left_arm.frame_number)
					lara.gun_status = LG_UNDRAW_GUNS;
			} else if (lara.num_flares) {
				if (lara.num_flares != -1)
					lara.num_flares--;

				lara.request_gun_type = WEAPON_FLARE;
			}
		}

		if (input & IN_DRAW || lara.request_gun_type != lara.gun_type) {
			state = lara_item->current_anim_state;

			if ((state == AS_DUCK || state == AS_DUCKROTL || state == AS_DUCKROTR) &&
			        (lara.request_gun_type == WEAPON_SHOTGUN || lara.request_gun_type == WEAPON_CROSSBOW || lara.request_gun_type == WEAPON_GRENADE)) {
				if (lara.gun_type == WEAPON_FLARE)
					lara.request_gun_type = WEAPON_FLARE;
			} else if (lara.request_gun_type != WEAPON_FLARE && (lara.vehicle != NO_ITEM || lara.water_status != LW_ABOVE_WATER &&
			           (lara.water_status != LW_WADE || lara.water_surface_dist <= -weapons[lara.gun_type].gun_height))) {
				lara.last_gun_type = lara.request_gun_type;

				if (lara.gun_type != WEAPON_FLARE)
					lara.gun_type = lara.request_gun_type;
				else
					lara.request_gun_type = WEAPON_FLARE;
			} else {
				if (lara.gun_type == WEAPON_FLARE) {
					CreateFlare(FLARE_ITEM, 0);
					undraw_flare_meshes();
					lara.flare_control_left = 0;
					lara.flare_age = 0;
				}

				lara.gun_type = lara.request_gun_type;
				InitialiseNewWeapon();
				lara.gun_status = LG_DRAW_GUNS;
				lara.right_arm.frame_number = 0;
				lara.left_arm.frame_number = 0;
			}
		}
	} else if (lara.gun_status == LG_READY) {
		if (input & IN_DRAW || lara.request_gun_type != lara.gun_type || lara.water_status != LW_ABOVE_WATER &&
		        (lara.water_status != LW_WADE || lara.water_surface_dist < -weapons[lara.gun_type].gun_height))
			lara.gun_status = LG_UNDRAW_GUNS;
	} else if (lara.gun_status == LG_HANDS_BUSY && input & IN_FLARE && lara_item->current_anim_state == AS_ALL4S && lara_item->anim_number == LARA_ANIM_ALL4S) {
		lara.request_gun_type = WEAPON_FLARE;
	}

	switch (lara.gun_status) {
		case LG_NO_ARMS:

			if (lara.gun_type == WEAPON_FLARE) {
				if (lara.vehicle == NO_ITEM && !CheckForHoldingState(lara_item->current_anim_state))
					lara.flare_control_left = 0;
				else if (lara.flare_control_left) {
					if (lara.left_arm.frame_number) {
						lara.left_arm.frame_number++;

						if (lara.left_arm.frame_number == 110)
							lara.left_arm.frame_number = 0;
					}
				} else {
					lara.flare_control_left = 1;
					lara.left_arm.frame_number = 95;
				}

				DoFlareInHand(lara.flare_age);
				set_flare_arm(lara.left_arm.frame_number);
			}

			break;

		case LG_HANDS_BUSY:

			if (lara.gun_type == WEAPON_FLARE && lara.mesh_ptrs[LM_LHAND] == meshes[objects[FLARE_ANIM].mesh_index + LM_LHAND * 2]) {
				lara.flare_control_left = lara.vehicle != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state);
				DoFlareInHand(lara.flare_age);
				set_flare_arm(lara.left_arm.frame_number);
			}

			break;

		case LG_DRAW_GUNS:

			if (lara.gun_type != WEAPON_FLARE && lara.gun_type != WEAPON_NONE)
				lara.last_gun_type = lara.gun_type;

			switch (lara.gun_type) {
				case WEAPON_PISTOLS:
				case WEAPON_REVOLVER:
				case WEAPON_UZI:

					if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA && camera.type != HEAVY_CAMERA && !mod_camera_info->disable_battle_camera)
						camera.type = COMBAT_CAMERA;

					draw_pistols(lara.gun_type);
					break;

				case WEAPON_SHOTGUN:
				case WEAPON_GRENADE:
				case WEAPON_CROSSBOW:

					if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA && camera.type != HEAVY_CAMERA && !mod_camera_info->disable_battle_camera)
						camera.type = COMBAT_CAMERA;

					draw_shotgun(lara.gun_type);
					break;

				case WEAPON_FLARE:
					draw_flare();
					break;

				default:
					lara.gun_status = LG_NO_ARMS;
					break;
			}

			break;

		case LG_UNDRAW_GUNS:
			lara.mesh_ptrs[LM_HEAD] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_HEAD * 2];

			switch (lara.gun_type) {
				case WEAPON_PISTOLS:
				case WEAPON_REVOLVER:
				case WEAPON_UZI:
					undraw_pistols(lara.gun_type);
					break;

				case WEAPON_SHOTGUN:
				case WEAPON_GRENADE:
				case WEAPON_CROSSBOW:
					undraw_shotgun(lara.gun_type);
					break;

				case WEAPON_FLARE:
					undraw_flare();
					break;
			}

			break;

		case LG_READY:
			if (input & IN_ACTION && T4PlusGetLaraScreamSlotID() >= 0)
				lara.mesh_ptrs[LM_HEAD] = meshes[objects[T4PlusGetLaraScreamSlotID()].mesh_index + LM_HEAD * 2];
			else
				lara.mesh_ptrs[LM_HEAD] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_HEAD * 2];

			if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA && camera.type != HEAVY_CAMERA && !mod_camera_info->disable_battle_camera)
				camera.type = COMBAT_CAMERA;

			if (input & IN_ACTION) {
				if (!*get_current_ammo_pointer(lara.gun_type)) {
					SoundEffect(SFX_SARLID_PALACES, &lara_item->pos, SFX_DEFAULT);
					// TRLE: Fix for automatically changing back to pistols if we don't have them.
					if (lara.pistols_type_carried & W_PRESENT) {
						lara.request_gun_type = WEAPON_PISTOLS;
					} else {
						lara.request_gun_type = WEAPON_NONE;
					}
					return;
				}
			}

			switch (lara.gun_type) {
				case WEAPON_PISTOLS:
				case WEAPON_UZI:
					PistolHandler(lara.gun_type);
					break;

				case WEAPON_REVOLVER:
				case WEAPON_SHOTGUN:
				case WEAPON_GRENADE:
				case WEAPON_CROSSBOW:
					RifleHandler(lara.gun_type);
					break;
			}

			break;

		case LG_FLARE:
			draw_flare();
			break;
	}
}
