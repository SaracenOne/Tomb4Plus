#pragma once
#include "../../../global/types.h"

void InitialiseDeathSlide(int16_t item_number);
void DeathSlideCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void ControlDeathSlide(int16_t item_number);
