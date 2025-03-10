#pragma once
#include "../global/types.h"

#define MAX_SPLASHES 4
#define MAX_RIPPLES 16

void ControlSmokeEmitter(int16_t item_number);
void TriggerExplosionSmokeEnd(int32_t x, int32_t y, int32_t z, int32_t uw);
void TriggerExplosionSmoke(int32_t x, int32_t y, int32_t z, int32_t uw);
void TriggerFlareSparks(int32_t x, int32_t y, int32_t z, int32_t xvel, int32_t yvel, int32_t zvel, int32_t smoke);
void TriggerDynamic(int32_t x, int32_t y, int32_t z, int32_t falloff, int32_t r, int32_t g, int32_t b);
void ClearDynamics();
void ControlEnemyMissile(int16_t fx_number);
void SetupRipple(int32_t x, int32_t y, int32_t z, int32_t size, int32_t flags);
void TriggerUnderwaterBlood(int32_t x, int32_t y, int32_t z, int32_t size);
void TriggerWaterfallMist(int32_t x, int32_t y, int32_t z, int32_t ang);
void TriggerDartSmoke(int32_t x, int32_t y, int32_t z, int32_t xv, int32_t zv, int32_t hit);
void KillAllCurrentItems(int16_t item_number);
void KillEverything();
void TriggerExplosionBubble(int32_t x, int32_t y, int32_t z, int16_t room_number);
void ControlColouredLights(int16_t item_number);
void DetatchSpark(int32_t num, int32_t type);
int32_t GetFreeSpark();
void UpdateSparks();
void TriggerRicochetSpark(GAME_VECTOR* pos, int32_t ang, int32_t num, int32_t smoke_only);
void TriggerExplosionSparks(int32_t x, int32_t y, int32_t z, int32_t extras, int32_t dynamic, int32_t uw, int16_t room_number);
void TriggerFireFlame(int32_t x, int32_t y, int32_t z, int32_t body_part, int32_t type);
void TriggerSuperJetFlame(ITEM_INFO* item, int32_t yvel, int32_t deadly);
void TriggerRocketSmoke(int32_t x, int32_t y, int32_t z, int32_t col);
void SetupSplash(SPLASH_SETUP* setup);
void UpdateSplashes();

extern DYNAMIC dynamics[MAX_DYNAMICS * 2];
extern SPLASH_STRUCT splashes[MAX_SPLASHES];
extern RIPPLE_STRUCT ripples[MAX_RIPPLES];
extern SPLASH_SETUP splash_setup;
extern SPARKS spark[MAX_SPARKS];
extern int32_t wibble;
extern int32_t SplashCount;
extern int32_t KillEverythingFlag;
extern int32_t SmokeCountL;
extern int32_t SmokeCountR;
extern int32_t SmokeWeapon;
extern int32_t SmokeWindX;
extern int32_t SmokeWindZ;

extern void TriggerBreath(int32_t x, int32_t y, int32_t z, int32_t xv, int32_t yv, int32_t zv);