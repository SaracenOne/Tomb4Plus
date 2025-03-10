#pragma once
#include "../global/types.h"

int32_t GetSpheres(ITEM_INFO* item, SPHERE* ptr, int32_t WorldSpace);
int32_t TestCollision(ITEM_INFO* item, ITEM_INFO* l);
void InitInterpolate2(int32_t frac, int32_t rate);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* pos, int32_t joint);

extern SPHERE Slist[34];
extern int8_t GotLaraSpheres;
