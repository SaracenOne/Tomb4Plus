#pragma once
#include "../../../global/types.h"

int32_t GetFreeScarab();
void ClearScarabs();
void TriggerScarab(int16_t item_number);
void UpdateScarabs();
void DrawScarabs();
void InitialiseScarabGenerator(int16_t item_number);

#define MAX_SCARABS 128

extern SCARAB_STRUCT Scarabs[MAX_SCARABS];
