#pragma once
#include "../global/types.h"

#define COLLISION_RANGE (BLOCK_SIZE * 3)

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
int32_t GetCollidedObjects(ITEM_INFO* item, int32_t rad, int32_t noInvisible, ITEM_INFO** StoredItems, MESH_INFO** StoredStatics, int32_t StoreLara);
void GenericDeadlyBoundingBoxCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void GenericSphereBoxCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void CreatureCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
int32_t FindGridShift(int32_t src, int32_t dst);
int16_t GetTiltType(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);
bool CollideStaticObjects(COLL_INFO* coll, int32_t x, int32_t y, int32_t z, int16_t room_number, int32_t hite);
void UpdateLaraRoom(ITEM_INFO* item, int32_t height);
void LaraBaddieCollision(ITEM_INFO* l, COLL_INFO* coll);
void ObjectCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void ObjectCollisionNoBigPush(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void TrapCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
bool ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int32_t spaz, int32_t BigPush);
int32_t TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int32_t rad);
int32_t TestBoundsCollideStatic(int16_t* bounds, PHD_3DPOS* pos, int32_t rad);
int32_t ItemPushLaraStatic(ITEM_INFO* l, int16_t* bounds, PHD_3DPOS* pos, COLL_INFO* coll);
int32_t TestLaraPosition(int16_t* bounds, ITEM_INFO* item, ITEM_INFO* l);
void AlignLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
int32_t Move3DPosTo3DPos(PHD_3DPOS* pos, PHD_3DPOS* dest, int32_t speed, int16_t rotation);
int32_t MoveLaraPosition(PHD_VECTOR* v, ITEM_INFO* item, ITEM_INFO* l);
int32_t TestBoundsCollide2(ITEM_INFO* item, ITEM_INFO* l, int32_t rad);
void StargateCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void CogCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void GetCollisionInfo(COLL_INFO* coll, int32_t x, int32_t y, int32_t z, int16_t room_number, int32_t hite);

extern int16_t GlobalCollisionBounds[6];
