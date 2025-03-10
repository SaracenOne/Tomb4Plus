#include "../tomb4/pch.h"
#include "lara1gun.h"
#include "traps.h"
#include "objects.h"
#include "larafire.h"
#include "box.h"
#include "items.h"
#include "delstuff.h"
#include "control.h"
#include "sound.h"
#include "../specific/function_stubs.h"
#include "tomb4fx.h"
#include "lara2gun.h"
#include "effect2.h"
#include "sphere.h"
#include "switch.h"
#include "collide.h"
#include "debris.h"
#include "../specific/polyinsert.h"
#include "../specific/3dmath.h"
#include "camera.h"
#include "../specific/input.h"
#include "lara.h"
#include "savegame.h"
#include "gameflow.h"
#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_objects.h"
#include "../tomb4/tomb4.h"

void DoGrenadeDamageOnBaddie(ITEM_INFO* baddie, ITEM_INFO* item) {
	if (baddie->flags & 0x8000)
		return;

	if (baddie == lara_item && lara_item->hit_points > 0) {
		lara_item->hit_points -= 50;

		if (!(room[item->room_number].flags & ROOM_UNDERWATER) && lara_item->hit_points <= 50)
			LaraBurn();
	} else if (!item->item_flags[2]) {
		baddie->hit_status = 1;

		if ((!objects[baddie->object_number].undead || baddie->object_number == SKELETON || baddie->object_number == MUMMY) &&
		        baddie->object_number != AHMET) {
			HitTarget(baddie, 0, 30, 1);

			if (baddie != lara_item) {
				savegame.Game.AmmoHits++;

				if (baddie->hit_points <= 0) {
					savegame.Level.Kills++;

					if (baddie->object_number != BABOON_NORMAL && baddie->object_number != BABOON_INV && baddie->object_number != BABOON_SILENT)
						CreatureDie(int16_t(baddie - items), 1);
				}
			}
		}
	}
}

void FireCrossbow(PHD_3DPOS* pos) {
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	PHD_VECTOR vec;
	int16_t* ammo;
	int32_t h;
	int16_t item_number;

	ammo = get_current_ammo_pointer(WEAPON_CROSSBOW);

	if (!*ammo)
		return;

	lara.has_fired = 1;
	item_number = CreateItem();

	if (item_number != NO_ITEM) {
		item = &items[item_number];
		item->object_number = CROSSBOW_BOLT;
		item->shade = -0x3DF0;

		if (pos) {
			item->room_number = lara_item->room_number;
			item->pos.x_pos = pos->x_pos;
			item->pos.y_pos = pos->y_pos;
			item->pos.z_pos = pos->z_pos;
			InitialiseItem(item_number);
			item->pos.x_rot = pos->x_rot;
			item->pos.y_rot = pos->y_rot;
			item->pos.z_rot = pos->z_rot;
		} else {
			if (*ammo != -1)
				--*ammo;

			vec.x = 0;
			vec.y = 228;
			vec.z = 32;
			GetLaraJointPos(&vec, LMX_HAND_R);
			item->room_number = lara_item->room_number;
			floor = GetFloor(vec.x, vec.y, vec.z, &item->room_number);
			h = GetHeight(floor, vec.x, vec.y, vec.z);

			if (h >= vec.y) {
				item->pos.x_pos = vec.x;
				item->pos.y_pos = vec.y;
				item->pos.z_pos = vec.z;
			} else {
				item->pos.x_pos = lara_item->pos.x_pos;
				item->pos.y_pos = vec.y;
				item->pos.z_pos = lara_item->pos.z_pos;
				item->room_number = lara_item->room_number;
			}

			InitialiseItem(item_number);
			item->pos.x_rot = lara.left_arm.x_rot + lara_item->pos.x_rot;
			item->pos.y_rot = lara.left_arm.y_rot + lara_item->pos.y_rot;
			item->pos.z_rot = 0;

			if (!lara.left_arm.lock) {
				item->pos.x_rot += lara.torso_x_rot;
				item->pos.y_rot += lara.torso_y_rot;
			}
		}

		item->speed = HALF_BLOCK_SIZE;
		AddActiveItem(item_number);

		if (lara.crossbow_type_carried & W_AMMO1)
			item->item_flags[0] = 1;
		else if (lara.crossbow_type_carried & W_AMMO2)
			item->item_flags[0] = 2;
		else
			item->item_flags[0] = 3;

		SoundEffect(SFX_LARA_CROSSBOW, 0, SFX_DEFAULT);
		savegame.Game.AmmoUsed++;
	}
}

void draw_shotgun_meshes(int32_t weapon_type) {
	lara.back_gun = 0;
	lara.mesh_ptrs[LM_RHAND] = meshes[objects[WeaponObjectMesh(weapon_type)].mesh_index + 2 * LM_RHAND];
}

void undraw_shotgun_meshes(int32_t weapon_type) {
	lara.back_gun = (int16_t)WeaponObject(weapon_type);
	lara.mesh_ptrs[LM_RHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + 2 * LM_RHAND];
}

void ready_shotgun(int32_t weapon_type) {
	lara.gun_status = LG_READY;
	lara.target = 0;

	lara.left_arm.x_rot = 0;
	lara.left_arm.y_rot = 0;
	lara.left_arm.z_rot = 0;
	lara.left_arm.frame_number = 0;
	lara.left_arm.lock = 0;
	lara.left_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;

	lara.right_arm.x_rot = 0;
	lara.right_arm.y_rot = 0;
	lara.right_arm.z_rot = 0;
	lara.right_arm.frame_number = 0;
	lara.right_arm.lock = 0;
	lara.right_arm.frame_base = lara.left_arm.frame_base;
}

void FireShotgun() {
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	int32_t fired, scatter;
	int16_t angles[2];
	int16_t dangles[2];

	angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
	angles[1] = lara.left_arm.x_rot;

	if (!lara.left_arm.lock) {
		angles[0] += lara.torso_y_rot;
		angles[1] += lara.torso_x_rot;
	}

	fired = 0;

	if (lara.shotgun_type_carried & W_AMMO1)
		scatter = 1820;
	else
		scatter = 5460;

	for (int i = 0; i < 6; i++) {
		dangles[0] = int16_t(angles[0] + scatter * (GetRandomControl() - 0x4000) / 0x10000);
		dangles[1] = int16_t(angles[1] + scatter * (GetRandomControl() - 0x4000) / 0x10000);

		if (FireWeapon(WEAPON_SHOTGUN, lara.target, lara_item, dangles))
			fired = 1;
	}

	if (fired) {
		pos.x = 0;
		pos.y = 228;
		pos.z = 32;
		GetLaraJointPos(&pos, LMX_HAND_R);

		pos2.x = 0;
		pos2.y = 1508;
		pos2.z = 32;
		GetLaraJointPos(&pos2, LMX_HAND_R);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_SHOTGUN;

		if (lara_item->mesh_bits) {
			for (int i = 0; i < 7; i++)
				TriggerGunSmoke(pos.x, pos.y, pos.z, pos2.x - pos.x, pos2.y - pos.y, pos2.z - pos.z, 1, SmokeWeapon, 32);

			//	for (int i = 0; i < 12; i++)
			//empty func call here
		}

		lara.right_arm.flash_gun = weapons[WEAPON_SHOTGUN].flash_time;
		SoundEffect(SFX_EXPLOSION1, &lara_item->pos, 0x1400000 | SFX_SETPITCH);
		SoundEffect(weapons[WEAPON_SHOTGUN].sample_num, &lara_item->pos, SFX_DEFAULT);
		savegame.Game.AmmoUsed++;
	}
}

void FireGrenade() {
	ITEM_INFO* item;
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	int16_t* ammo;
	int32_t h;
	int16_t item_number;

	ammo = get_current_ammo_pointer(WEAPON_GRENADE);

	if (!*ammo)
		return;

	lara.has_fired = 1;
	item_number = CreateItem();

	if (item_number == NO_ITEM)
		return;

	item = &items[item_number];
	item->shade = -0x3DF0;
	item->object_number = GRENADE;
	item->room_number = lara_item->room_number;

	pos.x = 0;
	pos.y = 276;
	pos.z = 80;
	GetLaraJointPos(&pos, LMX_HAND_R);
	item->pos.x_pos = pos.x;
	item->pos.y_pos = pos.y;
	item->pos.z_pos = pos.z;
	h = GetHeight(GetFloor(pos.x, pos.y, pos.z, &item->room_number), pos.x, pos.y, pos.z);

	if (h < pos.y) {
		item->pos.x_pos = lara_item->pos.x_pos;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = lara_item->pos.z_pos;
		item->room_number = lara_item->room_number;
	}

	pos2.x = 0;
	pos2.y = 1204;
	pos2.z = 80;
	GetLaraJointPos(&pos2, LMX_HAND_R);

	SmokeCountL = 32;
	SmokeWeapon = 5;

	for (int i = 0; i < 5; i++)
		TriggerGunSmoke(pos.x, pos.y, pos.z, pos2.x - pos.x, pos2.y - pos.y, pos2.z - pos.z, 1, SmokeWeapon, SmokeCountL);

	InitialiseItem(item_number);
	item->pos.x_rot = lara.left_arm.x_rot + lara_item->pos.x_rot;
	item->pos.y_rot = lara.left_arm.y_rot + lara_item->pos.y_rot;
	item->pos.z_rot = 0;

	if (!lara.left_arm.lock) {
		item->pos.x_rot += lara.torso_x_rot;
		item->pos.y_rot += lara.torso_y_rot;
	}

	item->speed = 128;
	item->current_anim_state = item->pos.x_rot;
	item->fallspeed = (-128 * phd_sin(item->pos.x_rot)) >> W2V_SHIFT;
	item->goal_anim_state = item->pos.y_rot;
	item->required_anim_state = 0;
	item->hit_points = 120;
	AddActiveItem(item_number);

	if (*ammo != -1)
		--*ammo;

	if (lara.grenade_type_carried & W_AMMO1)
		item->item_flags[0] = 1;
	else if (lara.grenade_type_carried & W_AMMO2)
		item->item_flags[0] = 2;
	else
		item->item_flags[0] = 3;

	savegame.Game.AmmoUsed++;
}

void AnimateShotgun(int32_t weapon_type) {
	ITEM_INFO* item;
	PHD_VECTOR pos;
	static int32_t m16_firing = 0;
	static int32_t harpoon_fired = 0;

	item = &items[lara.weapon_item];

	if (SmokeCountL) {
		if (SmokeWeapon == WEAPON_GRENADE) {
			pos.x = 0;
			pos.y = 180;
			pos.z = 80;
		} else if (SmokeWeapon == WEAPON_SHOTGUN) {
			pos.x = -16;
			pos.y = 228;
			pos.z = 32;
		}

		GetLaraJointPos(&pos, LMX_HAND_R);

		if (lara_item->mesh_bits)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
	}

	switch (item->current_anim_state) {
		case 0:
			m16_firing = 0;

			if (harpoon_fired) {
				item->goal_anim_state = 5;
				harpoon_fired = 0;
			}

			else if (lara.water_status == LW_UNDERWATER)
				item->goal_anim_state = 6;
			else if (input & IN_ACTION && !lara.target || lara.left_arm.lock)
				item->goal_anim_state = 2;
			else
				item->goal_anim_state = 4;

			break;

		case 2:

			if (item->frame_number == anims[item->anim_number].frame_base) {
				item->goal_anim_state = 4;

				if (lara.water_status != 1 && !harpoon_fired) {
					if (input & IN_ACTION) {
						if (!lara.target || lara.left_arm.lock) {
							if (weapon_type == WEAPON_GRENADE)
								FireGrenade();
							else if (weapon_type == WEAPON_CROSSBOW)
								FireCrossbow(0);
							else
								FireShotgun();

							item->goal_anim_state = 2;
						}
					} else if (lara.left_arm.lock)
						item->goal_anim_state = 0;
				}

				if (item->goal_anim_state != 2) {
					if (m16_firing) {
						SoundEffect(SFX_EXPLOSION1, &lara_item->pos, 0x5000000 | SFX_SETPITCH);
						m16_firing = 0;
					}
				}
			} else if (m16_firing) {
				SoundEffect(SFX_EXPLOSION1, &lara_item->pos, 0x5000000 | SFX_SETPITCH);
				SoundEffect(SFX_MP5_FIRE, &lara_item->pos, SFX_DEFAULT);
			} else if (weapon_type == 4 && !(input & IN_ACTION) && !lara.left_arm.lock)
				item->goal_anim_state = 4;
			if (item->frame_number - anims[item->anim_number].frame_base == 12 && weapon_type == WEAPON_SHOTGUN)
				TriggerGunShell(1, SHOTGUNSHELL, 4);

			break;

		case 6:
			m16_firing = 0;

			if (harpoon_fired) {
				item->goal_anim_state = 5;
				harpoon_fired = 0;
			} else if (lara.water_status != LW_UNDERWATER)
				item->goal_anim_state = 0;
			else if (input & IN_ACTION && !lara.target || lara.left_arm.lock)
				item->goal_anim_state = 8;
			else
				item->goal_anim_state = 7;

			break;
	}

	AnimateItem(item);
	lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.right_arm.anim_number = item->anim_number;
	lara.left_arm.frame_base = lara.right_arm.frame_base;
	lara.left_arm.frame_number = lara.right_arm.frame_number;
	lara.left_arm.anim_number = lara.right_arm.anim_number;
}

void RifleHandler(int32_t weapon_type) {
	WEAPON_INFO* winfo;
	PHD_VECTOR pos;
	int32_t r, g, b;

	winfo = &weapons[weapon_type];
	LaraGetNewTarget(winfo);

	if (input & IN_ACTION)
		LaraTargetInfo(winfo);

	AimWeapon(winfo, &lara.left_arm);

	if (lara.left_arm.lock) {
		lara.torso_x_rot = lara.left_arm.x_rot;
		lara.torso_y_rot = lara.left_arm.y_rot;

		if (camera.old_type != LOOK_CAMERA && !BinocularRange) {
			lara.head_y_rot = 0;
			lara.head_x_rot = 0;
		}
	}

	if (weapon_type == WEAPON_REVOLVER)
		AnimatePistols(weapon_type);
	else
		AnimateShotgun(weapon_type);

	if (lara.right_arm.flash_gun) {
		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 128;
		b = GetRandomControl() & 63;

		if (weapon_type == WEAPON_SHOTGUN) {
			pos.x = (GetRandomControl() & 0xFF) + (phd_sin(lara_item->pos.y_rot) >> 4) + lara_item->pos.x_pos;
			pos.y = ((GetRandomControl() & 0x7F) - 575) + lara_item->pos.y_pos;
			pos.z = (GetRandomControl() & 0xFF) + (phd_cos(lara_item->pos.y_rot) >> 4) + lara_item->pos.z_pos;

			TriggerDynamic(pos.x, pos.y, pos.z, 12, r, g, b);

			for (int i = 0; i < t4p_mirror_count; i++) {
				if (lara_item->room_number == t4p_mirror_info[i].mirror_room) {
					PHD_VECTOR mirrored_pos = T4PMirrorVectorOnPlane(&t4p_mirror_info[i], pos);

					TriggerDynamic(mirrored_pos.x, mirrored_pos.y, mirrored_pos.z, 12, r, g, b);
				}
			}
		} else if (weapon_type == WEAPON_REVOLVER) {
			pos.x = (GetRandomControl() & 0xFF) - 128;
			pos.y = (GetRandomControl() & 0x7F) - 63;
			pos.z = (GetRandomControl() & 0xFF) - 128;
			GetLaraJointPos(&pos, LMX_HAND_R);

			TriggerDynamic(pos.x, pos.y, pos.z, 12, r, g, b);

			for (int i = 0; i < t4p_mirror_count; i++) {
				if (lara_item->room_number == t4p_mirror_info[i].mirror_room) {
					PHD_VECTOR mirrored_pos = T4PMirrorVectorOnPlane(&t4p_mirror_info[i], pos);

					TriggerDynamic(mirrored_pos.x, mirrored_pos.y, mirrored_pos.z, 12, r, g, b);
				}
			}
		}
	}
}

void CrossbowHitSwitchType78(ITEM_INFO* item, ITEM_INFO* target, int32_t MustHitLastNode) {
	SPHERE* ptr1;
	int32_t dx, dy, dz, num1, cs, cd, speed;
	int16_t TriggerItems[8];
	int16_t NumTrigs, room_number;

	if (target->flags & IFL_SWITCH_ONESHOT && target->object_number != SKELETON)
		return;

	if (!MustHitLastNode) {
		num1 = objects[target->object_number].nmeshes;
		cs = num1 - 1;
	} else {
		num1 = GetSpheres(target, Slist, 1);
		cs = -1;
		cd = 0x7FFFFFFF;

		if (target->object_number == SKELETON) {
			for (int i = 0; i < 8; i++) {
				speed = item->speed * phd_cos(item->pos.x_rot) >> W2V_SHIFT;
				item->pos.x_pos += speed * phd_sin(item->pos.y_rot) >> 17;
				item->pos.y_pos += item->speed * phd_sin(-item->pos.x_rot) >> 17;
				item->pos.z_pos += speed * phd_cos(item->pos.y_rot) >> 17;
				ptr1 = Slist;

				for (int j = 0; j < num1; j++) {
					dx = ptr1->x - item->pos.x_pos;
					dy = ptr1->y - item->pos.y_pos;
					dz = ptr1->z - item->pos.z_pos;
					dy = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

					if (dy < SQUARE(ptr1->r)) {
						cs = j;
						break;
					}

					ptr1++;
				}

				if (cs != -1)
					break;
			}
		} else {
			ptr1 = Slist;

			for (int i = 0; i < num1; i++) {
				dx = ptr1->x - item->pos.x_pos;
				dy = ptr1->y - item->pos.y_pos;
				dz = ptr1->z - item->pos.z_pos;
				dy = SQUARE(dx) + SQUARE(dy) + SQUARE(dz) - SQUARE(ptr1->r);

				if (dy < cd) {
					cd = dy;
					cs = i;
				}

				ptr1++;
			}
		}
	}

	if (target->object_number == SKELETON) {
		if (cs != -1 && objects[target->object_number].explodable_meshbits & 1 << cs) {
			ExplodeItemNode(target, cs, 0, 0x40);
			target->mesh_bits &= ~(1 << cs);
		}
	} else {
		if (cs == num1 - 1) {
			if (target->flags & IFL_CODEBITS && (target->flags & IFL_CODEBITS) != IFL_CODEBITS) {
				room_number = target->room_number;
				GetHeight(GetFloor(target->pos.x_pos, target->pos.y_pos - CLICK_SIZE, target->pos.z_pos, &room_number),
				          target->pos.x_pos, target->pos.y_pos - CLICK_SIZE, target->pos.z_pos);
				TestTriggers(trigger_index, true, target->flags & IFL_CODEBITS);
			} else {
				NumTrigs = (int16_t)GetSwitchTrigger(target, TriggerItems, 1);

				for (int i = 0; i < NumTrigs; i++) {
					AddActiveItem(TriggerItems[i]);
					items[TriggerItems[i]].status = ITEM_ACTIVE;
					items[TriggerItems[i]].flags |= IFL_CODEBITS;
				}
			}

			if (target->object_number == SWITCH_TYPE7)
				ExplodeItemNode(target, objects[SWITCH_TYPE7].nmeshes - 1, 0, 0x40);

			AddActiveItem(int16_t(target - items));
			target->flags |= IFL_CODEBITS | IFL_SWITCH_ONESHOT;
			target->status = ITEM_ACTIVE;
		}
	}
}

void TriggerUnderwaterExplosion(ITEM_INFO* item, int32_t vehicle) {
	int32_t x, y, z, wh;

	if (vehicle) {
		x = (GetRandomControl() & 0x1FF) + item->pos.x_pos - CLICK_SIZE;
		z = (GetRandomControl() & 0x1FF) + item->pos.z_pos - CLICK_SIZE;
		TriggerExplosionBubble(x, item->pos.y_pos, z, item->room_number);
		TriggerExplosionSparks(x, item->pos.y_pos, z, 2, -1, 1, item->room_number);
		wh = GetWaterHeight(x, item->pos.y_pos, z, item->room_number);

		if (wh != NO_HEIGHT)
			TriggerSmallSplash(x, wh, z, 8);
	} else {
		TriggerExplosionBubble(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
		TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -2, 1, item->room_number);

		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -1, 1, item->room_number);

		wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

		if (wh != NO_HEIGHT) {
			y = item->pos.y_pos - wh;

			if (y < (BLOCK_SIZE * 2)) {
				splash_setup.x = item->pos.x_pos;
				splash_setup.y = wh;
				splash_setup.z = item->pos.z_pos;
				wh = (BLOCK_SIZE * 2) - y;
				splash_setup.InnerRadVel = 160;
				splash_setup.MiddleSize = 224;
				splash_setup.OuterRad = 272;
				splash_setup.InnerRad = int16_t((wh >> 6) + 16);
				splash_setup.InnerSize = int16_t((wh >> 6) + 12);
				splash_setup.InnerYVel = int16_t((-HALF_BLOCK_SIZE - wh) << 3);
				splash_setup.pad1 = int16_t((wh >> 6) + 24);
				splash_setup.MiddleRad = int16_t((wh >> 6) + 24);
				splash_setup.MiddleRadVel = int16_t((-(HALF_BLOCK_SIZE + CLICK_SIZE) - wh) << 2);
				splash_setup.MiddleYVel = int16_t((wh >> 6) + 32);
				splash_setup.pad2 = int16_t((wh >> 6) + 32);
				SetupSplash(&splash_setup);
			}
		}
	}
}

void draw_shotgun(int32_t weapon_type) {
	ITEM_INFO* item;

	if (lara.weapon_item == NO_ITEM) {
		lara.weapon_item = CreateItem();
		item = &items[lara.weapon_item];
		item->object_number = (int16_t)WeaponObject(weapon_type);

		if (weapon_type == WEAPON_GRENADE)
			item->anim_number = objects[GRENADE_GUN_ANIM].anim_index;
		else
			item->anim_number = objects[item->object_number].anim_index + 1;

		item->frame_number = anims[item->anim_number].frame_base;
		item->status = ITEM_ACTIVE;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
		item->room_number = 255;
		lara.left_arm.frame_base = objects[item->object_number].frame_base;
		lara.right_arm.frame_base = objects[item->object_number].frame_base;
	} else
		item = &items[lara.weapon_item];

	AnimateItem(item);

	if (!item->current_anim_state || item->current_anim_state == 6)
		ready_shotgun(weapon_type);
	else if (item->frame_number - anims[item->anim_number].frame_base == weapons[weapon_type].draw_frame)
		draw_shotgun_meshes(weapon_type);
	else if (lara.water_status == LW_UNDERWATER)
		item->goal_anim_state = 6;

	lara.left_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.left_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.left_arm.anim_number = item->anim_number;
	lara.right_arm.anim_number = item->anim_number;
}

void undraw_shotgun(int32_t weapon_type) {
	ITEM_INFO* item;

	item = &items[lara.weapon_item];

	if (lara.water_status == LW_SURFACE)
		item->goal_anim_state = 9;
	else
		item->goal_anim_state = 3;

	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED) {
		lara.gun_status = LG_NO_ARMS;
		lara.target = 0;
		lara.right_arm.lock = 0;
		lara.left_arm.lock = 0;
		KillItem(lara.weapon_item);
		lara.weapon_item = NO_ITEM;
		lara.right_arm.frame_number = 0;
		lara.left_arm.frame_number = 0;
	} else if (item->current_anim_state == 3 &&
	           anims[item->anim_number].frame_base == item->frame_number - (weapon_type == WEAPON_GRENADE ? 16 : 21))
		undraw_shotgun_meshes(weapon_type);

	lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
	lara.left_arm.frame_base = lara.right_arm.frame_base;
	lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	lara.left_arm.frame_number = lara.right_arm.frame_number;
	lara.right_arm.anim_number = item->anim_number;
	lara.left_arm.anim_number = lara.right_arm.anim_number;
}

void ControlCrossbow(int16_t item_number) {
	ITEM_INFO** itemlist;
	MESH_INFO** meshlist;
	ITEM_INFO* item;
	ITEM_INFO* target;
	MESH_INFO* mesh;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	PHD_VECTOR oldPos;
	int32_t abovewater, exploded, collided, rad, j;
	int16_t room_number;

	abovewater = 0;
	exploded = 0;
	collided = 0;
	item = &items[item_number];

	oldPos.x = item->pos.x_pos;
	oldPos.y = item->pos.y_pos;
	oldPos.z = item->pos.z_pos;

	if (room[item->room_number].flags & ROOM_UNDERWATER) {
		if (item->speed > QUARTER_CLICK_SIZE)
			item->speed -= item->speed >> 4;

		if (GlobalCounter & 1)
			CreateBubble(&item->pos, item->room_number, 4, 7);
	} else
		abovewater = 1;

	item->pos.x_pos += (((item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT) * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.y_pos += (item->speed * phd_sin(-item->pos.x_rot)) >> W2V_SHIFT;
	item->pos.z_pos += (((item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT) * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) < item->pos.y_pos ||
	        GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) > item->pos.y_pos) {
		item->pos.x_pos = oldPos.x;
		item->pos.y_pos = oldPos.y;
		item->pos.z_pos = oldPos.z;

		if (item->item_flags[0] == 3)
			exploded = 1;
		else {
			ExplodeItemNode(item, 0, 0, 0x100);
			KillItem(item_number);
			return;
		}
	}

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	r = &room[room_number];

	if (r->flags & ROOM_UNDERWATER && abovewater) {
		TriggerSmallSplash(item->pos.x_pos, r->maxceiling, item->pos.z_pos, 8);
		SetupRipple(item->pos.x_pos, r->maxceiling, item->pos.z_pos, (GetRandomControl() & 7) + 8, 0);
	}

	if (exploded)
		rad = (BLOCK_SIZE * 2);
	else
		rad = HALF_CLICK_SIZE;

	for (int i = 0; i < 2; i++) {
		itemlist = (ITEM_INFO**)&tsv_buffer[0x2000];
		meshlist = (MESH_INFO**)&tsv_buffer[0x3000];
		GetCollidedObjects(item, rad, 1, itemlist, meshlist, 1);

		if (!*itemlist && !*meshlist)
			break;

		collided = 1;

		if (item->item_flags[0] != 3 || exploded) {
			j = 0;
			target = itemlist[j];

			while (target) {
				if (exploded) {
					if (target->object_number >= SMASH_OBJECT1 && target->object_number <= SMASH_OBJECT8) {
						TriggerExplosionSparks(target->pos.x_pos, target->pos.y_pos, target->pos.z_pos, 3, -2, 0, target->room_number);
						target->pos.y_pos -= 128;
						TriggerShockwave((PHD_VECTOR*)&target->pos, 0x1300030, 96, 0x18806000, 0);
						target->pos.y_pos += 128;
						ExplodeItemNode(target, 0, 0, 128);
						SmashObject(int16_t(target - items));
						KillItem(int16_t(target - items));
					} else if (target->object_number == SWITCH_TYPE7 || target->object_number == SWITCH_TYPE8)
						CrossbowHitSwitchType78(item, target, 0);
					else if (objects[target->object_number].intelligent)
						DoGrenadeDamageOnBaddie(target, item);
				} else if (target->object_number == SWITCH_TYPE7 || target->object_number == SWITCH_TYPE8 || target->object_number == SKELETON)
					CrossbowHitSwitchType78(item, target, 1);
				else if (objects[target->object_number].intelligent) {
					HitTarget(target, (GAME_VECTOR*)&item->pos, weapons[WEAPON_CROSSBOW].damage, 0);

					if (item->item_flags[0] == 2 && !objects[target->object_number].undead)
						target->poisoned = 1;
				}

				j++;
				target = itemlist[j];
			}

			j = 0;
			mesh = meshlist[0];

			while (mesh) {
				MOD_LEVEL_STATIC_INFO* static_info = &get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number];
				if (static_info->lara_guns_can_shatter) {
					if (exploded) {
						TriggerExplosionSparks(mesh->x, mesh->y, mesh->z, 3, -2, 0, item->room_number);
						mesh->y -= 128;
						TriggerShockwave((PHD_VECTOR*)&mesh->x, 0xB00028, QUARTER_CLICK_SIZE, 0x10806000, 0);
						mesh->y += 128;
					}

					ShatterObject(0, mesh, -128, item->room_number, 0);
					SmashedMeshRoom[SmashedMeshCount] = item->room_number;
					SmashedMesh[SmashedMeshCount] = mesh;
					SmashedMeshCount++;
					mesh->Flags &= ~1;
				}

				j++;
				mesh = meshlist[j];
			}

			break;
		}

		exploded = 1;
		rad = (BLOCK_SIZE * 2);
	}

	if (exploded) {
		if (room[item->room_number].flags & ROOM_UNDERWATER)
			TriggerUnderwaterExplosion(item, 0);
		else {
			item->pos.y_pos -= 128;
			TriggerShockwave((PHD_VECTOR*)&item->pos, 0x1300030, 96, 0x18806000, 0);
			item->pos.y_pos += 128;
			TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->room_number);

			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->room_number);
		}

		AlertNearbyGuards(item);
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0x1800000 | SFX_SETPITCH);
		SoundEffect(SFX_EXPLOSION2, &item->pos, SFX_DEFAULT);
	}

	if (collided || exploded) {
		ExplodeItemNode(item, 0, 0, 0x100);
		KillItem(item_number);
	}
}

void ControlGrenade(int16_t item_number) {
	ITEM_INFO** itemlist;
	MESH_INFO** meshlist;
	ITEM_INFO* item;
	ITEM_INFO* item2;
	ITEM_INFO* target;
	MESH_INFO* mesh;
	FLOOR_INFO* floor;
	PHD_VECTOR oldPos;
	PHD_VECTOR pos;
	int32_t abovewater, xv, yv, zv, exploded, rad, j;
	int16_t new_num, yrot, room_number, NumTrigs;
	int16_t TriggerItems[8];

	item = &items[item_number];

	if (item->item_flags[1]) {
		item->item_flags[1]--;

		if (!item->item_flags[1]) {
			KillItem(item_number);
			return;
		}

		if (item->item_flags[0] == 3) {
			if (item->item_flags[1] == 1) {
				FlashFader = 32;
				FlashFadeR = 255;
				FlashFadeG = 255;
				FlashFadeB = 255;
				lara.blindTimer = 120;
			} else {
				FlashFader = 32;
				FlashFadeR = (GetRandomControl() & 0x1F) + 224;
				FlashFadeG = FlashFadeR - (GetRandomControl() & 0x1F);
				FlashFadeB = FlashFadeG;
			}

			if (IsVolumetric() &&
			        (
			            tomb4.volumetric_flash_grenades == VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_ONLY
			        )) {
				FlashFader = 0;
			}

			TriggerFlashSmoke(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
			TriggerFlashSmoke(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
		} else {
			new_num = CreateItem();

			if (new_num != NO_ITEM) {
				item2 = &items[new_num];
				item2->shade = -0x3DF0;
				item2->object_number = GRENADE;
				item2->room_number = item->room_number;
				item2->pos.x_pos = (GetRandomControl() & 0x1FF) + item->pos.x_pos - CLICK_SIZE;
				item2->pos.y_pos = item->pos.y_pos - CLICK_SIZE;
				item2->pos.z_pos = (GetRandomControl() & 0x1FF) + item->pos.z_pos - CLICK_SIZE;
				InitialiseItem(new_num);
				item2->pos.x_rot = (GetRandomControl() & 0x3FFF) + 0x2000;
				item2->pos.y_rot = int16_t(GetRandomControl() * 2);
				item2->pos.z_rot = 0;
				item2->speed = QUARTER_CLICK_SIZE;
				item2->fallspeed = (-QUARTER_CLICK_SIZE * phd_sin(item2->pos.x_rot)) >> W2V_SHIFT;
				item2->current_anim_state = item2->pos.x_rot;
				item2->goal_anim_state = item2->pos.y_rot;
				item2->required_anim_state = 0;
				AddActiveItem(new_num);
				item2->status = ITEM_INVISIBLE;
				item2->item_flags[0] = 4;
				item2->item_flags[2] = item->item_flags[2];

				if (room[item2->room_number].flags & ROOM_UNDERWATER)
					item2->hit_points = 1;
				else
					item2->hit_points = 3000;
			}
		}

		return;
	}

	oldPos.x = item->pos.x_pos;
	oldPos.y = item->pos.y_pos;
	oldPos.z = item->pos.z_pos;
	item->shade = -0x3DF0;

	if (room[item->room_number].flags & ROOM_UNDERWATER) {
		abovewater = 0;
		item->fallspeed += (5 - item->fallspeed) >> 1;
		item->speed -= item->speed >> 2;

		if (item->speed) {
			item->pos.z_rot += DEGREES_TO_ROTATION(1) * ((item->speed >> 4) + 3);

			if (item->required_anim_state)
				item->pos.y_rot += DEGREES_TO_ROTATION(1) * ((item->speed >> 2) + 3);
			else
				item->pos.x_rot += DEGREES_TO_ROTATION(1) * ((item->speed >> 2) + 3);
		}
	} else {
		abovewater = 1;
		item->fallspeed += 3;

		if (item->speed) {
			item->pos.z_rot += DEGREES_TO_ROTATION(1) * ((item->speed >> 2) + 7);

			if (item->required_anim_state)
				item->pos.y_rot += DEGREES_TO_ROTATION(1) * ((item->speed >> 1) + 7);
			else
				item->pos.x_rot += DEGREES_TO_ROTATION(1) * ((item->speed >> 1) + 7);
		}
	}

	if (item->speed && abovewater) {
		phd_PushUnitMatrix();
		phd_SetTrans(0, 0, 0);
		phd_RotYXZ(item->pos.y_rot + 0x8000, item->pos.x_rot, item->pos.z_rot);
		phd_TranslateRel(0, 0, -QUARTER_CLICK_SIZE);
		pos.x = (int32_t)mMXPtr[M03];
		pos.y = (int32_t)mMXPtr[M13];
		pos.z = (int32_t)mMXPtr[M23];
		phd_PopMatrix();

		TriggerRocketSmoke(item->pos.x_pos + pos.x, item->pos.y_pos + pos.y, item->pos.z_pos + pos.z, -1);
	}

	xv = (item->speed * phd_sin(item->goal_anim_state)) >> W2V_SHIFT;
	yv = item->fallspeed;
	zv = (item->speed * phd_cos(item->goal_anim_state)) >> W2V_SHIFT;
	item->pos.x_pos += xv;
	item->pos.y_pos += yv;
	item->pos.z_pos += zv;

	if (item->item_flags[0] == 4) {
		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) < item->pos.y_pos ||
		        GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) > item->pos.y_pos)
			item->hit_points = 1;
	} else {
		yrot = item->pos.y_rot;
		item->pos.y_rot = item->goal_anim_state;
		DoProperDetection(item_number, oldPos.x, oldPos.y, oldPos.z, xv, yv, zv);
		item->goal_anim_state = item->pos.y_rot;
		item->pos.y_rot = yrot;
	}

	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room[room_number].flags & ROOM_UNDERWATER && abovewater) {
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = room[room_number].maxceiling;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerRad = 32;
		splash_setup.InnerSize = 8;
		splash_setup.InnerRadVel = 320;
		splash_setup.InnerYVel = -40 * item->fallspeed;
		splash_setup.pad1 = 48;
		splash_setup.MiddleRad = 32;
		splash_setup.MiddleSize = 480;
		splash_setup.MiddleYVel = 32;
		splash_setup.MiddleRadVel = -20 * item->fallspeed;
		splash_setup.pad2 = 128;
		splash_setup.OuterRad = 544;
		SetupSplash(&splash_setup);

		if (item->item_flags[0] == 4)
			item->hit_points = 1;
	}

	if (item->item_flags[0] == 4)
		TriggerFireFlame(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, -1, 1);

	exploded = 0;
	rad = 0;

	if (item->hit_points) {
		item->hit_points--;

		if (!item->hit_points) {
			rad = (BLOCK_SIZE * 2);
			exploded = 1;
		} else if (item->hit_points > 118)
			return;
	}

	if (item->item_flags[0] != 3 || !exploded) {
		for (int i = 0; i < 2; i++) {
			itemlist = (ITEM_INFO**)&tsv_buffer[0x2000];
			meshlist = (MESH_INFO**)&tsv_buffer[0x3000];
			GetCollidedObjects(item, rad, 1, itemlist, meshlist, 1);

			if (exploded) {
				j = 0;
				target = itemlist[j];

				while (target) {
					if (target->object_number >= SMASH_OBJECT1 && target->object_number <= SMASH_OBJECT8) {
						TriggerExplosionSparks(target->pos.x_pos, target->pos.y_pos, target->pos.z_pos, 3, -2, 0, target->room_number);
						target->pos.y_pos -= 128;
						TriggerShockwave((PHD_VECTOR*)&target->pos, 0x1300030, 96, 0x18806000, 0);
						target->pos.y_pos += 128;
						ExplodeItemNode(target, 0, 0, 128);
						SmashObject(int16_t(target - items));
						KillItem(int16_t(target - items));
					} else if ((target->object_number == SWITCH_TYPE7 || target->object_number == SWITCH_TYPE8) && !(target->flags & IFL_SWITCH_ONESHOT)) {
						if (!(target->flags & IFL_CODEBITS) || (target->flags & IFL_CODEBITS) == IFL_CODEBITS) {
							NumTrigs = (int16_t)GetSwitchTrigger(target, TriggerItems, 1);

							for (int i = 0; i < NumTrigs; i++) {
								AddActiveItem(TriggerItems[i]);
								items[TriggerItems[i]].status = ITEM_ACTIVE;
								items[TriggerItems[i]].flags |= IFL_CODEBITS;
							}
						} else {
							room_number = item->room_number;
							GetHeight(GetFloor(target->pos.x_pos, target->pos.y_pos - CLICK_SIZE, target->pos.z_pos, &room_number),
							          target->pos.x_pos, target->pos.y_pos - CLICK_SIZE, target->pos.z_pos);
							TestTriggers(trigger_index, true, target->flags & IFL_CODEBITS);
						}

						if (target->object_number == SWITCH_TYPE7)
							ExplodeItemNode(target, objects[SWITCH_TYPE7].nmeshes - 1, 0, 0x40);

						AddActiveItem(int16_t(target - items));
						target->status = ITEM_ACTIVE;
						target->flags |= IFL_SWITCH_ONESHOT | IFL_CODEBITS;
					} else if (objects[target->object_number].intelligent || (target->object_number == T4PlusGetLaraSlotID() && get_game_mod_global_info()->grenades_damage_lara))
						DoGrenadeDamageOnBaddie(target, item);

					j++;
					target = itemlist[j];
				}

				j = 0;
				mesh = meshlist[0];

				while (mesh) {
					if (get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number].lara_guns_can_shatter) {
						Log(0, "Shatter");
						TriggerExplosionSparks(mesh->x, mesh->y, mesh->z, 3, -2, 0, item->room_number);
						mesh->y -= HALF_CLICK_SIZE;
						TriggerShockwave((PHD_VECTOR*)&mesh->x, 0xB00028, 64, 0x10806000, 0);
						mesh->y += HALF_CLICK_SIZE;
						ShatterObject(0, mesh, -HALF_CLICK_SIZE, item->room_number, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->room_number;
						SmashedMesh[SmashedMeshCount] = mesh;
						SmashedMeshCount++;
						mesh->Flags &= ~1;
					}

					j++;
					mesh = meshlist[j];
				}
			} else {
				if (!itemlist[0] && !meshlist[0])
					break;

				exploded = 1;

				if (item->item_flags[0] == 3)
					break;

				rad = (BLOCK_SIZE * 2);
			}
		}
	}

	if (exploded) {
		if (item->item_flags[0] == 3) {
			FlashFader = 32;
			FlashFadeR = 255;
			FlashFadeG = 255;
			FlashFadeB = 255;

			if (IsVolumetric() &&
			        (
			            tomb4.volumetric_flash_grenades == VOLUMETRIC_FLASH_GRENADES_DEFAULT ||
			            tomb4.volumetric_flash_grenades == VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_ONLY ||
			            tomb4.volumetric_flash_grenades == VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_AND_FLASH
			        )) {
				FlashFader = 0;
				TriggerFXFogBulb(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 4096, 255, 255, 255, 255, item->room_number);
			}

			TriggerFlashSmoke(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
			TriggerFlashSmoke(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
		} else if (room[item->room_number].flags & ROOM_UNDERWATER)
			TriggerUnderwaterExplosion(item, 0);
		else {
			item->pos.y_pos -= 128;
			TriggerShockwave((PHD_VECTOR*)&item->pos, 0x1300030, 96, 0x18806000, 0);
			item->pos.y_pos += 128;
			TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->room_number);

			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->room_number);
		}

		AlertNearbyGuards(item);
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0x1800004);
		SoundEffect(SFX_EXPLOSION2, &item->pos, 0);

		if (item->item_flags[0] == 1 || item->item_flags[0] == 4)
			KillItem(item_number);
		else {
			item->mesh_bits = 0;

			if (item->item_flags[0] == 2)
				item->item_flags[1] = 4;
			else
				item->item_flags[1] = 16;
		}
	}
}
