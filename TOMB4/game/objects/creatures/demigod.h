#pragma once
#include "../../../global/types.h"

void TriggerDemigodMissile(PHD_3DPOS* pos, int16_t room_number, int16_t type);
void TriggerDemigodMissileFlame(int16_t fx_number, int32_t xv, int32_t yv, int32_t zv);
void TriggerHammerSmoke(int32_t x, int32_t y, int32_t z, int32_t num);
void DoDemigodEffects(int16_t item_number);
void InitialiseDemigod(int16_t item_number);
void DemigodControl(int16_t item_number);
