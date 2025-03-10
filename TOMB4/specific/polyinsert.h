#pragma once
#include "../global/types.h"

#define MAXIMUM_CLIPPER_BUFFER_SIZE 20
#define MAXIMUM_LEVEL_FOGBULBS 64 // Original was 20

#define MAXIMUM_ACTIVE_FOGBULBS 32 // Original was 5
#define MAXIMUM_ACTIVE_FXBULBS 32 // Original was 5

void HWR_DrawSortList(GFXTLBUMPVERTEX *info, int16_t num_verts, int16_t texture, int16_t type);
void DrawSortList();
void CreateFogPos(FOGBULB_STRUCT* FogBulb);
void ControlFXBulb(FOGBULB_STRUCT* FogBulb);
void CreateFXBulbs();
void ClearFXFogBulbs();
void TriggerFXFogBulb(int32_t x, int32_t y, int32_t z, int32_t FXRad, int32_t density, int32_t r, int32_t g, int32_t b, int32_t room_number);
bool IsVolumetric();
int DistCompare(const void* a, const void* b);
void InitialiseFogBulbs();
void OmniEffect(GFXTLVERTEX* v);
void OmniFog(GFXTLVERTEX* v, bool multi_colour_fog);
void AddTriClippedSorted(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
void AddQuadClippedSorted(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
void AddLineClippedSorted(GFXTLVERTEX* v0, GFXTLVERTEX* v1, int16_t drawtype);
void InitialiseSortList();
void DoSort(int32_t left, int32_t right, SORTLIST** list);
void SortPolyList(int32_t count, SORTLIST** list);
void mD3DTransform(FVECTOR* vec, GFXMATRIX* mx);
void AddClippedPoly(GFXTLBUMPVERTEX* dest, int32_t nPoints, GFXTLBUMPVERTEX* v, TEXTURESTRUCT* pTex);
void AddTriClippedZBuffer(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
void AddQuadClippedZBuffer(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
void SubdivideEdge(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v, int16_t* c, float tu1, float tv1, float tu2, float tv2, float* tu, float* tv);
void SubdivideQuad(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2, GFXTLVERTEX* v3, TEXTURESTRUCT* tex, int32_t double_sided, int32_t steps, int16_t* c);
void SubdivideTri(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2, TEXTURESTRUCT* tex, int32_t double_sided, int32_t steps, int16_t* c);
void AddTriSubdivide(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
void AddQuadSubdivide(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
void CalcColorSplit(GFXCOLOR s, GFXCOLOR* r);

extern GFXTLBUMPVERTEX XYUVClipperBuffer[MAXIMUM_CLIPPER_BUFFER_SIZE];
extern GFXTLBUMPVERTEX zClipperBuffer[MAXIMUM_CLIPPER_BUFFER_SIZE];

extern FOGBULB_STRUCT FogBulbs[MAXIMUM_LEVEL_FOGBULBS];
extern int32_t NumLevelFogBulbs;

extern int32_t nPolys;
extern int32_t nClippedPolys;
extern int32_t DrawPrimitiveCnt;

// TRLE: increased size (x16)
#define MAXIMUM_SORT_LIST_SIZE 16384 * 16
#define MAXIMUM_SORT_BUFFER_SIZE 655360 * 16

extern SORTLIST* SortList[MAXIMUM_SORT_LIST_SIZE];
extern int32_t SortCount;
