#pragma once
#include "../global/types.h"

void DoGrenadeDamageOnBaddie(ITEM_INFO* baddie, ITEM_INFO* item);
void FireCrossbow(PHD_3DPOS* pos);
void draw_shotgun_meshes(int32_t weapon_type);
void undraw_shotgun_meshes(int32_t weapon_type);
void ready_shotgun(int32_t weapon_type);
void FireShotgun();
void FireGrenade();
void AnimateShotgun(int32_t weapon_type);
void RifleHandler(int32_t weapon_type);
void CrossbowHitSwitchType78(ITEM_INFO* item, ITEM_INFO* target, int32_t MustHitLastNode);
void TriggerUnderwaterExplosion(ITEM_INFO* item, int32_t vehicle);
void draw_shotgun(int32_t weapon_type);
void undraw_shotgun(int32_t weapon_type);
void ControlCrossbow(int16_t item_number);
void ControlGrenade(int16_t item_number);
