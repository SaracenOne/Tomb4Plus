#pragma once

#include "../../../global/types.h"

void InitialiseGuardian(int16_t item_number);
void TriggerGuardianSparks(GAME_VECTOR* pos, int32_t size, int32_t rgb, int32_t power);
void TriggerBaseLightning(ITEM_INFO* item);
void GuardianControl(int16_t item_number);


struct GUARDIAN_TARGET {
	int32_t x;
	int32_t y;
	int32_t z;
	LIGHTNING_STRUCT* elptr[2];
	LIGHTNING_STRUCT* blptr[4];
	int8_t ricochet[2];
	int8_t TrackSpeed;
	int8_t TrackLara;
	int16_t Xdiff;
	int16_t Ydiff;
};