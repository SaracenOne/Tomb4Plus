#pragma once
#include "../global/types.h"

#define MAX_SEQUENCES 3
#define MAX_USED_SEQUENCES 6

void FullBlockSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
int32_t SwitchTrigger(int16_t item_number, int16_t timer);
int32_t GetSwitchTrigger(ITEM_INFO* item, int16_t* ItemNos, int32_t AttatchedToSwitch);
void TestTriggersAtXYZ(int32_t x, int32_t y, int32_t z, int16_t room_number, bool heavy, int16_t flags);
void SwitchControl(int16_t item_number);
void SwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void SwitchCollision2(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void SwitchType78Collision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void UnderwaterSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void PulleyCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void TurnSwitchControl(int16_t item_number);
void TurnSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void RailSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void JumpSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void CrowbarSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void FullBlockSwitchControl(int16_t item_number);
void CogSwitchControl(int16_t item_number);
void CogSwitchCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);

extern PHD_VECTOR OldPickupPos;
extern uint8_t CurrentSequence;
extern uint8_t Sequences[MAX_SEQUENCES];
extern uint8_t SequenceUsed[MAX_USED_SEQUENCES];
extern uint8_t SequenceResults[MAX_SEQUENCES][MAX_SEQUENCES][MAX_SEQUENCES];
