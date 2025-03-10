#include "../../../tomb4/pch.h"
#include "big_scarab.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../effects.h"
#include "../../../specific/3dmath.h"
#include "../../control.h"
#include "../../../specific/output.h"
#include "../../effect2.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"

static BITE_INFO beetle_bite = { 0, 0, 0, 12 };

static int32_t next_scarab = 0;

void InitialiseBigScarab(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[BIG_BEETLE].anim_index + 3;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
}

void BigScarabControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* beetle;
	AI_INFO info;
	int16_t angle;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, BIG_BEETLE);

	angle = 0;
	item = &items[item_number];
	beetle = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0) {
		if (item->current_anim_state != 6) {
			if (item->current_anim_state == 7) {
				if (item->pos.y_pos >= item->floor) {
					item->pos.y_pos = item->floor;
					item->gravity_status = 0;
					item->fallspeed = 0;
					item->goal_anim_state = 8;
				}
			} else if (item->current_anim_state == 8) {
				item->pos.x_rot = 0;
				item->pos.y_pos = item->floor;
			} else {
				item->anim_number = objects[BIG_BEETLE].anim_index + 5;
				item->frame_number = anims[item->anim_number].frame_base;
				item->gravity_status = 1;
				item->current_anim_state = 6;
				item->speed = 0;
				item->pos.x_rot = 0;
			}
		}

		item->pos.x_rot = 0;
	} else {
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, true);

		if (beetle->flags)
			beetle->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, beetle->maximum_turn);

		if (info.distance > 0x900000 || !(GetRandomControl() & 0x7F) || item->hit_status)
			beetle->flags = 0;

		switch (item->current_anim_state) {
			case 1:
				item->pos.y_pos = item->floor;
				beetle->maximum_turn = DEGREES_TO_ROTATION(1);

				if (item->hit_status || info.distance < 0x900000 || beetle->hurt_by_lara || item->ai_bits == MODIFY)
					item->goal_anim_state = 2;

				break;

			case 3:
				beetle->maximum_turn = DEGREES_TO_ROTATION(7);

				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead) {
					if (info.distance < 0x10000)
						item->goal_anim_state = 9;
				}

				break;

			case 4:
				beetle->maximum_turn = DEGREES_TO_ROTATION(7);

				if (info.ahead) {
					if (info.distance < 0x10000)
						item->goal_anim_state = 4;
					else {
						item->goal_anim_state = 9;
						item->required_anim_state = 3;
					}
				} else if (info.distance < 0x10000)
					item->goal_anim_state = 9;
				else {
					item->goal_anim_state = 9;
					item->required_anim_state = 3;
				}

				if (!beetle->flags && item->touch_bits & 0x60) {
					lara_item->hit_points -= mod_object_customization->damage_1;
					lara_item->hit_status = 1;
					CreatureEffectT(item, &beetle_bite, 5, -1, DoBloodSplat);
					beetle->flags = 1;
				}

				break;

			case 5:
				beetle->flags = 0;
				item->pos.y_pos += 51;

				if (item->pos.y_pos > item->floor)
					item->pos.y_pos = item->floor;

				break;

			case 9:
				beetle->maximum_turn = DEGREES_TO_ROTATION(7);

				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (item->hit_status || GetRandomControl() < 384 || item->ai_bits == MODIFY ||
				         (beetle->mood == BORED_MOOD || GetRandomControl() < 128 && !beetle->hurt_by_lara && item->ai_bits != MODIFY))
					item->goal_anim_state = 3;
				else if (info.ahead && info.distance < 0x10000 && !beetle->flags)
					item->goal_anim_state = 4;

				break;
		}
	}

	CreatureTilt(item, angle * 2);
	CreatureAnimation(item_number, angle, angle);
}