#pragma once
#include "../global/vars.h"

void inject_box(bool replace);

void CreatureDie(short item_number, long explode);

#define AlertNearbyGuards	( (void(__cdecl*)(ITEM_INFO*)) 0x004425D0 )
