#pragma once
#include "../../../global/types.h"

void InitialiseBaboon(int16_t item_number);
void BaboonControl(int16_t item_number);
void FindCrowbarSwitch(ITEM_INFO* item, int16_t switch_index);
void ReTriggerBaboon(int16_t item_number);
void ExplodeBaboon(ITEM_INFO* item);
