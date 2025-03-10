#pragma once
#include "../../../global/types.h"

void SetCutSceneCamera(ITEM_INFO* item);
void ClearCutSceneCamera();
void GetAIEnemy(CREATURE_INFO* info, int32_t tfl);
void DoVonCroyCutscene(ITEM_INFO* item, CREATURE_INFO* info);
void InitialiseVoncroy(int16_t item_number);
void VoncroyRaceControl(int16_t item_number);
void VoncroyControl(int16_t item_number);

#define MAX_VONCROY_FLAGS 64

extern uint8_t VonCroyCutFlags[MAX_VONCROY_FLAGS];
extern int8_t bVoncroyCutScene;
