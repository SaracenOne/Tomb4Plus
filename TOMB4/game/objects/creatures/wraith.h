#pragma once
#include "../../../global/types.h"

#define MAX_WRAITH_DATA 8

void InitialiseWraith(int16_t item_number);
void TriggerWraithFlame(int32_t x, int32_t y, int32_t z, int16_t xv, int16_t yv, int16_t zv, int32_t objnum);
void TriggerWraithEffect(int32_t x, int32_t y, int32_t z, int16_t vel, int32_t objnum);
void WraithControl(int16_t item_number);
