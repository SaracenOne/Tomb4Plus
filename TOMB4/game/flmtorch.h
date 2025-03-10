#pragma once
#include "../global/types.h"

void TriggerTorchFlame(int16_t item_number, int32_t node);
void FireCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void DoFlameTorch();
void GetFlameTorch();
void FlameTorchControl(int16_t item_number);
