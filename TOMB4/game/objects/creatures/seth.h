#pragma once
#include "../../../global/types.h"

void TriggerSethMissileFlame(int16_t fx_number, int32_t xv, int32_t yv, int32_t zv);
void TriggerSethMissile(PHD_3DPOS* pos, int16_t room_number, int16_t type);
void TriggerSethSparks(int32_t x, int32_t y, int32_t z, int16_t xv, int16_t yv, int16_t zv);
void TriggerSethFlame(int16_t item_number, uint8_t NodeNumber, int16_t size);
void DoSethEffects(int16_t item_number);
void InitialiseSeth(int16_t item_number);
void SethControl(int16_t item_number);
