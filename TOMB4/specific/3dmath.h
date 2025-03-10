#pragma once
#include "../global/types.h"

void phd_PushMatrix();
void phd_PushUnitMatrix();
void phd_SetTrans(int32_t x, int32_t y, int32_t z);
int32_t phd_TranslateRel(int32_t x, int32_t y, int32_t z);
void phd_RotX(int16_t angle);
void phd_RotY(int16_t angle);
void phd_RotZ(int16_t angle);
void phd_RotYXZpack(int32_t angles);
void phd_RotYXZ(int16_t y, int16_t x, int16_t z);
void phd_TranslateAbs(int32_t x, int32_t y, int32_t z);
void phd_GetVectorAngles(int32_t x, int32_t y, int32_t z, int16_t* angles);
uint32_t mGetAngle(int32_t x, int32_t z, int32_t x1, int32_t z1);
void AlterFOV(int16_t fov);
int32_t phd_atan(int32_t x, int32_t y);
uint32_t phd_sqrt(uint32_t num);
void ScaleCurrentMatrix(PHD_VECTOR* vec);
void SetupZRange(int32_t znear, int32_t zfar);
void InitWindow(int32_t x, int32_t y, int32_t w, int32_t h, int32_t znear, int32_t zfar, int32_t fov, int32_t a, int32_t b);
void phd_GenerateW2V(PHD_3DPOS* viewPos);
void phd_LookAt(int32_t sx, int32_t sy, int32_t sz, int32_t tx, int32_t ty, int32_t tz, int16_t roll);

extern float one;
extern float mone;
extern float LevelFogStart;
extern float LevelFogEnd;
extern float ClipRange;

extern float* mMXPtr;
extern float mW2V[indices_count];
extern float fMXStack[20 * indices_count];
extern float fcossin_tbl[65536];

extern int32_t* phd_mxptr;
extern int32_t w2v_matrix[indices_count];
extern int32_t matrix_stack[20 * indices_count];

extern float f_centerx;
extern float f_centery;
extern float f_top;
extern float f_left;
extern float f_bottom;
extern float f_right;
extern float f_znear;
extern float f_zfar;
extern float f_mznear;
extern float f_mzfar;
extern float f_persp;
extern float f_mpersp;
extern float f_oneopersp;
extern float f_moneopersp;
extern float f_perspoznear;
extern float f_mperspoznear;
extern float f_moneoznear;
extern float f_a;
extern float f_b;
extern float f_boo;

extern int32_t phd_winheight;
extern int32_t phd_winwidth;
extern int32_t phd_centerx;
extern int32_t phd_centery;
extern int32_t phd_top;
extern int32_t phd_left;
extern int32_t phd_bottom;
extern int32_t phd_right;
extern int32_t phd_znear;
extern int32_t phd_zfar;
extern int32_t phd_persp;
extern int16_t phd_winxmax;
extern int16_t phd_winxmin;
extern int16_t phd_winymax;
extern int16_t phd_winymin;

__inline int16_t phd_sin(int32_t angle) {
	angle >>= 3;
	return 4 * rcossin_tbl[angle & 0x1FFE];
}

__inline int16_t phd_cos(int32_t angle) {
	angle >>= 3;
	return 4 * rcossin_tbl[(angle & 0x1FFE) + 1];
}

__inline float fSin(int32_t angle) {
	return fcossin_tbl[(uint16_t)angle];
}

__inline float fCos(int32_t angle) {
	return fcossin_tbl[uint16_t(angle + 0x4000)];
}

__inline void mPopMatrix() {
	mMXPtr -= indices_count;
}

__inline void phd_PopMatrix() {
	phd_mxptr -= indices_count;
	mPopMatrix();
}
