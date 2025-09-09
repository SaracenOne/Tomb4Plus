#pragma once
#include "../global/types.h"

#define INFINITE_HEALTH -16384

int32_t ControlPhase(int32_t nframes, int32_t demo_mode);
void FlipMap(int32_t FlipNumber);
void RemoveRoomFlipItems(ROOM_INFO* r);
void AddRoomFlipItems(ROOM_INFO* r);
void TestTriggers(int16_t* data, bool heavy, int32_t heavy_flags);
int16_t GetDoor(FLOOR_INFO* floor);
int32_t CheckNoColFloorTriangle(FLOOR_INFO* floor, int32_t x, int32_t z);
int32_t CheckNoColCeilingTriangle(FLOOR_INFO* floor, int32_t x, int32_t z);
FLOOR_INFO* GetFloor(int32_t x, int32_t y, int32_t z, int16_t* room_number);
int32_t GetWaterHeight(int32_t x, int32_t y, int32_t z, int16_t room_number);
int32_t GetHeight(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);
int32_t GetCeiling(FLOOR_INFO* floor, int32_t x, int32_t y, int32_t z);
void AlterFloorHeight(ITEM_INFO* item, int32_t height);
void InterpolateAngle(int16_t dest, int16_t* src, int16_t* diff, int16_t speed);
void TranslateItem(ITEM_INFO* item, int16_t x, int16_t y, int16_t z);
int32_t GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void UpdateSky();
void KillMoveEffects();
void KillMoveItems();
void RefreshCamera(int16_t type, int16_t* data);
bool TriggerActive(ITEM_INFO* item);
void TriggerNormalCDTrack(int16_t value, int16_t flags, int16_t type);
void TriggerCDTrack(int16_t value, int16_t flags, int16_t type);
int32_t ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
int32_t xLOS(GAME_VECTOR* start, GAME_VECTOR* target);
int32_t zLOS(GAME_VECTOR* start, GAME_VECTOR* target);
int32_t LOS(GAME_VECTOR* start, GAME_VECTOR* target);
void FireCrossBowFromLaserSight(GAME_VECTOR* start, GAME_VECTOR* target);
int32_t ExplodeItemNode(ITEM_INFO* item, int32_t Node, int32_t NoXZVel, int32_t bits);
int32_t IsRoomOutside(int32_t x, int32_t y, int32_t z);
int32_t ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* target, PHD_VECTOR* Coord, MESH_INFO** StaticMesh);
int32_t GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, int32_t DrawTarget, int32_t firing);
void UpdateItemRoom(int16_t item_number, int16_t y_test_offset);
void ApplyItemGravity(ITEM_INFO* item);
void AnimateItem(ITEM_INFO* item);
int32_t RayBoxIntersect(PHD_VECTOR* min, PHD_VECTOR* max, PHD_VECTOR* mid, PHD_VECTOR* dir, PHD_VECTOR* Coord);
int32_t DoRayBox(GAME_VECTOR* start, GAME_VECTOR* target, int16_t* bounds, PHD_3DPOS* ItemPos, PHD_VECTOR* Coord, int16_t item_number);
int32_t GetMaximumFloor(FLOOR_INFO* floor, int32_t x, int32_t z);
int32_t GetMinimumCeiling(FLOOR_INFO* floor, int32_t x, int32_t z);

#define MAX_ROOMS 256

extern size_t vanilla_item_malloc_offset;

extern ITEM_INFO* items;
extern ANIM_STRUCT* anims;
extern ROOM_INFO* room;
extern int16_t** meshes;
extern int32_t* bones;
extern int32_t level_items;
extern int16_t number_rooms;

extern int16_t* OutsideRoomOffsets;
extern int8_t* OutsideRoomTable;
extern int16_t IsRoomOutsideNo;

extern MESH_INFO* SmashedMesh[16];
extern int16_t SmashedMeshRoom[16];
extern int16_t SmashedMeshCount;

#define MAX_FLIPMAPS 32 // bumped from 10

extern int32_t flipmap[MAX_FLIPMAPS];
extern int32_t flip_stats[MAX_FLIPMAPS];
extern int32_t flip_status;
extern int32_t flipeffect;
extern int32_t fliptimer;

extern int16_t *trigger_index;
extern int32_t tiltxoff;
extern int32_t tiltyoff;
extern int32_t OnObject;
extern int32_t height_type;

extern bool InItemControlLoop;
extern int16_t ItemNewRooms[MAX_ROOMS][2];
extern int16_t ItemNewRoomNo;

extern uint8_t CurrentAtmosphere;
extern bool IsAtmospherePlaying;
extern int8_t cd_flags[128];

extern uint32_t FmvSceneTriggered;
extern uint32_t CutSceneTriggered;
extern bool SetDebounce;
extern int32_t framecount;
extern int32_t reset_flag;
extern int32_t WeaponDelay;
extern int32_t LaserSightX;
extern int32_t LaserSightY;
extern int32_t LaserSightZ;
extern uint16_t GlobalCounter;
extern int16_t XSoff1;
extern int16_t XSoff2;
extern int16_t YSoff1;
extern int16_t YSoff2;
extern int16_t ZSoff1;
extern int16_t ZSoff2;
extern int16_t FXType;
extern int8_t PoisonFlag;
extern int8_t TriggerTimer;
extern int8_t LaserSightActive;
extern int8_t DeathMenuActive;
