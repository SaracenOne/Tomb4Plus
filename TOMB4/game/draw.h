#pragma once
#include "../global/types.h"

void InitInterpolate(int32_t frac, int32_t rate);
void phd_PopMatrix_I();
void phd_PushMatrix_I();
void phd_RotY_I(int16_t ang);
void phd_RotX_I(int16_t ang);
void phd_RotZ_I(int16_t ang);
void phd_TranslateRel_I(int32_t x, int32_t y, int32_t z);
void phd_TranslateRel_ID(int32_t x, int32_t y, int32_t z, int32_t x2, int32_t y2, int32_t z2);
void phd_RotYXZ_I(int16_t y, int16_t x, int16_t z);
void gar_RotYXZsuperpack_I(int16_t** pprot1, int16_t** pprot2, int32_t skip);
void gar_RotYXZsuperpack(int16_t** pprot, int32_t skip);
void phd_PutPolygons_I(int16_t* ptr, int32_t clip);
void mInterpolateMatrix();
void mInterpolateArmMatrix(float* mx);
void S_InsertRoom(int16_t room_number);
void CalculateObjectLighting(ITEM_INFO* item, int16_t* frame);
void CalculateObjectLightingLara();
void DrawAnimatingItem(ITEM_INFO* item);
void DrawRooms(int16_t CurrentRoom);
void RenderIt(int16_t CurrentRoom);
int32_t DrawPhaseGame();
void GetRoomBounds();
void SetRoomBounds(int16_t* door, int32_t rn, ROOM_INFO* actualRoom);
void DrawEffect(int16_t fx_num);
void PrintObjects(int16_t room_number);
int32_t GetFrames(ITEM_INFO* item, int16_t* frm[], int32_t* rate);
int16_t* GetBoundsAccurate(ITEM_INFO* item);
int16_t* GetBestFrame(ITEM_INFO* item);
void UpdateSkyLightning();
void mRotBoundingBoxNoPerspLegacy(int16_t* bounds, int16_t* rotatedBounds);
void mRotBoundingBoxNoPersp(int16_t* bounds, int16_t* rotatedBounds);
void calc_animating_item_clip_window(ITEM_INFO* item, int16_t* bounds);

extern STATIC_INFO static_objects[];

extern int32_t IM_rate;
extern int32_t IM_frac;

extern float* mIMptr;
extern float mIMstack[indices_count * 64];

extern int32_t current_room;
extern int16_t no_rotation[12];

extern int32_t outside;

extern int16_t SkyPos;
extern int16_t SkyPos2;

extern uint16_t LightningRGB[3];
extern uint16_t LightningRGBs[3];
extern int16_t LightningCount;
extern int16_t dLightningRand;
