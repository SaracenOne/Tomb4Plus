#include "../../../tomb4/pch.h"
#include "doberman.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../effects.h"
#include "../../../specific/function_stubs.h"
#include "../../control.h"
#include "../../lara.h"

static BITE_INFO doberman_bite = { 0, 30, 141, 20 };

#define DOBERMAN_LAY_DOWN_ANIMATION 6
#define DOBERMAN_STAND_IDLE_ANIMATION 10
#define DOBERMAN_DEATH_ANIMATION 13

void InitialiseDoberman(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->trigger_flags) {
		item->current_anim_state = 5;
		item->anim_number = objects[item->object_number].anim_index + DOBERMAN_LAY_DOWN_ANIMATION;
		item->status -= ITEM_INVISIBLE;
	} else {
		item->current_anim_state = 6;
		item->anim_number = objects[item->object_number].anim_index + DOBERMAN_STAND_IDLE_ANIMATION;
	}

	item->frame_number = anims[item->anim_number].frame_base;
}

void DobermanControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* dog;
	AI_INFO info;
	int16_t angle, head, head_x, torso_y, tilt, random;

	if (!CreatureActive(item_number))
		return;

	angle = 0;
	tilt = 0;
	head_x = 0;
	head = 0;
	torso_y = 0;
	item = &items[item_number];
	dog = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0) {
		if (item->current_anim_state != 10) {
			item->anim_number = objects[DOG].anim_index + DOBERMAN_DEATH_ANIMATION;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 10;
		}
	} else {
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, false);
		CreatureMood(item, &info, false);
		angle = CreatureTurn(item, dog->maximum_turn);

		switch (item->current_anim_state) {
			case 1:
				dog->maximum_turn = DEGREES_TO_ROTATION(3);

				if (dog->mood == BORED_MOOD) {
					random = (int16_t)GetRandomControl();

					if (random < 768) {
						item->goal_anim_state = 3;
						item->required_anim_state = 4;
					} else if (random < 1536) {
						item->goal_anim_state = 3;
						item->required_anim_state = 5;
					} else if (random < 2816)
						item->goal_anim_state = 3;
				} else
					item->goal_anim_state = 2;

				break;

			case 2:
				tilt = angle;
				dog->maximum_turn = DEGREES_TO_ROTATION(6);

				if (dog->mood == BORED_MOOD)
					item->goal_anim_state = 3;
				else if (info.distance < 0x90000)
					item->goal_anim_state = 8;

				break;

			case 3:
				dog->maximum_turn = 0;
				dog->flags = 0;

				if (dog->mood == BORED_MOOD) {
					if (item->required_anim_state)
						item->goal_anim_state = item->required_anim_state;
					else {
						random = (int16_t)GetRandomControl();

						if (random < 768)
							item->goal_anim_state = 4;
						else if (random < 1536)
							item->goal_anim_state = 5;
						else if (random < 9728)
							item->goal_anim_state = 1;
					}
				} else if (dog->mood == ESCAPE_MOOD || info.distance >= 0x1C639 || !info.ahead)
					item->goal_anim_state = 2;
				else
					item->goal_anim_state = 7;

				break;

			case 4:

				if (dog->mood != BORED_MOOD || GetRandomControl() < 1280)
					item->goal_anim_state = 3;

				break;

			case 5:

				if (dog->mood != BORED_MOOD || GetRandomControl() < 256)
					item->goal_anim_state = 3;

				break;

			case 6:

				if (dog->mood != BORED_MOOD || GetRandomControl() < 512)
					item->goal_anim_state = 3;

				break;

			case 7:
				dog->maximum_turn = HALF_DEGREES_TO_ROTATION(1);

				if (dog->flags != 1 && info.ahead && item->touch_bits & 0x122000) {
					CreatureEffect(item, &doberman_bite, DoBloodSplat);
					lara_item->hit_points -= 30;
					lara_item->hit_status = 1;
					dog->flags = 1;
				}

				if (info.distance > 0x1C639 && info.distance < 0x718E4)
					item->goal_anim_state = 9;
				else
					item->goal_anim_state = 3;

				break;

			case 8:

				if (dog->flags != 2 && item->touch_bits & 0x122000) {
					CreatureEffect(item, &doberman_bite, DoBloodSplat);
					lara_item->hit_points -= 80;
					lara_item->hit_status = 1;
					dog->flags = 2;
				}

				if (info.distance < 0x1C639)
					item->goal_anim_state = 7;
				else if (info.distance < 0x718E4)
					item->goal_anim_state = 9;

				break;

			case 9:
				dog->maximum_turn = DEGREES_TO_ROTATION(6);

				if (dog->flags != 3 && item->touch_bits & 0x122000) {
					CreatureEffect(item, &doberman_bite, DoBloodSplat);
					lara_item->hit_points -= 50;
					lara_item->hit_status = 1;
					dog->flags = 3;
				}

				if (info.distance < 0x1C639)
					item->goal_anim_state = 7;

				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, head);
	CreatureJoint(item, 2, head_x);
	CreatureAnimation(item_number, angle, tilt);
}
