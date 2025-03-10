#include "../tomb4/pch.h"
#include "laraflar.h"
#include "../specific/3dmath.h"
#include "objects.h"
#include "../specific/output.h"
#include "../specific/function_stubs.h"
#include "effect2.h"
#include "delstuff.h"
#include "items.h"
#include "control.h"
#include "collide.h"
#include "lara_states.h"
#include "sound.h"
#include "larafire.h"
#include "lara.h"
#include "gameflow.h"
#include "draw.h"
#include "tomb4fx.h"

#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_objects.h"
#include "../tomb4/tomb4plus/t4plus_environment.h"

void DrawFlareInAir(ITEM_INFO* item) {
	int16_t* bounds;

	bounds = GetBoundsAccurate(item);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos - bounds[3], item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	phd_PutPolygons_train(meshes[objects[FLARE_ITEM].mesh_index], 0);
	phd_PopMatrix();

	for (int i = 0; i < t4p_mirror_count; i++) {
		if (item->room_number == t4p_mirror_info[i].mirror_room) {
			phd_PushMatrix();
			PHD_3DPOS mirrored_pos = T4PMirrorUnrotated3DPosOnPlane(&t4p_mirror_info[i], item->pos);
			phd_TranslateAbs(mirrored_pos.x_pos, mirrored_pos.y_pos, mirrored_pos.z_pos);
			phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
			phd_PutPolygons_train(meshes[objects[FLARE_ITEM].mesh_index], 0);
			phd_PopMatrix();
		}
	}
}

void draw_flare_meshes() {
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[FLARE_ANIM].mesh_index + LM_LHAND * 2];
}

void undraw_flare_meshes() {
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + LM_LHAND * 2];
}

int32_t DoFlareLight(PHD_VECTOR* pos, int32_t flare_age) {
	int32_t x, y, z, r, g, b, rnd, falloff, ret;

	MOD_LEVEL_FLARE_INFO *flare_info = get_game_mod_level_flare_info(gfCurrentLevel);

	if (flare_age >= flare_info->flare_lifetime_in_ticks || !flare_age)
		return 0;

	rnd = GetRandomControl();
	x = pos->x + ((rnd & 0xF) << 3);
	y = pos->y + (rnd >> 1 & 120) - CLICK_SIZE;
	z = pos->z + (rnd >> 5 & 120);
	ret = 1;

	if (flare_age < 4) {
		falloff = (rnd & 3) + 4 + flare_age * 4;

		if (falloff > 16)
			falloff -= (rnd >> 12) & 3;

		r = (rnd >> 4 & 0x1F) + 8 * flare_age + 32;
		g = (rnd & 0x1F) + 16 * (flare_age + 10);
		b = 16 * flare_age + ((rnd >> 8) & 0x1F);
	} else if (flare_age < 16) {
		falloff = (rnd & 1) + flare_age + 2;
		r = ((rnd >> 4) & 0x1F) + 4 * flare_age + 64;
		g = (rnd & 0x3F) + 4 * flare_age + 128;
		b = ((rnd >> 8) & 0x1F) + 4 * flare_age + 16;
	} else if (flare_age < (flare_info->flare_lifetime_in_ticks - 90)) {
		falloff = flare_info->light_intensity;
		r = (rnd >> 4 & 0x1F) + flare_info->light_color_r;
		g = (rnd & 0x3F) + flare_info->light_color_g;
		b = ((rnd >> 8) & 0x3F) + (((rnd >> 6) & 0x7F) >> 2) + flare_info->light_color_b;
	} else if (flare_age < (flare_info->flare_lifetime_in_ticks - 24)) {
		if (rnd > 0x2000) {
			falloff = flare_info->light_intensity;
			r = (rnd & 0x1F) + 128;
			g = (rnd & 0x3F) + 192;
			b = ((rnd >> 8) & 0x3F) + (((rnd >> 6) & 0x7F) >> 2);
		} else {
			r = (GetRandomControl() & 0x3F) + 64;
			g = (GetRandomControl() & 0x3F) + 192;
			b = GetRandomControl() & 0x7F;
			falloff = (GetRandomControl() & 6) + (flare_info->light_intensity / 2);
			ret = 0;
		}
	} else {
		falloff = flare_info->light_intensity - int32_t(float(((flare_age - (flare_info->flare_lifetime_in_ticks - 24)) >> 1)) * ((float)flare_info->light_intensity / 16.0f));
		r = (GetRandomControl() & 0x3F) + 64;
		g = (GetRandomControl() & 0x3F) + 192;
		b = GetRandomControl() & 0x1F;
		ret = rnd & 1;
	}

	// Clamp the lighting values
	if (r >= 0xff)
		r = 0xff;
	if (g >= 0xff)
		g = 0xff;
	if (b >= 0xff)
		b = 0xff;

	if (flare_info->flat_light) {
		TriggerDynamic(x, y, z, falloff, flare_info->light_color_r, flare_info->light_color_g, flare_info->light_color_b);
	} else {
		TriggerDynamic(x, y, z, falloff, r, g, b);
	}

	if (flare_info->has_sparks) {
		if (flare_age < flare_info->flare_lifetime_in_ticks - 24) {
			uint32_t flare_spark_rnd = GetRandomControl();
			TriggerFlareSparks(pos->x, pos->y, pos->z, 0, int32_t((float(flare_spark_rnd) * -0.025f)), 0, flare_info->sparks_include_smoke);
		}
	}

	if (flare_info->has_glow) {
		SPARKS *sptr = &spark[GetFreeSpark()];
		sptr->On = 1;
		sptr->sR = flare_info->light_color_r;
		sptr->sG = flare_info->light_color_g;
		sptr->sB = flare_info->light_color_b;
		sptr->dR = sptr->sR;
		sptr->dG = sptr->sG;
		sptr->dB = sptr->sB;
		sptr->Life = 4;
		sptr->sLife = 4;
		sptr->ColFadeSpeed = 2;
		sptr->FadeToBlack = 0;
		sptr->TransType = 2;
		sptr->x = pos->x;
		sptr->y = pos->y;
		sptr->z = pos->z;
		sptr->Zvel = 0;
		sptr->Yvel = 0;
		sptr->Xvel = 0;
		sptr->Flags = SF_SCALE | SF_DEF;
		sptr->Scalar = 3;
		sptr->MaxYvel = 0;
		sptr->Def = objects[T4PlusGetDefaultSpritesSlotID()].mesh_index + 11;
		sptr->Gravity = 0;
		sptr->Size = QUARTER_CLICK_SIZE;
		sptr->dSize = sptr->Size;
		sptr->sSize = sptr->Size;
	}

	return ret;
}

void DoFlareInHand(int32_t flare_age) {
	PHD_VECTOR pos;

	pos.x = 11;
	pos.y = 32;
	pos.z = 41;
	GetLaraJointPos(&pos, LMX_HAND_L);
	lara.left_arm.flash_gun = (int16_t)DoFlareLight(&pos, flare_age);

	for (int i = 0; i < t4p_mirror_count; i++) {
		if (lara_item->room_number == t4p_mirror_info[i].mirror_room) {
			PHD_VECTOR mirrored_vector = pos;
			mirrored_vector = T4PMirrorVectorOnPlane(&t4p_mirror_info[i], mirrored_vector);
			DoFlareLight(&mirrored_vector, flare_age);
		}
	}

	// TRLE
	MOD_LEVEL_FLARE_INFO *flare_info = get_game_mod_level_flare_info(gfCurrentLevel);

	if (lara.flare_age < flare_info->flare_lifetime_in_ticks)
		lara.flare_age++;
	else if (lara.gun_status == LG_NO_ARMS)
		lara.gun_status = LG_UNDRAW_GUNS;
}

void CreateFlare(int16_t object, int32_t thrown) {
	ITEM_INFO** itemlist;
	MESH_INFO** meshlist;
	ITEM_INFO* flare;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	int32_t collided;
	int16_t flare_item, room_number;

	flare_item = CreateItem();

	if (flare_item != NO_ITEM) {
		collided = 0;
		flare = &items[flare_item];
		flare->object_number = object;
		flare->room_number = lara_item->room_number;

		pos.x = -16;
		pos.y = 32;
		pos.z = 42;
		GetLaraJointPos(&pos, LMX_HAND_L);
		flare->pos.x_pos = pos.x;
		flare->pos.y_pos = pos.y;
		flare->pos.z_pos = pos.z;

		room_number = lara_item->room_number;
		floor = GetFloor(pos.x, pos.y, pos.z, &room_number);
		itemlist = (ITEM_INFO**)&tsv_buffer[0];
		meshlist = (MESH_INFO**)&tsv_buffer[1024];

		if (GetCollidedObjects(flare, 0, 1, itemlist, meshlist, 0) || pos.y > GetHeight(floor, pos.x, pos.y, pos.z)) {
			collided = 1;
			flare->pos.y_rot = lara_item->pos.y_rot - 0x8000;
			flare->pos.x_pos = lara_item->pos.x_pos + (80 * phd_sin(flare->pos.y_rot) >> W2V_SHIFT);
			flare->pos.z_pos = lara_item->pos.z_pos + (80 * phd_cos(flare->pos.y_rot) >> W2V_SHIFT);
			flare->room_number = lara_item->room_number;
		} else {
			if (thrown)
				flare->pos.y_rot = lara_item->pos.y_rot;
			else
				flare->pos.y_rot = lara_item->pos.y_rot - 0x2000;

			flare->room_number = room_number;
		}

		InitialiseItem(flare_item);
		flare->pos.x_rot = 0;
		flare->pos.z_rot = 0;
		flare->shade = -1;

		if (thrown) {
			flare->speed = lara_item->speed + 50;
			flare->fallspeed = lara_item->fallspeed - 50;
		} else {
			flare->speed = lara_item->speed + 10;
			flare->fallspeed = lara_item->fallspeed + 50;
		}

		if (collided)
			flare->speed >>= 1;

		if (object == FLARE_ITEM) {
			if (DoFlareLight((PHD_VECTOR*)&flare->pos, lara.flare_age))
				flare->data = (void*)size_t((lara.flare_age | (int16_t)0x8000));
			else
				flare->data = (void*)size_t((lara.flare_age & (int16_t)0x7FFF));
		} else
			flare->item_flags[3] = lara.LitTorch;

		AddActiveItem(flare_item);
		flare->status = ITEM_ACTIVE;
	}
}

void set_flare_arm(int32_t frame) {
	int16_t anim_base;

	anim_base = objects[FLARE_ANIM].anim_index;

	if (frame >= 1) {
		if (frame < 33)
			anim_base += 1;
		else if (frame < 72)
			anim_base += 2;
		else if (frame < 95)
			anim_base += 3;
		else
			anim_base += 4;
	}

	lara.left_arm.anim_number = anim_base;
	lara.left_arm.frame_base = anims[anim_base].frame_ptr;
}

void ready_flare() {
	lara.gun_status = LG_NO_ARMS;
	lara.left_arm.x_rot = 0;
	lara.left_arm.y_rot = 0;
	lara.left_arm.z_rot = 0;
	lara.right_arm.x_rot = 0;
	lara.right_arm.y_rot = 0;
	lara.right_arm.z_rot = 0;
	lara.right_arm.lock = 0;
	lara.left_arm.lock = 0;
	lara.target = 0;
}

void draw_flare() {
	int16_t ani;

	if (lara_item->current_anim_state == AS_FLAREPICKUP || lara_item->current_anim_state == AS_PICKUP) {
		DoFlareInHand(lara.flare_age);
		lara.flare_control_left = 0;
		ani = 93;
	} else {
		lara.flare_control_left = 1;
		ani = lara.left_arm.frame_number + 1;

		if (ani < 33 || ani > 94)
			ani = 33;
		else if (ani == 46)
			draw_flare_meshes();
		else if (ani >= 72 && ani <= 93) {
			if (ani == 72) {
				if (room[lara_item->room_number].flags & ROOM_UNDERWATER)
					SoundEffect(SFX_OBJ_GEM_SMASH, &lara_item->pos, SFX_WATER);
				else
					SoundEffect(SFX_OBJ_GEM_SMASH, &lara_item->pos, SFX_DEFAULT);

				lara.flare_age = 1;
			}

			DoFlareInHand(lara.flare_age);
		} else if (ani == 94) {
			ready_flare();
			ani = 0;
			DoFlareInHand(lara.flare_age);
		}
	}

	lara.left_arm.frame_number = ani;
	set_flare_arm(ani);
}

void undraw_flare() {
	int16_t ani, ani2;

	ani = lara.left_arm.frame_number;
	ani2 = lara.flare_frame;
	lara.flare_control_left = 1;

	if (lara_item->goal_anim_state == AS_STOP && lara.vehicle == NO_ITEM) {
		if (lara_item->anim_number == ANIM_BREATH) {
			lara_item->anim_number = ANIM_THROWFLARE;
			ani2 = ani + anims[ANIM_THROWFLARE].frame_base;
			lara.flare_frame = ani2;
			lara_item->frame_number = ani2;
		}

		if (lara_item->anim_number == ANIM_THROWFLARE) {
			lara.flare_control_left = 0;

			if (ani2 >= anims[ANIM_THROWFLARE].frame_base + 31) {
				lara.request_gun_type = lara.last_gun_type;
				lara.gun_type = lara.last_gun_type;
				lara.gun_status = LG_NO_ARMS;
				InitialiseNewWeapon();
				lara.target = 0;
				lara.right_arm.lock = 0;
				lara.left_arm.lock = 0;
				lara_item->anim_number = ANIM_STOP;
				lara_item->frame_number = anims[ANIM_STOP].frame_base;
				lara_item->current_anim_state = AS_STOP;
				lara_item->goal_anim_state = AS_STOP;
				lara.flare_frame = anims[ANIM_STOP].frame_base;
				return;
			}

			ani2++;
			lara.flare_frame = ani2;
		}
	} else if (lara_item->current_anim_state == AS_STOP && lara.vehicle == NO_ITEM) {
		lara_item->anim_number = ANIM_STOP;
		lara_item->frame_number = anims[ANIM_STOP].frame_base;
	}

	if (ani >= 33 && ani < 72)
		ani = 1;

	if (!ani)
		ani = 1;
	else if (ani >= 72 && ani < 95) {
		ani++;

		if (ani == 94)
			ani = 1;
	} else if (ani >= 1 && ani < 33) {
		ani++;

		if (ani == 21) {
			CreateFlare(FLARE_ITEM, 1);
			undraw_flare_meshes();
			lara.flare_age = 0;
		} else if (ani == 33) {
			ani = 0;
			lara.gun_type = lara.last_gun_type;
			lara.request_gun_type = lara.last_gun_type;
			lara.gun_status = LG_NO_ARMS;;
			InitialiseNewWeapon();
			lara.target = 0;
			lara.left_arm.lock = 0;
			lara.right_arm.lock = 0;
			lara.flare_control_left = 0;
			lara.flare_frame = 0;
		}
	} else if (ani >= 95 && ani < 110) {
		ani++;

		if (ani == 110)
			ani = 1;
	}

	if (ani >= 1 && ani < 21)
		DoFlareInHand(lara.flare_age);

	lara.left_arm.frame_number = ani;
	set_flare_arm(lara.left_arm.frame_number);
}

void FlareControl(int16_t item_number) {
	ITEM_INFO* flare;
	int32_t x, y, z, xv, yv, zv, flare_age;

	flare = &items[item_number];

	// T4Plus - swamp
	if (T4PlusIsRoomSwamp(&room[flare->room_number])) {
		KillItem(item_number);
		return;
	}

	if (flare->fallspeed) {
		flare->pos.x_rot += DEGREES_TO_ROTATION(3);
		flare->pos.z_rot += DEGREES_TO_ROTATION(5);
	} else {
		flare->pos.x_rot = 0;
		flare->pos.z_rot = 0;
	}

	x = flare->pos.x_pos;
	y = flare->pos.y_pos;
	z = flare->pos.z_pos;
	xv = flare->speed * phd_sin(flare->pos.y_rot) >> W2V_SHIFT;
	zv = flare->speed * phd_cos(flare->pos.y_rot) >> W2V_SHIFT;
	flare->pos.x_pos += xv;
	flare->pos.z_pos += zv;

	if (room[flare->room_number].flags & ROOM_UNDERWATER) {
		flare->fallspeed += (5 - flare->fallspeed) >> 1;
		flare->speed += (5 - flare->speed) >> 1;
	} else
		flare->fallspeed += 6;

	yv = flare->fallspeed;
	flare->pos.y_pos += yv;
	DoProperDetection(item_number, x, y, z, xv, yv, zv);
	flare_age = int32_t(size_t(flare->data) & 0x7FFF);

	if (flare_age >= 900) {
		if (!flare->fallspeed && !flare->speed) {
			KillItem(item_number);
			return;
		}
	} else
		flare_age++;

	if (DoFlareLight((PHD_VECTOR*)&flare->pos, flare_age)) {
		for (int i = 0; i < t4p_mirror_count; i++) {
			if (flare->room_number == t4p_mirror_info[i].mirror_room) {
				flare->pos = T4PMirrorUnrotated3DPosOnPlane(&t4p_mirror_info[i], flare->pos);
				DoFlareLight((PHD_VECTOR*)&flare->pos, flare_age);
				flare->pos = T4PMirrorUnrotated3DPosOnPlane(&t4p_mirror_info[i], flare->pos);
			}
		}

		flare_age |= 0x8000;
	}

	flare->data = (void*)(size_t(flare_age) & 0xffffffff);
}
