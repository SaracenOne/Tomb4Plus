#include "../../../tomb4/pch.h"
#include "laradouble.h"
#include "../../box.h"
#include "../../sound.h"
#include "../../control.h"
#include "../../lara.h"

#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"

void InitialiseLaraDouble(int16_t item_number) {
	InitialiseCreature(item_number);
}

void LaraDoubleControl(int16_t item_number) {
	ITEM_INFO* item;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, LARA_DOUBLE);

	item = &items[item_number];
	SoundEffect(SFX_METAL_SCRAPE_LOOP1, &item->pos, SFX_DEFAULT);

	if (!CreatureActive(item_number))
		return;

	if (item->hit_status)
		lara_item->hit_points += item->hit_points - mod_object_customization->damage_1;

	item->hit_points = DEFAULT_LARA_MAX_HEALTH;
	AnimateItem(item);
}
