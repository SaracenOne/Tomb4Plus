#include "../tomb4/pch.h"
#include "lara2gun.h"
#include "objects.h"
#include "larafire.h"
#include "sound.h"
#include "delstuff.h"
#include "tomb4fx.h"
#include "../specific/function_stubs.h"
#include "effect2.h"
#include "camera.h"
#include "../specific/input.h"
#include "lara.h"
#include "savegame.h"
#include "gameflow.h"
#include "control.h"

#include "../tomb4/tomb4plus/t4plus_objects.h"
#include "../tomb4/tomb4plus/t4plus_mirror.h"

static PISTOL_DEF PistolTable[4] = {
	{ T4PlusGetLaraSlotID, 0, 0, 0, 0 },
	{ T4PlusGetPistolsAnimSlotID, 4, 5, 13, 24 },
	{ T4PlusGetRevolverAnimSlotID, 7, 8, 15, 29 },
	{ T4PlusGetUziAnimSlotID, 4, 5, 13, 24 }
};

void undraw_pistol_mesh_left(int32_t weapon_type) {
	if (weapon_type != WEAPON_REVOLVER) {
		WeaponObject(weapon_type);
		lara.mesh_ptrs[LM_LHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_LHAND * 2];

		// TRLE: prevent switching if we have removed the holsters
		if (weapon_type == WEAPON_PISTOLS)
			lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersPistolsSlotID() : T4PlusGetLaraSlotID();
		else if (weapon_type == WEAPON_UZI)
			lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersUzisSlotID() : T4PlusGetLaraSlotID();
	}
}

void undraw_pistol_mesh_right(int32_t weapon_type) {
	WeaponObject(weapon_type);
	lara.mesh_ptrs[LM_RHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_RHAND * 2];

	// TRLE: prevent switching if we have removed the holsters
	if (weapon_type == WEAPON_PISTOLS)
		lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersPistolsSlotID() : T4PlusGetLaraSlotID();
	else if (weapon_type == WEAPON_UZI)
		lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersUzisSlotID () : T4PlusGetLaraSlotID();
	else if (weapon_type == WEAPON_REVOLVER)
		lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersRevolverSlotID() : T4PlusGetLaraSlotID();
}

static void set_arm_info(LARA_ARM* arm, int32_t frame) {
	PISTOL_DEF* p;
	int32_t anim_base;

	p = &PistolTable[lara.gun_type];
	anim_base = objects[p->ObjectFunc()].anim_index;

	if (frame >= p->Draw1Anim) {
		if (frame >= p->Draw2Anim) {
			if (frame < p->RecoilAnim)
				anim_base += 2;
			else
				anim_base += 3;
		} else
			anim_base++;
	}

	arm->anim_number = (int16_t)anim_base;
	arm->frame_number = (int16_t)frame;
	arm->frame_base = anims[anim_base].frame_ptr;
}

void ready_pistols(int32_t weapon_type) {
	lara.gun_status = LG_READY;
	lara.left_arm.x_rot = 0;
	lara.left_arm.y_rot = 0;
	lara.left_arm.z_rot = 0;
	lara.right_arm.x_rot = 0;
	lara.right_arm.y_rot = 0;
	lara.right_arm.z_rot = 0;
	lara.right_arm.frame_number = 0;
	lara.left_arm.frame_number = 0;
	lara.target = 0;
	lara.right_arm.lock = 0;
	lara.left_arm.lock = 0;
	lara.right_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;
	lara.left_arm.frame_base = lara.right_arm.frame_base;
}

void draw_pistol_meshes(int32_t weapon_type) {
	int32_t mesh_index;

	mesh_index = objects[WeaponObjectMesh(weapon_type)].mesh_index;
	// TRLE: check if we have removed the holsters.
	lara.holster = lara.holster != T4PlusGetLaraSlotID() ? T4PlusGetLaraHolstersSlotID() : T4PlusGetLaraSlotID();
	lara.mesh_ptrs[LM_RHAND] = meshes[mesh_index + LM_RHAND * 2];

	if (weapon_type != WEAPON_REVOLVER)
		lara.mesh_ptrs[LM_LHAND] = meshes[mesh_index + LM_LHAND * 2];
}

void draw_pistols(int32_t weapon_type) {
	PISTOL_DEF* p;
	int16_t ani;

	ani = lara.left_arm.frame_number + 1;
	p = &PistolTable[lara.gun_type];

	if (ani < p->Draw1Anim || ani > p->RecoilAnim - 1)
		ani = p->Draw1Anim;
	else if (ani == p->Draw2Anim) {
		draw_pistol_meshes(weapon_type);
		SoundEffect(SFX_LARA_DRAW, &lara_item->pos, SFX_DEFAULT);
	} else if (ani == p->RecoilAnim - 1) {
		ready_pistols(weapon_type);
		ani = 0;
	}

	set_arm_info(&lara.right_arm, ani);
	set_arm_info(&lara.left_arm, ani);
}

void undraw_pistols(int32_t weapon_type) {
	PISTOL_DEF* p;
	int16_t anil, anir;

	p = &PistolTable[lara.gun_type];
	anil = lara.left_arm.frame_number;

	if (lara.left_arm.frame_number >= p->RecoilAnim)
		anil = p->Draw1Anim2;
	else if (lara.left_arm.frame_number > 0 && lara.left_arm.frame_number < p->Draw1Anim) {
		lara.left_arm.x_rot -= lara.left_arm.x_rot / anil;
		lara.left_arm.y_rot -= lara.left_arm.y_rot / anil;
		anil--;
	} else if (!lara.left_arm.frame_number) {
		lara.left_arm.x_rot = 0;
		lara.left_arm.y_rot = 0;
		lara.left_arm.z_rot = 0;
		anil = p->RecoilAnim - 1;
	} else if (lara.left_arm.frame_number > p->Draw1Anim) {
		anil--;

		if (anil == p->Draw2Anim - 1) {
			undraw_pistol_mesh_left(weapon_type);
			SoundEffect(SFX_LARA_HOLSTER, &lara_item->pos, SFX_DEFAULT);
		}
	}

	set_arm_info(&lara.left_arm, anil);
	anir = lara.right_arm.frame_number;

	if (lara.right_arm.frame_number >= p->RecoilAnim)
		anir = p->Draw1Anim2;
	else if (lara.right_arm.frame_number > 0 && lara.right_arm.frame_number < p->Draw1Anim) {
		lara.right_arm.x_rot -= lara.right_arm.x_rot / anir;
		lara.right_arm.y_rot -= lara.right_arm.y_rot / anir;
		anir--;
	} else if (!lara.right_arm.frame_number) {
		lara.right_arm.z_rot = 0;
		lara.right_arm.y_rot = 0;
		lara.right_arm.x_rot = 0;
		anir = p->RecoilAnim - 1;
	} else if (lara.right_arm.frame_number > p->Draw1Anim) {
		anir--;

		if (anir == p->Draw2Anim - 1) {
			undraw_pistol_mesh_right(weapon_type);
			SoundEffect(SFX_LARA_HOLSTER, &lara_item->pos, SFX_DEFAULT);
		}
	}

	set_arm_info(&lara.right_arm, anir);

	if (anil == p->Draw1Anim && anir == p->Draw1Anim) {
		lara.gun_status = LG_NO_ARMS;
		lara.left_arm.frame_number = 0;
		lara.right_arm.frame_number = 0;
		lara.target = 0;
		lara.right_arm.lock = 0;
		lara.left_arm.lock = 0;
	}

	if (!(input & IN_LOOK)) {
		lara.head_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) >> 2;
		lara.torso_x_rot = lara.head_x_rot;
		lara.head_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) >> 2;
		lara.torso_y_rot = lara.head_y_rot;
	}
}

void AnimatePistols(int32_t weapon_type) {
	PISTOL_DEF* p;
	WEAPON_INFO* winfo;
	PHD_VECTOR pos;
	static int32_t uzi_left;
	static int32_t uzi_right;
	int16_t angles[2];
	int16_t anil, anir, sound_already;

	sound_already = 0;

	if (lara_item->mesh_bits) {
		if (SmokeCountL) {
			switch (SmokeWeapon) {
				case WEAPON_PISTOLS:
					pos.x = 4;
					pos.y = 128;
					pos.z = 40;
					break;

				case WEAPON_REVOLVER:
					pos.x = 16;
					pos.y = 160;
					pos.z = 56;
					break;

				case WEAPON_UZI:
					pos.x = 8;
					pos.y = 140;
					pos.z = 48;
					break;
			}

			GetLaraJointPos(&pos, LMX_HAND_L);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
		}

		if (SmokeCountR) {
			switch (SmokeWeapon) {
				case WEAPON_PISTOLS:
					pos.x = -16;
					pos.y = 128;
					pos.z = 40;
					break;

				case WEAPON_REVOLVER:
					pos.x = -32;
					pos.y = 160;
					pos.z = 56;
					break;

				case WEAPON_UZI:
					pos.x = -16;
					pos.y = 140;
					pos.z = 48;
					break;
			}

			GetLaraJointPos(&pos, LMX_HAND_R);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountR);
		}
	}

	p = &PistolTable[lara.gun_type];
	winfo = &weapons[weapon_type];
	anir = lara.right_arm.frame_number;

	if (lara.right_arm.lock || input & IN_ACTION && !lara.target) {
		if (lara.right_arm.frame_number >= 0 && lara.right_arm.frame_number < p->Draw1Anim2)
			anir++;
		else if (lara.right_arm.frame_number == p->Draw1Anim2) {
			if (input & IN_ACTION) {
				if (weapon_type != WEAPON_REVOLVER) {
					angles[0] = lara.right_arm.y_rot + lara_item->pos.y_rot;
					angles[1] = lara.right_arm.x_rot;

					if (FireWeapon(weapon_type, lara.target, lara_item, angles)) {
						SmokeCountR = 28;
						SmokeWeapon = weapon_type;
						TriggerGunShell(1, GUNSHELL, weapon_type);
						lara.right_arm.flash_gun = winfo->flash_time;
						SoundEffect(SFX_EXPLOSION1, &lara_item->pos, 0x2000000 | SFX_SETPITCH);
						SoundEffect(winfo->sample_num, &lara_item->pos, SFX_DEFAULT);
						sound_already = 1;

						if (weapon_type == WEAPON_UZI)
							uzi_right = 1;

						savegame.Game.AmmoUsed++;
					}
				}

				anir = p->RecoilAnim;
			} else if (uzi_right) {
				SoundEffect(winfo->sample_num + 1, &lara_item->pos, SFX_DEFAULT);
				uzi_right = 0;
			}
		} else if (lara.right_arm.frame_number >= p->RecoilAnim) {
			if (weapon_type == WEAPON_UZI) {
				SoundEffect(winfo->sample_num, &lara_item->pos, SFX_DEFAULT);
				uzi_right = 1;
			}

			anir++;

			if (anir == p->RecoilAnim + winfo->recoil_frame)
				anir = p->Draw1Anim2;
		}
	} else {
		if (lara.right_arm.frame_number >= p->RecoilAnim)
			anir = p->Draw1Anim2;
		else if (lara.right_arm.frame_number > 0 && lara.right_arm.frame_number <= p->Draw1Anim2)
			anir--;

		if (uzi_right) {
			SoundEffect(winfo->sample_num + 1, &lara_item->pos, SFX_DEFAULT);
			uzi_right = 0;
		}
	}

	set_arm_info(&lara.right_arm, anir);
	anil = lara.left_arm.frame_number;

	if (lara.left_arm.lock || input & IN_ACTION && !lara.target) {
		if (lara.left_arm.frame_number >= 0 && lara.left_arm.frame_number < p->Draw1Anim2)
			anil++;
		else if (lara.left_arm.frame_number == p->Draw1Anim2) {
			if (input & IN_ACTION) {
				angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
				angles[1] = lara.left_arm.x_rot;

				if (FireWeapon(weapon_type, lara.target, lara_item, angles)) {
					if (weapon_type == WEAPON_REVOLVER) {
						SmokeCountR = 28;
						SmokeWeapon = WEAPON_REVOLVER;
						lara.right_arm.flash_gun = winfo->flash_time;
					} else {
						SmokeCountL = 28;
						SmokeWeapon = weapon_type;
						TriggerGunShell(0, GUNSHELL, weapon_type);
						lara.left_arm.flash_gun = winfo->flash_time;
					}

					if (!sound_already) {
						SoundEffect(SFX_EXPLOSION1, &lara_item->pos, 0x2000000 | SFX_SETPITCH);
						SoundEffect(winfo->sample_num, &lara_item->pos, SFX_DEFAULT);
					}

					if (weapon_type == WEAPON_UZI)
						uzi_left = 1;

					savegame.Game.AmmoUsed++;
				}

				anil = p->RecoilAnim;
			} else if (uzi_left) {
				SoundEffect(winfo->sample_num + 1, &lara_item->pos, SFX_DEFAULT);
				uzi_left = 0;
			}
		} else if (lara.left_arm.frame_number >= p->RecoilAnim) {
			if (weapon_type == WEAPON_UZI) {
				SoundEffect(winfo->sample_num, &lara_item->pos, SFX_DEFAULT);
				uzi_left = 1;
			}

			anil++;

			if (anil == p->RecoilAnim + winfo->recoil_frame)
				anil = p->Draw1Anim2;
		}
	} else {
		if (lara.left_arm.frame_number >= p->RecoilAnim)
			anil = p->Draw1Anim2;
		else if (lara.left_arm.frame_number > 0 && lara.left_arm.frame_number <= p->Draw1Anim2)
			anil--;

		if (uzi_left) {
			SoundEffect(winfo->sample_num + 1, &lara_item->pos, SFX_DEFAULT);
			uzi_left = 0;
		}
	}

	set_arm_info(&lara.left_arm, anil);
}

void PistolHandler(int32_t weapon_type) {
	WEAPON_INFO* winfo;
	PHD_VECTOR pos;
	int32_t r, g, b;

	winfo = &weapons[weapon_type];
	LaraGetNewTarget(winfo);

	if (input & IN_ACTION)
		LaraTargetInfo(winfo);

	AimWeapon(winfo, &lara.left_arm);
	AimWeapon(winfo, &lara.right_arm);

	if (lara.left_arm.lock && lara.right_arm.lock) {
		lara.torso_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) >> 2;
		lara.torso_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) >> 2;

		if (camera.old_type != LOOK_CAMERA) {
			lara.head_y_rot = lara.torso_y_rot;
			lara.head_x_rot = lara.torso_x_rot;
		}
	} else if (lara.left_arm.lock && !lara.right_arm.lock) {
		lara.torso_y_rot = lara.left_arm.y_rot >> 1;
		lara.torso_x_rot = lara.left_arm.x_rot >> 1;

		if (camera.old_type != LOOK_CAMERA) {
			lara.head_y_rot = lara.torso_y_rot;
			lara.head_x_rot = lara.torso_x_rot;
		}
	} else if (!lara.left_arm.lock && lara.right_arm.lock) {
		lara.torso_y_rot = lara.right_arm.y_rot >> 1;
		lara.torso_x_rot = lara.right_arm.x_rot >> 1;

		if (camera.old_type != LOOK_CAMERA) {
			lara.head_y_rot = lara.torso_y_rot;
			lara.head_x_rot = lara.torso_x_rot;
		}
	}

	AnimatePistols(weapon_type);

	if (lara.left_arm.flash_gun || lara.right_arm.flash_gun) {
		pos.x = (GetRandomControl() & 0xFF) - 128;
		pos.y = (GetRandomControl() & 0x7F) - 63;
		pos.z = (GetRandomControl() & 0xFF) - 128;

		if (lara.left_arm.flash_gun)
			GetLaraJointPos(&pos, LMX_LARM_L);
		else
			GetLaraJointPos(&pos, LMX_HAND_L);

		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 128;
		b = GetRandomControl() & 0x3F;

		TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);

		for (int32_t i = 0; i < t4p_mirror_count; i++) {
			if (lara_item->room_number == t4p_mirror_info[i].mirror_room) {
				PHD_VECTOR mirrored_pos = T4PMirrorVectorOnPlane(&t4p_mirror_info[i], pos);

				TriggerDynamic(mirrored_pos.x, mirrored_pos.y, mirrored_pos.z, 10, r, g, b);
			}
		}
	}
}
