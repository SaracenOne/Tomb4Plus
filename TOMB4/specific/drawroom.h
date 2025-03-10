#pragma once
#include "../global/types.h"

void ProcessRoomDynamics(ROOM_INFO* r);
void ProcessRoomVertices(ROOM_INFO* r);
void ProcessRoomData(ROOM_INFO* r, bool multi_colour_fog);
void InsertRoom(ROOM_INFO* r);
void CalcTriFaceNormal(GFXVECTOR* p1, GFXVECTOR* p2, GFXVECTOR* p3, GFXVECTOR* N);
void ProcessMeshData(int32_t num_meshes);
void InitBuckets();
void DrawBucket(TEXTUREBUCKET* bucket);
void FindBucket(int32_t tpage, GFXTLBUMPVERTEX** Vpp, int32_t** nVtxpp);
void DrawBuckets();
void CreateVertexNormals(ROOM_INFO* r);

extern MESH_DATA** mesh_vtxbuf;
extern TEXTUREBUCKET Bucket[MAX_BUCKETS]; // TRLE: increased bucket count (x8)
extern float clip_left;
extern float clip_top;
extern float clip_right;
extern float clip_bottom;
extern int32_t bWaterEffect;
extern int32_t num_level_meshes;

extern int32_t water_color_R;
extern int32_t water_color_G;
extern int32_t water_color_B;
