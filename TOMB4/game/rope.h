#pragma once
#include "../global/types.h"

#define MAXIMUM_ROPES 64

void DrawRopeList();
void ProjectRopePoints(ROPE_STRUCT* Rope);
PHD_VECTOR* Normalise(PHD_VECTOR* v);
void vMul(PHD_VECTOR* v, int32_t scale, PHD_VECTOR* d);
int32_t mDotProduct(PHD_VECTOR* a, PHD_VECTOR* b);
void mCrossProduct(PHD_VECTOR* a, PHD_VECTOR* b, PHD_VECTOR* n);
void phd_GetMatrixAngles(int32_t* m, int16_t* dest);
void GetRopePos(ROPE_STRUCT* rope, int32_t pos, int32_t* x, int32_t* y, int32_t* z);
void AlignLaraToRope(ITEM_INFO* l);
void ModelRigid(PHD_VECTOR* pa, PHD_VECTOR* pb, PHD_VECTOR* va, PHD_VECTOR* vb, int32_t rlength);
void ModelRigidRope(PHD_VECTOR* pa, PHD_VECTOR* pb, PHD_VECTOR* va, PHD_VECTOR* vb, int32_t rlength);
void SetPendulumPoint(ROPE_STRUCT* Rope, int32_t node);
void SetPendulumVelocity(int32_t x, int32_t y, int32_t z);
void CalculateRope(ROPE_STRUCT* Rope);
int32_t RopeNodeCollision(ROPE_STRUCT* rope, int32_t x, int32_t y, int32_t z, int32_t rad);
void RopeControl(int16_t item_num);
void RopeCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);

extern ROPE_STRUCT RopeList[MAXIMUM_ROPES];
extern PENDULUM CurrentPendulum;
extern int32_t nRope;
