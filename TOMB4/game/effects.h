#pragma once
#include "../global/types.h"

void SetFog(ITEM_INFO* item);
void finish_level_effect(ITEM_INFO* item);
void turn180_effect(ITEM_INFO* item);
void floor_shake_effect(ITEM_INFO* item);
void SoundFlipEffect(ITEM_INFO* item);
void RubbleFX(ITEM_INFO* item);
void PoseidonSFX(ITEM_INFO* item);
void ActivateCamera(ITEM_INFO* item);
void ActivateKey(ITEM_INFO* item);
void SwapCrowbar(ITEM_INFO* item);
void ExplosionFX(ITEM_INFO* item);
void LaraLocation(ITEM_INFO* item);
void LaraLocationPad(ITEM_INFO* item);
void GhostTrap(ITEM_INFO* item);
void KillActiveBaddies(ITEM_INFO* item);
void lara_hands_free(ITEM_INFO* item);
void draw_right_gun(ITEM_INFO* item);
void draw_left_gun(ITEM_INFO* item);
void shoot_right_gun(ITEM_INFO* item);
void shoot_left_gun(ITEM_INFO* item);
void swap_meshes_with_meshswap1(ITEM_INFO* item);
void swap_meshes_with_meshswap2(ITEM_INFO* item);
void swap_meshes_with_meshswap3(ITEM_INFO* item);
void invisibility_on(ITEM_INFO* item);
void invisibility_off(ITEM_INFO* item);
void reset_hair(ITEM_INFO* item);
void ClearScarabsPatch(ITEM_INFO* item);
void MeshSwapToPour(ITEM_INFO* item);
void MeshSwapFromPour(ITEM_INFO* item);
void void_effect(ITEM_INFO* item);
void WaterFall(int16_t item_number);
void WadeSplash(ITEM_INFO* item, int32_t water, int32_t depth);
void Splash(ITEM_INFO* item);
int16_t DoBloodSplat(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t ang, int16_t room_number);
void DoLotsOfBlood(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t ang, int16_t room_number, int32_t num);
void Richochet(GAME_VECTOR* pos);
void SoundEffects();
int32_t ItemNearLara(PHD_3DPOS* pos, int32_t rad);

#define FOG_TABLE_SIZE 28

extern FX_INFO* effects;
extern OBJECT_VECTOR* sound_effects;
extern int32_t GlobalFogOff;
extern int32_t number_sound_effects;
extern int32_t FogTableColor[FOG_TABLE_SIZE];
extern void(*effect_routines[])(ITEM_INFO* item);

// TRLE
extern void LaraBreath(ITEM_INFO* item);
