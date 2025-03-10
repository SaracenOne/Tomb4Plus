#include "../tomb4/pch.h"
#include "function_table.h"
#include "function_stubs.h"
#include "dxshell.h"
#include "polyinsert.h"
#include "3dmath.h"
#include "winmain.h"
#include "bgfx.h"
#include "../game/gameflow.h"
#include "../tomb4/tomb4plus/t4plus_weather.h"

void (*AddQuadSorted)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
void (*AddTriSorted)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
void (*AddQuadZBuffer)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, int16_t v3, TEXTURESTRUCT* tex, int32_t double_sided);
void (*AddTriZBuffer)(GFXTLVERTEX* v, int16_t v0, int16_t v1, int16_t v2, TEXTURESTRUCT* tex, int32_t double_sided);
void (*AddLineSorted)(GFXTLVERTEX* v0, GFXTLVERTEX* v1, int16_t drawtype);
bool (*IsVisible)(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2);
int32_t(*_BeginScene)();
int32_t(*_EndScene)();

GFXTLVERTEX MyVertexBuffer[0x2000];

void SetDistanceFogColor(int32_t r, int32_t g, int32_t b) {
	r &= 0xFF;
	g &= 0xFF;
	b &= 0xFF;
	gfDistanceFog.r = int8_t(r & 0xff);
	gfDistanceFog.g = int8_t(g & 0xff);
	gfDistanceFog.b = int8_t(b & 0xff);
	gfDistanceFog.a = 0
	                  ;
	if (t4_override_fog_mode == T4P_FOG_FORCE_VOLUMETRIC) {
		bgfx_fog_color[0] = 0.0f;
		bgfx_fog_color[1] = 0.0f;
		bgfx_fog_color[2] = 0.0f;
		bgfx_fog_color[3] = 1.0f;
	} else {
		bgfx_fog_color[0] = (float)r / 255.0f;
		bgfx_fog_color[1] = (float)g / 255.0f;
		bgfx_fog_color[2] = (float)b / 255.0f;
		bgfx_fog_color[3] = 1.0f;
	}
}

void UpdateDistanceFogColor() {
	SetDistanceFogColor(gfDistanceFog.r, gfDistanceFog.g, gfDistanceFog.b);
}

void SetVolumetricFogColor(int32_t r, int32_t g, int32_t b) {
	r &= 0xFF;
	g &= 0xFF;
	b &= 0xFF;

	gfVolumetricFog.r = int8_t(r & 0xff);
	gfVolumetricFog.g = int8_t(g & 0xff);
	gfVolumetricFog.b = int8_t(b & 0xff);
	gfVolumetricFog.a = 0;
	bgfx_volumetric_fog_color[0] = (float)r / 255.0f;
	bgfx_volumetric_fog_color[1] = (float)g / 255.0f;
	bgfx_volumetric_fog_color[2] = (float)b / 255.0f;
	bgfx_volumetric_fog_color[3] = 1.0f;
}

void HWInitialise() {
	Log(2, "HWIntialise");	//nice typo
	InitializeBGFX();
}

bool _NVisible(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2) {
	return (v0->sy - v1->sy) * (v2->sx - v1->sx) - (v2->sy - v1->sy) * (v0->sx - v1->sx) < 0;
}

bool _Visible(GFXTLVERTEX* v0, GFXTLVERTEX* v1, GFXTLVERTEX* v2) {
	return (v0->sy - v1->sy) * (v2->sx - v1->sx) - (v2->sy - v1->sy) * (v0->sx - v1->sx) > 0;
}

void SetCullCW() {
	IsVisible = _Visible;
}

void SetCullCCW() {
	IsVisible = _NVisible;
}

int32_t HWBeginScene() {
	if (App.dx.InScene)
		Log(1, "Already In Scene");

	App.dx.InScene = 1;
	App.dx.DoneBlit = 0;
	while (App.dx.WaitAtBeginScene) {};
	StartBGFXFrame();
	return 1;
}

int32_t HWEndScene() {
	App.dx.InScene = 0;
	EndBGFXFrame();
	return 1;
}

void InitialiseFunctionTable() {
	_BeginScene = HWBeginScene;
	_EndScene = HWEndScene;
	IsVisible = _NVisible;

	AddQuadZBuffer = AddQuadClippedZBuffer;
	AddTriZBuffer = AddTriClippedZBuffer;
	AddQuadSorted = AddQuadClippedSorted;
	AddTriSorted = AddTriClippedSorted;

	AddLineSorted = AddLineClippedSorted;
}
