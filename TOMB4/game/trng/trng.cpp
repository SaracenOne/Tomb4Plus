#include "../../tomb4/pch.h"

#include "../../specific/audio.h"
#include "../../specific/function_stubs.h"
#include "../control.h"
#include "../effects.h"
#include "../objects.h"
#include "../lara.h"
#include "../gameflow.h"
#include "trng.h"
#include "trng_extra_state.h"
#include "trng_script_parser.h"

#include "../../tomb4/mod_config.h"
#include "../../specific/file.h"
#include "../../specific/3dmath.h"
#include "trng_animation.h"
#include "trng_triggergroup.h"
#include "trng_organizer.h"
#include "trng_flipeffect.h"
#include "trng_action.h"
#include "trng_condition.h"
#include "trng_progressive_action.h"
#include "../../tomb4/tomb4plus/t4plus_items.h"

NGLevelInfo ng_level_info[MOD_LEVEL_COUNT];

int32_t ng_floor_id_size = 0;
char *ng_floor_id_table = NULL;

int32_t ng_total_flip_rooms = 0;
int16_t ng_flip_rooms[NG_MAX_FLIP_ROOMS];

NGAnimatedTexture ng_animated_texture;

int32_t ng_script_id_count = 0;
NGScriptIDTableEntry ng_script_id_table[NG_SCRIPT_ID_TABLE_SIZE];

int32_t ng_room_remap_count = 0;
NGRoomRemapTableEntry ng_room_remap_table[NG_ROOM_REMAP_TABLE_SIZE];

int32_t ng_static_id_count = 0;
NGStaticTableEntry ng_static_id_table[NG_STATIC_ID_TABLE_SIZE];

void NGPreloadLevelInfo(int32_t current_level, FILE *level_fp) {
	int32_t ngle_ident = 0;
	int32_t ngle_offset = 0;

	if (current_level >= MOD_LEVEL_COUNT) {
		return;
	}

	// Check footer for NGLE info
	fseek(level_fp, -8L, SEEK_END);
	fread(&ngle_ident, 1, sizeof(int32_t), level_fp);
	fread(&ngle_offset, 1, sizeof(int32_t), level_fp);

	if (ngle_ident == NGLE_END_SIGNATURE) {
		ng_level_info[current_level].ngle_footer_found = true;
		fseek(level_fp, -ngle_offset, SEEK_END);

		uint16_t start_ident = 0;
		fread(&start_ident, 1, sizeof(int16_t), level_fp);
		if (start_ident == NGLE_START_SIGNATURE) {
			ng_level_info[current_level].is_ngle_level = true;
			while (1) {
				uint16_t chunk_size = 0;
				fread(&chunk_size, 1, sizeof(uint16_t), level_fp);
				uint16_t chunk_ident = 0;
				fread(&chunk_ident, 1, sizeof(uint16_t), level_fp);

				if (chunk_size == 0 || chunk_ident == 0) {
					break;
				}

				switch (chunk_ident) {
					// Level Flags
					case 0x800d: {
						uint32_t flags;
						fread(&flags, 1, sizeof(uint32_t), level_fp);
						if (flags & 0x01) {
							ng_level_info[current_level].is_using_ngle_triggers = true;
						}
						if (flags & 0x02) {
							ng_level_info[current_level].is_using_global_sound_map = true;

							get_game_mod_level_audio_info(current_level)->motorboat_idle_sfx_id = 1053;
							get_game_mod_level_audio_info(current_level)->motorboat_moving_sfx_id = 1055;

							get_game_mod_level_audio_info(current_level)->rubber_boat_idle_sfx_id = 1423;
							get_game_mod_level_audio_info(current_level)->rubber_boat_moving_sfx_id = 1425;
						}
						break;
						default: {
							fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
							break;
						}
					}
				}
			}
		} else {
			return;
		}
	}
}

void NGPreloadAllLevelInfo(uint32_t valid_level_count) {
	char name[80];

	for (uint32_t i = 0; i < valid_level_count; i++) {
		memset(name, 0x00, 80);
		int16_t level_filename_id = gfLevelFilenames[i];
		if (level_filename_id >= 0) {
			strcpy(name, &gfFilenameWad[gfFilenameOffset[level_filename_id]]);
			strcat(name, ".TR4");

			for (int i = 0; i < strlen(name); i++) {
				if (name[i] == '\\') {
					name[i] = '/';
				} else {
					name[i] = tolower((uint8_t)name[i]);
				}
			}

			FILE *level_fp = T4PFileOpen((const char *)name);
			if (level_fp) {
				NGPreloadLevelInfo(i, level_fp);
			}
		}
	}
}

void NGLoadLevelInfo(FILE* level_fp) {
	memset(&ng_script_id_table, 0x00, NG_SCRIPT_ID_TABLE_SIZE * sizeof(int16_t));
	memset(&ng_room_remap_table, -1, NG_ROOM_REMAP_TABLE_SIZE * sizeof(int16_t));
	memset(&ng_static_id_table, 0x00, NG_STATIC_ID_TABLE_SIZE * sizeof(int16_t));
	memset(&ng_animated_texture, 0x00, sizeof(NGAnimatedTexture));

	ng_script_id_count = 0;
	ng_room_remap_count = 0;
	ng_static_id_count = 0;

	int32_t level_version = 0;
	int32_t ngle_ident = 0;
	int32_t ngle_offset = 0;

	ng_floor_id_size = 0;
	ng_floor_id_table = NULL;

	// Check footer for NGLE info
	fseek(level_fp, -8L, SEEK_END);
	fread(&ngle_ident, 1, sizeof(int32_t), level_fp);
	fread(&ngle_offset, 1, sizeof(int32_t), level_fp);

	if (ngle_ident == NGLE_END_SIGNATURE) {
		fseek(level_fp, -ngle_offset, SEEK_END);

		uint16_t start_ident = 0;
		fread(&start_ident, 1, sizeof(int16_t), level_fp);
		if (start_ident == NGLE_START_SIGNATURE) {
			while (1) {
				uint16_t chunk_size = 0;
				fread(&chunk_size, 1, sizeof(uint16_t), level_fp);
				uint16_t chunk_ident = 0;
				fread(&chunk_ident, 1, sizeof(uint16_t), level_fp);

				if (chunk_size == 0 || chunk_ident == 0) {
					break;
				}

				switch (chunk_ident) {
					// Animated Textures
					case 0x8002: {
						;
						if (chunk_size == ((sizeof(NGAnimatedTexture) / 2) + sizeof(int16_t))) {
							fread(&ng_animated_texture, sizeof(NGAnimatedTexture), 1, level_fp);
							ng_animated_texture.test = true;
						} else {
							memset(&ng_animated_texture, 0, sizeof(NGAnimatedTexture));
							fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						}
						break;
					}
					// Moveables Table
					case 0x8005: {
						int32_t start_position = ftell(level_fp);
						int32_t target_offset = (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2);
						ng_script_id_count = target_offset / sizeof(NGScriptIDTableEntry);
						fread(&ng_script_id_table, 1, target_offset, level_fp);
						break;
					}
					// Extra room flags
					case 0x8009: {
						uint16_t room_count;
						fread(&room_count, 1, sizeof(uint16_t), level_fp);

						for (int32_t i = 0; i < room_count; i++) {
							uint8_t flags[8];
							fread(&flags, sizeof(flags), sizeof(uint8_t), level_fp);
						}
						break;
					}
					// Level Flags
					case 0x800d: {
						fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						break;
					}
					// Tex Partial
					case 0x8017: {
						fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						break;
					}
					// Remap Tails
					case 0x8018: {
						fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						break;
					}
					// Statics Table
					case 0x8021: {
						int32_t target_offset = (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2);
						ng_static_id_count = target_offset / sizeof(NGStaticTableEntry);
						fread(&ng_static_id_table, 1, target_offset, level_fp);
						break;
					}
					// Level version
					case 0x8024: {
						uint16_t version_info[4] = {};
						for (int32_t i = 0; i < 4; i++) {
							fread(&version_info[i], 1, sizeof(uint16_t), level_fp);
						}
						break;
					}
					// TOM version
					case 0x8025: {
						uint16_t version_info[4] = {};
						for (int32_t i = 0; i < 4; i++) {
							fread(&version_info[i], 1, sizeof(uint16_t), level_fp);
						}
						break;
					}
					// Room remap table
					case 0x8037: {
						int32_t target_offset = (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2);
						ng_room_remap_count = target_offset / sizeof(NGRoomRemapTableEntry);
						fread(&ng_room_remap_table, 1, target_offset, level_fp);
						break;
					}
					// Plugin Names
					case 0x8047: {
						fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						break;
					}
					// Floor ID table
					case 0x8048: {
						ng_floor_id_size = (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2);
						if (ng_floor_id_size > sizeof(int16_t)) {
							ng_floor_id_table = (char*)game_malloc(sizeof(int8_t) * ng_floor_id_size);
							fread(ng_floor_id_table, 1, ng_floor_id_size, level_fp);
						} else {
							fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						}
						break;
					}
					// Remap Plugin IDs
					case 0x804e: {
						fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						break;
					}
					default: {
						NGLog(NG_LOG_TYPE_PRINT, "NGLoadInfo: Unknown NG level chunk ident: 0x%04x", chunk_ident);
						fseek(level_fp, (chunk_size * sizeof(int16_t)) - (sizeof(int16_t) * 2), SEEK_CUR);
						break;
					}
				}
			}
		}
	} else {
		return;
	}
}

// Move the item in a direction by the number of units
void NGMoveItemByUnits(uint16_t item_id, NG_DIRECTIONS direction, int32_t units) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		switch (direction) {
			case NG_NORTH: {
				item->pos.z_pos += units;
				return;
			}
			case NG_EAST: {
				item->pos.x_pos += units;
				return;
			}
			case NG_SOUTH: {
				item->pos.z_pos -= units;
				return;
			}
			case NG_WEST: {
				item->pos.x_pos -= units;
				return;
			}
			case NG_UP: {
				item->pos.y_pos -= units;
				return;
			}
			case NG_DOWN: {
				item->pos.y_pos += units;
				return;
			}
		}
	}
}

// Move the item in an angle by the number of units
void NGMoveItemHorizontalByUnits(uint16_t item_id, int16_t angle, int32_t units) {
	int32_t c = (int32_t)units * phd_cos(angle) >> W2V_SHIFT;
	int32_t s = (int32_t)units * phd_sin(angle) >> W2V_SHIFT;

	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		item->pos.x_pos += s;
		item->pos.z_pos += c;
	}
}

// Move the item up or down by the number of units
void NGMoveItemVerticalByUnits(uint16_t item_id, int32_t units) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		item->pos.y_pos += units;
	}
}

void NGRotateItemX(uint16_t item_id, int16_t rotation) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		item->pos.x_rot += rotation;
	}
}

void NGRotateItemY(uint16_t item_id, int16_t rotation) {
	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		item->pos.y_rot += rotation;
	}
}

// Move the item in a direction by the number of units
void NGStaticItemByUnits(uint16_t static_id, NG_DIRECTIONS direction, int32_t units) {
	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		if (room[room_number].num_meshes > entry->mesh_id) {
			MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
			if (mesh) {
				switch (direction) {
					case NG_NORTH: {
						mesh->z += units;
						return;
					}
					case NG_EAST: {
						mesh->x += units;
						return;
					}
					case NG_SOUTH: {
						mesh->z -= units;
						return;
					}
					case NG_WEST: {
						mesh->x -= units;
						return;
					}
					case NG_UP: {
						mesh->y -= units;
						return;
					}
					case NG_DOWN: {
						mesh->y += units;
						return;
					}
				}
			}
		}
	}
}

void NGMoveStaticHorizontalByUnits(uint16_t static_id, int16_t angle, int32_t units) {
	int32_t c = (int32_t)units * phd_cos(angle) >> W2V_SHIFT;
	int32_t s = (int32_t)units * phd_sin(angle) >> W2V_SHIFT;

	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		if (room[room_number].num_meshes > entry->mesh_id) {
			MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
			if (mesh) {
				mesh->x += s;
				mesh->z += c;
			}
		}
	}
}

void NGMoveStaticVerticalByUnits(uint16_t static_id, int32_t units) {
	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		if (room[room_number].num_meshes > entry->mesh_id) {
			MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
			if (mesh) {
				mesh->y += units;
			}
		}
	}
}

GAME_VECTOR NGGetGameVectorForStatic(uint16_t static_id) {
	GAME_VECTOR game_vector;
	game_vector.x = 0;
	game_vector.y = 0;
	game_vector.z = 0;
	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		if (room[room_number].num_meshes > entry->mesh_id) {
			MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
			if (mesh) {
				game_vector.x = mesh->x;
				game_vector.y = mesh->y;
				game_vector.z = mesh->z;
				game_vector.room_number = room_number;
			}
		}
	}
	return game_vector;
}

void NGRotateStaticX(uint16_t static_id, int16_t rotation) {
	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGRotateStaticX unimplemented!");
	}
}

void NGRotateStaticY(uint16_t static_id, int16_t rotation) {
	NGStaticTableEntry* entry = &ng_static_id_table[static_id];
	int32_t room_number = ng_room_remap_table[entry->remapped_room_index].room_index;
	if (room_number >= 0 && room_number < number_rooms) {
		MESH_INFO* mesh = &room[room_number].mesh[entry->mesh_id];
		if (mesh) {
			mesh->y_rot += rotation;
		}
	}
}

int32_t NGFloat2Int(float x) {
	return (int32_t)(x > 0.0 ? x + 0.5 : x - 0.5);
}

bool NGIsSourcePositionNearTargetPos(PHD_3DPOS *source_pos, PHD_3DPOS *target_pos, int32_t distance, bool ignore_y) {
	int32_t diff;

	diff = target_pos->x_pos - source_pos->x_pos;
	if (diff < -distance || diff > distance)
		return false;

	if (!ignore_y) {
		diff = target_pos->y_pos - source_pos->y_pos;
		if (diff < -distance || diff > distance)
			return false;
	}

	diff = target_pos->z_pos - source_pos->z_pos;
	if (diff < -distance || diff > distance)
		return false;

	return true;

}

bool NGIsSourcePositionLessThanDistanceToTargetPosition(PHD_3DPOS *source_pos, PHD_3DPOS *target_pos, int32_t distance, bool ignore_y) {
	int32_t diffX, diffY, diffZ;

	if (!NGIsSourcePositionNearTargetPos(source_pos, target_pos, distance, ignore_y))
		return false;

	diffX = (int32_t)target_pos->x_pos - (int32_t)source_pos->x_pos;
	if (ignore_y) {
		diffY = 0;
	} else {
		diffY = (int32_t)target_pos->y_pos - (int32_t)source_pos->y_pos;
	}
	diffZ = (int32_t)target_pos->z_pos - (int32_t)source_pos->z_pos;

	diffX *= diffX;
	diffY *= diffY;
	diffZ *= diffZ;

	if (diffX < 0 || diffY < 0 || diffZ < 0)
		return false;

	int32_t total = NGFloat2Int((float)sqrt(diffX + diffY + diffZ));
	if (total <= distance)
		return true;

	return false;
}

void NGSetItemAnimation(uint16_t item_id,
                        uint32_t animation,
                        bool update_state_id,
                        bool update_next_state_id,
                        bool update_speed,
                        bool force_reset) {

	ITEM_INFO *item = T4PlusGetItemInfoForID(item_id);
	if (item) {
		int16_t new_animation = objects[item->object_number].anim_index + animation;

		if (!force_reset && item->anim_number == new_animation) {
			return;
		}

		item->anim_number = new_animation;
		item->frame_number = anims[new_animation].frame_base;

		if (update_state_id) {
			item->current_anim_state = anims[item->anim_number].current_anim_state;
		}

		if (update_next_state_id) {
			item->goal_anim_state = anims[item->anim_number].current_anim_state;
		}

		if (update_speed) {
			item->speed = int16_t(anims[item->anim_number].velocity & 0xffff);
		}
	}
}

void NGLevelSetup() {
	NGLoadTablesForLevel(gfCurrentLevel);
	NGSetupLevelExtraState();

	// Loaded from a savegame
	if (gfGameMode == GF_GAME_MODE_SAVEGAME) {
		ng_loaded_savegame = true;
	} else {
		ng_loaded_savegame = false;
	}

	stored_condition_count = 0;
	stored_is_inside_dummy_trigger = false;
	stored_last_floor_address = nullptr;
	stored_base_floor_trigger_now = nullptr;
	stored_is_heavy_testing = false;
	stored_last_item_index = -1;
	stored_item_index_enabled_trigger = -1;
	stored_item_index_current = -1;
	stored_item_index_conditional = -1;
	stored_last_trigger_timer = 0;
	stored_test_conditions_found = false;
	stored_save_trigger_buttons = 0;
	stored_test_dummy_failed = false;

	// Flipeffects
	scanned_flipeffect_count = 0;
	for (int32_t i = 0; i < NG_MAX_SCANNED_FLIPEFFECTS; i++) {
		memset(&scanned_flipeffects[i], 0, sizeof(NGScannedFlipEffect));
	}
	old_flipeffect_count = 0;
	for (int32_t i = 0; i < NG_MAX_OLD_FLIPEFFECTS; i++) {
		memset(&old_flipeffects[i], 0, sizeof(NGOldTrigger));
	}

	// Actions
	scanned_action_count = 0;
	for (int32_t i = 0; i < NG_MAX_SCANNED_ACTIONS; i++) {
		memset(&scanned_actions[i], 0, sizeof(NGScannedAction));
	}
	old_action_count = 0;
	for (int32_t i = 0; i < NG_MAX_OLD_ACTIONS; i++) {
		memset(&old_actions[i], 0, sizeof(NGOldTrigger));
	}

	// Conditions
	old_condition_count = 0;
	for (int32_t i = 0; i < NG_MAX_OLD_CONDITIONS; i++) {
		memset(&old_conditions[i], 0, sizeof(NGOldTrigger));
	}

	// Progressive Actions
	progressive_action_count = 0;
	for (int32_t i = 0; i < NG_MAX_PROGRESSIVE_ACTIONS; i++) {
		memset(&progressive_actions[i], 0, sizeof(NGProgressiveAction));
	}

	// Resumed TriggerGroups
	resumed_trigger_group_count = 0;
	for (int32_t i = 0; i < NG_MAX_PROGRESSIVE_ACTIONS; i++) {
		resumed_trigger_groups[i] = -1;
	}

	// Performed TriggerGroups
	last_performed_trigger_group = -1;
	for (int32_t i = 0; i < NG_MAX_PROGRESSIVE_ACTIONS; i++) {
		performed_trigger_groups[i] = -1;
	}

	// Moved item indicies
	moved_item_indicies_count = 0;
	for (int32_t i = 0; i < NG_MAX_SAVED_COORDINATES; i++) {
		moved_item_indicies[i] = -1;
	}

	NGInitializeFlipMaps();
}

void NGSignalForManagementCreatedItems() {
	// TODO
}

void NGFrameStart() {
	NGFrameStartExtraState();

	NGResetScanActions();
	NGResetScanFlipEffects();

	//

	NGStoreTestDummyFailed(false);

	//

	NGStoreTestConditionsFound(false);

	//
}

void NGPreFrameFinish() {
	// TODO: replace with actual progressive action system.
	NGProcessTriggerGroups();

	NGExecuteProgressiveActions();
}

void NGFrameFinish() {
	NGSignalForManagementCreatedItems();

	// TODO: FMV stuff

	// TODO: Singleshot resumed stuff

	// TODO: Store Lara's last HP Value

	// TODO: diagnostics

	// TODO: counter and game commands

	// TODO: roll boats

	// TODO: store item used by Lara

	// TODO: set time of last command

	// TODO: Store health again (?)

	// TODO: Standby

	// TODO: Control Death Creatures

	// TODO: Test old CD triggers

	// TODO: demo title thing

	//
	NGProcessGlobalTriggers(NO_ITEM);

	// TODO: cheats

	// TODO: static collision

	// TODO: adaptive far view

	//
	NGProcessOrganizers();

	// TODO: test takeaway weapons

	// TODO: manage damage

	// TODO: animation change stuff

	//
	NGProcessAnimations();

	// TODO: mirror stuff

	// TODO: keypad stuff

	// TODO: detector stuff

	// TODO: operation switch

	NGFrameFinishExtraState();
}

bool NGIsUsingNGNewTriggers() {
	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();

	return global_info->trng_new_triggers && ng_level_info[gfCurrentLevel].is_using_ngle_triggers;
}

bool NGIsUsingNGAnimCommands() {
	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();

	return global_info->trng_anim_commands_enabled && ng_level_info[gfCurrentLevel].is_using_ngle_triggers;
}

bool NGIsUsingNGTimerfields() {
	MOD_GLOBAL_INFO* global_info = get_game_mod_global_info();

	return global_info->trng_timerfields_enabled && ng_level_info[gfCurrentLevel].is_using_ngle_triggers;
}

int32_t NGFindIndexForLaraStartPosWithMatchingOCB(uint32_t ocb) {
	for (int32_t i = 0; i < nAIObjects; i++) {
		if (AIObjects[i].object_number == LARA_START_POS && ocb == AIObjects[i].trigger_flags) {
			return i;
		}
	}

	return -1;
}

bool NGLaraHasInfiniteAir() {
	return ng_lara_infinite_air;
}

bool NGTestSelectedInventoryObjectAndManagementReplaced(int32_t inventory_object_id) {
	return NGProcessGlobalTriggers(inventory_object_id);
}

void NGSetUsedInventoryObject(int32_t inventory_object_id) {
	ng_used_inventory_object_for_frame = inventory_object_id;
}

void NGSetUsedSmallMedipack() {
	ng_used_small_medipack = true;
}

void NGSetUsedLargeMedipack() {
	ng_used_large_medipack = true;
}

void NGInit() {
}

void NGCleanup() {
	NGScriptCleanup();
}

void NGLog(NGLogType type, const char* s, ...) {
#ifndef DEBUG
	if (type == NG_LOG_TYPE_POSSIBLE_INACCURACY)
		return;
#endif

	va_list list;
	char buf[8192];

	va_start(list, s);
	vsprintf(buf, s, list);
	va_end(list);

	switch (type) {
		case NG_LOG_TYPE_PRINT: {
			Log(0, "NGLogPrint: %s", buf);
			break;
		}
		case NG_LOG_TYPE_UNIMPLEMENTED_FEATURE: {
			Log(0, "NGLogUnimplementedFeature: %s", buf);
			break;
		}
		case NG_LOG_TYPE_POSSIBLE_INACCURACY: {
			Log(0, "NGLogPossibleInaccuracy: %s", buf);
			break;
		}
		case NG_LOG_TYPE_ERROR: {
			Log(0, "NGLogError: %s", buf);
			break;
		}
	}
}

int32_t stored_condition_count = 0;
bool stored_is_inside_dummy_trigger = false;
int16_t *stored_last_floor_address = nullptr;
int16_t *stored_base_floor_trigger_now = nullptr;
bool stored_is_heavy_testing = false;
int16_t stored_last_item_index = -1;
int16_t stored_item_index_enabled_trigger = -1;
int16_t stored_item_index_current = -1;
int16_t stored_item_index_conditional = -1;
int32_t stored_last_trigger_timer = 0;
bool stored_test_conditions_found = false;
uint32_t stored_save_trigger_buttons = 0;
bool stored_test_dummy_failed = false;


void NGStoreLastFloorAddress(int16_t *p_floor_last_address) {
	stored_last_floor_address = p_floor_last_address;
}

int16_t *NGGetLastFloorAddress() {
	return stored_last_floor_address;
}

void NGStoreFloorTriggerNow(int16_t *p_trigger_now) {
	stored_base_floor_trigger_now = p_trigger_now;
}

int16_t* NGGetFloorTriggerNow() {
	return stored_base_floor_trigger_now;
}

void NGStoreIsHeavyTesting(bool p_is_heavy_testing) {
	stored_is_heavy_testing = p_is_heavy_testing;
}

bool NGGetIsHeavyTesting() {
	return stored_is_heavy_testing;
}

void NGStoreLastItemMovedIndex(int16_t item_num) {
	stored_last_item_index = item_num;
}

int16_t NGGetLastMovedItemIndex() {
	return stored_last_item_index;
}

void NGStoreItemIndexEnabledTrigger(int16_t item_num) {
	stored_item_index_enabled_trigger = item_num;
}

int16_t NGGetItemIndexEnabledTrigger() {
	return stored_item_index_enabled_trigger;
}

void NGStoreItemIndexCurrent(int16_t item_num) {
	stored_item_index_current = item_num;
}

int16_t NGGetItemIndexCurrent() {
	return stored_item_index_current;
}

void NGStoreItemIndexConditional(int16_t index) {
	stored_item_index_conditional = index;
}

int16_t NGGetItemIndexConditional() {
	return stored_item_index_conditional;
}

void NGStoreInsideConditionCount(int32_t count) {
	stored_condition_count = count;
}

int32_t NGGetInsideConditionCount() {
	return stored_condition_count;
}

void NGStoreIsInsideDummyTrigger(bool is_inside_dummy) {
	stored_is_inside_dummy_trigger = is_inside_dummy;
}

bool NGGetIsInsideDummyTrigger() {
	return stored_is_inside_dummy_trigger;
}

void NGStoreLastTriggerTimer(int32_t timer) {
	stored_last_trigger_timer = timer;
}

int32_t NGGetLastTriggerTimer() {
	return stored_last_trigger_timer;
}

void NGStoreTestConditionsFound(bool found) {
	stored_test_conditions_found = found;
}

bool NGGetTestConditionsFound() {
	return stored_test_conditions_found;
}

void NGStoreSaveTriggerButtons(uint32_t trigger_buttons) {
	stored_save_trigger_buttons = trigger_buttons;
}

uint32_t NGGetSaveTriggerButtons() {
	return stored_save_trigger_buttons;
}

void NGStoreTestDummyFailed(bool failed) {
	stored_test_dummy_failed = failed;
}

bool NGGetTestDummyFailed() {
	return stored_test_dummy_failed;
}

int32_t NGCalculateTriggerTimer(int16_t* data, int32_t timer) {
	int16_t trigger;
	do {
		trigger = *data++;
		int16_t value = trigger & 0x3FF;

		switch ((trigger & 0x3FFF) >> 10) {
			case TO_ACTION:
				if (NGIsUsingNGNewTriggers()) {
					trigger = *data++;
				}
				break;
			case TO_FLIPEFFECT:
				if (NGIsUsingNGNewTriggers()) {
					trigger = *data++;
				}
				break;
			case TO_CAMERA:
				trigger = *data++;
				break;
			case TO_TIMERFIELD:
				timer = (value & 0x3ff);

				if (timer & 0x200) {
					timer |= 0xFFFFFC00;
				}

				NGStoreLastTriggerTimer(timer);
				return timer;

				break;
			default:
				break;
		}
	} while (!(trigger & 0x8000));

	NGStoreLastTriggerTimer(timer);

	return timer;
}

bool NGUsingLegacyNGTriggerBehaviour() {
	return get_game_mod_global_info()->trng_legacy_ng_trigger_behaviour;
}

int32_t moved_item_indicies_count = 0;
int16_t moved_item_indicies[NG_MAX_SAVED_COORDINATES];

void NGAddItemMoved(int32_t item_id) {
	for (int32_t i = 0; i < moved_item_indicies_count; i++) {
		if (moved_item_indicies[i] == item_id) {
			return;
		}
	}

	moved_item_indicies[moved_item_indicies_count] = item_id;
	if (moved_item_indicies_count < NG_MAX_SAVED_COORDINATES - 1) {
		moved_item_indicies_count++;
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "Moved item indicies overflow");
	}
}

int32_t NGFindIndexForRoom(int32_t room_index) {
	if (room_index >= NG_MAX_FLIP_ROOMS) {
		return -1;
	}

	if (ng_flip_rooms[room_index] != -1) {
		return -1;
	}

	ROOM_INFO* current_room = &room[room_index];

	if (current_room->flipped_room == -1) {
		return room_index;
	}

	int8_t flipmap_index = current_room->FlipNumber;

	if (flipmap[flipmap_index]) {
		return current_room->flipped_room;
	}

	return room_index;
}

void NGInitializeFlipMaps() {
	ng_total_flip_rooms = number_rooms;

	for (int32_t i = 0; i < NG_MAX_FLIP_ROOMS; i++) {
		ng_flip_rooms[i] = -1;
	}

	for (int32_t i = 0; i < ng_total_flip_rooms; i++) {
		if (room[i].flipped_room != -1) {
			ng_flip_rooms[i] = room[i].flipped_room;
		}
	}
}