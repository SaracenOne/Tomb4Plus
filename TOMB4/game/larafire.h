#pragma once
#include "../global/types.h"

#define TARGET_LIST_SIZE (MAXIMUM_BADDIES + 3)

void InitialiseNewWeapon();
void LaraTargetInfo(WEAPON_INFO* winfo);
int16_t* get_current_ammo_pointer(int32_t weapon_type);
int32_t FireWeapon(int32_t weapon_type, ITEM_INFO* target, ITEM_INFO* src, int16_t* angles);
void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm);
void LaraGetNewTarget(WEAPON_INFO* winfo);
void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitpos, int32_t damage, int32_t grenade);
int32_t WeaponObject(int32_t weapon_type);
int32_t WeaponObjectMesh(int32_t weapon_type);
void DoProperDetection(int16_t item_number, int32_t x, int32_t y, int32_t z, int32_t xv, int32_t yv, int32_t zv);
void LaraGun();

extern WEAPON_INFO weapons[9];
