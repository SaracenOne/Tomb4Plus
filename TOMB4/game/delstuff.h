#pragma once
#include "../global/types.h"

void DrawLara(ITEM_INFO* item, bool mirror);
void GetLaraJointPos(PHD_VECTOR* pos, int32_t node);
void SetLaraUnderwaterNodes();
void Rich_CalcLaraMatrices_Normal(int16_t* frame, int32_t* bone, int32_t flag);
void Rich_CalcLaraMatrices_Interpolated(int16_t* frame1, int16_t* frame2, int32_t frac, int32_t rate, int32_t* bone, int32_t flag);
void CalcLaraMatrices(int32_t flag);

extern int16_t* GLaraShadowframe;
extern float lara_matrices[180];
extern float lara_joint_matrices[180];
extern int32_t LaraNodeAmbient[2];
extern int32_t bLaraUnderWater;
extern uint8_t LaraNodeUnderwater[15];
extern int8_t SkinVertNums[40][12];
extern int8_t ScratchVertNums[40][12];
extern int8_t HairRotScratchVertNums[5][12];
extern int8_t bLaraInWater;
