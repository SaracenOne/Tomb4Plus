#pragma once
#include "../global/types.h"

void DrawTrainStrips();
void S_PrintShadow(int16_t size, int16_t* box, ITEM_INFO* item);
void S_DrawDrawSparks(SPARKS* sptr, int32_t smallest_size, int32_t* xyptr, int32_t* zptr);
void DrawBikeSpeedo(int32_t ux, int32_t uy, int32_t vel, int32_t maxVel, int32_t turboVel, int32_t size, int32_t unk);
void Draw2DSprite(int32_t x, int32_t y, int32_t slot, int32_t unused, int32_t unused2);
void DrawJeepSpeedo(int32_t ux, int32_t uy, int32_t vel, int32_t maxVel, int32_t turboVel, int32_t size, int32_t spriteSlot);
void DrawDebris();
void DoScreenFade();
void DrawPsxTile(int32_t x_y, int32_t height_width, int32_t color, int32_t u0, int32_t u1);
void DrawFlash();
void S_DrawDarts(ITEM_INFO* item);
void ClipCheckPoint(GFXTLVERTEX* v, float x, float y, float z, int16_t* clip);
void DrawFlatSky(uint32_t color, int32_t zpos, int32_t ypos, int32_t drawtype);
void OutputSky();
void ProjectTriPoints(PHD_VECTOR* pos, int32_t& x, int32_t& y, int32_t& z);
void setXY4(GFXTLVERTEX* v, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t x4, int32_t y4, int32_t z, int16_t* clip);
void setXY3(GFXTLVERTEX* v, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t z, int16_t* clip);
void setXYZ4(GFXTLVERTEX* v, int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2, int32_t x3, int32_t y3, int32_t z3, int32_t x4, int32_t y4, int32_t z4, int16_t* clip);
void setXYZ3(GFXTLVERTEX* v, int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2, int32_t x3, int32_t y3, int32_t z3, int16_t* clip);
void SetFade(int32_t start, int32_t end);
void DrawLaserSightSprite();
void DrawSprite(int32_t x, int32_t y, int32_t slot, int32_t col, int32_t size, int32_t z);
void ShowTitle();
void SetUpLensFlare(int32_t x, int32_t y, int32_t z, GAME_VECTOR* lfobj);
void InitTarget_2();
void InitBinoculars();
void DrawBinoculars();
void DrawWraithTrail(ITEM_INFO* item);
void DrawDrips();
void DrawBubbles();
void DrawShockwaves();
void DrawTrainFloorStrip(int32_t x, int32_t z, TEXTURESTRUCT* tex, int32_t y_and_flags);
void S_DrawSplashes();
bool ClipLine(int32_t& x1, int32_t& y1, int32_t z1, int32_t& x2, int32_t& y2, int32_t z2, int32_t xMin, int32_t yMin, int32_t w, int32_t h);
void S_DrawFireSparks(int32_t size, int32_t life);
void DrawRope(ROPE_STRUCT* rope);
void DrawBlood();
void S_DrawSmokeSparks();
void DoUwEffect();
void DrawLightning();

extern MESH_DATA* targetMeshP;
extern int32_t DoFade;