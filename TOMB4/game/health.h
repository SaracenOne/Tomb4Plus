#pragma once
#include "../global/types.h"

int32_t FlashIt();
void DrawGameInfo(int32_t timed);
void DrawHealthBar(int32_t flash_state);
void DrawAirBar(int32_t flash_state);
void InitialisePickUpDisplay();
void DrawPickups();
void AddDisplayPickup(int16_t object_number);

#define MAX_PICKUP_DISPLAYABLE_COUNT 8

extern DISPLAYPU pickups[MAX_PICKUP_DISPLAYABLE_COUNT];
extern int32_t PickupX;
extern int16_t CurrentPickup;

extern int32_t health_bar_timer;
