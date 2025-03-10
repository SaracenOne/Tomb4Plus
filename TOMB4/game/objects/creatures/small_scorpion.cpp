#include "../../../tomb4/pch.h"
#include "small_scorpion.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/3dmath.h"
#include "../../control.h"
#include "../../lot.h"
#include "../../items.h"
#include "../../../specific/function_stubs.h"
#include "../../effects.h"
#include "../../lara.h"
#include "../../deltapak.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"
#include "../../../tomb4/tomb4plus/t4plus_objects.h"

static BITE_INFO s_stinger{ 0, 0, 0, 8 };
static BITE_INFO s_pincer{ 0, 0, 0, 23 };

void InitialiseSmallScorpion(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[SMALL_SCORPION].anim_index + 2;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
}

void SmallScorpionControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* enemy;
	CREATURE_INFO* scorpion;
	AI_INFO info;
	int16_t angle;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, SMALL_SCORPION);

	angle = 0;
	item = &items[item_number];
	scorpion = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0) {
		item->hit_points = 0;

		if (item->current_anim_state != 6 && item->current_anim_state != 7) {
			item->anim_number = objects[SMALL_SCORPION].anim_index + 5;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 6;
		}
	} else {
		if (item->ai_bits)
			GetAITarget(scorpion);
		else
			scorpion->enemy = lara_item;

		CreatureAIInfo(item, &info);
		enemy = scorpion->enemy;

		if (enemy != lara_item)
			phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, scorpion->maximum_turn);

		switch (item->current_anim_state) {
			case 1:
				scorpion->maximum_turn = 0;
				scorpion->flags = 0;

				if (info.distance > 0x1C6E39)
					item->goal_anim_state = 2;
				else if (info.bite) {
					scorpion->maximum_turn = DEGREES_TO_ROTATION(6);

					if (GetRandomControl() & 1 || enemy->object_number == TROOPS && enemy->hit_points <= 2)
						item->goal_anim_state = 4;
					else
						item->goal_anim_state = 5;
				} else if (!info.ahead)
					item->goal_anim_state = 2;

				break;

			case 2:
				scorpion->maximum_turn = DEGREES_TO_ROTATION(6);

				if (info.distance >= 0x1C639)
					item->goal_anim_state = 3;
				else
					item->goal_anim_state = 1;

				break;

			case 3:
				scorpion->maximum_turn = DEGREES_TO_ROTATION(8);

				if (info.distance < 0x1C639)
					item->goal_anim_state = 1;

				break;

			case 4:
			case 5:
				scorpion->maximum_turn = 0;

				if (abs(info.angle) < DEGREES_TO_ROTATION(6))
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= DEGREES_TO_ROTATION(6);
				else
					item->pos.y_rot += DEGREES_TO_ROTATION(6);

				if (!scorpion->flags && item->touch_bits & 0x1B00100) {
					if (item->frame_number > anims[item->anim_number].frame_base + 20 && item->frame_number < anims[item->anim_number].frame_base + 32) {
						lara_item->hit_points -= mod_object_customization->damage_1;
						lara_item->hit_status = 1;

						if (item->current_anim_state == 5) {
							// Tomb4Plus
							MOD_LEVEL_CREATURE_INFO *creature_info = get_game_mod_level_creature_info(gfCurrentLevel);
							if (creature_info->small_scorpion_is_poisonous)
								lara.dpoisoned += creature_info->small_scorpion_poison_strength;

							CreatureEffectT(item, &s_stinger, 3, item->pos.y_rot + 0x8000, DoBloodSplat);
						} else
							CreatureEffectT(item, &s_pincer, 3, item->pos.y_rot + 0x8000, DoBloodSplat);

						scorpion->flags = 1;
					}
				}

				break;
		}
	}

	CreatureAnimation(item_number, angle, 0);
}
