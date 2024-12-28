#pragma once
#include "../global/types.h"

#define INFINITE_HEALTH -16384

long ControlPhase(long nframes, long demo_mode);
void FlipMap(long FlipNumber);
void RemoveRoomFlipItems(ROOM_INFO* r);
void AddRoomFlipItems(ROOM_INFO* r);
void TestTriggers(short* data, bool heavy, long heavy_flags);
short GetDoor(FLOOR_INFO* floor);
long CheckNoColFloorTriangle(FLOOR_INFO* floor, long x, long z);
long CheckNoColCeilingTriangle(FLOOR_INFO* floor, long x, long z);
FLOOR_INFO* GetFloor(long x, long y, long z, short* room_number);
long GetWaterHeight(long x, long y, long z, short room_number);
long GetHeight(FLOOR_INFO* floor, long x, long y, long z);
long GetCeiling(FLOOR_INFO* floor, long x, long y, long z);
void AlterFloorHeight(ITEM_INFO* item, long height);
void InterpolateAngle(short dest, short* src, short* diff, short speed);
void TranslateItem(ITEM_INFO* item, short x, short y, short z);
long GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void UpdateSky();
void KillMoveEffects();
void KillMoveItems();
void RefreshCamera(short type, short* data);
bool TriggerActive(ITEM_INFO* item);
void TriggerNormalCDTrack(short value, short flags, short type);
void TriggerCDTrack(short value, short flags, short type);
long ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
long xLOS(GAME_VECTOR* start, GAME_VECTOR* target);
long zLOS(GAME_VECTOR* start, GAME_VECTOR* target);
long LOS(GAME_VECTOR* start, GAME_VECTOR* target);
void FireCrossBowFromLaserSight(GAME_VECTOR* start, GAME_VECTOR* target);
long ExplodeItemNode(ITEM_INFO* item, long Node, long NoXZVel, long bits);
long IsRoomOutside(long x, long y, long z);
long ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* target, PHD_VECTOR* Coord, MESH_INFO** StaticMesh);
long GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, long DrawTarget, long firing);
void UpdateItemRoom(short item_number, short y_test_offset);
void ApplyItemGravity(ITEM_INFO* item);
void AnimateItem(ITEM_INFO* item);
long RayBoxIntersect(PHD_VECTOR* min, PHD_VECTOR* max, PHD_VECTOR* mid, PHD_VECTOR* dir, PHD_VECTOR* Coord);
long DoRayBox(GAME_VECTOR* start, GAME_VECTOR* target, short* bounds, PHD_3DPOS* ItemPos, PHD_VECTOR* Coord, short item_number);
long GetMaximumFloor(FLOOR_INFO* floor, long x, long z);
long GetMinimumCeiling(FLOOR_INFO* floor, long x, long z);

extern size_t vanilla_item_malloc_offset;

extern ITEM_INFO* items;
extern ANIM_STRUCT* anims;
extern ROOM_INFO* room;
extern short** meshes;
extern long* bones;
extern int32_t level_items;
extern short number_rooms;

extern short* OutsideRoomOffsets;
extern char* OutsideRoomTable;
extern short IsRoomOutsideNo;

extern MESH_INFO* SmashedMesh[16];
extern short SmashedMeshRoom[16];
extern short SmashedMeshCount;

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
extern short ItemNewRooms[256][2];
extern short ItemNewRoomNo;

extern uchar CurrentAtmosphere;
extern bool IsAtmospherePlaying;
extern char cd_flags[128];

extern ulong FmvSceneTriggered;
extern ulong CutSceneTriggered;
extern long SetDebounce;
extern long framecount;
extern long reset_flag;
extern long WeaponDelay;
extern long LaserSightX;
extern long LaserSightY;
extern long LaserSightZ;
extern ushort GlobalCounter;
extern short XSoff1;
extern short XSoff2;
extern short YSoff1;
extern short YSoff2;
extern short ZSoff1;
extern short ZSoff2;
extern short FXType;
extern char PoisonFlag;
extern char TriggerTimer;
extern char LaserSightActive;
extern char DeathMenuActive;
