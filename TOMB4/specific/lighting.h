#pragma once
#include "../global/types.h"

void S_CalculateStaticMeshLight(int32_t x, int32_t y, int32_t z, int32_t shade, ROOM_INFO* r);
void InitItemDynamicLighting(ITEM_INFO* item);
void SetupDynamicLight(DYNAMIC* light, ITEM_INFO* item);
void SetupLight(PCLIGHT* light, ITEM_INFO* item, bool point_lights_affect_ambience, bool use_alt_attenuation_calculation, int32_t* ambient);
void mApplyMatrix(float* matrix, FVECTOR* start, FVECTOR* dest);
void mApplyTransposeMatrix(float* matrix, FVECTOR* start, FVECTOR* dest);
void CreateLightList(ITEM_INFO* item);
void FadeLightList(PCLIGHT* lights, int32_t nLights);
void InitObjectLighting(ITEM_INFO* item);
void CalcAmbientLight(ITEM_INFO* item);
void ResetLighting();

extern ITEM_INFO* current_item;
extern int32_t StaticMeshShade;
extern int32_t ambientR, ambientG, ambientB;

extern FVECTOR lGlobalMeshPos;
extern SUNLIGHT_STRUCT SunLights[64];
extern POINTLIGHT_STRUCT PointLights[64];
extern POINTLIGHT_STRUCT SpotLights[64];
extern int32_t nSunLights, nPointLights, nSpotLights, nShadowLights, nTotalLights;
