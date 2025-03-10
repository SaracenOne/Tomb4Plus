#pragma once
#include "../../../global/types.h"

void TriggerCrocgodMissile(PHD_3DPOS* pos, int16_t room_number, int16_t num);
void TriggerCrocgodMissileFlame(int16_t fx_number, int32_t xv, int32_t yv, int32_t zv);
void InitialiseCrocgod(int16_t item_number);
void CrocgodControl(int16_t item_number);
