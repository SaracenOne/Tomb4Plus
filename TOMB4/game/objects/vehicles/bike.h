#pragma once
#include "../../../global/types.h"

void InitialiseBike(int16_t item_number);
void DrawBikeExtras(ITEM_INFO* item);
void TriggerBikeBeam(ITEM_INFO* item);
int32_t GetOnBike(int16_t item_number, COLL_INFO* coll);
void DrawBikeBeam(ITEM_INFO* item);
void BikeExplode(ITEM_INFO* item);
void AnimateBike(ITEM_INFO* item, int32_t hitWall, int32_t killed);
void BikeStart(ITEM_INFO* item, ITEM_INFO* l);
int32_t TestHeight(ITEM_INFO* item, int32_t z, int32_t x, PHD_VECTOR* pos);
void BikeCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
int32_t BikeBaddieCollision(ITEM_INFO* bike);
void BikeCollideStaticObjects(int32_t x, int32_t y, int32_t z, int16_t room_number, int32_t height);
int32_t BikeDynamics(ITEM_INFO* item);
void BikeControl(int16_t item_number);
