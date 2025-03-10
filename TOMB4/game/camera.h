#pragma once
#include "../global/types.h"

#define MAX_CAMERA_CHASE_IDEALS 5

void InitialiseCamera();
void MoveCamera(GAME_VECTOR* ideal, int32_t speed);
int32_t mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, int32_t push);
int32_t CameraCollisionBounds(GAME_VECTOR* ideal, int32_t push, int32_t yfirst);
void LaraTorch(PHD_VECTOR* Soffset, PHD_VECTOR* Eoffset, int16_t yrot, int32_t brightness);
void ChaseCamera(ITEM_INFO* item);
void CombatCamera(ITEM_INFO* item);
void LookCamera(ITEM_INFO* item);
void FixedCamera();
void BinocularCamera(ITEM_INFO* item);
void CalculateCamera();

extern bool freeze_camera_button_pressed;
extern bool camera_frozen;
extern CAMERA_INFO camera;

extern GAME_VECTOR ForcedFixedCamera;
extern int8_t UseForcedFixedCamera;

extern PHD_VECTOR LaraTorchStart;
extern PHD_VECTOR LaraTorchEnd;
extern int32_t bLaraTorch;
extern int32_t LaraTorchIntensity;
extern int32_t LaraTorchYRot;

extern camera_type BinocularOldCamera;
extern int32_t BinocularOn;
extern int32_t BinocularRange;
extern int32_t ExittingBinos;
extern int32_t LaserSight;

extern SHATTER_ITEM ShatterItem;

extern bool tr5_camera_behaviour;
