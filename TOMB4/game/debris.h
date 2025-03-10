#pragma once
#include "../global/types.h"

void UpdateDebris();
void TriggerDebris(GAME_VECTOR* pos, void* TextInfo, int16_t* Offsets, int32_t* Vels, int16_t rgb);
int32_t GetFreeDebris();
void ShatterObject(SHATTER_ITEM* shatter_item, MESH_INFO* StaticMesh, int16_t Num, int16_t RoomNumber, int32_t NoXZVel);

extern DEBRIS_STRUCT debris[256];
extern int32_t next_debris;
extern int16_t DebrisFlags;
