#pragma once
#include "../global/vars.h"

void inject_tomb4fx(bool replace);

LIGHTNING_STRUCT* TriggerLightning(PHD_VECTOR* s, PHD_VECTOR* d, char variation, long rgb, uchar flags, uchar size, uchar segments);
long ExplodingDeath2(short item_number, long mesh_bits, short Flags);

#define ClearFires	( (void(__cdecl*)()) 0x004384F0 )
#define UpdateFadeClip	( (void(__cdecl*)()) 0x00439D60 )
#define TriggerLaraDrips	( (void(__cdecl*)()) 0x0043A080 )
#define UpdateFireSparks	( (void(__cdecl*)()) 0x00437F20 )
#define UpdateSmokeSparks	( (void(__cdecl*)()) 0x00438700 )
#define UpdateBubbles	( (void(__cdecl*)()) 0x00439970 )
#define UpdateBlood	( (void(__cdecl*)()) 0x00438D90 )
#define UpdateDrips	( (void(__cdecl*)()) 0x00439F80 )
#define UpdateGunShells	( (void(__cdecl*)()) 0x00439340 )
#define UpdateShockwaves	( (void(__cdecl*)()) 0x0043AD10 )
#define UpdateLightning	( (void(__cdecl*)()) 0x0043AF80 )
#define TriggerLightningGlow	( (void(__cdecl*)(long, long, long, long)) 0x0043B330 )
#define	SetFadeClip	( (void(__cdecl*)(short, short)) 0x00439D40 )
#define	SetScreenFadeOut	( (void(__cdecl*)(long, long)) 0x00439DB0 )
#define	SetScreenFadeIn	( (void(__cdecl*)(long)) 0x00439E00 )
