#pragma once
#include "../../../global/types.h"

#define MAX_LOCUSTS 64

int32_t GetFreeLocust();
void TriggerLocust(ITEM_INFO* item);
void InitialiseLocustEmitter(int16_t item_number);
void ControlLocustEmitter(int16_t item_number);
void DrawLocusts();
void UpdateLocusts();

extern LOCUST_STRUCT Locusts[MAX_LOCUSTS];
