#pragma once
#include "../global/types.h"

void InitialiseMapper(int16_t item_number);
void InitialiseLightningConductor(int16_t item_number);
void InitialiseDoor(int16_t item_number);
void InitialiseTrapDoor(int16_t item_number);
void InitialiseFallingBlock2(int16_t item_number);
void InitialiseFlameEmitter(int16_t item_number);
void InitialiseFlameEmitter2(int16_t item_number);
void InitialiseFlameEmitter3(int16_t item_number);
void InitialiseJobySpike(int16_t item_number);
void InitialiseTwoBlockPlatform(int16_t item_number);
void InitialiseSlicerDicer(int16_t item_number);
void InitialiseScaledSpike(int16_t item_number);
void InitialiseRaisingBlock(int16_t item_number);
void InitialiseBurningFloor(int16_t item_number);
void InitialiseSethBlade(int16_t item_number);
void InitialiseObelisk(int16_t item_number);
void InitialiseMineHelicopter(int16_t item_number);
void InitialiseSmashObject(int16_t item_number);
void InitialiseStatuePlinth(int16_t item_number);
void InitialiseSmokeEmitter(int16_t item_number);
void InitialisePulley(int16_t item_number);
void InitialisePickUp(int16_t item_number);
void CreateRope(ROPE_STRUCT* rope, PHD_VECTOR* pos, PHD_VECTOR* dir, int32_t slength, ITEM_INFO* item);
void InitialiseRope(int16_t item_number);
void init_all_ropes();
void InitialiseEffects();
