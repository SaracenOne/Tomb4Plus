#pragma once

#define MAX_PICKUP_DISPLAYABLE_LIFETIME 45

extern void T4PlusSetValidLaraGunType();

extern int32_t T4PlusGetInventoryCount(int16_t object_number);
extern void T4PlusSetInventoryCount(int16_t object_number, int32_t count, bool update_weapon_state);
extern void T4ShowObjectPickup(int32_t object_number, int16_t displayable_lifetime);