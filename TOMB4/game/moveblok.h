#pragma once
#include "../global/types.h"

int32_t GetMoveableBlockHeight(int16_t item_number);
void InitialiseMovingBlock(int16_t item_number);
void MovableBlock(int16_t item_number);
void MovableBlockCollision(int16_t item_number, ITEM_INFO* laraitem, COLL_INFO* coll);
void InitialisePlanetEffect(int16_t item_number);
void ControlPlanetEffect(int16_t item_number);
void DrawPlanetEffect(ITEM_INFO* item);
