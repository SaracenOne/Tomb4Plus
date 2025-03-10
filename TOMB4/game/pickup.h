#pragma once
#include "../global/types.h"

void SarcophagusCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void KeyHoleCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDoneCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDone(ITEM_INFO* item, int16_t item_number);
void AnimatingPickUp(int16_t item_number);
int16_t* FindPlinth(ITEM_INFO* item);
int32_t KeyTrigger(int16_t item_num);
int32_t PickupTrigger(int16_t item_num);
void RegeneratePickups();
void PickUpCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleHoleCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);

extern uint8_t NumRPickups;
extern uint8_t RPickups[16];
extern int8_t KeyTriggerActive;
