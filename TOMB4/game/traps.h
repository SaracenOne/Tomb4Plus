#pragma once
#include "../global/types.h"

extern int16_t SPxzoffs[8];
extern int16_t SPyoffs[8];

void FlameEmitterControl(int16_t item_number);
void TwoBlockPlatformFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void TwoBlockPlatformCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void ControlTwoBlockPlatform(int16_t item_number);
void ControlJobySpike(int16_t item_number);
void DrawScaledSpike(ITEM_INFO* item);
void ControlSlicerDicer(int16_t item_number);
void ControlSprinkler(int16_t item_number);
void ControlMineHelicopter(int16_t item_number);
void MineCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void FallingSquishyBlockCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void ControlFallingSquishyBlock(int16_t item_number);
void ControlLRSquishyBlock(int16_t item_number);
void ControlSethBlade(int16_t item_number);
void ControlPlinthBlade(int16_t item_number);
void ControlMovingBlade(int16_t item_number);
void ControlCatwalkBlade(int16_t item_number);
void ControlBirdBlade(int16_t item_number);
void Control4xFloorRoofBlade(int16_t item_number);
void ControlSpikeball(int16_t item_number);
void ControlHammer(int16_t item_number);
void ControlStargate(int16_t item_number);
void ControlPlough(int16_t item_number);
void ControlChain(int16_t item_number);
void ControlBurningFloor(int16_t item_number);
void ControlRaisingBlock(int16_t item_number);
void ControlScaledSpike(int16_t item_number);
void FlameEmitter3Control(int16_t item_number);
void FlameControl(int16_t fx_number);
void FlameEmitter2Control(int16_t item_number);
void LaraBurn();
void LavaBurn(ITEM_INFO* item);
int32_t TestBoundsCollideTeethSpikes(ITEM_INFO* item);
void ControlRollingBall(int16_t item_number);
void RollingBallCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void DartsControl(int16_t item_number);
void DartEmitterControl(int16_t item_number);
void FallingCeiling(int16_t item_number);
void ControlSmashableBikeWall(int16_t item_number);
void ControlFallingBlock2(int16_t item_number);
void FallingBlockCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void FallingBlockFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void FallingBlock(int16_t item_number);
void FallingBlockCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void CeilingTrapDoorCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void FloorTrapDoorCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void OpenTrapDoor(ITEM_INFO* item);
void CloseTrapDoor(ITEM_INFO* item);
void TrapDoorControl(int16_t item_number);
void ControlObelisk(int16_t item_number);

#define MAX_LIBRARY_TABS 8
extern int8_t LibraryTab[MAX_LIBRARY_TABS];