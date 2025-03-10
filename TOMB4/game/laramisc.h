#pragma once
#include "../global/types.h"

void LaraCheatyBits();
void InitialiseLaraLoad(int16_t item_number);
void InitialiseLaraAnims(ITEM_INFO* item);
void LaraInitialiseMeshes();
void AnimateLara(ITEM_INFO* item);
void LaraControl(int16_t item_number);

extern COLL_INFO mycoll;
