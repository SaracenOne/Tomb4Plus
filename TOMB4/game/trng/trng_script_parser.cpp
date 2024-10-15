#include "../../tomb4/pch.h"

#include "../gameflow.h"

#include "trng.h"
#include "trng_condition.h"
#include "trng_script_parser.h"

#include "../../tomb4/mod_config.h"
#include "trng_globaltrigger.h"
#include "../../specific/function_stubs.h"

char *ng_strings[MAX_NG_STRINGS];
NG_LEVEL ng_levels[MAX_NG_LEVELS];
NG_PLUGIN ng_plugins[MAX_NG_PLUGINS];

NG_GLOBAL_TRIGGER current_global_triggers[MAX_NG_GLOBAL_TRIGGERS];
NG_TRIGGER_GROUP current_trigger_groups[MAX_NG_TRIGGER_GROUPS];
NG_ORGANIZER current_organizers[MAX_NG_ORGANIZERS];
NG_ITEM_GROUP current_item_groups[MAX_NG_ITEM_GROUPS];
NG_ANIMATION current_animations[MAX_NG_ANIMATIONS];
NG_MULTI_ENV_CONDITION current_multi_env_conditions[MAX_NG_MULTI_ENV_CONDITIONS];
NG_TEST_POSITION current_test_positions[MAX_NG_TEST_POSITIONS];

// Params
NG_MOVE_ITEM current_move_items[MAX_NG_MOVE_ITEMS];
NG_ROTATE_ITEM current_rotate_items[MAX_NG_ROTATE_ITEMS];
NG_BIG_NUMBER current_big_numbers[MAX_NG_BIG_NUMBERS];

void NGDecryptScriptBlock(char *block) {
	const unsigned char XOR_CYPHER[] = { 0xEF, 0x55, 0xE1, 0xF8, 0x3D, 0x6F, 0xD6, 0x19, 0xDA, 0x97, 0x1D, 0x8B, 0x85, 0x0F, 0xB4, 0x0A, 0xC4 };
	const unsigned char SWIZZLE_CYPHER[] = { 0x39, 0x31, 0x01, 0x07, 0x24, 0x25, 0x00, 0x11, 0x2D, 0x0D, 0x28, 0x2C, 0x2E, 0x21, 0x1E, 0x22, 0x14, 0x29, 0x1A, 0x13, 0x3B, 0x35, 0x2B, 0x02, 0x16, 0x06, 0x17, 0x09, 0x1F, 0x0A, 0x15, 0x0F, 0x05, 0x08, 0x2A, 0x18, 0x37, 0x0E, 0x30, 0x38, 0x2F, 0x3C, 0x0C, 0x27, 0x1C, 0x20, 0x10, 0x1B, 0x34, 0x23, 0x3E, 0x3A, 0x3F, 0x0B, 0x12, 0x26, 0x04, 0x36, 0x32, 0x3D, 0x33, 0x19, 0x1D, 0x03 };

	int block_counter = 0;
	for (int i = 0; i < 64; i++) {
		if (block_counter >= sizeof(XOR_CYPHER)) {
			block_counter = 0;
		}

		block[i] ^= XOR_CYPHER[block_counter];
		block_counter++;
	}

	char decrypted_block[64] = {};

	for (int i = 0; i < 64; i++) {
		int j = 0;
		while (j < 64 && SWIZZLE_CYPHER[j] != i) {
			j++;
		}
		decrypted_block[i] = block[j];
	}

	for (int i = 0; i < 64; i++) {
		block[i] = decrypted_block[i];
	}
}

void NGScriptInit(char* gfScriptFile, size_t offset, size_t len) {
	for (int i = 0; i < MAX_NG_LEVELS; i++) {
		ng_levels[i].records = NULL;
	}

	for (int i = 0; i < MAX_NG_STRINGS; i++) {
		ng_strings[i] = NULL;
	}

	for (int i = 0; i < MAX_NG_PLUGINS; i++) {
		ng_plugins[i].is_enabled;
		ng_plugins[i].ng_plugin_string_id = 0;
	}

	bool ng_header_found = false;

	unsigned int footer_ident = NG_READ_32(gfScriptFile, offset);
	if (footer_ident != 0x454c474e) { // NGLE
		return;
	}

	unsigned int footer_offset = NG_READ_32(gfScriptFile, offset);
	offset -= footer_offset;

	unsigned short header_ident = NG_READ_16(gfScriptFile, offset);
	if (header_ident != 0x474e) { // NGLE
		return;
	}

	ng_header_found = true;

	if (ng_header_found) {
		if (!is_mod_trng_version_equal_or_greater_than_target(0, 0, 0, 0)) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGHeader found in Script.dat in invalid TRNG version!");
			return;
		}

		size_t options_header_block_start_position = offset;

		unsigned short options_header_block_size = NG_READ_16(gfScriptFile, offset);
		size_t options_header_block_end_pos = options_header_block_start_position + (options_header_block_size * sizeof(short));
		unsigned short options_block_unknown_variable = NG_READ_16(gfScriptFile, offset);

		bool is_encrypted = false;

		while (1) {
			size_t data_block_start_start_position = offset;
			unsigned char current_data_block_size_wide = NG_READ_8(gfScriptFile, offset);

			unsigned char block_type = NG_READ_8(gfScriptFile, offset);

			if (offset >= options_header_block_end_pos) {
				if (offset != options_header_block_end_pos) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Options header block size mismatch!");
				}
				break;
			}

			switch (block_type) {
				// Settings
				case 0x10: {
					short flags = NG_READ_16(gfScriptFile, offset);
					is_encrypted = flags & 0x08;
					if (is_encrypted) {
						NGLog(NG_LOG_TYPE_PRINT, "NGScriptInit: Gameflow script is encrypted...");
					}
					break;
				}
			}

			size_t command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			offset = command_block_end_position;
		}

		if (is_encrypted) {
			NGDecryptScriptBlock(gfScriptFile);
		}
	}
}

#define NG_FREE_RECORD(record_name_lowercase) level.records->record_name_lowercase##_count = 0; \
if (level.records->record_name_lowercase##_table) { \
	SYSTEM_FREE(level.records->record_name_lowercase##_table); \
	level.records->record_name_lowercase##_table = NULL; \
}

#define NG_ALLOCATE_RECORD(record_name_lowercase, record_name_uppercase, struct_name) level.records->record_name_lowercase##_count = struct_name##.record_name_lowercase##_table_count; \
level.records->record_name_lowercase##_table = NULL; \
if (struct_name##.record_name_lowercase##_table_count) { \
	level.records->record_name_lowercase##_table = (NG_##record_name_uppercase##_RECORD*)SYSTEM_MALLOC(sizeof(NG_##record_name_uppercase##_RECORD) * struct_name##.record_name_lowercase##_table_count); \
	if (!level.records->record_name_lowercase##_table) { \
		NGLog(NG_LOG_TYPE_ERROR, "NGReallocateLevel: Memory allocation failed!"); \
		return false; \
	} \
	memset(level.records->record_name_lowercase##_table, 0x00, sizeof(NG_##record_name_uppercase##_RECORD) * struct_name##.record_name_lowercase##_table_count); \
} \

struct NG_TABLE_ALLOCATION_COUNT {
	unsigned int global_trigger_table_count = 0;
	unsigned int trigger_group_table_count = 0;
	unsigned int organizer_table_count = 0;
	unsigned int item_group_table_count = 0;
	unsigned int animation_table_count = 0;
	unsigned int multi_env_condition_table_count = 0;
	unsigned int test_position_table_count = 0;

	// Param
	unsigned int move_item_table_count = 0;
	unsigned int rotate_item_table_count = 0;
	unsigned int big_number_table_count = 0;
};

void NGFreeLevel(NG_LEVEL& level) {
	if (level.records) {
		NG_FREE_RECORD(global_trigger);
		NG_FREE_RECORD(trigger_group);
		NG_FREE_RECORD(organizer);
		NG_FREE_RECORD(item_group);
		NG_FREE_RECORD(animation);
		NG_FREE_RECORD(multi_env_condition);
		NG_FREE_RECORD(test_position);

		// Param
		NG_FREE_RECORD(move_item);
		NG_FREE_RECORD(rotate_item);
		NG_FREE_RECORD(big_number);

		SYSTEM_FREE(level.records);
	}
}

bool NGReallocateLevel(
	NG_LEVEL& level,
	NG_TABLE_ALLOCATION_COUNT &allocation_struct) {
	NGFreeLevel(level);

	level.records = (NG_LEVEL_RECORD_DATA*)SYSTEM_MALLOC(sizeof(NG_LEVEL_RECORD_DATA));

	if (level.records) {
		NG_ALLOCATE_RECORD(global_trigger, GLOBAL_TRIGGER, allocation_struct);
		NG_ALLOCATE_RECORD(trigger_group, TRIGGER_GROUP, allocation_struct);
		NG_ALLOCATE_RECORD(organizer, ORGANIZER, allocation_struct);
		NG_ALLOCATE_RECORD(item_group, ITEM_GROUP, allocation_struct);
		NG_ALLOCATE_RECORD(animation, ANIMATION, allocation_struct);
		NG_ALLOCATE_RECORD(multi_env_condition, MULTI_ENV_CONDITION, allocation_struct);
		NG_ALLOCATE_RECORD(test_position, TEST_POSITION, allocation_struct);

		NG_ALLOCATE_RECORD(move_item, MOVE_ITEM, allocation_struct);
		NG_ALLOCATE_RECORD(rotate_item, ROTATE_ITEM, allocation_struct);
		NG_ALLOCATE_RECORD(big_number, BIG_NUMBER, allocation_struct);

		return true;
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "NGReallocateLevel: Memory allocation failed!");
		return false;
	}
}

char *NGGetString(short string_id) {
	if (string_id >= 0) {
		if (string_id > TXT_NUM_STRINGS) {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid string ID");
		} else {
			return GetFixedStringForTextID(string_id);
		}
	} else {
		short ng_string_id = string_id & ~(0x8000);
		if (ng_string_id < MAX_NG_STRINGS) {
			return ng_strings[ng_string_id];
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "MAX_NG_STRINGS exceeded!");
		}
	}

	return NULL;
}

char *NGGetPluginString(short plugin_id) {
	if (plugin_id < MAX_NG_PLUGINS) {
		if (ng_plugins[plugin_id].is_enabled) {
			return NGGetString(ng_plugins[plugin_id].ng_plugin_string_id);
		}
	} else {
		NGLog(NG_LOG_TYPE_ERROR, "MAX_NG_STRINGS exceeded!");
	}

	return NULL;
}

int NGGetT4PluginID(short plugin_id) {
	if (plugin_id < MAX_NG_PLUGINS) {
		if (ng_plugins[plugin_id].t4plus_plugin) {
			return ng_plugins[plugin_id].t4plus_plugin;
		}
	}
	else {
		NGLog(NG_LOG_TYPE_ERROR, "MAX_NG_STRINGS exceeded!");
	}

	return NULL;
}

void NGScriptCleanup() {
	for (int i = 0; i < MAX_NG_LEVELS; i++) {
		NGFreeLevel(ng_levels[i]);
	}

	for (int i = 0; i < MAX_NG_STRINGS; i++) {
		if (ng_strings[i]) {
			SYSTEM_FREE(ng_strings[i]);
			ng_strings[i] = NULL;
		}
	}
}

#define NG_LOAD_RECORD_TABLE(record_name_lowercase, record_name_uppercase) for (int i = 0; i < ng_levels[level].records->record_name_lowercase##_count; i++) { \
unsigned int id = ng_levels[level].records->record_name_lowercase##_table[i].record_id; \
memcpy(&current_##record_name_lowercase##s[id], &ng_levels[level].records->record_name_lowercase##_table[i].record, sizeof(NG_##record_name_uppercase)); \
}

void NGLoadTablesForLevel(unsigned int level) {
	memset(&current_global_triggers, 0x00, sizeof(NG_GLOBAL_TRIGGER)* MAX_NG_GLOBAL_TRIGGERS);
	memset(&current_trigger_groups, 0x00, sizeof(NG_TRIGGER_GROUP)* MAX_NG_TRIGGER_GROUPS);
	memset(&current_organizers, 0x00, sizeof(NG_ORGANIZER)* MAX_NG_ORGANIZERS);
	memset(&current_item_groups, 0x00, sizeof(NG_ITEM_GROUP)* MAX_NG_ITEM_GROUPS);
	memset(&current_animations, 0x00, sizeof(NG_ANIMATION) * MAX_NG_ANIMATIONS);
	memset(&current_multi_env_conditions, 0x00, sizeof(NG_MULTI_ENV_CONDITION) * MAX_NG_MULTI_ENV_CONDITIONS);
	memset(&current_test_positions, 0x00, sizeof(NG_TEST_POSITION) * MAX_NG_TEST_POSITIONS);

	// Params
	memset(&current_move_items, 0x00, sizeof(NG_MOVE_ITEM) * MAX_NG_MOVE_ITEMS);
	memset(&current_rotate_items, 0x00, sizeof(NG_MOVE_ITEM) * MAX_NG_ROTATE_ITEMS);
	memset(&current_big_numbers, 0x00, sizeof(NG_BIG_NUMBER) * MAX_NG_BIG_NUMBERS);

	if (ng_levels[level].records) {
		NG_LOAD_RECORD_TABLE(global_trigger, GLOBAL_TRIGGER)
		NG_LOAD_RECORD_TABLE(trigger_group, TRIGGER_GROUP);
		NG_LOAD_RECORD_TABLE(organizer, ORGANIZER);
		NG_LOAD_RECORD_TABLE(item_group, ITEM_GROUP);
		NG_LOAD_RECORD_TABLE(animation, ANIMATION);
		NG_LOAD_RECORD_TABLE(multi_env_condition, MULTI_ENV_CONDITION);
		NG_LOAD_RECORD_TABLE(test_position, TEST_POSITION);

		NG_LOAD_RECORD_TABLE(move_item, MOVE_ITEM);
		NG_LOAD_RECORD_TABLE(rotate_item, ROTATE_ITEM);
		NG_LOAD_RECORD_TABLE(big_number, BIG_NUMBER);
	}
}

void NGSetupFlareCustomization(int current_level,
	unsigned short flare_flags,
	unsigned short flare_lifetime_in_seconds,
	unsigned char flare_light_r,
	unsigned char flare_light_g,
	unsigned char flare_light_b,
	unsigned char flare_light_intensity) {

	MOD_LEVEL_FLARE_INFO* flare_info = get_game_mod_level_flare_info(current_level);

	if (flare_light_r != 0xff && flare_light_g != 0xff && flare_light_b != 0xff) {
		flare_info->light_color_r = flare_light_r;
		flare_info->light_color_g = flare_light_g;
		flare_info->light_color_b = flare_light_b;
	}
	if (flare_light_intensity != 0xff)
		flare_info->light_intensity = flare_light_intensity;
	if (flare_lifetime_in_seconds != 0xffff)
		flare_info->flare_lifetime_in_ticks = flare_lifetime_in_seconds * 30;
	if (flare_flags != 0xffff) {
		flare_info->has_sparks = flare_flags & 0x0001;
		flare_info->has_fire = flare_flags & 0x0002; // Unsupported
		if (flare_info->has_fire)
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Flare fire effect unimplemented!");
		flare_info->sparks_include_smoke = flare_flags & 0x0004;
		flare_info->has_glow = flare_flags & 0x0008;
		flare_info->flat_light = flare_flags & 0x0010;
	}
}

void NGSetupBugfixCustomization(int current_level, unsigned short bug_fix_flags) {
	if (bug_fix_flags & ~(BUGF_TRANSPARENT_WHITE_ON_FOG | BUGF_DART_NO_POISON_LARA | BUGF_LAND_WATER_SFX_ENEMIES)) {
		NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: CUST_FIX_BUGS unknown flags!");
	}

	if (bug_fix_flags & BUGF_TRANSPARENT_WHITE_ON_FOG) {
		NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: BUGF_TRANSPARENT_WHITE_ON_FOG unsupported! (level %u)", current_level);
	}

	if (bug_fix_flags & BUGF_DART_NO_POISON_LARA) {
		get_game_mod_level_misc_info(current_level)->darts_poison_fix = true;
	} else {
		get_game_mod_level_misc_info(current_level)->darts_poison_fix = false;
	}

	if (bug_fix_flags & BUGF_LAND_WATER_SFX_ENEMIES) {
		get_game_mod_level_misc_info(current_level)->enemy_gun_hit_underwater_sfx_fix = true;
	} else {
		get_game_mod_level_misc_info(current_level)->enemy_gun_hit_underwater_sfx_fix = false;
	}
}

size_t NGReadLevelBlock(char* gfScriptFile, size_t offset, NG_LEVEL_RECORD_TABLES *tables, int current_level, int world_far_view) {
	memset(tables->level_global_triggers_table, 0x00, sizeof(NG_GLOBAL_TRIGGER_RECORD) * MAX_NG_GLOBAL_TRIGGERS);
	memset(tables->level_trigger_group_table, 0x00, sizeof(NG_TRIGGER_GROUP_RECORD) * MAX_NG_TRIGGER_GROUPS);
	memset(tables->level_organizer_table, 0x00, sizeof(NG_ORGANIZER_RECORD) * MAX_NG_ORGANIZERS);
	memset(tables->level_item_group_table, 0x00, sizeof(NG_ITEM_GROUP_RECORD) * MAX_NG_ITEM_GROUPS);
	memset(tables->level_animation_table, 0x00, sizeof(NG_ANIMATION_RECORD) * MAX_NG_ANIMATIONS);
	memset(tables->level_multi_env_condition_table, 0x00, sizeof(NG_MULTI_ENV_CONDITION_RECORD) * MAX_NG_MULTI_ENV_CONDITIONS);
	memset(tables->level_test_position_table, 0x00, sizeof(NG_TEST_POSITION_RECORD) * MAX_NG_TEST_POSITIONS);

	// Params
	memset(tables->level_move_item_table, 0x00, sizeof(NG_MOVE_ITEM_RECORD) * MAX_NG_MOVE_ITEMS);
	memset(tables->level_rotate_item_table , 0x00, sizeof(NG_ROTATE_ITEM_RECORD) * MAX_NG_ROTATE_ITEMS);
	memset(tables->level_big_number_table, 0x00, sizeof(NG_BIG_NUMBER_RECORD) * MAX_NG_BIG_NUMBERS);

	tables->level_global_trigger_count = 0;
	tables->level_trigger_group_count = 0;
	tables->level_organizer_count = 0;
	tables->level_item_group_count = 0;
	tables->level_animation_count = 0;
	tables->level_multi_env_condition_count = 0;
	tables->level_test_position_count = 0;

	// Params
	tables->level_move_item_count = 0;
	tables->level_rotate_item_count = 0;
	tables->level_big_number_count = 0;

	size_t level_block_start_position = offset;
	unsigned short level_block_size = NG_READ_16(gfScriptFile, offset);
	unsigned short level_block_unknown_variable = NG_READ_16(gfScriptFile, offset);

	if (level_block_size == 0) {
		return 0;
	}

	// Defaults for TRNG levels
	get_game_mod_level_audio_info(current_level)->old_cd_trigger_system = false;
	get_game_mod_level_audio_info(current_level)->new_audio_system = true;

	size_t level_block_end_pos = level_block_start_position + level_block_size * sizeof(short);

	size_t command_blocks_parsed = 0;
	size_t command_blocks_failed = 0;

	NGLog(NG_LOG_TYPE_PRINT, "NGReadNGGameflowInfo: === Parsing Level %u ===", current_level);

	short far_view = -1;

	// Do the commands
	while (1) {
		size_t data_block_start_start_position = offset;
		unsigned char current_data_block_size_wide = NG_READ_8(gfScriptFile, offset);

		unsigned char block_type = NG_READ_8(gfScriptFile, offset);

		size_t command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));

		if (offset >= level_block_end_pos) {
			if (offset != level_block_end_pos) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Level block size mismatch! (level %u)", current_level);
			}
			offset = level_block_end_pos;
			break;
		}

		command_blocks_parsed++;

		switch (block_type) {
		case 0x01: {
			// AssignSlot
			unsigned int plugin_id = 0;
			unsigned short dest_slot = NG_READ_16(gfScriptFile, offset);
			unsigned short src_slot = 0;
			if (!is_mod_trng_version_equal_or_greater_than_target(1, 3, 0, 0)) {
				src_slot = NG_READ_16(gfScriptFile, offset);
			} else {
				src_slot = NG_READ_16(gfScriptFile, offset);
				plugin_id = NG_READ_16(gfScriptFile, offset);
			}
#
			if (plugin_id != 0) {
				char* plugin_string = NGGetPluginString(plugin_id);
				if (plugin_string) {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin (%s) AssignSlot(%u, %u) commands are not currently supported (level %u)", plugin_string, dest_slot, src_slot, current_level);
				} else {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin (%u) AssignSlot(%u, %u) are not currently supported (level %u)", plugin_id, dest_slot, src_slot, current_level);
				}
			} else {
				switch (src_slot) {
					case 1:
						game_mod_config.level_info[current_level].objects_info.rubber_boat_slot = dest_slot;
						assign_slot_for_level(current_level, dest_slot, RUBBER_BOAT);
						break;
					case 2:
						game_mod_config.level_info[current_level].objects_info.motor_boat_slot = dest_slot;
						assign_slot_for_level(current_level, dest_slot, MOTOR_BOAT);
						break;
					case 5:
						game_mod_config.level_info[current_level].objects_info.rubber_boat_extra_slot = dest_slot;
						//assign_slot_for_level(current_level, dest_slot, RUBBER_BOAT_LARA);
						break;
					case 6:
						game_mod_config.level_info[current_level].objects_info.motor_boat_extra_slot = dest_slot;
						//assign_slot_for_level(current_level, dest_slot, MOTOR_BOAT_LARA);
						break;
					case 501:
						game_mod_config.level_info[current_level].objects_info.rubber_boat_slot = dest_slot;
						assign_slot_for_level(current_level, dest_slot, RUBBER_BOAT);
						break;
					case 502:
						game_mod_config.level_info[current_level].objects_info.motor_boat_slot = dest_slot;
						assign_slot_for_level(current_level, dest_slot, MOTOR_BOAT);
						break;
					case 505:
						game_mod_config.level_info[current_level].objects_info.rubber_boat_extra_slot = dest_slot;
						//assign_slot_for_level(current_level, dest_slot, RUBBER_BOAT_LARA);
						break;
					case 506:
						game_mod_config.level_info[current_level].objects_info.motor_boat_extra_slot = dest_slot;
						//assign_slot_for_level(current_level, dest_slot, MOTOR_BOAT_LARA);
						break;
					default:
						assign_slot_for_level(current_level, dest_slot, src_slot);
						break;
				}
			}

			break;
		}
		case 0x02: {
			// Snow
			unsigned short snow_type = NG_READ_16(gfScriptFile, offset);
			switch (snow_type) {
			case 0:
				get_game_mod_level_misc_info(current_level)->snow_type = T4P_WEATHER_DISABLED;
				break;
			case 1:
				get_game_mod_level_misc_info(current_level)->snow_type = T4P_WEATHER_ENABLED_IN_SPECIFIC_ROOMS;
				break;
			case 2:
				get_game_mod_level_misc_info(current_level)->snow_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
				break;
			default:
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: unknown snow type! (level %u)", current_level);
				break;
			}
			break;
		}
		case 0x03: {
			// LevelFarView
			far_view = NG_READ_16(gfScriptFile, offset);

			if (far_view != -1) {
				if (far_view > world_far_view)
					far_view = world_far_view;

				get_game_mod_level_environment_info(current_level)->far_view = (unsigned int)far_view * 1024;
			}
			break;
		}
		case 0x04: {
			// FogRange
			// Negative font values appear to control the intensity, but that is currently unsupported
			short fog_start = NG_READ_16(gfScriptFile, offset);
			short fog_end = NG_READ_16(gfScriptFile, offset);

			if (fog_start != -1) {
				get_game_mod_level_environment_info(current_level)->fog_start_range = fog_start * 1024;
			} else {
				get_game_mod_level_environment_info(current_level)->fog_start_range = 12 * 1024;
			}

			if (fog_end != -1) {
				get_game_mod_level_environment_info(current_level)->fog_end_range = fog_end * 1024;
			} else {
				if (far_view != -1) {
					get_game_mod_level_environment_info(current_level)->fog_end_range = far_view * 1024;
				} else {
					if (world_far_view != -1) {
						get_game_mod_level_environment_info(current_level)->fog_end_range = world_far_view * 1024;
					}
				}
			}
			break;
		}
		case 0x05: {
			// WorldViewFar
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: WorldViewFar is not implemented! (level %u)", current_level);
			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x06: {
			// TextFormat
			unsigned short text_color_id = NG_READ_16(gfScriptFile, offset);
			if (text_color_id != 0xffff) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: TextFormat color unimplemented! (level %u)", current_level);
			}
			unsigned short text_format_flags = NG_READ_16(gfScriptFile, offset);
			if (text_format_flags != 0xffff) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: TextFormat flags unimplemented! (level %u)", current_level);
			}

			unsigned short text_blink_time = NG_READ_16(gfScriptFile, offset);
			if (text_blink_time != 0xffff) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: TextFormat blink time unimplemented! (level %u)", current_level);
			}

			unsigned short size_character_menu = NG_READ_16(gfScriptFile, offset);
			if (size_character_menu != 0xffff) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: TextFormat size character menu unimplemented! (level %u)", current_level);
			}

			break;
		}
		case 0x07: {
			// Rain
			unsigned short rain_type = NG_READ_16(gfScriptFile, offset);
			switch (rain_type) {
			case 0:
				get_game_mod_level_misc_info(current_level)->rain_type = T4P_WEATHER_DISABLED;
				break;
			case 1:
				get_game_mod_level_misc_info(current_level)->rain_type = T4P_WEATHER_ENABLED_IN_SPECIFIC_ROOMS;
				break;
			case 2:
				get_game_mod_level_misc_info(current_level)->rain_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
				break;
			default:
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: unknown rain type! (level %u)", current_level);
				break;
			}
			break;
		}
		case 0x08: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Damage unimplemented! (level %u)", current_level);

			// Damage (WIP)
			unsigned short damage_flags = NG_READ_16(gfScriptFile, offset);
			unsigned short seconds_for_death = NG_READ_16(gfScriptFile, offset);
			unsigned short seconds_for_bar_restore = NG_READ_16(gfScriptFile, offset);
			unsigned int bar_color = NG_READ_32(gfScriptFile, offset);
			unsigned short bar_name = NG_READ_16(gfScriptFile, offset);
			unsigned short blink_percentage = NG_READ_16(gfScriptFile, offset);

			break;
		}
		case 0x09: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Enemy configuration unfinished! (level %u)", current_level);

			// Enemy (WIP)
			unsigned short slot = NG_READ_16(gfScriptFile, offset);

			MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = nullptr;
			if (current_level == 0) {
				for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
					mod_object_customization = get_game_mod_level_object_customization_for_slot(i, slot);
				}
			} else {
				mod_object_customization = get_game_mod_level_object_customization_for_slot(current_level, slot);
			}
			
			if (!mod_object_customization) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Invalid slot for enemy info! (level %u)", current_level);
				break;
			}

			unsigned short hp = NG_READ_16(gfScriptFile, offset);
			if (hp != 0xffff) {
				mod_object_customization->hit_points = hp;
				mod_object_customization->override_hit_points = true;
			}

			if (offset < command_block_end_position) {
				unsigned short nef_flags = NG_READ_16(gfScriptFile, offset);
				if (nef_flags != 0xffff) {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Enemy EF_ flags unsupported (level %u)", current_level);
				}
			}

			if (offset < command_block_end_position) {
				unsigned short tomb_flags = NG_READ_16(gfScriptFile, offset);
				if (tomb_flags != 0xffff) {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Enemy TombFlags unsupported (level %u)", current_level);
				}
			}

			if (offset < command_block_end_position) {
				unsigned short extra_flags = NG_READ_16(gfScriptFile, offset);
				if (extra_flags != 0xffff) {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Enemy extra_flags unsupported (level %u)", current_level);
				}
			}

			if (offset < command_block_end_position) {
				unsigned short damage_1 = NG_READ_16(gfScriptFile, offset);
				if (damage_1 != 0xffff) {
					mod_object_customization->damage_1 = damage_1;
				}
			}
			if (offset < command_block_end_position) {
				unsigned short damage_2 = NG_READ_16(gfScriptFile, offset);
				if (damage_2 != 0xffff) {
					mod_object_customization->damage_2 = damage_2;
				}
			}
			if (offset < command_block_end_position) {
				unsigned short damage_3 = NG_READ_16(gfScriptFile, offset);
				if (damage_3 != 0xffff) {
					mod_object_customization->damage_3 = damage_3;
				}
			}

			break;
		}
		case 0x0a: {
			// Animation (WIP)
			if (tables->level_animation_count >= MAX_NG_ANIMATIONS) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Animation overflow! (level %u)", current_level);
				return 0;
				// Broken
			}

			tables->level_animation_table[tables->level_animation_count].record_id = tables->level_animation_count;

			tables->level_animation_table[tables->level_animation_count].record.animation_index = NG_READ_16(gfScriptFile, offset);
			tables->level_animation_table[tables->level_animation_count].record.key_1 = NG_READ_16(gfScriptFile, offset);
			if (tables->level_animation_table[tables->level_animation_count].record.key_1 == 0xffff) {
				tables->level_animation_table[tables->level_animation_count].record.key_1 = 0;
			}
			tables->level_animation_table[tables->level_animation_count].record.key_2 = NG_READ_16(gfScriptFile, offset);
			if (tables->level_animation_table[tables->level_animation_count].record.key_2 == 0xffff) {
				tables->level_animation_table[tables->level_animation_count].record.key_2 = 0;
			}
			tables->level_animation_table[tables->level_animation_count].record.fan_flags = NG_READ_16(gfScriptFile, offset);
			if (tables->level_animation_table[tables->level_animation_count].record.fan_flags == 0xffff) {
				tables->level_animation_table[tables->level_animation_count].record.fan_flags = 0;
			}
			tables->level_animation_table[tables->level_animation_count].record.environment.env_condition = NG_READ_16(gfScriptFile, offset);
			if (tables->level_animation_table[tables->level_animation_count].record.environment.env_condition == 0xffff) {
				tables->level_animation_table[tables->level_animation_count].record.environment.env_condition = 0;
			}
			tables->level_animation_table[tables->level_animation_count].record.environment.distance_for_env = NG_READ_16(gfScriptFile, offset);
			if (tables->level_animation_table[tables->level_animation_count].record.environment.distance_for_env == 0xffff) {
				tables->level_animation_table[tables->level_animation_count].record.environment.distance_for_env = 0;
			}
			tables->level_animation_table[tables->level_animation_count].record.environment.extra = NG_READ_16(gfScriptFile, offset);
			if (tables->level_animation_table[tables->level_animation_count].record.environment.extra == 0xffff) {
				tables->level_animation_table[tables->level_animation_count].record.environment.extra = 0;
			}

			while ((offset != command_block_end_position)) {
				if (tables->level_animation_table[tables->level_animation_count].record.state_or_animation_condition_count >= NG_ANIMATION_CONDTION_MAX_SIZE) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Animation state/condition overflow! (level %u)", current_level);
					return 0;
					// Broken
				}

				tables->level_animation_table[tables->level_animation_count].record.state_or_animation_condition_array[
					tables->level_animation_table[tables->level_animation_count].record.state_or_animation_condition_count] = NG_READ_16(gfScriptFile, offset);

					tables->level_animation_table[tables->level_animation_count].record.state_or_animation_condition_count++;
			}

			tables->level_animation_count++;

			break;
		}
		case 0x0b: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: MirrorEffect Unimplemented! (level %u)", current_level);

			// MirrorEffect (WIP)
			unsigned short in_front_room = NG_READ_16(gfScriptFile, offset);
			unsigned short hidden_room = NG_READ_16(gfScriptFile, offset);
			unsigned short mirror_type = NG_READ_16(gfScriptFile, offset);

			// The rest of it should be an array of animatings:
			while ((offset != command_block_end_position)) {
				unsigned short animating_index = NG_READ_16(gfScriptFile, offset);
				if (animating_index != 0xffff) {

				}
			}
			break;
		}
		case 0x0c: {
			// Elevator
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Elevator Unimplemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x0d: {
			// Keypad
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Keypad Unimplemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x0e: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: AddEffect Unimplemented! (level %u)", current_level);

			// AddEffect
			unsigned short id = NG_READ_16(gfScriptFile, offset);
			unsigned short effect_type = NG_READ_16(gfScriptFile, offset); // Obsolete
			unsigned short flags_effects = NG_READ_16(gfScriptFile, offset);
			unsigned short joint_type = NG_READ_16(gfScriptFile, offset);
			short disp_x = NG_READ_16(gfScriptFile, offset);
			short disp_y = NG_READ_16(gfScriptFile, offset);
			short disp_z = NG_READ_16(gfScriptFile, offset);
			unsigned short durate_emit = NG_READ_16(gfScriptFile, offset);
			unsigned short durate_pause = NG_READ_16(gfScriptFile, offset);

			switch ((NG_ADD_EFFECT_TYPE)effect_type) {
			case NG_ADD_MIST: {
				if (offset == command_block_end_position)
					break;
				unsigned short size_of_mist_ball = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short number_of_mist_balls = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short color_of_mist = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short persistence_of_mist = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				break;
			}
			case NG_ADD_LIGHT_BLINK:
			case NG_ADD_LIGHT_FLAT:
			case NG_ADD_LIGHT_GLOVE:
			case NG_ADD_LIGHT_SPOT: {
				if (offset == command_block_end_position)
					break;
				unsigned short light_intensity = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short maximum_spotlight_distance = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short light_color = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				break;
			}
			case NG_ADD_FLAME: {
				if (offset == command_block_end_position)
					break;
				unsigned short flame_intensity = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short lara_burn_settings = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short flame_direction = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				unsigned short flame_unknown = NG_READ_16(gfScriptFile, offset);
				if (offset == command_block_end_position)
					break;
				break;
			}
			}
			break;
		}
		case 0x0f: {
			// Detector
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Detector Unimplemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x11: {
			// TextureSequence
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: TextureSequence Unimplemented! (level %u)", current_level);
			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x12: {
			// Equipment (TODO: add support for LOAD_AMMO types)
			unsigned short object_id = NG_READ_16(gfScriptFile, offset);
			unsigned short amount = NG_READ_16(gfScriptFile, offset);

			// Special-case for shotgun.
			if (object_id == SHOTGUN_AMMO1_ITEM || object_id == SHOTGUN_AMMO2_ITEM) {
				amount *= 6;
			}

			int i = 0;
			int last = MOD_LEVEL_COUNT;

			if (current_level != 0) {
				i = current_level;
				last = current_level + 1;
			}

			for (; i < last; i++) {
				MOD_EQUIPMENT_MODIFIER* equipment_modifiers = get_game_mod_level_stat_info(i)->equipment_modifiers;

				int current_modifier_idx = 0;
				for (current_modifier_idx = 0; current_modifier_idx < MAX_EQUIPMENT_MODIFIERS; current_modifier_idx++) {
					if (equipment_modifiers[current_modifier_idx].object_id == -1) {
						equipment_modifiers[current_modifier_idx].object_id = object_id;
						equipment_modifiers[current_modifier_idx].amount = amount;

						break;
					}
				}

				if (current_modifier_idx >= MAX_EQUIPMENT_MODIFIERS) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Max equipment modifiers exceeded (level %u)", i);
				}
			}

			break;
		}
		case 0x13: {
			// MultiEnvCondition (WIP)
			unsigned short id = NG_READ_16(gfScriptFile, offset);

			if (id >= MAX_NG_MULTI_ENV_CONDITIONS) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Multi Env Condition id is not valid! (level %u)", current_level);
				return 0;
				// Broken
			}

			tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record_id = id;

			while (offset < command_block_end_position) {
				if (tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record.env_condition_triplet_count >= NG_MULTI_ENV_CONDITION_MAX_TRIPLETS) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: MultiEnvCondition triplet overflow! (level %u)", current_level);
					return 0;
					// Broken
				}

				int index = tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record.env_condition_triplet_count;

				tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record.env_condition_triplet_array[index].env_condition = NG_READ_16(gfScriptFile, offset);
				tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record.env_condition_triplet_array[index].distance_for_env = NG_READ_16(gfScriptFile, offset);
				tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record.env_condition_triplet_array[index].extra = NG_READ_16(gfScriptFile, offset);

				tables->level_multi_env_condition_table[tables->level_multi_env_condition_count].record.env_condition_triplet_count++;
			}

			tables->level_multi_env_condition_count++;

			break;
		}
		case 0x14: {
			// Customize (WIP)
			unsigned int customization_category = 0;
			unsigned int plugin_id = 0;
			if (!is_mod_trng_version_equal_or_greater_than_target(1, 3, 0, 0)) {
				customization_category = NG_READ_16(gfScriptFile, offset);
			} else {
				customization_category = NG_READ_16(gfScriptFile, offset);
				plugin_id = NG_READ_16(gfScriptFile, offset);
			}

			if (plugin_id == 0) {
				switch (customization_category) {
				case CUST_DISABLE_SCREAMING_HEAD: {

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						get_game_mod_level_objects_info(i)->lara_scream_slot = -1;
					}
					break;
				}
				case CUST_SET_SECRET_NUMBER: {
					unsigned short secret_count = NG_READ_16(gfScriptFile, offset);

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						get_game_mod_level_stat_info(i)->secret_count = secret_count;
					}

					break;
				}
				case CUST_SET_CREDITS_LEVEL: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SET_CREDITS_LEVEL unimplemented! (level %u)", current_level);
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_DISABLE_FORCING_ANIM_96: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_DISABLE_FORCING_ANIM_96 unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_ROLLINGBALL_PUSHING: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_ROLLINGBALL_PUSHING unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_NEW_SOUND_ENGINE: {
					unsigned short new_sound_engine_flags = NG_READ_16(gfScriptFile, offset);
					if (new_sound_engine_flags == 0xffff || new_sound_engine_flags == 0) {
						new_sound_engine_flags = 0;
					} else {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_NEW_SOUND_ENGINE flags not supported! (level %u)", current_level);
					}

					unsigned short sound_extension = NG_READ_16(gfScriptFile, offset); // Obsolete
					unsigned short long_fade_out = NG_READ_16(gfScriptFile, offset);
					if (long_fade_out != 0xffff) {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_NEW_SOUND_ENGINE custom long_fade_out unsupported! (level %u)", current_level);
					}

					unsigned short short_fade_out = NG_READ_16(gfScriptFile, offset);
					if (short_fade_out != 0xffff) {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_NEW_SOUND_ENGINE custom short_fade_out unsupported! (level %u)", current_level);
					}

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SPEED_MOVING: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SPEED_MOVING unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SHATTER_RANGE: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SHATTER_RANGE unimplemented! (level %u)", current_level);

					unsigned short first_static_as_shatter = NG_READ_16(gfScriptFile, offset);
					unsigned short last_static_as_shatter = NG_READ_16(gfScriptFile, offset);
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_WEAPON: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_WEAPON unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_AMMO: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_AMMO unimplemented! (level %u)", current_level);

					unsigned short ammo_slot = NG_READ_16(gfScriptFile, offset);
					unsigned short ammo_flags = NG_READ_16(gfScriptFile, offset);
					unsigned short damage = NG_READ_16(gfScriptFile, offset);
					unsigned short shots_for_box = NG_READ_16(gfScriptFile, offset);
					unsigned short shots_with_weapon = NG_READ_16(gfScriptFile, offset);
					unsigned short extra = NG_READ_16(gfScriptFile, offset);
					unsigned short id_trigger_group_when_hit_enemy = NG_READ_16(gfScriptFile, offset);
					unsigned short damage_for_explosion = NG_READ_16(gfScriptFile, offset);
					unsigned short speed = 0xffff;
					if (offset < command_block_end_position) {
						speed = NG_READ_16(gfScriptFile, offset);
					}
					unsigned short gravity = 0xffff;
					if (offset < command_block_end_position) {
						gravity = NG_READ_16(gfScriptFile, offset);
					}
					unsigned short id_add_effect_to_ammo = 0xffff;
					if (offset < command_block_end_position) {
						id_add_effect_to_ammo = NG_READ_16(gfScriptFile, offset);
					}
					unsigned short id_trigger_group_at_end = 0xffff;
					if (offset < command_block_end_position) {
						id_trigger_group_at_end = NG_READ_16(gfScriptFile, offset);
					}

					MOD_LEVEL_WEAPON_INFO *weapon_info = get_game_mod_level_weapon_info(current_level);
					MOD_LEVEL_AMMO_INFO *ammo_info = nullptr;

					switch (ammo_slot) {
						case PISTOLS_AMMO_ITEM:
							ammo_info = &weapon_info->pistol_ammo_info;
							break;
						case UZI_AMMO_ITEM:
							ammo_info = &weapon_info->uzi_ammo_info;
							break;
						case SHOTGUN_AMMO1_ITEM:
							ammo_info = &weapon_info->shotgun_1_ammo_info;
							break;
						case SHOTGUN_AMMO2_ITEM:
							ammo_info = &weapon_info->shotgun_2_ammo_info;
							break;
						case CROSSBOW_AMMO1_ITEM:
							ammo_info = &weapon_info->crossbow_1_ammo_info;
							break;
						case CROSSBOW_AMMO2_ITEM:
							ammo_info = &weapon_info->crossbow_2_ammo_info;
							break;
						case CROSSBOW_AMMO3_ITEM:
							ammo_info = &weapon_info->crossbow_3_ammo_info;
							break;
						case GRENADE_GUN_AMMO1_ITEM:
							ammo_info = &weapon_info->grenade_1_ammo_info;
							break;
						case GRENADE_GUN_AMMO2_ITEM:
							ammo_info = &weapon_info->grenade_2_ammo_info;
							break;
						case GRENADE_GUN_AMMO3_ITEM:
							ammo_info = &weapon_info->grenade_3_ammo_info;
							break;
						case SIXSHOOTER_AMMO_ITEM:
							ammo_info = &weapon_info->six_shooter_ammo_info;
							break;
						default:
							NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: CUST_AMMO unknown ammo type! (level %u)", current_level);
							break;
					}

					if (ammo_info) {
						if (ammo_flags != 0xffff) {
							if (ammo_flags & AMMO_PUSH_TARGET)
								NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: AMMO_FLAG_PUSH_TARGET unsupported! (level %u)", current_level);
							if (ammo_flags & AMMO_PUSH_TARGET)
								NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: AMMO_FLAG_PUSH_LARA unsupported! (level %u)", current_level);
							if (ammo_flags & AMMO_SET_GRENADE_TIMER)
								NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: AMMO_SET_GRENADE_TIMER unsupported! (level %u)", current_level);
							if (ammo_flags & AMMO_ADD_GUN_SHELL)
								ammo_info->add_pistol_shell = true;
							if (ammo_flags & AMMO_ADD_SHOTGUN_SHELL)
								ammo_info->add_shotgun_shell = true;
							if (ammo_flags & AMMO_REMOVE_SHOTGUN_SHELL)
								ammo_info->add_shotgun_shell = false;
						}

						if (damage != 0xffff) {
							ammo_info->damage = damage;
						}

						if (damage_for_explosion != 0xffff) {
							ammo_info->explosion_damage = damage_for_explosion;
						}

						if (shots_for_box != 0xffff) {
							ammo_info->ammo_pickup_amount = shots_for_box;
						}

						if (shots_with_weapon != 0xffff) {
							ammo_info->weapon_pickup_amount = shots_with_weapon;
						}

						if (speed != 0xffff) {
							ammo_info->speed = speed;
						}

						if (gravity != 0xffff) {
							ammo_info->gravity = gravity;
						}

						if (id_trigger_group_when_hit_enemy != 0xffff) {
							ammo_info->trng_trigger_id_when_enemy_hit = id_trigger_group_when_hit_enemy;
						}

						if (id_trigger_group_at_end != 0xffff) {
							ammo_info->trng_trigger_id_at_end = id_trigger_group_at_end;
						}

						if (id_add_effect_to_ammo != 0xffff) {
							NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TRNG AddEffect unsupported! (level %u)", current_level);
							ammo_info->trng_effect = id_add_effect_to_ammo;
						}
					}

					break;
				}
				case CUST_SHOW_AMMO_COUNTER: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SHOW_AMMO_COUNTER unimplemented! (level %u)", current_level);

					unsigned short color = NG_READ_16(gfScriptFile, offset);
					unsigned short format_flags = NG_READ_16(gfScriptFile, offset); // Obsolete
					unsigned short blink_time = NG_READ_16(gfScriptFile, offset);
					unsigned short size_character = NG_READ_16(gfScriptFile, offset);
					unsigned short show_counter_flags = 0xffff;
					if (offset < command_block_end_position) {
						show_counter_flags = NG_READ_16(gfScriptFile, offset);
					}
					break;
				}
				case CUST_SET_INV_ITEM: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SET_INV_ITEM unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SET_JEEP_KEY_SLOT: {
					short jeep_key_slot = NG_READ_16(gfScriptFile, offset);

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						if (jeep_key_slot != -1) {
							get_game_mod_level_objects_info(i)->jeep_key_slot = jeep_key_slot;
						}
					}
					break;
				}
				case CUST_STATIC_TRANSPARENCY: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_STATIC_TRANSPARENCY unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SET_STATIC_DAMAGE: {
					short damage = NG_READ_16(gfScriptFile, offset);
					short poison_intensity = NG_READ_16(gfScriptFile, offset);

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						if (damage != -1) {
							get_game_mod_level_misc_info(i)->damage_static_interaction = damage;
						}
						if (poison_intensity != -1) {
							get_game_mod_level_misc_info(i)->posion_static_interaction = poison_intensity;
						}
					}
					break;
				}
				case CUST_LOOK_TRANSPARENT: {
					short is_enabled = NG_READ_16(gfScriptFile, offset);

					bool use_look_transparency = true;
					if (is_enabled == 0) {
						use_look_transparency = false;
					} else if (is_enabled == 1) {
						use_look_transparency = true;
					} else {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_LOOK_TRANSPARENT type unimplemented! (level %u)", current_level);
					}

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						get_game_mod_level_lara_info(i)->use_look_transparency = use_look_transparency;
					}
					break;
				}
				case CUST_HAIR_TYPE: {
					unsigned short hair_type = NG_READ_16(gfScriptFile, offset);

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						switch (hair_type) {
						case 0x00:
							break;
						case 0x01:
							get_game_mod_level_lara_info(i)->hair_type = LARA_HAIR_TYPE_NONE;
							break;
						case 0x02:
							get_game_mod_level_lara_info(i)->hair_type = LARA_HAIR_TYPE_PIGTAILS;
							break;
						case 0x03:
							get_game_mod_level_lara_info(i)->hair_type = LARA_HAIR_TYPE_BRAID;
							break;
						case 0x04:
							NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_HAIR_TYPE hair type 04 unimplemented! (level %u)", i);
							break;
						case 0xffff:
							break;
						}
					}
					break;
				}
				case CUST_KEEP_DEAD_ENEMIES: {
					short is_enabled = NG_READ_16(gfScriptFile, offset);

					bool fade_dead_enemies = true;
					if (is_enabled == 1) {
						fade_dead_enemies = false;
					} else if (is_enabled == 0) {
						fade_dead_enemies = true;
					} else {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_KEEP_DEAD_ENEMIES type unimplemented! (level %u)", current_level);
					}

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						get_game_mod_level_creature_info(i)->fade_dead_enemies = fade_dead_enemies;
					}
					break;
				}
				case CUST_SET_OLD_CD_TRIGGER: {
					short is_enabled = NG_READ_16(gfScriptFile, offset);

					bool old_cd_trigger = true;
					if (is_enabled == 1) {
						old_cd_trigger = false;
					}
					else if (is_enabled == 0) {
						old_cd_trigger = true;
					} else {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SET_OLD_CD_TRIGGER type unimplemented! (level %u)", current_level);
					}

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						get_game_mod_level_audio_info(i)->old_cd_trigger_system = old_cd_trigger;
					}
					break;
				}
				case CUST_ESCAPE_FLY_CAMERA: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_ESCAPE_FLY_CAMERA unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_PAUSE_FLY_CAMERA: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_PAUSE_FLY_CAMERA unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_TEXT_ON_FLY_SCREEN: {
					bool text_on_fly_screen = NG_READ_16(gfScriptFile, offset);
					get_game_mod_level_misc_info(current_level)->draw_legend_on_flyby = text_on_fly_screen;

					break;
				}
				case CUST_CD_SINGLE_PLAYBACK: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_CD_SINGLE_PLAYBACK unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_ADD_DEATH_ANIMATION: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_ADD_DEATH_ANIMATION unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_BAR: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_BAR unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_NO_TIME_IN_SAVELIST: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_NO_TIME_IN_SAVELIST unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_PARALLEL_BARS: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_PARALLEL_BARS unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_CAMERA: {
					short fcam_properties = NG_READ_16(gfScriptFile, offset);
					short distance_chase_cam = NG_READ_16(gfScriptFile, offset);
					short v_orient_chase_cam = NG_READ_16(gfScriptFile, offset);
					short h_orient_chase_cam = NG_READ_16(gfScriptFile, offset);
					short distance_combat_cam = NG_READ_16(gfScriptFile, offset);
					short v_orient_combat_cam = NG_READ_16(gfScriptFile, offset);
					short distance_look_cam = NG_READ_16(gfScriptFile, offset);
					short height_look_cam = NG_READ_16(gfScriptFile, offset);
					short speed_camera = NG_READ_16(gfScriptFile, offset);

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						if (fcam_properties != -1) {
							if (fcam_properties & 0x01) {
								get_game_mod_level_camera_info(i)->disable_battle_camera = true;
							}
							if (fcam_properties & ~0x01) {
								NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_CAMERA unsupported FCAM_ property! (level %u)", current_level);
							}
						}
						if (distance_chase_cam != -1) {
							get_game_mod_level_camera_info(i)->chase_camera_distance = distance_chase_cam;
						}
						if (v_orient_chase_cam != -1) {
							get_game_mod_level_camera_info(i)->chase_camera_vertical_orientation = v_orient_chase_cam;
						}
						if (h_orient_chase_cam != -1) {
							NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: h_orient_chase_cam is not supported! (level %u)", current_level);
						}
						if (distance_combat_cam != -1) {
							get_game_mod_level_camera_info(i)->combat_camera_distance = distance_combat_cam;
						}
						if (v_orient_combat_cam != -1) {
							get_game_mod_level_camera_info(i)->combat_camera_vertical_orientation = v_orient_combat_cam;
						}
						if (distance_look_cam != -1) {
							get_game_mod_level_camera_info(i)->look_camera_distance = distance_look_cam;
						}
						if (height_look_cam != -1) {
							get_game_mod_level_camera_info(i)->look_camera_height = height_look_cam;
						}
						if (speed_camera != -1) {
							get_game_mod_level_camera_info(i)->camera_speed = speed_camera;
						}
					}

					break;
				}
				case CUST_DISABLE_MISSING_SOUNDS: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_DISABLE_MISSING_SOUNDS unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_INNER_SCREENSHOT: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_INNER_SCREENSHOT unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_FMV_CUTSCENE: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_FMV_CUTSCENE unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_FIX_WATER_FOG_BUG: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_FIX_WATER_FOG_BUG unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SAVE_LOCUST: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SAVE_LOCUST unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_LIGHT_OBJECT: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_LIGHT_OBJECT unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_HARPOON: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_HARPOON unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SCREENSHOT_CAPTURE: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SCREENSHOT_CAPTURE unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_RAIN: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_RAIN unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_TR5_UNDERWATER_COLLISIONS: {
					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						get_game_mod_level_lara_info(i)->use_tr5_swimming_collision = true;
					}
					break;
				}
				case CUST_DARTS: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_DARTS unimplemented! (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_FLARE: {
					unsigned short flare_flags = NG_READ_16(gfScriptFile, offset);
					unsigned short flare_lifetime_in_seconds = NG_READ_16(gfScriptFile, offset);
					unsigned char flare_light_r = NG_READ_16(gfScriptFile, offset);
					unsigned char flare_light_g = NG_READ_16(gfScriptFile, offset);
					unsigned char flare_light_b = NG_READ_16(gfScriptFile, offset);
					unsigned char flare_light_intensity = NG_READ_16(gfScriptFile, offset);

					int i = 0;
					int last = MOD_LEVEL_COUNT;

					if (current_level != 0) {
						i = current_level;
						last = current_level + 1;
					}

					for (; i < last; i++) {
						NGSetupFlareCustomization(i, flare_flags, flare_lifetime_in_seconds, flare_light_r, flare_light_g, flare_light_b, flare_light_intensity);
					}
					break;
				}
				case CUST_SET_TEXT_COLOR: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SET_TEXT_COLOR unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SET_STILL_COLLISION: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SET_STILL_COLLISION unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_WATERFALL_SPEED: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_WATERFALL_SPEED unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_ROLLING_BOAT: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_ROLLING_BOAT unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SFX: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SFX unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_TITLE_FMV: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_TITLE_FMV unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_KEEP_LARA_HP: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_KEEP_LARA_HP unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_BINOCULARS: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_BINOCULARS unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_BACKGROUND: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_BACKGROUND unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_DISABLE_PUSH_AWAY_ANIMATION: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_DISABLE_PUSH_AWAY_ANIMATION unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_SLOT_FLAGS: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SLOT_FLAGS unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case CUST_FIX_BUGS: {
					unsigned char bug_fix_flags_lower = NG_READ_8(gfScriptFile, offset);
					unsigned char bug_fix_flags_upper = 0;

					if (offset != command_block_end_position) {
						bug_fix_flags_upper = NG_READ_8(gfScriptFile, offset);
						if (bug_fix_flags_upper) {
							NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_FIX_BUGS unknown upper bits! (level %u)", current_level);
						}
					}

					if (current_level == 0) {
						for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
							NGSetupBugfixCustomization(i, (bug_fix_flags_upper << 8) | bug_fix_flags_lower);
						}
					}
					else {
						NGSetupBugfixCustomization(current_level, (bug_fix_flags_upper << 8) | bug_fix_flags_lower);
					}

					// Sometimes this can be one or two bytes, so just set it
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));

					break;
				}
				case CUST_SHATTER_SPECIFIC: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: CUST_SHATTER_SPECIFIC unimplemented (level %u)", current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				default: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Unimplemented NG customization category: %u (level %u)", customization_category, current_level);

					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				}

				size_t command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));

				if (offset != command_block_end_position) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Customize block size mismatch for category %u (level %u)", customization_category, current_level);
				}

				// Skip to the end
				offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			} else {
				if (offset != command_block_end_position) {
					char* plugin_string = NGGetPluginString(plugin_id);
					if (plugin_string) {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin (%s) customizations are not currently supported (level %u)", plugin_string, current_level);
					}
					else {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin (%u) customizations are not currently supported (level %u)", plugin_id, current_level);
					}
				}

				// Skip to the end
				offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			}
			break;
		}
		case 0x15:
			// TriggerGroup (legacy/plugin)
			// Older builds of TRNG seem to use this opcode for TriggerGroups. Newer ones seem to use it for triggers exported from plugins
			if (!is_mod_trng_version_equal_or_greater_than_target(1, 3, 0, 0)) {
				// TriggerGroup (WIP)
				unsigned short id = NG_READ_16(gfScriptFile, offset);

				NGLog(NG_LOG_TYPE_PRINT, "Triggergroup %u: (level %u)", id, current_level);

				if (id >= MAX_NG_TRIGGER_GROUPS) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TriggerGroup id (%u) is not valid! (level %u)", id, current_level);
					return 0;
					// Broken
				}

				tables->level_trigger_group_table[tables->level_trigger_group_count].record_id = id;

				unsigned char data_index = 0;
				while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
					unsigned short first_field = NG_READ_16(gfScriptFile, offset);
					// I assume this indicates the end of the command.
					if (first_field == 0x0000 || first_field == 0xffff) {
						break;
					}
					unsigned short second_field = NG_READ_16(gfScriptFile, offset);
					unsigned short third_field = NG_READ_16(gfScriptFile, offset);

					NGLog(NG_LOG_TYPE_PRINT, "0x%04x, 0x%04x, 0x%04x", first_field, second_field, third_field);

					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].plugin_id = 0;
					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].flags = first_field;
					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].object = second_field;
					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].timer = third_field;

					data_index++;
					if (data_index >= NG_TRIGGER_GROUP_DATA_SIZE) {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TriggerGroup size overflow! (level %u)", current_level);
						return 0;
					}

					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data_size = data_index;
				}
				tables->level_trigger_group_count++;
			} else {
				unsigned short id = NG_READ_16(gfScriptFile, offset);

				if (id >= MAX_NG_TRIGGER_GROUPS) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TriggerGroup id (%u) is not valid! (level %u)", id, current_level);
					return 0;
					// Broken
				}

				tables->level_trigger_group_table[tables->level_trigger_group_count].record_id = id;

				unsigned char data_index = 0;
				while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
					unsigned short first_field = NG_READ_16(gfScriptFile, offset);
					// I assume this indicates the end of the command.
					if (first_field == 0x0000 || first_field == 0xffff) {
						break;
					}

					unsigned short plugin_id = NG_READ_16(gfScriptFile, offset);
					unsigned short second_field_lower = NG_READ_16(gfScriptFile, offset);
					unsigned short second_field_upper = NG_READ_16(gfScriptFile, offset);
					unsigned short third_field_lower = NG_READ_16(gfScriptFile, offset);
					unsigned short third_field_upper = NG_READ_16(gfScriptFile, offset);

					if (plugin_id != 0) {
						char *plugin_string = NGGetPluginString(plugin_id);
						if (plugin_string) {
							if (NGGetT4PluginID(plugin_id) == -1) {
								NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TriggerGroup %u - Plugin TriggerGroup plugin:%s, first_field:0x%x, second_field:%u, third_field:0x%x (level %u)",
									id,
									plugin_string,
									first_field,
									((int)second_field_upper << 16 | (int)second_field_lower),
									((int)third_field_upper << 16 | (int)third_field_lower),
									current_level);
							}
						} else {
							NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "TriggerGroup %u - Plugin TriggerGroup plugin_id:%u, first_field:0x%x, second_field:%u, third_field:0x%x (level %u)",
								id,
								plugin_id,
								first_field,
								((int)second_field_upper << 16 | (int)second_field_lower),
								((int)third_field_upper << 16 | (int)third_field_lower),
								current_level);
						}
					}

					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].plugin_id = plugin_id;
					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].flags = first_field;
					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].object = second_field_lower;
					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].timer = third_field_lower;

					data_index++;
					if (data_index >= NG_TRIGGER_GROUP_DATA_SIZE) {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TriggerGroup size overflow! (level %u)", current_level);
						return 0;
					}

					tables->level_trigger_group_table[tables->level_trigger_group_count].record.data_size = data_index;
				}
				tables->level_trigger_group_count++;
			}
			break;
		case 0x16: {
			// Global Trigger
			unsigned short id = NG_READ_16(gfScriptFile, offset);

			if (id >= MAX_NG_GLOBAL_TRIGGERS) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Global Trigger id (%u) is not valid! (level %u)", id, current_level);
				return 0;
				// Broken
			}

			tables->level_global_triggers_table[tables->level_global_trigger_count].record_id = id;

			unsigned short flags = NG_READ_16(gfScriptFile, offset);
			if (flags == 0xffff)
				flags = 0;

			tables->level_global_triggers_table[tables->level_global_trigger_count].record.flags = flags;

			unsigned short global_trigger_type = NG_READ_16(gfScriptFile, offset);
			if (global_trigger_type != GT_USED_INVENTORY_ITEM &&
				global_trigger_type != GT_ENEMY_KILLED &&
				global_trigger_type != GT_LARA_HP_LESS_THAN &&
				global_trigger_type != GT_LARA_HP_HIGHER_THAN &&
				global_trigger_type != GT_LARA_POISONED &&
				global_trigger_type != GT_CONDITION_GROUP &&
				global_trigger_type != GT_COLLIDE_ITEM &&
				global_trigger_type != GT_COLLIDE_SLOT &&
				global_trigger_type != GT_COLLIDE_CREATURE &&
				global_trigger_type != GT_COLLIDE_STATIC_SLOT &&
				global_trigger_type != GT_ALWAYS &&
				global_trigger_type != GT_TRNG_G_TIMER_EQUALS &&
				global_trigger_type != GT_TRNG_L_TIMER_EQUALS &&
				global_trigger_type != GT_SELECTED_INVENTORY_ITEM) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Unimplemented GlobalTrigger type %u (level %u)", global_trigger_type, current_level);
			}
			tables->level_global_triggers_table[tables->level_global_trigger_count].record.type = global_trigger_type;

			tables->level_global_triggers_table[tables->level_global_trigger_count].record.parameter = NG_READ_32(gfScriptFile, offset);
			tables->level_global_triggers_table[tables->level_global_trigger_count].record.condition_trigger_group = NG_READ_16(gfScriptFile, offset);
			tables->level_global_triggers_table[tables->level_global_trigger_count].record.perform_trigger_group = NG_READ_16(gfScriptFile, offset);
			// The block may end here on older version of TRNG
			if (offset < command_block_end_position) {
				tables->level_global_triggers_table[tables->level_global_trigger_count].record.on_false_trigger_group = NG_READ_16(gfScriptFile, offset);
			} else {
				tables->level_global_triggers_table[tables->level_global_trigger_count].record.on_false_trigger_group = 0xffff;
			}

			tables->level_global_trigger_count++;

			break;
		}
		case 0x17: {
			// Organizer
			// 2 bytes - ID
			// 2 bytes - flags (0x01 - FO_ENABLED, 0x02 - FO_LOOP, 0x04 - FO_TICK_TIME)
			// 2 bytes - dummy (?)
			// 2 bytes - execution time
			// 2 bytes - trigger group
			// ... cont.

			unsigned short id = NG_READ_16(gfScriptFile, offset);

			if (id >= MAX_NG_ORGANIZERS) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Organizer id (%u) is not valid! (level %u)", id, current_level);
				return 0;
				// Broken
			}

			tables->level_organizer_table[tables->level_organizer_count].record_id = id;

			unsigned short flags = NG_READ_16(gfScriptFile, offset);;
			if (flags == 0xffff)
				flags = 0;

			// FO_DEMO_ORGANIZER
			if (flags & 0x08) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Organizer FO_DEMO_ORGANIZER flag not supported (level %u)", current_level);
				break;
			}

			tables->level_organizer_table[tables->level_organizer_count].record.flags = flags;
			tables->level_organizer_table[tables->level_organizer_count].record.parameters = NG_READ_16(gfScriptFile, offset);
			if (!(tables->level_organizer_table[tables->level_organizer_count].record.parameters == 0 || tables->level_organizer_table[tables->level_organizer_count].record.parameters == -1)) {
				NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Organizer parameters are not supported! (level %u)", current_level);

			}

			tables->level_organizer_table[tables->level_organizer_count].record.appointment_count = 0;

			unsigned int index = 0;
			unsigned int current_time = 0;
			while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
				int relative_time = NG_READ_16(gfScriptFile, offset);

				// Organizer complete
				if (offset == command_block_end_position) {
					break;
				}

				// !FO_TICK_TIME
				if (!(flags & 0x04)) {
					relative_time *= 30;
				}

				current_time += relative_time;
				tables->level_organizer_table[tables->level_organizer_count].record.appointments[index].time = current_time;
				tables->level_organizer_table[tables->level_organizer_count].record.appointments[index].trigger_group = NG_READ_16(gfScriptFile, offset);

				index++;

				tables->level_organizer_table[tables->level_organizer_count].record.appointment_count = index;

				if (index >= NG_ORGANIZER_MAX_APPOINTMENTS) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Organizer appointment size overflow! (level %u)", current_level);
					return 0;
				}
			}
			tables->level_organizer_count++;
			break;
		}
		case 0x18: {
			// SoundSettings
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Turbo is not implemented! (level %u)", current_level);
			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x19: {
			// Item Groups
			unsigned short id = NG_READ_16(gfScriptFile, offset);

			if (id < MAX_NG_ITEM_GROUPS) {
				tables->level_item_group_table[tables->level_item_group_count].record_id = id;

				unsigned char index = 0;
				while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
					tables->level_item_group_table[tables->level_item_group_count].record.item_list[index] = NG_READ_16(gfScriptFile, offset);

					index++;
					if (index >= NG_ITEM_GROUP_MAX_LIST) {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: ItemGroup record size overflow! (level %u)", current_level);
						return 0;
					}
				}
				tables->level_item_group_table[tables->level_item_group_count].record.item_count = index;
				tables->level_item_group_count++;
				// Broken
			} else {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: ItemGroup id (%u) is not valid! (level %u)", id, current_level);
			}
			break;
		}
		case 0x1a: {
			// ColorRGB (WIP)
			unsigned short id = NG_READ_16(gfScriptFile, offset);
			unsigned short r = NG_READ_16(gfScriptFile, offset);
			unsigned short g = NG_READ_16(gfScriptFile, offset);
			unsigned short b = NG_READ_16(gfScriptFile, offset);

			break;
		}
		case 0x1b: {
			// Parameters (WIP)
			unsigned int param_category = 0;
			unsigned int plugin_id = 0;
			if (!is_mod_trng_version_equal_or_greater_than_target(1, 3, 0, 0)) {
				param_category = NG_READ_16(gfScriptFile, offset);
			} else {
				param_category = NG_READ_16(gfScriptFile, offset);
				plugin_id = NG_READ_16(gfScriptFile, offset);
			}

			if (plugin_id == 0) {
				switch (param_category) {
				case PARAM_MOVE_ITEM: {
					unsigned short id = NG_READ_16(gfScriptFile, offset);

					if (id >= MAX_NG_MOVE_ITEMS) {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Move item id (%u) is not valid! (level %u)", id, current_level);
						return 0;
						// Broken
					}

					unsigned short flags = NG_READ_16(gfScriptFile, offset);
					if (flags == 0xffff || flags == 0) {
						flags = 0;
					} else {
						if (flags & ~(FMOV_INFINITE_LOOP | FMOV_HEAVY_AT_END | FMOV_TRIGGERS_ALL | FMOV_HEAVY_ALL)) {
							NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: PARAM_MOVE_ITEM flags unsupported! (level %u)", current_level);
						}
					}
					unsigned short index_item = NG_READ_16(gfScriptFile, offset);
					unsigned short direction = NG_READ_16(gfScriptFile, offset);
					unsigned short distance = NG_READ_16(gfScriptFile, offset);
					unsigned short speed = NG_READ_16(gfScriptFile, offset);
					short moving_sound = NG_READ_16(gfScriptFile, offset);
					short final_sound = NG_READ_16(gfScriptFile, offset);
					short extra = 0;
					if (offset < command_block_end_position) {
						extra = NG_READ_16(gfScriptFile, offset);
					}

					tables->level_move_item_table[tables->level_move_item_count].record_id = id;
					tables->level_move_item_table[tables->level_move_item_count].record.flags = flags;
					tables->level_move_item_table[tables->level_move_item_count].record.index_item = index_item;
					tables->level_move_item_table[tables->level_move_item_count].record.direction = direction;
					tables->level_move_item_table[tables->level_move_item_count].record.distance = distance;
					tables->level_move_item_table[tables->level_move_item_count].record.speed = speed;
					tables->level_move_item_table[tables->level_move_item_count].record.moving_sound = moving_sound;
					tables->level_move_item_table[tables->level_move_item_count].record.final_sound = final_sound;
					tables->level_move_item_table[tables->level_move_item_count].record.extra = extra;

					tables->level_move_item_count++;

					break;
				}
				case PARAM_ROTATE_ITEM: {
					unsigned short id = NG_READ_16(gfScriptFile, offset);

					if (id >= MAX_NG_ROTATE_ITEMS) {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Rotate item id (%u) is not valid! (level %u)", id, current_level);
						return 0;
						// Broken
					}

					unsigned short flags = NG_READ_16(gfScriptFile, offset);
					if (flags == 0xffff || flags == 0) {
						flags = 0;
					}

					unsigned short index_item = NG_READ_16(gfScriptFile, offset);

					unsigned short dir_h_rotation = NG_READ_16(gfScriptFile, offset);
					unsigned short h_rotation_angle = NG_READ_16(gfScriptFile, offset);
					unsigned short speed_h_rotation = NG_READ_16(gfScriptFile, offset);

					unsigned short dir_v_rotation = NG_READ_16(gfScriptFile, offset);
					unsigned short v_rotation_angle = NG_READ_16(gfScriptFile, offset);
					unsigned short speed_v_rotation = NG_READ_16(gfScriptFile, offset);

					short moving_sound = NG_READ_16(gfScriptFile, offset);
					short final_sound = NG_READ_16(gfScriptFile, offset);

					tables->level_rotate_item_table[tables->level_rotate_item_count].record_id = id;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.flags = flags;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.index_item = index_item;

					tables->level_rotate_item_table[tables->level_rotate_item_count].record.dir_h_rotation = dir_h_rotation;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.h_rotation_angle = h_rotation_angle;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.speed_h_rotation = speed_h_rotation;

					tables->level_rotate_item_table[tables->level_rotate_item_count].record.dir_v_rotation = dir_v_rotation;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.v_rotation_angle = v_rotation_angle;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.speed_v_rotation = speed_v_rotation;

					tables->level_rotate_item_table[tables->level_rotate_item_count].record.moving_sound = moving_sound;
					tables->level_rotate_item_table[tables->level_rotate_item_count].record.final_sound = final_sound;

					tables->level_move_item_count++;

					break;
				}
				case PARAM_COLOR_ITEM: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_COLOR_ITEM not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_PRINT_TEXT: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_PRINT_TEXT not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_SET_CAMERA: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_SET_CAMERA not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_BIG_NUMBERS: {
					if (tables->level_big_number_count >= MAX_NG_BIG_NUMBERS) {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Big number id (%u) is not valid! (level %u)", tables->level_big_number_count, current_level);
						return 0;
						// Broken
					}

					while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
						tables->level_big_number_table[tables->level_big_number_count].record_id = tables->level_big_number_count;
						tables->level_big_number_table[tables->level_big_number_count].record.big_number = NG_READ_16(gfScriptFile, offset);
						tables->level_big_number_count++;
					}

					break;
				}
				case PARAM_SCALE_ITEM: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_SCALE_ITEM not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_SHOW_SPRITE: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_SHOW_SPRITE not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_TRIANGLE: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_TRIANGLE not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_QUADRILATERAL: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_QUADRILATERAL not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_CIRCLE: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_CIRCLE not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_LIGHTNING: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_LIGHTNING not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_WTEXT: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_WTEXT not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_RECT: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_RECT not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_SWAP_ANIMATIONS: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_SWAP_ANIMATIONS not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_ACTOR_SPEECH: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_ACTOR_SPEECH not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				case PARAM_INPUT_BOX: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category PARAM_INPUT_BOX not implemented! (level %u)", current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				default: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Parameter category %u not implemented! (level %u)", param_category, current_level);

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				}
			} else {
				if (offset != command_block_end_position) {
					char* plugin_string = NGGetPluginString(plugin_id);
					if (plugin_string) {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin (%s) parameters are not currently supported (level %u)", plugin_string, current_level);
					}
					else {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin (%u) parameters are not currently supported (level %u)", plugin_id, current_level);
					}
				}

				// Skip to the end
				offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			}
			break;
		}
		case 0x1c: {
			// Turbo (WIP)
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Turbo is not implemented! (level %u)", current_level);
			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x1d: {
			// WindowTitle (WIP)
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: WindowTitle is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x1e: {
			// TestPosition (WIP)
			unsigned short id = NG_READ_16(gfScriptFile, offset);

			if (id >= MAX_NG_TEST_POSITIONS) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Test position id (%u) is not valid! (level %u)", id, current_level);
				return 0;
				// Broken
			}

			tables->level_test_position_table[tables->level_test_position_count].record_id = id;

			tables->level_test_position_table[tables->level_test_position_count].record.flags = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.moveable_slot = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.x_distance_min = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.x_distance_max = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.y_distance_min = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.y_distance_max = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.z_distance_min = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.z_distance_max = NG_READ_16(gfScriptFile, offset);

			tables->level_test_position_table[tables->level_test_position_count].record.h_orient_diff_min = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.h_orient_diff_max = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.v_orient_diff_min = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.v_orient_diff_max = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.r_orient_diff_min = NG_READ_16(gfScriptFile, offset);
			tables->level_test_position_table[tables->level_test_position_count].record.r_orient_diff_max = NG_READ_16(gfScriptFile, offset);

			tables->level_test_position_count++;
			break;
		}
		case 0x1f: {
			// LogItem
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: LogItem is not implemented (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x20: {
			// WindowsFont
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: WindowsFont(?) is not implemented! (level %u)", current_level);

			unsigned short id = NG_READ_16(gfScriptFile, offset);
			unsigned short window_font_name = NG_READ_16(gfScriptFile, offset);
			unsigned short windows_font_flags = NG_READ_16(gfScriptFile, offset);
			unsigned short size_font = NG_READ_16(gfScriptFile, offset);
			unsigned short color_rgb_id = NG_READ_16(gfScriptFile, offset);
			unsigned short shadow_color_rgb_id = NG_READ_16(gfScriptFile, offset);

			break;
		}
		case 0x21: {
			// Diary
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Diary is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x22: {
			// Image
			unsigned short image_command_id = NG_READ_16(gfScriptFile, offset);
			unsigned short image_file = NG_READ_16(gfScriptFile, offset);
			unsigned short image_flags = NG_READ_16(gfScriptFile, offset);
			unsigned short effect_time = NG_READ_16(gfScriptFile, offset);
			unsigned short audio_track = NG_READ_16(gfScriptFile, offset);
			unsigned short x_position = NG_READ_16(gfScriptFile, offset);
			unsigned short y_position = NG_READ_16(gfScriptFile, offset);
			unsigned short size_x = NG_READ_16(gfScriptFile, offset);
			unsigned short size_y = NG_READ_16(gfScriptFile, offset);

			break;
		}
		case 0x23: {
			// SavegamePanel
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: SavegamePanel is not implemented (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x24: {
			// DiagnosticType
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: DiagnosticType is not implemented (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x25: {
			// Switch
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Switch is not implemented (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x26: {
			// CombineItems
			unsigned short first_item = NG_READ_16(gfScriptFile, offset);
			unsigned short second_item = NG_READ_16(gfScriptFile, offset);
			unsigned short final_item = NG_READ_16(gfScriptFile, offset);
			break;
		}
		case 0x27: {
			// Standby
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Standby is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x28: {
			// AnimationSlot
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: AnimationSlot is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x29: {
			// DefaultWindowsFont
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: DefaultWindowsFont is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x2a: {
			// Demo
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Demo is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x2b: {
			// Plugin
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x2c: {
			// LaraStartPos
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: LaraStartPos is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x2d: {
			// StaticMIP
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: StaticMIP is not implemented! (level %u)", current_level);

			// Skip to the end
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			break;
		}
		case 0x2e: {
			// TriggerGroupWord
			unsigned short id = NG_READ_16(gfScriptFile, offset);

			NGLog(NG_LOG_TYPE_PRINT, "Triggergroup %u: (level %u)", id, current_level);

			if (id >= MAX_NG_TRIGGER_GROUPS) {
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TriggerGroup id (%u) is not valid! (level %u)", id, current_level);

				return 0;
				// Broken
			}

			tables->level_trigger_group_table[tables->level_trigger_group_count].record_id = id;

			unsigned char data_index = 0;
			while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
				unsigned short first_field = NG_READ_16(gfScriptFile, offset);
				// I assume this indicates the end of the command.
				if (first_field == 0x0000 || first_field == 0xffff) {
					break;
				}
				unsigned short second_field = NG_READ_16(gfScriptFile, offset);
				unsigned short third_field = NG_READ_16(gfScriptFile, offset);

				NGLog(NG_LOG_TYPE_PRINT, "0x%04x, 0x%04x, 0x%04x", first_field, second_field, third_field);

				tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].plugin_id = 0;
				tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].flags = first_field;
				tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].object = second_field;
				tables->level_trigger_group_table[tables->level_trigger_group_count].record.data[data_index].timer = third_field;

				data_index++;
				if (data_index > NG_TRIGGER_GROUP_DATA_SIZE) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: TriggerGroup size overflow! (level %u)", current_level);
					return 0;
				}

				tables->level_trigger_group_table[tables->level_trigger_group_count].record.data_size = data_index;
			}
			tables->level_trigger_group_count++;
			break;
		}
		case 0xc9: {
			// Level flags
			unsigned short flags = NG_READ_16(gfScriptFile, offset);

			#define UPDATE_LEVEL_INFO_WITH_FLAGS \
			MOD_LEVEL_MISC_INFO *misc_info = get_game_mod_level_misc_info(current_level); \
			if (flags & 0x04) \
				misc_info->override_fog_mode = T4P_FOG_FORCE_VOLUMETRIC; \
			if (flags & 0x08) \
				misc_info->override_fog_mode = T4P_FOG_FORCE_DISTANT;

			if (current_level == 0) {
				for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
					UPDATE_LEVEL_INFO_WITH_FLAGS
				}
			} else {
				UPDATE_LEVEL_INFO_WITH_FLAGS
			}

			break;
		}
		default: {
			offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			command_blocks_failed++;

			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Unimplemented NG level data block type: %u! (level %u)", block_type, current_level);
			break;
		}
		}
		if (offset != command_block_end_position) {
			intmax_t size_difference = offset - command_block_end_position;
			NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Level command block size mismatch for command %u! (level %u), off by %i", block_type, current_level, size_difference);
		}
		offset = command_block_end_position;
	}

	return offset;
}

void NGAllocateLevelRecordTablesContent(NG_LEVEL_RECORD_TABLES *tables) {
	tables->level_global_trigger_count = 0;
	tables->level_trigger_group_count = 0;
	tables->level_organizer_count = 0;
	tables->level_item_group_count = 0;
	tables->level_animation_count = 0;
	tables->level_multi_env_condition_count = 0;
	tables->level_test_position_count = 0;

	// Params
	tables->level_move_item_count = 0;
	tables->level_rotate_item_count = 0;
	tables->level_big_number_count = 0;

	tables->level_global_triggers_table = (NG_GLOBAL_TRIGGER_RECORD*)SYSTEM_MALLOC(sizeof(NG_GLOBAL_TRIGGER_RECORD) * MAX_NG_GLOBAL_TRIGGERS);
	tables->level_trigger_group_table = (NG_TRIGGER_GROUP_RECORD*)SYSTEM_MALLOC(sizeof(NG_TRIGGER_GROUP_RECORD) * MAX_NG_TRIGGER_GROUPS);
	tables->level_organizer_table = (NG_ORGANIZER_RECORD*)SYSTEM_MALLOC(sizeof(NG_ORGANIZER_RECORD) * MAX_NG_ORGANIZERS);
	tables->level_item_group_table = (NG_ITEM_GROUP_RECORD*)SYSTEM_MALLOC(sizeof(NG_ITEM_GROUP_RECORD) * MAX_NG_ITEM_GROUPS);
	tables->level_animation_table = (NG_ANIMATION_RECORD*)SYSTEM_MALLOC(sizeof(NG_ANIMATION_RECORD) * MAX_NG_ANIMATIONS);
	tables->level_multi_env_condition_table = (NG_MULTI_ENV_CONDITION_RECORD*)SYSTEM_MALLOC(sizeof(NG_MULTI_ENV_CONDITION_RECORD) * MAX_NG_MULTI_ENV_CONDITIONS);
	tables->level_test_position_table = (NG_TEST_POSITION_RECORD*)SYSTEM_MALLOC(sizeof(NG_TEST_POSITION_RECORD) * MAX_NG_TEST_POSITIONS);

	// Params
	tables->level_move_item_table = (NG_MOVE_ITEM_RECORD*)SYSTEM_MALLOC(sizeof(NG_MOVE_ITEM_RECORD) * MAX_NG_MOVE_ITEMS);
	tables->level_rotate_item_table = (NG_ROTATE_ITEM_RECORD*)SYSTEM_MALLOC(sizeof(NG_ROTATE_ITEM_RECORD) * MAX_NG_ROTATE_ITEMS);
	tables->level_big_number_table = (NG_BIG_NUMBER_RECORD*)SYSTEM_MALLOC(sizeof(NG_BIG_NUMBER_RECORD) * MAX_NG_BIG_NUMBERS);
}

void NGFreeLevelRecordTablesContent(NG_LEVEL_RECORD_TABLES *tables) {
	SYSTEM_FREE(tables->level_global_triggers_table);
	SYSTEM_FREE(tables->level_trigger_group_table);
	SYSTEM_FREE(tables->level_organizer_table);
	SYSTEM_FREE(tables->level_item_group_table);
	SYSTEM_FREE(tables->level_animation_table);
	SYSTEM_FREE(tables->level_multi_env_condition_table);
	SYSTEM_FREE(tables->level_test_position_table);

	// Params
	SYSTEM_FREE(tables->level_move_item_table);
	SYSTEM_FREE(tables->level_rotate_item_table);
	SYSTEM_FREE(tables->level_big_number_table);
}

void NGReadNGGameflowInfo(char *gfScriptFile, size_t offset, size_t len) {
	bool ng_header_found = false;

	unsigned int footer_ident = NG_READ_32(gfScriptFile, offset);
	if (footer_ident != 0x454c474e) { // NGLE
		return;
	}

	unsigned int footer_offset = NG_READ_32(gfScriptFile, offset);
	offset -= footer_offset;

	unsigned short header_ident = NG_READ_16(gfScriptFile, offset);
	if (header_ident != 0x474e) { // NGLE
		return;
	}

	ng_header_found = true;

	if (ng_header_found) {
		if (!is_mod_trng_version_equal_or_greater_than_target(0, 0, 0, 0)) {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGHeader found in Script.dat in invalid TRNG version!");
			return;
		}

		// TRNG Stuff
		get_game_mod_global_info()->trng_extended_flipmap_bitmask = true;
		get_game_mod_global_info()->trng_new_triggers = true;
		get_game_mod_global_info()->trng_anim_commands_enabled = true;
		get_game_mod_global_info()->trng_timerfields_enabled = true;
		get_game_mod_global_info()->trng_rollingball_extended_ocb = true;
		get_game_mod_global_info()->trng_statics_extended_ocb = true;
		get_game_mod_global_info()->trng_pushable_extended_ocb = true;
		if (!is_mod_trng_version_equal_or_greater_than_target(1, 2, 2, 3)) {
			get_game_mod_global_info()->trng_switch_extended_ocb = true;
		}
		get_game_mod_global_info()->trng_hack_allow_meshes_with_exactly_256_vertices = true;
		get_game_mod_global_info()->trng_advanced_block_raising_behaviour = true;
		get_game_mod_global_info()->trng_pushables_have_gravity = true;
		get_game_mod_global_info()->trng_legacy_ng_trigger_behaviour = !is_mod_trng_version_equal_or_greater_than_target(1, 3, 0, 0);

		for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
			MOD_LEVEL_ENVIRONMENT_INFO *environment_info = get_game_mod_level_environment_info(i);
			environment_info->room_swamp_flag = ROOM_SWAMP;
			environment_info->room_cold_flag = ROOM_COLD;
			environment_info->room_damage_flag = ROOM_DAMAGE;
			
			MOD_LEVEL_GFX_INFO* gfx_info = get_game_mod_level_gfx_info(i);
			gfx_info->cold_breath = COLD_BREATH_ENABLED_IN_COLD_ROOMS;
		}

		size_t options_header_block_start_position = offset;

		unsigned short options_header_block_size = NG_READ_16(gfScriptFile, offset);
		size_t options_header_block_end_pos = options_header_block_start_position + (options_header_block_size * sizeof(short));
		unsigned short options_block_unknown_variable = NG_READ_16(gfScriptFile, offset);

		unsigned short world_far_view = 127;

		while (1) {
			size_t data_block_start_start_position = offset;
			unsigned char current_data_block_size_wide = NG_READ_8(gfScriptFile, offset);

			unsigned char block_type = NG_READ_8(gfScriptFile, offset);

			if (offset >= options_header_block_end_pos) {
				if (offset != options_header_block_end_pos) {
					NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Options header block size mismatch!");
				}
				break;
			}

			switch (block_type) {
				// WorldFarView
				case 0x05: {
					world_far_view = NG_READ_16(gfScriptFile, offset);
					if (world_far_view == 0xffff) {
						world_far_view = 127;
					}
					for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
						get_game_mod_level_environment_info(i)->far_view = (unsigned int)world_far_view * 1024;
					}
					break;
				}
				// Settings
				case 0x10: {
					short flags = NG_READ_16(gfScriptFile, offset);
					break;
				}
				// DiagnosticType
				case 0x24: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: DiagnosticType is not implemented.");

					// Skip to the end
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
				// Plugin
				case 0x2b: {
					unsigned short plugin_id = NG_READ_16(gfScriptFile, offset);
					if (plugin_id >= MAX_NG_PLUGINS) {
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin id exceeds MAX_NG_PLUGINS!");
						break;
					}

					unsigned short plugin_string_id = NG_READ_16(gfScriptFile, offset);

					unsigned short plugin_settings = NG_READ_16(gfScriptFile, offset);
					if (plugin_settings != 0xffff)
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin settings is not supported!");

					unsigned short disable_array = NG_READ_16(gfScriptFile, offset);
					if (disable_array != 0xffff)
						NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Plugin disable array is not supported!");

					ng_plugins[plugin_id].ng_plugin_string_id = plugin_string_id;
					ng_plugins[plugin_id].is_enabled = true;

					char *plugin_string = NGGetPluginString(plugin_id);

					if (plugin_string) {
						ng_plugins[plugin_id].t4plus_plugin = T4PlusFindRegisteredPluginByName(plugin_string);
						if (ng_plugins[plugin_id].t4plus_plugin == -1) {
							NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Plugin %s was not included in the game's manifest.", plugin_string);
						} else {
							NGLog(NG_LOG_TYPE_PRINT, "NGReadNGGameflowInfo: Plugin %s was successfully linked with NG plugin interface.", plugin_string);
						}
					} else {
						NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Plugin string was not found.");
					}

					break;
				}
				// FlagOptions
				case 0xc8: {
					unsigned short flags = NG_READ_16(gfScriptFile, offset);
					bool diagnostics = flags & 0x01;
					bool crypt_script = flags & 0x02;
					bool crypt_savegame = flags & 0x04;
					bool crs = flags & 0x08;
					bool bump_mapping_enable = flags & 0x10;
					bool bump_mapping_disable = flags & 0x20;
					get_game_mod_global_info()->show_lara_in_title = flags & 0x40;
					bool disable_advanced_audio_engine = flags & 0x80;
					bool enable_trlm_option = flags & 0x100;
					break;
				}
				default: {
					NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGReadNGGameflowInfo: Unimplemented NG option data block type: %u", block_type);
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
			}

			size_t command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			if (offset != command_block_end_position) {
				intmax_t size_difference = offset - command_block_end_position;
				NGLog(NG_LOG_TYPE_ERROR, "NGReadNGGameflowInfo: Options command block size mismatch for command %u! off by %i", block_type, size_difference);
			}
			offset = command_block_end_position;
		}
		
		offset = options_header_block_end_pos;

		unsigned short second_header_block_size = NG_READ_16(gfScriptFile, offset);
		offset += (second_header_block_size - 1) * sizeof(short);

		int current_level = 0;

		// Allocate the trigger table for each level.
		NG_LEVEL_RECORD_TABLES record_tables;

		NGAllocateLevelRecordTablesContent(&record_tables);

		// Do the levels
		while (1) {
			// Call level block load function here!
			offset = NGReadLevelBlock(gfScriptFile, offset, &record_tables, current_level, world_far_view);
			if (offset == 0) {
				break;
			}

			NG_TABLE_ALLOCATION_COUNT table_allocation_count;

			table_allocation_count.global_trigger_table_count = record_tables.level_global_trigger_count;
			table_allocation_count.trigger_group_table_count = record_tables.level_trigger_group_count;
			table_allocation_count.organizer_table_count = record_tables.level_organizer_count;
			table_allocation_count.item_group_table_count = record_tables.level_item_group_count;
			table_allocation_count.animation_table_count = record_tables.level_animation_count;
			table_allocation_count.multi_env_condition_table_count = record_tables.level_multi_env_condition_count;
			table_allocation_count.test_position_table_count = record_tables.level_test_position_count;

			// Params
			table_allocation_count.move_item_table_count = record_tables.level_move_item_count;
			table_allocation_count.rotate_item_table_count = record_tables.level_rotate_item_count;
			table_allocation_count.big_number_table_count = record_tables.level_big_number_count;

			// Now save the tables
			NGReallocateLevel(ng_levels[current_level], table_allocation_count);
			
			memcpy(ng_levels[current_level].records->global_trigger_table, record_tables.level_global_triggers_table, sizeof(NG_GLOBAL_TRIGGER_RECORD) * record_tables.level_global_trigger_count);
			memcpy(ng_levels[current_level].records->trigger_group_table, record_tables.level_trigger_group_table, sizeof(NG_TRIGGER_GROUP_RECORD) * record_tables.level_trigger_group_count);
			memcpy(ng_levels[current_level].records->organizer_table, record_tables.level_organizer_table, sizeof(NG_ORGANIZER_RECORD) * record_tables.level_organizer_count);
			memcpy(ng_levels[current_level].records->item_group_table, record_tables.level_item_group_table, sizeof(NG_ITEM_GROUP_RECORD) * record_tables.level_item_group_count);
			memcpy(ng_levels[current_level].records->animation_table, record_tables.level_animation_table, sizeof(NG_ANIMATION_RECORD) * record_tables.level_animation_count);
			memcpy(ng_levels[current_level].records->multi_env_condition_table, record_tables.level_multi_env_condition_table, sizeof(NG_MULTI_ENV_CONDITION_RECORD) * record_tables.level_multi_env_condition_count);
			memcpy(ng_levels[current_level].records->test_position_table, record_tables.level_test_position_table, sizeof(NG_TEST_POSITION_RECORD) * record_tables.level_test_position_count);

			// Params
			memcpy(ng_levels[current_level].records->move_item_table, record_tables.level_move_item_table, sizeof(NG_MOVE_ITEM_RECORD) * record_tables.level_move_item_count);
			memcpy(ng_levels[current_level].records->rotate_item_table, record_tables.level_rotate_item_table, sizeof(NG_ROTATE_ITEM_RECORD)* record_tables.level_rotate_item_count);
			memcpy(ng_levels[current_level].records->big_number_table, record_tables.level_big_number_table, sizeof(NG_BIG_NUMBER_RECORD) * record_tables.level_big_number_count);

			current_level++;
		}

		// Cleanup
		NGFreeLevelRecordTablesContent(&record_tables);
	}
}

void NGReadNGExtraStrings(char *gfLanguageFile, size_t offset, size_t len) {
	unsigned int footer_ident = NG_READ_32(gfLanguageFile, offset);
	if (footer_ident != 0x454c474e) { // NGLE
		return;
	}

	unsigned int footer_offset = NG_READ_32(gfLanguageFile, offset);
	offset -= footer_offset;

	unsigned short header_ident = NG_READ_16(gfLanguageFile, offset);
	if (header_ident != 0x474e) { // NGLE
		return;
	}

	unsigned short block_len = NG_READ_16(gfLanguageFile, offset);
	unsigned short block_type = NG_READ_16(gfLanguageFile, offset);

	if (block_type != 0x800a) {
		return;
	}

	unsigned short string_count = NG_READ_16(gfLanguageFile, offset);

	for (int i = 0; i < string_count; i++) {
		unsigned int string_id = NG_READ_16(gfLanguageFile, offset);
		unsigned int string_len = NG_READ_16(gfLanguageFile, offset);
		string_len *= 2; // Two byte

		if (string_id < 0 || string_id >= MAX_NG_STRINGS) {
			NGLog(NG_LOG_TYPE_ERROR, "Invalid string ID %u!", string_id);
			continue;
		}

		char *current_string = (char *)SYSTEM_MALLOC(string_len);
		if (current_string) {
			memset(current_string, 0x00, string_len);

			for (unsigned int j = 0; j < string_len; j++) {
				current_string[j] = NG_READ_8(gfLanguageFile, offset);
				if (current_string[j] != 0x00) {
					current_string[j] ^= 0xa5;
				}
			}
			
			// Detect duplicates...
			if (ng_strings[string_id]) {
				SYSTEM_FREE(ng_strings[string_id]);
				ng_strings[string_id] = NULL;
				NGLog(NG_LOG_TYPE_ERROR, "Duplicate string ID %u!", string_id);
			}

			ng_strings[string_id] = current_string;
			NGLog(NG_LOG_TYPE_PRINT, "NGString (%u): %s", string_id, current_string);
		} else {
			NGLog(NG_LOG_TYPE_ERROR, "Failed to allocate memory for string %u!", string_id);
		}
	}
}