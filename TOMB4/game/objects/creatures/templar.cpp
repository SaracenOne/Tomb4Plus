#include "../../../tomb4/pch.h"
#include "../../gameflow.h"
#include "templar.h"
#include "../../box.h"
#include "../../objects.h"
#include "../../../specific/function_stubs.h"
#include "../../sphere.h"
#include "../../effect2.h"
#include "../../../specific/3dmath.h"
#include "../../effects.h"
#include "../../debris.h"
#include "../../sound.h"
#include "../../control.h"
#include "../../lara.h"

#include "../../../tomb4/mod_config.h"
#include "../../trng/trng.h"

static BITE_INFO templar_hit{ 0, 0, 0, 11 };

void InitialiseTemplar(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	InitialiseCreature(item_number);
	item->anim_number = objects[KNIGHTS_TEMPLAR].anim_index + 2;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = 1;
	item->goal_anim_state = 1;
	item->mesh_bits &= ~0x800;
}

void TemplarControl(int16_t item_number) {
	ITEM_INFO* item;
	CREATURE_INFO* knight;
	ROOM_INFO* r;
	MESH_INFO* mesh;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	AI_INFO info;
	int16_t anim, head, torso_x, torso_y, angle;

	if (!CreatureActive(item_number))
		return;

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, KNIGHTS_TEMPLAR);

	item = &items[item_number];
	anim = item->anim_number - objects[KNIGHTS_TEMPLAR].anim_index;

	if ((!anim || anim == 1 || anim == 11 || anim == 12) && (GetRandomControl() & 1) != 0) {
		MOD_LEVEL_CREATURE_INFO *creature_info = get_game_mod_level_creature_info(gfCurrentLevel);

		if (!creature_info->remove_knights_templar_sparks) {
			pos.x = 0;
			pos.y = QUARTER_CLICK_SIZE - (QUARTER_CLICK_SIZE / 4);
			pos.z = HALF_BLOCK_SIZE - QUARTER_CLICK_SIZE;
			GetJointAbsPosition(item, &pos, 10);
			TriggerFlareSparks(pos.x, pos.y, pos.z, (GetRandomControl() & 0x1FF) - 256, -128 - (GetRandomControl() & 0x7F), (GetRandomControl() & 0x1FF) - 256, 0);
		}
	}

	knight = (CREATURE_INFO*)item->data;
	head = 0;
	torso_x = 0;
	torso_y = 0;

	if (item->hit_points < objects[KNIGHTS_TEMPLAR].hit_points)
		item->hit_points = objects[KNIGHTS_TEMPLAR].hit_points;

	if (item->ai_bits)
		GetAITarget(knight);
	else if (knight->hurt_by_lara)
		knight->enemy = lara_item;

	CreatureAIInfo(item, &info);

	if (knight->enemy != lara_item)
		phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

	GetCreatureMood(item, &info, true);
	CreatureMood(item, &info, true);
	angle = CreatureTurn(item, knight->maximum_turn);

	if (info.ahead) {
		head = info.angle >> 1;
		torso_y = info.angle >> 1;
		torso_x = info.x_angle;
	}

	switch (item->current_anim_state) {
		case 1:
			knight->flags = 0;
			knight->maximum_turn = DEGREES_TO_ROTATION(2);

			if (info.distance > 0x718E4) {
				if (lara.target == item)
					item->goal_anim_state = 6;
				else
					item->goal_anim_state = 2;
			} else if (GetRandomControl() & 1)
				item->goal_anim_state = 4;
			else if (GetRandomControl() & 1)
				item->goal_anim_state = 3;
			else
				item->goal_anim_state = 5;

			break;

		case 2:
			knight->maximum_turn = DEGREES_TO_ROTATION(7);

			if (lara.target == item || info.distance <= 0x718E4)
				item->goal_anim_state = 1;

			break;

		case 3:
		case 4:
		case 5:
			knight->maximum_turn = 0;

			if (abs(info.angle) < DEGREES_TO_ROTATION(1))
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= DEGREES_TO_ROTATION(1);
			else
				item->pos.y_rot += DEGREES_TO_ROTATION(1);

			if (item->frame_number > anims[item->anim_number].frame_base + 42 && item->frame_number < anims[item->anim_number].frame_base + 51) {
				r = &room[item->room_number];
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				GetJointAbsPosition(item, &pos, 11);

				floor = &r->floor[((pos.z - r->z) >> WALL_SHIFT) + r->x_size * ((pos.x - r->x) >> WALL_SHIFT)];

				if (floor->stopper) {
					for (int32_t i = 0; i < r->num_meshes; i++) {
						mesh = &r->mesh[i];

						MOD_LEVEL_STATIC_INFO* static_info = &get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number];
						if (!((pos.z ^ mesh->z) & ~0x3FF) && !((pos.x ^ mesh->x) & ~0x3FF) && static_info->creatures_can_shatter) {
							ShatterObject(0, mesh, -64, lara_item->room_number, 0);
							if (static_info->shatter_sound_id >= 0) {
								SoundEffect(static_info->shatter_sound_id, &item->pos, SFX_DEFAULT);
							}
							mesh->Flags &= ~1;
							floor->stopper = 0;
							GetHeight(floor, pos.x, pos.y, pos.z);
							TestTriggers(trigger_index, true, 0);
						}
					}
				}

				if (!knight->flags && item->touch_bits & 0xC00) {
					lara_item->hit_points -= mod_object_customization->damage_1;
					lara_item->hit_status = 1;
					CreatureEffectT(item, &templar_hit, 20, -1, DoBloodSplat);
					knight->flags = 1;
				}
			}

			break;

		case 6:
			knight->maximum_turn = 0;

			if (abs(info.angle) < DEGREES_TO_ROTATION(1))
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= DEGREES_TO_ROTATION(1);
			else
				item->pos.y_rot += DEGREES_TO_ROTATION(1);

			if (item->hit_status) {
				if (GetRandomControl() & 1)
					item->goal_anim_state = 7;
				else
					item->goal_anim_state = 8;
			} else if (info.distance > 465124 && lara.target == item)
				item->goal_anim_state = 6;
			else
				item->goal_anim_state = 1;

			break;
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);
	CreatureAnimation(item_number, angle, 0);
}
