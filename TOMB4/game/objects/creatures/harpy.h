#pragma once
#include "../../../global/types.h"

void TriggerHarpyMissileFlame(int16_t fx_number, int32_t xv, int32_t yv, int32_t zv);
void TriggerHarpyMissile(PHD_3DPOS* pos, int16_t room_number, int16_t mesh);
void TriggerHarpySparks(int32_t x, int32_t y, int32_t z, int16_t xv, int16_t yv, int16_t zv);
void TriggerHarpyFlame(int16_t item_number, uint8_t NodeNumber, int16_t size);
void DoHarpyEffects(ITEM_INFO* item, int16_t item_number);
void InitialiseHarpy(int16_t item_number);
void HarpyControl(int16_t item_number);
