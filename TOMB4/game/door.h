#pragma once
#include "../global/types.h"

void ShutThatDoor(DOORPOS_DATA* d);
void OpenThatDoor(DOORPOS_DATA* d);
void DoorControl(int16_t item_number);
void DoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);
void PushPullKickDoorControl(int16_t item_number);
void PushPullKickDoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);
void DoubleDoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);
void UnderwaterDoorCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);
void SequenceDoorControl(int16_t item_number);
