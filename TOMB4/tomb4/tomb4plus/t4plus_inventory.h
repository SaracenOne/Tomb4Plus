#pragma once

#define MAX_PICKUP_DISPLAYABLE_LIFETIME 45

extern void T4PlusSetValidLaraGunType();

extern int T4PlusGetInventoryCount(int16_t object_number);
extern void T4PlusSetInventoryCount(int16_t object_number, int count, bool update_weapon_state);
extern void T4ShowObjectPickup(int object_number, int16_t displayable_lifetime);