#include "../../../tomb4/pch.h"
#include "ahmet.h"
#include "../../gameflow.h"
#include "../../collide.h"
#include "../../lara_states.h"
#include "../../delstuff.h"
#include "../../tomb4fx.h"
#include "../../../specific/function_stubs.h"
#include "../../sound.h"
#include "../../items.h"
#include "../../control.h"
#include "../../objects.h"
#include "../../lot.h"
#include "../../switch.h"
#include "../../sphere.h"
#include "../../effect2.h"
#include "../../box.h"
#include "../../../specific/3dmath.h"
#include "../../people.h"
#include "../../effects.h"
#include "../../lara.h"
#include "../../../tomb4/mod_config.h"

int16_t ScalesBounds[] = {
	-(BLOCK_SIZE + CLICK_SIZE + HALF_CLICK_SIZE),
	-(HALF_BLOCK_SIZE + HALF_CLICK_SIZE),
	0,
	0,
	-HALF_BLOCK_SIZE,
	HALF_BLOCK_SIZE,
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10),
	-DEGREES_TO_ROTATION(30),
	DEGREES_TO_ROTATION(30),
	-DEGREES_TO_ROTATION(10),
	DEGREES_TO_ROTATION(10)
};

static BITE_INFO ahmet_bite = { 0, 0, 0, 11 };
static BITE_INFO ahmet_left_claw = { 0, 0, 0, 16 };
static BITE_INFO ahmet_right_claw = { 0, 0, 0, 22 };

void ScalesCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	DRIP_STRUCT* drip;
	PHD_VECTOR pos;
	int16_t roty;

	item = &items[item_number];

	if (TestBoundsCollide(item, l, 100)) {
		if ((l->anim_number == ANIM_POURWATERSKIN || l->anim_number == ANIM_FILLSCALE) && item->current_anim_state == 1) {
			roty = item->pos.y_rot;
			item->pos.y_rot = (l->pos.y_rot + 0x2000) & 0xC000;
			ScalesBounds[0] = -(BLOCK_SIZE + CLICK_SIZE + HALF_CLICK_SIZE);
			ScalesBounds[1] = -(HALF_BLOCK_SIZE + HALF_CLICK_SIZE);
			ScalesBounds[4] = -HALF_BLOCK_SIZE;
			ScalesBounds[5] = 0;

			if (TestLaraPosition(ScalesBounds, item, l)) {
				if (l->anim_number == ANIM_POURWATERSKIN) {
					l->anim_number = ANIM_FILLSCALE;
					l->frame_number = anims[ANIM_FILLSCALE].frame_base;
				} else if (l->frame_number == anims[ANIM_FILLSCALE].frame_base + 51)
					SoundEffect(SFX_POUR, &l->pos, SFX_DEFAULT);
				else if (l->frame_number == anims[ANIM_FILLSCALE].frame_base + 74) {
					AddActiveItem(item_number);
					item->status = ITEM_ACTIVE;

					if (l->item_flags[3] < item->trigger_flags)			//too little
						item->goal_anim_state = 4;
					else if (l->item_flags[3] == item->trigger_flags)	//nice
						item->goal_anim_state = 2;
					else												//too much
						item->goal_anim_state = 3;
				}
			}

			item->pos.y_rot = roty;
		} else {
			GlobalCollisionBounds[0] = (HALF_BLOCK_SIZE + HALF_CLICK_SIZE);
			GlobalCollisionBounds[1] = (BLOCK_SIZE + CLICK_SIZE);
			GlobalCollisionBounds[2] = -(BLOCK_SIZE + CLICK_SIZE);
			GlobalCollisionBounds[3] = 0;
			GlobalCollisionBounds[4] = -CLICK_SIZE;
			GlobalCollisionBounds[5] = (CLICK_SIZE + HALF_CLICK_SIZE);
			ItemPushLara(item, l, coll, 0, 2);
			GlobalCollisionBounds[0] = -CLICK_SIZE;
			GlobalCollisionBounds[1] = CLICK_SIZE;
			ItemPushLara(item, l, coll, 0, 2);
			GlobalCollisionBounds[0] = -(BLOCK_SIZE + CLICK_SIZE);
			GlobalCollisionBounds[1] = -(HALF_BLOCK_SIZE + HALF_CLICK_SIZE);
			ItemPushLara(item, l, coll, 0, 2);
		}
	}

	if (l->frame_number >= anims[ANIM_POURWATERSKIN].frame_base + 44 && l->frame_number <= anims[ANIM_POURWATERSKIN].frame_base + 72 ||
	        l->frame_number >= anims[ANIM_FILLSCALE].frame_base + 51 && l->frame_number <= anims[ANIM_FILLSCALE].frame_base + 74) {
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		GetLaraJointPos(&pos, LMX_HAND_L);
		drip = &Drips[GetFreeDrip()];
		drip->x = pos.x;
		drip->y = pos.y;
		drip->z = pos.z;
		drip->On = 1;
		drip->R = (GetRandomControl() & 0xF) + 24;
		drip->G = (GetRandomControl() & 0xF) + 44;
		drip->B = (GetRandomControl() & 0xF) + 56;
		drip->Yvel = (GetRandomControl() & 0x1F) + 32;
		drip->Gravity = (GetRandomControl() & 0x1F) + 32;
		drip->Life = (GetRandomControl() & 0x1F) + 16;
		drip->RoomNumber = l->room_number;
	}
}

int32_t ReTriggerAhmet(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->current_anim_state == 7 && item->frame_number == anims[item->anim_number].frame_end) {
		FlashFadeR = 255;
		FlashFadeG = 64;
		FlashFadeB = 0;
		FlashFader = 32;
		item->pos.x_pos = ((item->item_flags[0] & 0xFFFF) << 10) | HALF_BLOCK_SIZE;
		item->pos.y_pos = (item->item_flags[1] & 0xFFFF) << 8;
		item->pos.z_pos = ((item->item_flags[2] & 0xFFFF) << 10) | HALF_BLOCK_SIZE;
		IsRoomOutside(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (item->room_number != IsRoomOutsideNo)
			ItemNewRoom(item_number, IsRoomOutsideNo);

		item->anim_number = objects[AHMET].anim_index;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
		item->hit_points = objects[AHMET].hit_points;
		AddActiveItem(item_number);
		item->flags &= ~IFL_INVISIBLE;
		item->after_death = 0;
		item->collidable = 1;
		item->status = ITEM_ACTIVE;
		EnableBaddieAI(item_number, 1);
		item->trigger_flags = 1;
		return 1;
	}

	return 0;
}

void ScalesControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	FLOOR_INFO* floor;
	int32_t flags, numTriggers;
	int16_t itemNos[8];
	int16_t room_number;

	item = &items[item_number];

	if (item->frame_number == anims[item->anim_number].frame_end) {
		if (item->current_anim_state == 1 || item->item_flags[1]) {
			if (item->anim_number == objects[item->object_number].anim_index) {
				RemoveActiveItem(item_number);
				item->status = ITEM_INACTIVE;
				item->item_flags[1] = 0;
			} else if (ReTriggerAhmet(lara.spaz_effect_count)) {
				for (numTriggers = GetSwitchTrigger(item, itemNos, 0); numTriggers > 0; numTriggers--) {
					item2 = &items[itemNos[numTriggers - 1]];

					if (item2->object_number != FLAME_EMITTER2)
						item2->flags = 0x400;
				}

				item->goal_anim_state = 1;
			}
		} else {
			if (item->current_anim_state == 2) {
				flags = -0x200;
				RemoveActiveItem(item_number);
				item->status = ITEM_INACTIVE;
			} else {
				flags = -0x400;
				item->item_flags[1] = 1;
			}

			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
			TestTriggers(trigger_index, true, flags);
		}
	}

	AnimateItem(item);
}

void ExplodeAhmet(ITEM_INFO* item) {
	SPHERE* sphere;
	int32_t spheres;

	if (!(wibble & 7)) {
		for (spheres = GetSpheres(item, Slist, 1); spheres > 0; spheres--) {
			sphere = &Slist[spheres];
			TriggerFireFlame(sphere->x, sphere->y, sphere->z, -1, 1);
		}
	}

	TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 13, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
	SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);
}

void InitialiseAhmet(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[AHMET].anim_index;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
	item->item_flags[0] = int16_t(item->pos.x_pos >> 10);
	item->item_flags[1] = int16_t(item->pos.y_pos >> 8);
	item->item_flags[2] = int16_t(item->pos.z_pos >> 10);
}

void AhmetControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* enemy;
	CREATURE_INFO* ahmet;
	FLOOR_INFO* floor;
	AI_INFO info;
	AI_INFO larainfo;
	int32_t dx, dz;
	int16_t angle, head, room_number, frame, base;

	if (!CreatureActive(item_number))
		return;

	item = &items[item_number];

	if (item->trigger_flags == 1) {
		item->trigger_flags = 0;
		return;
	}

	ahmet = (CREATURE_INFO*)item->data;
	angle = 0;
	head = 0;

	MOD_LEVEL_CREATURE_INFO *mod_creature_info = get_game_mod_level_creature_info(gfCurrentLevel);
	MOD_LEVEL_OBJECT_CUSTOMIZATION* mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, AHMET);

	if (item->hit_points <= 0) {
		if (item->current_anim_state == 7) {
			if (!mod_creature_info->remove_ahmet_death_loop) {
				if (item->frame_number == anims[item->anim_number].frame_end) {
					item->frame_number = anims[item->anim_number].frame_end - 1;
					return;
				}
			}
		} else {
			item->anim_number = objects[item->object_number].anim_index + 10;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 7;
			lara.spaz_effect_count = item_number;
		}

		if (!mod_creature_info->remove_ahmet_death_flames) {
			ExplodeAhmet(item);
		}
	} else {
		if (item->ai_bits)
			GetAITarget(ahmet);

		CreatureAIInfo(item, &info);

		if (ahmet->enemy == lara_item) {
			larainfo.angle = info.angle;
			larainfo.distance = info.distance;
		} else {
			dx = lara_item->pos.x_pos - item->pos.x_pos;
			dz = lara_item->pos.z_pos - item->pos.z_pos;
			larainfo.angle = int16_t(phd_atan(dz, dx) - item->pos.y_rot);
			larainfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, true);
		CreatureMood(item, &info, true);
		angle = CreatureTurn(item, ahmet->maximum_turn);
		enemy = ahmet->enemy;
		ahmet->enemy = lara_item;

		if (larainfo.distance < 0x100000 || item->hit_status || TargetVisible(item, &larainfo))
			AlertAllGuards(item_number);

		ahmet->enemy = enemy;
		frame = item->frame_number;
		base = anims[item->anim_number].frame_base;

		switch (item->current_anim_state) {
			case 1:
				ahmet->maximum_turn = 0;
				ahmet->flags = 0;

				if (item->ai_bits & GUARD) {
					head = AIGuard(ahmet);
					item->goal_anim_state = 1;
				} else if (item->ai_bits & PATROL1) {
					item->goal_anim_state = 2;
					head = 0;
				} else if (ahmet->mood == BORED_MOOD || ahmet->mood == ESCAPE_MOOD) {
					if (lara.target != item && info.ahead)
						item->goal_anim_state = 1;
					else
						item->goal_anim_state = 3;
				} else if (info.bite && info.distance < 0x718E4)
					item->goal_anim_state = 4;
				else if (info.angle >= 0x2000 || info.angle <= -2000 || info.distance >= 0x190000) {
					if (item->required_anim_state)
						item->goal_anim_state = item->required_anim_state;
					else if (info.ahead && info.distance < 0x640000)
						item->goal_anim_state = 2;
					else
						item->goal_anim_state = 3;
				} else if (GetRandomControl() & 1)
					item->goal_anim_state = 5;
				else
					item->goal_anim_state = 6;

				break;

			case 2:
				ahmet->maximum_turn = DEGREES_TO_ROTATION(5);

				if (item->ai_bits & PATROL1) {
					item->goal_anim_state = 2;
					head = 0;
				} else if (info.bite && info.distance < 0x190000)
					item->goal_anim_state = 1;
				else
					item->goal_anim_state = 3;

				break;

			case 3:
				ahmet->maximum_turn = DEGREES_TO_ROTATION(8);

				if (item->ai_bits & GUARD || ahmet->mood == BORED_MOOD || ahmet->mood == ESCAPE_MOOD &&
				        lara.target != item && info.ahead || info.bite && info.distance < 0x190000)
					item->goal_anim_state = 1;
				else if (info.distance < 0x640000 && info.ahead && (info.enemy_facing < -0x4000 || info.enemy_facing > 0x4000))
					item->goal_anim_state = 2;

				ahmet->flags = 0;
				break;

			case 4:
				ahmet->maximum_turn = 0;

				if (abs(info.angle) < DEGREES_TO_ROTATION(5))
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= DEGREES_TO_ROTATION(5);
				else
					item->pos.y_rot += DEGREES_TO_ROTATION(5);

				if (frame > base + 7 && !(ahmet->flags & 1) && item->touch_bits & 0x3C000) {
					lara_item->hit_status = 1;
					lara_item->hit_points -= mod_object_customization->damage_1;
					CreatureEffectT(item, &ahmet_left_claw, 10, -1, DoBloodSplat);
					ahmet->flags |= 1;
				} else if (frame > base + 32 && !(ahmet->flags & 2) && item->touch_bits & 0xF00000) {
					lara_item->hit_status = 1;
					lara_item->hit_points -= mod_object_customization->damage_1;
					CreatureEffectT(item, &ahmet_right_claw, 10, -1, DoBloodSplat);
					ahmet->flags |= 2;
				}

				break;

			case 5:
				ahmet->maximum_turn = 0;

				if (item->anim_number == objects[AHMET].anim_index + 3) {
					if (abs(info.angle) < DEGREES_TO_ROTATION(5))
						item->pos.y_rot += info.angle;
				} else if (!ahmet->flags && item->anim_number == objects[AHMET].anim_index + 4 && frame > base + 11 && item->touch_bits & 0xC00) {
					lara_item->hit_status = 1;
					lara_item->hit_points -= mod_object_customization->damage_2;
					CreatureEffectT(item, &ahmet_bite, 20, -1, DoBloodSplat);
					ahmet->flags = 1;
				}

				break;

			case 6:
				ahmet->maximum_turn = 0;


				if (item->anim_number == objects[AHMET].anim_index + 7) {
					if (abs(info.angle) < DEGREES_TO_ROTATION(5))
						item->pos.y_rot += info.angle;
					else if (info.angle < 0)
						item->pos.y_rot -= DEGREES_TO_ROTATION(5);
					else
						item->pos.y_rot += DEGREES_TO_ROTATION(5);
				} else if (frame > base + 21 && !(ahmet->flags & 1) && item->touch_bits & 0x3C000) {
					lara_item->hit_status = 1;
					lara_item->hit_points -= mod_object_customization->damage_1;
					CreatureEffectT(item, &ahmet_left_claw, 10, -1, DoBloodSplat);
					ahmet->flags |= 1;
				} else if (frame > base + 14 && !(ahmet->flags & 2) && item->touch_bits & 0xF00000) {
					lara_item->hit_status = 1;
					lara_item->hit_points -= mod_object_customization->damage_1;
					CreatureEffectT(item, &ahmet_right_claw, 10, -1, DoBloodSplat);
					ahmet->flags |= 2;
				}

				break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, head);
	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	if (!mod_creature_info->disable_ahmet_heavy_trigger)
		TestTriggers(trigger_index, true, 0);
	CreatureAnimation(item_number, angle, 0);
}
