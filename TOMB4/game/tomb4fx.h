#pragma once
#include "../global/types.h"

LIGHTNING_STRUCT* TriggerLightning(PHD_VECTOR* s, PHD_VECTOR* d, int8_t variation, int32_t rgb, uint8_t flags, uint8_t size, uint8_t segments);
int32_t ExplodingDeath2(int16_t item_number, int32_t mesh_bits, int16_t Flags);
void DrawGunshells();
void TriggerGunSmoke(int32_t x, int32_t y, int32_t z, int32_t xVel, int32_t yVel, int32_t zVel, int32_t notLara, int32_t weaponType, int32_t shade);
void LaraBubbles(ITEM_INFO* item);
void UpdateDrips();
int32_t GetFreeFireSpark();
void TriggerGlobalStaticFlame();
void TriggerGlobalFireFlame();
void keep_those_fires_burning();
void UpdateFireSparks();
void ClearFires();
void AddFire(int32_t x, int32_t y, int32_t z, int32_t size, int16_t room_number, int16_t fade);
void S_DrawFires();
int32_t GetFreeSmokeSpark();
void UpdateSmokeSparks();
void TriggerShatterSmoke(int32_t x, int32_t y, int32_t z);
void DrawLensFlares(ITEM_INFO* item);
void DrawWeaponMissile(ITEM_INFO* item);
int32_t GetFreeGunshell();
void TriggerGunShell(int16_t leftright, int16_t objnum, int32_t weapon);
void UpdateGunShells();
void TriggerSmallSplash(int32_t x, int32_t y, int32_t z, int32_t num);
void TriggerGunflash(SVECTOR* pos);
void SetGunFlash(int16_t weapon);
void DrawGunflashes();
int32_t GetFreeBlood();
void UpdateBlood();
void TriggerBlood(int32_t x, int32_t y, int32_t z, int32_t angle, int32_t num);
int32_t GetFreeBubble();
void CreateBubble(PHD_3DPOS* pos, int16_t room_number, int32_t size, int32_t biggest);
void UpdateBubbles();
int32_t GetFreeDrip();
void TriggerLaraDrips();
int32_t GetFreeShockwave();
void TriggerShockwave(PHD_VECTOR* pos, int32_t InnerOuterRads, int32_t speed, int32_t rgb, int32_t XRotFlags);
void TriggerShockwaveHitEffect(int32_t x, int32_t y, int32_t z, int32_t rgb, int16_t dir, int32_t speed);
void UpdateShockwaves();
void UpdateLightning();
int32_t LSpline(int32_t x, int32_t* knots, int32_t nk);
void CalcLightningSpline(PHD_VECTOR* pos, SVECTOR* dest, LIGHTNING_STRUCT* lptr);
void TriggerLightningGlow(int32_t x, int32_t y, int32_t z, int32_t rgb);
void TriggerFlashSmoke(int32_t x, int32_t y, int32_t z, int16_t room_number);
void S_DrawSparks();
void SetFadeClip(int16_t height, int16_t speed);
void SetFadeClipImmediate(int16_t height); // TRNG
void UpdateFadeClip();
void SetScreenFadeOut(int16_t speed, int16_t back);
void SetScreenFadeIn(int16_t speed);
void Fade();

#define MAX_SNOW_SIZES 32
#define MAX_LIGHTNING 16
#define MAX_GUNSHELLS 24
#define MAX_DRIPS 32
#define MAX_SMOKE_SPARKS 32
#define MAX_BUBBLES 40
#define MAX_SHOCKWAVES 16
#define MAX_FIRE_SPARKS 20
#define MAX_BLOOD 32
#define MAX_GUN_FLASHES 4
#define MAX_FIRES 32

#define TSV_BUFFER_SIZE 16384

#define NODE_ID_LARA_TORCH 0
#define NODE_ID_GUIDE_TORCH 1
#define NODE_ID_SETH_A 2
#define NODE_ID_SETH_B 3
#define NODE_ID_HARPY_A 4
#define NODE_ID_HARPY_B 5
#define NODE_ID_UNKNOWN_2 6
#define NODE_ID_HYDRA 7
#define MAX_NODE_OFFSETS 16

extern float SnowSizes[MAX_SNOW_SIZES]; // TRLE
extern NODEOFFSET_INFO NodeOffsets[MAX_NODE_OFFSETS];
extern LIGHTNING_STRUCT Lightning[MAX_LIGHTNING];
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELLS];
extern DRIP_STRUCT Drips[MAX_DRIPS];
extern SMOKE_SPARKS smoke_spark[MAX_SMOKE_SPARKS];
extern BUBBLE_STRUCT Bubbles[MAX_BUBBLES];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVES];
extern FIRE_SPARKS fire_spark[MAX_FIRE_SPARKS];
extern BLOOD_STRUCT blood[MAX_BLOOD];
extern GUNFLASH_STRUCT Gunflashes[MAX_GUN_FLASHES];
extern FIRE_LIST fires[MAX_FIRES];
extern int32_t next_fire_spark;
extern int32_t next_smoke_spark;
extern int32_t next_gunshell;
extern int32_t next_bubble;
extern int32_t next_drip;
extern int32_t next_blood;
extern int16_t FlashFadeR;
extern int16_t FlashFadeG;
extern int16_t FlashFadeB;
extern int16_t FlashFader;
extern int16_t ScreenFade;
extern int16_t dScreenFade;
extern int16_t ScreenFadeBack;
extern int16_t ScreenFadedOut;
extern int16_t ScreenFading;
extern int16_t FadeScreenHeight;
extern int16_t DestFadeScreenHeight;
extern int16_t FadeClipSpeed;
extern int16_t ScreenFadeSpeed;
extern int8_t tsv_buffer[16384];
