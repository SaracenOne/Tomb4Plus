#pragma once
#include "../../../global/types.h"

void InitialiseHydra(int16_t item_number);
void HydraControl(int16_t item_number);
void TriggerHydraMissile(PHD_3DPOS* pos, int16_t room_number, int16_t num);
void TriggerHydraMissileFlame(PHD_VECTOR* pos, int32_t xv, int32_t yv, int32_t zv);
void TriggerHydraPowerupFlames(int16_t item_number, int32_t shade);
