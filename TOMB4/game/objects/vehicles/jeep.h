#pragma once
#include "../../../global/types.h"

void InitialiseJeep(int16_t item_number);
void DrawJeepExtras(ITEM_INFO* item);
void JeepExplode(ITEM_INFO* item);
void JeepCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
int32_t GetCollisionAnim(ITEM_INFO* item, PHD_VECTOR* pos, BIKEINFO* vehicle);
int32_t DoShift(ITEM_INFO* item, PHD_VECTOR* newPos, PHD_VECTOR* oldPos);
void JeepBaddieCollision(ITEM_INFO* item);
void JeepCollideStaticObjects(int32_t x, int32_t y, int32_t z, int16_t room_number, int32_t height);
int32_t JeepDynamics(ITEM_INFO* item);
void JeepControl(int16_t item_number);
void JeepStart(ITEM_INFO* item, ITEM_INFO* l);