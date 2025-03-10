#pragma once
#include "../global/types.h"

void SetDistanceFogColor(int32_t r, int32_t g, int32_t b);
void UpdateDistanceFogColor();
void SetVolumetricFogColor(int32_t r, int32_t g, int32_t b);
void HWInitialise();
bool _NVisible(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2);
bool _Visible(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2);
void SetCullCW();
void SetCullCCW();
int32_t HWBeginScene();
int32_t HWEndScene();
void InitialiseFunctionTable();

extern void (*AddQuadSorted)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
extern void (*AddTriSorted)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
extern void (*AddQuadZBuffer)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
extern void (*AddTriZBuffer)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
extern void (*AddLineSorted)(GFXTLVERTEX* v0, GFXTLVERTEX* v1, int16_t drawtype);
extern bool (*IsVisible)(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2);
extern int32_t (*_BeginScene)();
extern int32_t (*_EndScene)();

extern GFXTLVERTEX MyVertexBuffer[0x2000];