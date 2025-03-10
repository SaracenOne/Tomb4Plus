#pragma once
#include "../../../global/types.h"

void InitialiseInjuredSas(int16_t item_number);
void InjuredSasControl(int16_t item_number);
void DragSASCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void InitialiseSas(int16_t item_number);
void SasControl(int16_t item_number);
