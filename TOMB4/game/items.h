#pragma once
#include "../global/types.h"

void InitialiseItemArray(int16_t num);
void KillItem(int16_t item_num);
int16_t CreateItem();
void InitialiseItem(int16_t item_num);
void RemoveActiveItem(int16_t item_num);
void RemoveDrawnItem(int16_t item_num);
void AddActiveItem(int16_t item_num);
void ItemNewRoom(int16_t item_num, int16_t room_num);
void InitialiseFXArray(int32_t allocmem);
int16_t CreateEffect(int16_t room_num);
void KillEffect(int16_t fx_num);
void EffectNewRoom(int16_t fx_num, int16_t room_num);

extern int16_t next_fx_active;
extern int16_t next_item_active;
