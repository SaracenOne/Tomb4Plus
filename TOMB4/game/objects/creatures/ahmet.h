#pragma once
#include "../../../global/types.h"

void ScalesCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
int32_t ReTriggerAhmet(int16_t item_number);
void ScalesControl(int16_t item_number);
void ExplodeAhmet(ITEM_INFO* item);
void InitialiseAhmet(int16_t item_number);
void AhmetControl(int16_t item_number);
