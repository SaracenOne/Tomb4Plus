#include "../../tomb4/pch.h"

#include "../../specific/function_stubs.h"
#include "../../specific/dxshell.h"
#include "../../specific/file.h"
#include "../control.h"
#include "../effects.h"
#include "../objects.h"
#include "../door.h"
#include "../items.h"
#include "../lot.h"
#include "../lara.h"
#include "../newinv.h"
#include "trng.h"
#include "trng_arithmetic.h"
#include "trng_condition.h"
#include "trng_extra_state.h"

// TODO: there may be some missing types still needing support
int get_inventory_count(short object_number)
{
	if (object_number >= PUZZLE_ITEM1_COMBO1 && object_number <= PUZZLE_ITEM8_COMBO2)
		return (lara.puzzleitemscombo >> (object_number - PUZZLE_ITEM1_COMBO1)) & 1;
	else if (object_number >= PUZZLE_ITEM1 && object_number <= PUZZLE_ITEM12)
		return lara.puzzleitems[object_number - PUZZLE_ITEM1];
	else if (object_number >= KEY_ITEM1_COMBO1 && object_number <= KEY_ITEM8_COMBO2)
		return (lara.keyitemscombo >> (object_number - KEY_ITEM1_COMBO1)) & 1;
	else if (object_number >= KEY_ITEM1 && object_number <= KEY_ITEM12)
		return (lara.keyitems >> (object_number - KEY_ITEM1)) & 1;
	else if (object_number >= PICKUP_ITEM1_COMBO1 && object_number <= PICKUP_ITEM4_COMBO2)
		return (lara.pickupitemscombo >> (object_number - PICKUP_ITEM1_COMBO1)) & 1;
	else if (object_number >= PICKUP_ITEM1 && object_number <= PICKUP_ITEM4)
		return (lara.pickupitems >> (object_number - PICKUP_ITEM1)) & 1;
	else if (object_number >= QUEST_ITEM1 && object_number <= QUEST_ITEM6)
		return (lara.questitems >> (object_number - QUEST_ITEM1)) & 1;
	else if (object_number == CROWBAR_ITEM)
		return lara.crowbar;
	else if (object_number == PISTOLS_ITEM)
		return lara.pistols_type_carried;
	else if (object_number == PISTOLS_AMMO_ITEM)
		return lara.num_pistols_ammo;
	else if (object_number == SHOTGUN_ITEM)
		return lara.shotgun_type_carried;
	else if (object_number == SHOTGUN_AMMO1_ITEM)
		return lara.num_shotgun_ammo1;
	else if (object_number == SHOTGUN_AMMO2_ITEM)
		return lara.num_shotgun_ammo2;
	else if (object_number == SIXSHOOTER_ITEM)
		return lara.sixshooter_type_carried;
	else if (object_number == SIXSHOOTER_AMMO_ITEM)
		return lara.num_revolver_ammo;
	else if (object_number == UZI_ITEM)
		return lara.uzis_type_carried;
	else if (object_number == UZI_AMMO_ITEM)
		return lara.num_uzi_ammo;
	else if (object_number == CROSSBOW_ITEM)
		return lara.crossbow_type_carried;
	else if (object_number == CROSSBOW_AMMO1_ITEM)
		return lara.num_crossbow_ammo1;
	else if (object_number == CROSSBOW_AMMO2_ITEM)
		return lara.num_crossbow_ammo2;
	else if (object_number == CROSSBOW_AMMO3_ITEM)
		return lara.num_crossbow_ammo3;
	else if (object_number == GRENADE_GUN_ITEM)
		return lara.grenade_type_carried;
	else if (object_number == GRENADE_GUN_AMMO1_ITEM)
		return lara.num_grenade_ammo1;
	else if (object_number == GRENADE_GUN_AMMO2_ITEM)
		return lara.num_grenade_ammo2;
	else if (object_number == GRENADE_GUN_AMMO3_ITEM)
		return lara.num_grenade_ammo3;
	else if (object_number == SMALLMEDI_ITEM)
		return lara.num_small_medipack;
	else if (object_number == BIGMEDI_ITEM)
		return lara.num_large_medipack;
	else if (object_number == FLARE_ITEM)
		return lara.num_flares;
	else if (object_number == BINOCULARS_ITEM)
		return lara.binoculars;
	else
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: get_inventory_count: unimplemented inventory item type!");

	return 0;
}

bool NGCondition(short param, unsigned char extra, short timer) {
	switch (timer) {
	case INVENTORY_ITEM_IS_MISSING: {
		return get_inventory_count(param) == 0;
		break;
	}
	case INVENTORY_ITEM_IS_PRESENT: {
		return get_inventory_count(param) >= 0;
	}
	case INVENTORY_ITEM_HAS_AT_LEAST: {
		return get_inventory_count(param) >= extra;
		break;
	}
	case INVENTORY_ITEM_HAS_LESS_THAN: {
		return get_inventory_count(param) < extra;
		break;
	}
	case KEYBOARD_SCANCODE_IS_CURRENTLY: {
		int scancode = param;
		// TODO: find the difference between multishot and singleshot
		switch (extra) {
			// Inactive single shot
			case 0: {
				return keymap[scancode] == 0;
			}
			// Active single shot
			case 1: {
				return keymap[scancode] != 0;
			}
			// Inactive multi shot
			case 2: {
				return keymap[scancode] == 0;
			}
			// Inactive single shot
			case 3: {
				return keymap[scancode] != 0;
			}
		}

		return false;
	}
	case CREATURE_CURRENT_ANIMATION_0_31_IS:
	case CREATURE_CURRENT_ANIMATION_32_63_IS:
	case CREATURE_CURRENT_ANIMATION_64_95_IS:
		return items[param].anim_number - objects[items[param].object_number].anim_index == param;
	case CREATURE_IS_CURRENTLY_OF_STATE: {
		return items[param].current_anim_state == extra;
	}
	// Lara status is enabled/disabled
	case LARA_STATUS_IS_ENABLED_OR_DISABLED: {
		switch (param) {
			// Poisoned
			case 2:
				if ((lara_item->poisoned) == (bool)extra)
					return true;
				break;
			// Lara touching floor
			case 4:
				if ((lara_item->pos.y_pos == lara_item->floor) == (bool)extra)
					return true;
				break;
			default:
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: Unimplemented NGCondition Lara Status %u!", param);
				break;
		}
		break;
	}
	case LARA_IS_TOUCHING_MOVEABLE: {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_TOUCHING_MOVEABLE unimplemented!");
		return false;
		break;
	}
	case LARA_IS_VITALITY_IS_X_THAN: {
		switch (extra) {
			// Equal than
			case 0: {
				return lara_item->hit_points == param;
			}
			// Higher than
			case 1: {
				return lara_item->hit_points > param;
			}
			// Less than
			case 2: {
				return lara_item->hit_points < param;
			}
			default: {
				NGLog(NG_LOG_TYPE_ERROR, "NGCondition: LARA_IS_VITALITY_IS_X_THAN unknown param!");
			}
		}
		return false;
	}
	case LARA_IS_PERFORMING_ANIMATION: {
		if (lara_item->current_anim_state - objects[LARA].anim_index == param) {
			return true;
		} else {
			return false;
		}
		break;
	}
	case LARA_IS_TOUCHING_STATIC_ITEM: {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_TOUCHING_STATIC_ITEM unimplemented!");
		return false;
		break;
	}
	case LARA_IS_HOLDING_OR_DRIVING_ITEMS: {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_HOLDING_OR_DRIVING_ITEMS unimplemented!");
		return false;
		break;
	}
	case NUMERIC_VALUE_IS_LESS_THAN: {
		if (extra < NGNumericGetVariable(param))
			return true;
		else
			return false;
	}
	case NUMERIC_VALUE_IS_EQUAL_OR_GREATER_TO: {
		if (extra >= NGNumericGetVariable(param))
			return true;
		else
			return false;
	}
	case NUMERIC_VALUE_IS_EQUAL_TO: {
		if (extra == NGNumericGetVariable(param))
			return true;
		else
			return false;
	}
	case LARA_IS_LESS_OR_EVEN_CLICKS_DISTANT_TO_MOVEABLE: {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_LESS_OR_EVEN_CLICKS_DISTANT_TO_MOVEABLE unimplemented!");
		return false;
		break;
	}
	case LARA_IS_IN_ROOM_TYPE: {
		ROOM_INFO* r = &room[lara_item->room_number];
		if (r) {
			int room_flags = r->flags;
			switch (param) {
				case 0: {
					return room_flags & ROOM_UNDERWATER;
				}
				case 2: {
					return room_flags & ROOM_SKYBOX;
				}
				case 4: {
					return room_flags & 0x10;
				}
				case 5: {
					return room_flags & ROOM_NOT_INSIDE; // TODO: check this
				}
				case 10: {
					return room_flags & ROOM_SNOW;
				}
				case 11: {
					return room_flags & ROOM_RAIN;
				}
				case 12: {
					// TODO: check if this is actually cold water
					return room_flags & ROOM_COLD;
				}
				default: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_IN_ROOM_TYPE unsupported room type!");
					break;
				}
			}
			return false;
			break;
		}
	}
	case LARA_IS_TOUCHING_MOVEABLE_WITH_MESH_NUMBER: {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: LARA_IS_TOUCHING_MOVEABLE_WITH_MESH_NUMBER unimplemented!");
		return false;
		break;
	}
	default:
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGCondition: %u unimplemented!", timer);
		break;
	};

	return false;
};