#pragma once
#include "../global/types.h"

extern bool using_multi_color_fog_bulbs;

TR_FORCE_INLINE void CalculateVertexSpecular(
    FVECTOR vPos,
    float DistanceFogStart,
    float DistanceFogEnd,
    int32_t* cR,
    int32_t* cG,
    int32_t* cB,
    int32_t* sR,
    int32_t* sG,
    int32_t* sB,
    int32_t* sA);

void ProcessObjectMeshVertices(MESH_DATA* mesh);
void ProcessStaticMeshVertices(MESH_DATA* mesh);
void ProcessTrainMeshVertices(MESH_DATA* mesh);
void ProcessPickupMeshVertices(MESH_DATA* mesh);
void phd_PutPolygons(int16_t* objptr, int32_t clip);
void phd_PutPolygons_train(int16_t* objptr, int32_t x);
void _InsertRoom(ROOM_INFO* r);
void RenderLoadPic(int32_t unused);
void S_InitialisePolyList();
void phd_PutPolygonsPickup(int16_t* objptr, float x, float y, int32_t color);
void phd_PutPolygonSkyMesh(int16_t* objptr, int32_t clipstatus);
void S_DrawPickup(int16_t object_number);
int32_t S_GetObjectBounds(int16_t* bounds);
void do_boot_screen(int32_t language);
void S_AnimateTextures(int32_t n);
int32_t S_DumpScreen();
void S_OutputPolyList();
void StashSkinVertices(int32_t node);
void SkinVerticesToScratch(int32_t node);
int32_t GetRenderScale(int32_t unit);
int32_t GetFixedScale(int32_t unit);

extern int32_t GlobalAlpha;
extern int32_t GlobalAmbient;

extern float AnimatingTexturesV[16][8][3];