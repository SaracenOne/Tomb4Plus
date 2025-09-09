#include "../tomb4/pch.h"
#include "effects.h"
#include "../specific/polyinsert.h"
#include "../specific/function_table.h"
#include "sound.h"
#include "deltapak.h"
#include "objects.h"
#include "items.h"
#include "lot.h"
#include "hair.h"
#include "objects/effects/scarabs.h"
#include "tomb4fx.h"
#include "footprnt.h"
#include "effect2.h"
#include "control.h"
#include "draw.h"
#include "lara_states.h"
#include "../specific/function_stubs.h"
#include "../specific/3dmath.h"
#include "../specific/dxsound.h"
#include "../specific/bgfx.h"
#include "camera.h"
#include "lara.h"
#include "savegame.h"
#include "pickup.h"
#include "gameflow.h"

#include "delstuff.h"
#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_weather.h"
#include "trep/furr.h"
#include "../tomb4/tomb4plus/t4plus_objects.h"
#include "../tomb4/tomb4plus/t4plus_environment.h"
#include "trng/trng.h"
#include "trng/trng_condition.h"
#include "collide.h"

FX_INFO* effects;
OBJECT_VECTOR* sound_effects;
int32_t GlobalFogOff = 0;
int32_t number_sound_effects;

int32_t FogTableColor[FOG_TABLE_SIZE] = {
	0,
	RGBONLY(245,200,60),
	RGBONLY(120,196,112),
	RGBONLY(202,204,230),
	RGBONLY(128,64,0),
	RGBONLY(64,64,64),
	RGBONLY(243,232,236),
	RGBONLY(0,64,192),
	RGBONLY(0,128,0),
	RGBONLY(150,172,157),
	RGBONLY(128,128,128),
	RGBONLY(204,163,123),
	RGBONLY(177,162,140),
	RGBONLY(0,223,191),
	RGBONLY(111,255,223),
	RGBONLY(244,216,152),
	RGBONLY(248,192,60),
	RGBONLY(252,0,0),
	RGBONLY(198,95,87),
	RGBONLY(226,151,118),
	RGBONLY(248,235,206),
	RGBONLY(0,30,16),
	RGBONLY(250,222,167),
	RGBONLY(218,175,117),
	RGBONLY(225,191,78),
	RGBONLY(77,140,141),
	RGBONLY(4,181,154),
	RGBONLY(255,174,0)
};

// Base TR4 has 47 fileeffects, NGLE and TREP extends it
void(*effect_routines[])(ITEM_INFO* item) = {
	turn180_effect,
	floor_shake_effect,
	PoseidonSFX,
	LaraBubbles,
	finish_level_effect,
	ActivateCamera,
	ActivateKey,
	RubbleFX,
	SwapCrowbar,
	void_effect,
	SoundFlipEffect,
	ExplosionFX,
	lara_hands_free,
	void_effect,
	draw_right_gun,
	draw_left_gun,
	shoot_right_gun,
	shoot_left_gun,
	swap_meshes_with_meshswap1,
	swap_meshes_with_meshswap2,
	swap_meshes_with_meshswap3,
	invisibility_on,
	invisibility_off,
	void_effect,
	void_effect,
	void_effect,
	reset_hair,
	void_effect,
	SetFog,
	GhostTrap,
	LaraLocation,
	ClearScarabsPatch,
	AddFootPrint,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	MeshSwapToPour,
	MeshSwapFromPour,
	LaraLocationPad,
	KillActiveBaddies,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
};

void SetFog(ITEM_INFO* item) {
	GlobalFogOff = 0;

	if (TriggerTimer == 100)
		GlobalFogOff = 1;
	else {
		SetVolumetricFogColor(CLRR(FogTableColor[TriggerTimer]), CLRG(FogTableColor[TriggerTimer]), CLRB(FogTableColor[TriggerTimer]));
		savegame.fog_colour.r = gfVolumetricFog.r;
		savegame.fog_colour.g = gfVolumetricFog.g;
		savegame.fog_colour.b = gfVolumetricFog.b;
	}

	flipeffect = -1;
}

void finish_level_effect(ITEM_INFO* item) {
	gfLevelComplete = gfCurrentLevel + 1;
}

void turn180_effect(ITEM_INFO* item) {
	// TRLE - safety check
	if (!item)
		return;

	item->pos.y_rot += 0x8000;
	item->pos.x_rot = -item->pos.x_rot;
}

void floor_shake_effect(ITEM_INFO* item) {
	// TRLE - safety check
	if (!item)
		return;

	int32_t dx, dy, dz, dist;

	dx = item->pos.x_pos - camera.pos.x;
	dy = item->pos.y_pos - camera.pos.y;
	dz = item->pos.z_pos - camera.pos.z;

	if (abs(dx) < 0x4000 && abs(dy) < 0x4000 && abs(dz) < 0x4000) {
		dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
		camera.bounce = -66 * (0x100000 - dist / 256) / 0x100000;
	}
}

void SoundFlipEffect(ITEM_INFO* item) {
	SoundEffect(TriggerTimer, 0, SFX_DEFAULT);
	flipeffect = -1;
}

void RubbleFX(ITEM_INFO* item) {
	ITEM_INFO* eq;

	eq = find_an_item_with_object_type(EARTHQUAKE);

	if (eq) {
		AddActiveItem(int16_t(eq - items));
		eq->status = ITEM_ACTIVE;
		eq->flags |= IFL_CODEBITS;
	} else
		camera.bounce = -150;

	flipeffect = -1;
}

void PoseidonSFX(ITEM_INFO* item) {
	SoundEffect(SFX_WATER_FLUSHES, 0, SFX_DEFAULT);
	flipeffect = -1;
}

void ActivateCamera(ITEM_INFO* item) {
	KeyTriggerActive = 2;
}

void ActivateKey(ITEM_INFO* item) {
	KeyTriggerActive = 1;
}

void SwapCrowbar(ITEM_INFO* item) {
	int16_t* tmp;

	tmp = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + 2 * LM_RHAND];

	if (lara.mesh_ptrs[LM_RHAND] == tmp)
		lara.mesh_ptrs[LM_RHAND] = meshes[objects[CROWBAR_ANIM].mesh_index + (2 * LM_RHAND)];
	else
		lara.mesh_ptrs[LM_RHAND] = tmp;
}

void ExplosionFX(ITEM_INFO* item) {
	SoundEffect(SFX_EXPLOSION1, 0, SFX_DEFAULT);
	camera.bounce = -75;
	flipeffect = -1;
}

void LaraLocation(ITEM_INFO* item) {
	lara.location = TriggerTimer;

	if (TriggerTimer > lara.highest_location)
		lara.highest_location = TriggerTimer;

	flipeffect = -1;
}

void LaraLocationPad(ITEM_INFO* item) {
	flipeffect = -1;
	lara.locationPad = TriggerTimer;
	lara.location = TriggerTimer;
}

void GhostTrap(ITEM_INFO* item) {
	ITEM_INFO* wraith;
	int16_t nex;

	nex = next_item_active;

	if (next_item_active != NO_ITEM) {
		while (nex != NO_ITEM) {
			wraith = &items[nex];

			if (wraith->object_number == WRAITH3 && !wraith->hit_points) {
				wraith->hit_points = int16_t(item - items);
				break;
			}

			nex = wraith->next_active;
		}
	}

	flipeffect = -1;
}

void KillActiveBaddies(ITEM_INFO* item) {
	ITEM_INFO* target_item;
	int16_t item_num;

	for (item_num = next_item_active; item_num != NO_ITEM; item_num = target_item->next_active) {
		target_item = &items[item_num];

		if (objects[target_item->object_number].intelligent) {
			target_item->status = ITEM_INVISIBLE;

			if (item != ((void*)0xABCDEF)) {
				RemoveActiveItem(item_num);
				DisableBaddieAI(item_num);
				target_item->flags |= IFL_INVISIBLE;
			}
		}
	}

	flipeffect = -1;
}

void lara_hands_free(ITEM_INFO* item) {
	lara.gun_status = LG_NO_ARMS;

	// T4Plus: Bug fix for weapons getting stuck in perpetual animation loop.
	if (get_game_mod_global_info()->fix_lara_hands_free_flipeffect_bugs) {
		if (lara.gun_type != WEAPON_FLARE) {
			lara.gun_type = WEAPON_NONE;
		}
		if (lara.request_gun_type != WEAPON_FLARE) {
			lara.request_gun_type = WEAPON_NONE;
		}

		lara.left_arm.frame_number = 0;
		lara.right_arm.frame_number = 0;

		if (lara.gun_type != WEAPON_FLARE) {
			lara.mesh_ptrs[LM_LHAND] = meshes[objects[0].mesh_index + 2 * LM_LHAND];
			lara.mesh_ptrs[LM_RHAND] = meshes[objects[0].mesh_index + 2 * LM_RHAND];
		}
		lara.left_arm.frame_number = 0;
		lara.right_arm.frame_number = 0;
		lara.target = 0;
		lara.right_arm.lock = 0;
		lara.left_arm.lock = 0;

		if (lara.weapon_item != NO_ITEM) {
			KillItem(lara.weapon_item);
			lara.weapon_item = NO_ITEM;
		}
	}
}

void draw_right_gun(ITEM_INFO* item) {
	int16_t* tmp;

	tmp = lara.mesh_ptrs[LM_RTHIGH];
	lara.mesh_ptrs[LM_RTHIGH] = meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_RTHIGH * 2];
	meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_RTHIGH * 2] = tmp;

	tmp = lara.mesh_ptrs[LM_RHAND];
	lara.mesh_ptrs[LM_RHAND] = meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_RHAND * 2];
	meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_RHAND * 2] = tmp;
}

void draw_left_gun(ITEM_INFO* item) {
	int16_t* tmp;

	tmp = lara.mesh_ptrs[LM_LTHIGH];
	lara.mesh_ptrs[LM_LTHIGH] = meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_LTHIGH * 2];
	meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_LTHIGH * 2] = tmp;

	tmp = lara.mesh_ptrs[LM_LHAND];
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_LHAND * 2];
	meshes[objects[T4PlusGetPistolsAnimSlotID()].mesh_index + LM_LHAND * 2] = tmp;
}

void shoot_right_gun(ITEM_INFO* item) {
	lara.right_arm.flash_gun = 3;
}

void shoot_left_gun(ITEM_INFO* item) {
	lara.left_arm.flash_gun = 3;
}

void swap_meshes_with_meshswap1(ITEM_INFO* item) {
	// TRLE - safety check
	if (!item)
		return;

	OBJECT_INFO* obj;
	int16_t* tmp;

	obj = &objects[item->object_number];

	for (int32_t i = 0; i < obj->nmeshes; i++) {
		// Changed this to only use single index offsets.
		// Seems to be more compatible with TRLE, but may
		// require more extensive testing.
		tmp = meshes[obj->mesh_index + i];
		meshes[obj->mesh_index + i] = meshes[objects[T4PlusGetMeshSwap1SlotID()].mesh_index + i];
		meshes[objects[T4PlusGetMeshSwap1SlotID()].mesh_index + i] = tmp;
	}
}

void swap_meshes_with_meshswap2(ITEM_INFO* item) {
	// TRLE - safety check
	if (!item)
		return;

	OBJECT_INFO* obj;
	int16_t* tmp;

	obj = &objects[item->object_number];

	for (int32_t i = 0; i < obj->nmeshes; i++) {
		// Changed this to only use single index offsets.
		// Seems to be more compatible with TRLE, but may
		// require more extensive testing.
		tmp = meshes[obj->mesh_index + i];
		meshes[obj->mesh_index + i] = meshes[objects[T4PlusGetMeshSwap2SlotID()].mesh_index + i];
		meshes[objects[T4PlusGetMeshSwap2SlotID()].mesh_index + i] = tmp;
	}
}

void swap_meshes_with_meshswap3(ITEM_INFO* item) {
	// TRLE
	if (!item)
		return;

	OBJECT_INFO* obj;
	int16_t* tmp;

	obj = &objects[item->object_number];

	for (int32_t i = 0; i < obj->nmeshes; i++) {
		tmp = meshes[obj->mesh_index + i * 2];
		meshes[obj->mesh_index + i] = meshes[objects[T4PlusGetMeshSwap3SlotID()].mesh_index + i * 2];

		if (item == lara_item)
			lara.mesh_ptrs[i] = meshes[objects[T4PlusGetMeshSwap3SlotID()].mesh_index + i * 2];

		meshes[objects[T4PlusGetMeshSwap3SlotID()].mesh_index + i * 2] = tmp;
	}
}

void invisibility_on(ITEM_INFO* item) {
	// TRLE
	if (!item)
		return;

	item->status = ITEM_INVISIBLE;
}

void invisibility_off(ITEM_INFO* item) {
	if (game_mod_config.global_info.tomo_enable_weather_flipeffect) {
		switch (TriggerTimer) {
			case 1: // Rain
				t4p_rain_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
				t4p_snow_type = T4P_WEATHER_DISABLED;
				break;
			case 2: // Snow
				t4p_rain_type = T4P_WEATHER_DISABLED;
				t4p_snow_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
				break;
			case 3: // No weather
				t4p_rain_type = T4P_WEATHER_DISABLED;
				t4p_snow_type = T4P_WEATHER_DISABLED;
				break;
		}
	} else {
		// TRLE
		if (!item)
			return;

		item->status = ITEM_ACTIVE;
	}
}

void reset_hair(ITEM_INFO* item) {
	InitialiseHair();
}

void ClearScarabsPatch(ITEM_INFO* item) {
	ClearScarabs();
}

void MeshSwapToPour(ITEM_INFO* item) {
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[item->item_flags[2]].mesh_index + LM_LHAND * 2];
}

void MeshSwapFromPour(ITEM_INFO* item) {
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[T4PlusGetLaraSkinSlotID()].mesh_index + LM_LHAND * 2];
}

void void_effect(ITEM_INFO* item) {
	if (flipeffect >= FIRST_FURR_FLIPEFFECT) {
		furr_execute_furr_flipeffect(flipeffect);
	}
}

void WaterFall(int16_t item_number) {
	ITEM_INFO* item;
	int32_t dx, dy, dz;

	item = &items[item_number];
	dx = item->pos.x_pos - lara_item->pos.x_pos;
	dy = item->pos.y_pos - lara_item->pos.y_pos;
	dz = item->pos.z_pos - lara_item->pos.z_pos;

	if (dx >= -0x4000 && dx <= 0x4000 && dz >= -0x4000 && dz <= 0x4000 && dy >= -0x4000 && dy <= 0x4000) {
		//empty func call here

		if (!(wibble & 0xC)) {
			dx = (136 * phd_sin(item->pos.y_rot)) >> 12;
			dz = (136 * phd_cos(item->pos.y_rot)) >> 12;
			TriggerWaterfallMist(item->pos.x_pos + dx, item->pos.y_pos, item->pos.z_pos + dz, item->pos.y_rot >> 4);
		}

		SoundEffect(SFX_WATERFALL_LOOP, &item->pos, 0);
	}
}

void WadeSplash(ITEM_INFO* item, int32_t water, int32_t depth) {
	int16_t* bounds;
	int16_t room_number;

	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (!(room[room_number].flags & ROOM_UNDERWATER))
		return;

	bounds = GetBestFrame(item);

	if (item->pos.y_pos + bounds[2] > water || item->pos.y_pos + bounds[3] < water)
		return;

	if (item->fallspeed > 0 && depth < 474 && !SplashCount) {
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = water;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerRad = 16;
		splash_setup.InnerSize = 12;
		splash_setup.InnerRadVel = 160;
		splash_setup.InnerYVel = -72 * item->fallspeed;
		splash_setup.pad1 = 24;
		splash_setup.MiddleRad = 24;
		splash_setup.MiddleSize = 224;
		splash_setup.MiddleRadVel = -36 * item->fallspeed;
		splash_setup.MiddleYVel = 32;
		splash_setup.pad2 = 32;
		splash_setup.OuterRad = 272;
		SetupSplash(&splash_setup);
		SplashCount = 16;
	} else if (!(wibble & 0xF) && (!(GetRandomControl() & 0xF) || item->current_anim_state != AS_STOP)) {
		if (item->current_anim_state == AS_STOP)
			SetupRipple(item->pos.x_pos, water, item->pos.z_pos, (GetRandomControl() & 0xF) + 112, 16);
		else
			SetupRipple(item->pos.x_pos, water, item->pos.z_pos, (GetRandomControl() & 0xF) + 112, 18);
	}
}

void Splash(ITEM_INFO* item) {
	int16_t room_number;

	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room[room_number].flags & ROOM_UNDERWATER) {
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerRad = 32;
		splash_setup.InnerSize = 8;
		splash_setup.InnerRadVel = 320;
		splash_setup.InnerYVel = -40 * item->fallspeed;
		splash_setup.pad1 = 48;
		splash_setup.MiddleRad = 32;
		splash_setup.MiddleSize = 480;
		splash_setup.MiddleRadVel = -20 * item->fallspeed;
		splash_setup.MiddleYVel = 32;
		splash_setup.pad2 = 128;
		splash_setup.OuterRad = 544;
		SetupSplash(&splash_setup);
	}
}

int16_t DoBloodSplat(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t ang, int16_t room_number) {
	if (room[room_number].flags & ROOM_UNDERWATER)
		TriggerUnderwaterBlood(x, y, z, speed);
	else
		TriggerBlood(x, y, z, ang >> 4, speed);

	return -1;
}

void DoLotsOfBlood(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t ang, int16_t room_number, int32_t num) {
	int32_t bx, by, bz;

	for (; num > 0; num--) {
		bx = x - (GetRandomControl() << 9) / 0x8000 + CLICK_SIZE;
		by = y - (GetRandomControl() << 9) / 0x8000 + CLICK_SIZE;
		bz = z - (GetRandomControl() << 9) / 0x8000 + CLICK_SIZE;
		DoBloodSplat(bx, by, bz, speed, ang, room_number);
	}
}

void Richochet(GAME_VECTOR* pos) {
	TriggerRicochetSpark(pos, mGetAngle(pos->z, pos->x, lara_item->pos.z_pos, lara_item->pos.x_pos) >> 4, 3, 0);
	SoundEffect(SFX_LARA_RICOCHET, (PHD_3DPOS*)pos, SFX_DEFAULT);
}

void SoundEffects() {
	OBJECT_VECTOR* sfx;
	SoundSlot* slot;

	for (int32_t i = 0; i < number_sound_effects; i++) {
		sfx = &sound_effects[i];

		if (flip_status) {
			if (sfx->flags & 0x40)
				SoundEffect(sfx->data, (PHD_3DPOS*)sfx, 0);
		} else if (sfx->flags & 0x80)
			SoundEffect(sfx->data, (PHD_3DPOS*)sfx, 0);
	}

	if (NGIsUsingNGNewTriggers()) {
		NGProcessConditions();
	} else {
		if (flipeffect != -1)
			effect_routines[flipeffect](0);
	}

	if (!sound_active)
		return;

	for (int32_t i = 0; i < MAX_VOICES; i++) {
		slot = &LaSlot[i];

		if (slot->nSampleInfo < 0)
			continue;

		if ((sample_infos[slot->nSampleInfo].flags & 3) != 3) {
			if (!S_SoundSampleIsPlayingOnChannel(i))
				slot->nSampleInfo = -1;
			else {
				GetPanVolume(slot);
				S_SoundSetPanAndVolume(i, (int16_t)slot->nPan, (uint16_t)slot->nVolume);
			}
		} else {
			if (!slot->nVolume) {
				S_SoundStopSample(i);
				slot->nSampleInfo = -1;
			} else {
				S_SoundSetPanAndVolume(i, (int16_t)slot->nPan, (uint16_t)slot->nVolume);
				S_SoundSetPitch(i, slot->nPitch);
				slot->nVolume = 0;
			}
		}
	}
}

int32_t ItemNearLara(PHD_3DPOS* pos, int32_t rad) {
	int16_t* bounds;
	int32_t dx, dy, dz;

	dx = pos->x_pos - lara_item->pos.x_pos;
	dy = pos->y_pos - lara_item->pos.y_pos;
	dz = pos->z_pos - lara_item->pos.z_pos;

	if (dx >= -rad && dx <= rad && dz >= -rad && dz <= rad && dy >= -COLLISION_RANGE && dy <= COLLISION_RANGE && SQUARE(dx) + SQUARE(dz) <= SQUARE(rad)) {
		bounds = GetBoundsAccurate(lara_item);

		if (dy >= bounds[2] && dy <= bounds[3] + 100)
			return 1;
	}

	return 0;
}

// TRLE

void LaraBreath(ITEM_INFO* item) {
	PHD_VECTOR p;
	PHD_VECTOR v;

	if (lara.water_status == 1 || lara_item->hit_points < 0 || !(T4PlusDoesRoomCauseColdBreath(&room[lara_item->room_number])))
		return;

	if (lara_item->current_anim_state == AS_STOP) {
		if (lara_item->frame_number < anims[LARA_ANIM_BREATH].frame_base + 30)
			return;
	} else if (lara_item->current_anim_state == AS_SURFSWIM) {
		if (lara_item->frame_number < anims[LARA_ANIM_SURF].frame_base + 21)
			return;
	} else if (lara_item->current_anim_state == AS_DUCK) {
		if (lara_item->frame_number < anims[LARA_ANIM_DUCKBREATHE].frame_base + 32)
			return;
	} else if (lara_item->current_anim_state == AS_ALL4S) {
		if (lara_item->frame_number < anims[LARA_ANIM_ALL4S].frame_base + 28)
			return;
	} else if (wibble < 80 || wibble > 192)
		return;

	p.x = 0;
	p.y = -4;
	p.z = 64;
	GetLaraJointPos(&p, LMX_HEAD);

	v.x = (GetRandomControl() & 7) - 4;
	v.y = (GetRandomControl() & 7) - 8;
	v.z = (GetRandomControl() & 0x7F) + 64;
	GetLaraJointPos(&v, LMX_HEAD);

	TriggerBreath(p.x, p.y, p.z, v.x - p.x, v.y - p.y, v.z - p.z);
}