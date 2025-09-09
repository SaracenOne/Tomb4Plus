#include "../../../tomb4/pch.h"
#include "bat.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../effects.h"
#include "../../lara.h"
#include "../../control.h"
#include "../../lot.h"
#include "../../../tomb4/mod_config.h"
#include "../../gameflow.h"

static BITE_INFO bat_bite = { 0, 16, 45, 4 };

void InitialiseBat(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[BAT].anim_index + 5;
	item->frame_number = anims[item->anim_number].frame_base;
	item->goal_anim_state = 6;
	item->current_anim_state = 6;
}

void BatControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	CREATURE_INFO* bat;
	CREATURE_INFO* baddie;
	AI_INFO info;
	int32_t max_dist, dist, dx, dz;
	int16_t angle;

	if (!CreatureActive(item_number))
		return;

	item = &items[item_number];
	bat = (CREATURE_INFO*)item->data;
	angle = 0;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, BAT);

	if (item->hit_points <= 0) {
		if (item->current_anim_state == 3) {
			item->anim_number = objects[BAT].anim_index + 1;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 2;
			item->goal_anim_state = 2;
		} else {
			if (item->pos.y_pos >= item->floor) {
				item->goal_anim_state = 5;
				item->pos.y_pos = item->floor;
				item->gravity_status = 0;
			} else {
				item->gravity_status = 1;
				item->speed = 0;
				item->anim_number = objects[BAT].anim_index + 3;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = 4;
				item->goal_anim_state = 4;
			}
		}
	} else {
		if (item->ai_bits)
			GetAITarget(bat);
		else {
			max_dist = 0x7FFFFFFF;

			for (int32_t i = 0; i < MAXIMUM_BADDIES; i++) {
				baddie = &baddie_slots[i];

				if (baddie->item_num != NO_ITEM && baddie->item_num != item_number) {
					item2 = &items[baddie->item_num];

					if (item2->object_number == VON_CROY) {
						dx = item2->pos.x_pos - item->pos.x_pos;
						dz = item2->pos.z_pos - item->pos.z_pos;
						dist = SQUARE(dx) + SQUARE(dz);

						if (dist < max_dist) {
							bat->enemy = item2;
							max_dist = dist;
						}
					}
				}
			}
		}

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, false);

		if (bat->flags)
			bat->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, false);
		angle = CreatureTurn(item, DEGREES_TO_ROTATION(20));

		switch (item->current_anim_state) {
			case 2:

				if (info.distance < 0x10000 || !(GetRandomControl() & 0x3F))
					bat->flags = 0;

				if (!bat->flags && (item->touch_bits || bat->enemy != lara_item && info.distance < 0x10000 &&
				                    info.ahead && abs(item->pos.y_pos - bat->enemy->pos.y_pos) < ((CLICK_SIZE * 3) + HALF_CLICK_SIZE)))
					item->goal_anim_state = 3;

				break;

			case 3:

				if (!bat->flags && item->touch_bits || bat->enemy != lara_item && info.distance < 0x10000 &&
				        info.ahead && abs(item->pos.y_pos - bat->enemy->pos.y_pos) < ((CLICK_SIZE * 3) + HALF_CLICK_SIZE)) {
					CreatureEffect(item, &bat_bite, DoBloodSplat);

					if (bat->enemy == lara_item) {
						lara_item->hit_points -= mod_object_customization->damage_1;
						lara_item->hit_status = 1;
					}

					bat->flags = 1;
				} else {
					item->goal_anim_state = 2;
					bat->mood = BORED_MOOD;
				}

				break;

			case 6:

				if (info.distance < 0x1900000 || item->hit_status || bat->hurt_by_lara)
					item->goal_anim_state = 1;

				break;
		}
	}

	CreatureAnimation(item_number, angle, 0);
}
