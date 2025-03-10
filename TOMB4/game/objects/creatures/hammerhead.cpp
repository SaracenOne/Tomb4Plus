#include "../../../tomb4/pch.h"
#include "hammerhead.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/3dmath.h"
#include "../../effects.h"
#include "../../control.h"
#include "../../lara.h"
#include "../../gameflow.h"
#include "../../../tomb4/mod_config.h"

static BITE_INFO hammerhead_bite = { 0, 0, 0, 12 };

void InitialiseHammerhead(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[HAMMERHEAD].anim_index + 8;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 0;
	item->goal_anim_state = 0;
}

void HammerheadControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* shark;
	AI_INFO info;
	int16_t angle;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, HAMMERHEAD);

	item = &items[item_number];
	shark = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0) {
		item->hit_points = 0;

		if (item->current_anim_state != 5) {
			item->anim_number = objects[HAMMERHEAD].anim_index + 4;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 5;
		}

		CreatureFloat(item_number);
	} else {
		if (item->ai_bits)
			GetAITarget(shark);
		else if (shark->hurt_by_lara)
			shark->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (shark->enemy != lara_item)
			phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, shark->maximum_turn);

		switch (item->current_anim_state) {
			case 0:
				item->goal_anim_state = 1;
				shark->flags = 0;
				break;

			case 1:
				shark->maximum_turn = DEGREES_TO_ROTATION(7);

				if (info.distance > 0x100000)
					item->goal_anim_state = 2;
				else if (info.distance < 0x718E4)
					item->goal_anim_state = 3;

				break;

			case 2:

				if (info.distance < 0x100000)
					item->goal_anim_state = 1;

				break;

			case 3:

				if (!shark->flags && item->touch_bits & 0x3400) {
					lara_item->hit_points -= mod_object_customization->damage_1;
					lara_item->hit_status = 1;
					CreatureEffect(item, &hammerhead_bite, DoBloodSplat);
					shark->flags = 1;
				}

				break;
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, -2 * angle);
		CreatureJoint(item, 1, -2 * angle);
		CreatureJoint(item, 2, -2 * angle);
		CreatureJoint(item, 3, 2 * angle);

		if (item->current_anim_state == 6)
			AnimateItem(item);
		else {
			CreatureAnimation(item_number, angle, 0);
			CreatureUnderwater(item, 341);
		}
	}
}
