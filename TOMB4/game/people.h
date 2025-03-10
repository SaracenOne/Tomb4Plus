#pragma once
#include "../global/types.h"

int16_t GunShot(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot, int16_t room_number);
int16_t GunHit(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot, int16_t room_number);
int16_t GunMiss(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot, int16_t room_number);
int32_t TargetVisible(ITEM_INFO* item, AI_INFO* info);
int32_t Targetable(ITEM_INFO* item, AI_INFO* info);
int32_t ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, int16_t extra_rotation, int32_t damage);
