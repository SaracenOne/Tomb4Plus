#pragma once
#include "../global/types.h"

#ifdef LEVEL_EDITOR
#define MAXIMUM_BADDIES 64
#else
#define MAXIMUM_BADDIES 5
#endif

void InitialiseLOTarray(int32_t allocmem);
void DisableBaddieAI(int16_t item_number);
void ClearLOT(LOT_INFO* lot);
void CreateZone(ITEM_INFO* item);
void InitialiseSlot(int16_t item_number, int32_t slot);
int32_t EnableBaddieAI(int16_t item_number, int32_t Always);

extern CREATURE_INFO* baddie_slots;
