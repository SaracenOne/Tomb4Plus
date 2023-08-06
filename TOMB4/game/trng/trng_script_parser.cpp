#include "../../tomb4/pch.h"

#include "trng.h"
#include "trng_script_parser.h"

#include "../../tomb4/mod_config.h"

NG_LEVEL ng_levels[MAX_NG_LEVELS];

NG_GLOBAL_TRIGGER current_global_triggers[MAX_NG_GLOBAL_TRIGGERS];
NG_TRIGGER_GROUP current_trigger_groups[MAX_NG_TRIGGER_GROUPS];
NG_ORGANIZER current_organizers[MAX_NG_ORGANIZERS];
NG_ITEM_GROUP current_item_groups[MAX_NG_ITEM_GROUPS];

#define NG_READ_8(scr_buffer, scr_offset) scr_buffer[scr_offset]; \
offset += sizeof(char)

#define NG_READ_16(scr_buffer, scr_offset) (unsigned short)((unsigned char)scr_buffer[scr_offset]) | ((unsigned short)(scr_buffer[scr_offset + 1])) << 8; \
scr_offset += sizeof(short)

#define NG_READ_32(scr_buffer, scr_offset) (unsigned int)(((unsigned char)scr_buffer[scr_offset]) | (((unsigned int)(unsigned char)scr_buffer[scr_offset + 1]) << 8) | (((unsigned int)(unsigned char)scr_buffer[scr_offset + 2]) << 16) | (((unsigned int)(unsigned char)scr_buffer[scr_offset + 3]) << 24)); \
scr_offset += sizeof(int)

void NGInitLevelArray() {
	for (int i = 0; i < MAX_NG_LEVELS; i++) {
		ng_levels[i].records = NULL;
	}
}

void NGFreeLevel(NG_LEVEL& level) {
	if (level.records) {
		level.records->global_trigger_count = 0;
		if (level.records->global_triggers_table) {
			free(level.records->global_triggers_table);
			level.records->global_triggers_table = NULL;
		}

		level.records->trigger_group_count = 0;
		if (level.records->trigger_group_table) {
			free(level.records->trigger_group_table);
			level.records->trigger_group_table = NULL;
		}

		level.records->organizer_count = 0;
		if (level.records->organizer_table) {
			free(level.records->organizer_table);
			level.records->organizer_table = NULL;
		}

		level.records->item_group_count = 0;
		if (level.records->item_group_table) {
			free(level.records->item_group_table);
			level.records->item_group_table = NULL;
		}
	}
}

bool NGReallocateLevel(NG_LEVEL& level, unsigned int global_triggers_table_count, unsigned int trigger_group_table_count, unsigned int organizer_table_count, unsigned int item_group_table_count) {
	NGFreeLevel(level);

	level.records = (NG_LEVEL_RECORD_DATA*)malloc(sizeof(NG_LEVEL_RECORD_DATA));

	if (level.records) {
		level.records->global_triggers_table = NULL;
		level.records->trigger_group_table = NULL;
		level.records->organizer_table = NULL;
		level.records->item_group_table = NULL;
		
		level.records->global_trigger_count = global_triggers_table_count;
		if (global_triggers_table_count) {
			level.records->global_triggers_table = (NG_GLOBAL_TRIGGER_RECORD*)malloc(sizeof(NG_GLOBAL_TRIGGER_RECORD) * global_triggers_table_count);
			if (!level.records->global_triggers_table) {
				printf("Memory allocation failed!");
				return false;
			}
			memset(level.records->global_triggers_table, 0x00, sizeof(NG_GLOBAL_TRIGGER_RECORD) * global_triggers_table_count);
		}

		level.records->trigger_group_count = trigger_group_table_count;
		if (trigger_group_table_count) {
			level.records->trigger_group_table = (NG_TRIGGER_GROUP_RECORD*)malloc(sizeof(NG_TRIGGER_GROUP_RECORD) * trigger_group_table_count);
			if (!level.records->trigger_group_table) {
				printf("Memory allocation failed!");
				return false;
			}
			memset(level.records->trigger_group_table, 0x00, sizeof(NG_TRIGGER_GROUP_RECORD) * trigger_group_table_count);
		}


		level.records->organizer_count = organizer_table_count;
		if (organizer_table_count) {
			level.records->organizer_table = (NG_ORGANIZER_RECORD*)malloc(sizeof(NG_ORGANIZER_RECORD) * organizer_table_count);
			if (!level.records->organizer_table) {
				printf("Memory allocation failed!");
				return false;
			}
			memset(level.records->organizer_table, 0x00, sizeof(NG_ORGANIZER_RECORD) * organizer_table_count);
		}

		level.records->item_group_count = item_group_table_count;
		if (item_group_table_count) {
			level.records->item_group_table = (NG_ITEM_GROUP_RECORD*)malloc(sizeof(NG_ITEM_GROUP_RECORD) * item_group_table_count);
			if (!level.records->item_group_table) {
				printf("Memory allocation failed!");
				return false;
			}
			memset(level.records->item_group_table, 0x00, sizeof(NG_ITEM_GROUP) * item_group_table_count);
		}

		return true;
	} else {
		printf("Memory allocation failed!");
		return false;
	}
}

void NGLoadTablesForLevel(unsigned int level) {
	memset(&current_global_triggers, 0x00, sizeof(NG_GLOBAL_TRIGGER) * MAX_NG_GLOBAL_TRIGGERS);
	memset(&current_trigger_groups, 0x00, sizeof(NG_TRIGGER_GROUP) * MAX_NG_TRIGGER_GROUPS);
	memset(&current_organizers, 0x00, sizeof(NG_ORGANIZER) * MAX_NG_ORGANIZERS);
	memset(&current_item_groups, 0x00, sizeof(NG_ITEM_GROUP) * MAX_NG_ITEM_GROUPS);

	if (ng_levels[level].records) {
		for (int i = 0; i < ng_levels[level].records->global_trigger_count; i++) {
			unsigned int id = ng_levels[level].records->global_triggers_table[i].record_id;

			memcpy(&current_global_triggers[id], &ng_levels[level].records->global_triggers_table[i].global_trigger, sizeof(NG_GLOBAL_TRIGGER));
		}
		for (int i = 0; i < ng_levels[level].records->trigger_group_count; i++) {
			unsigned int id = ng_levels[level].records->trigger_group_table[i].record_id;

			memcpy(&current_trigger_groups[id], &ng_levels[level].records->trigger_group_table[i].trigger_group, sizeof(NG_TRIGGER_GROUP));
		}
		for (int i = 0; i < ng_levels[level].records->organizer_count; i++) {
			unsigned int id = ng_levels[level].records->organizer_table[i].record_id;

			memcpy(&current_organizers[id], &ng_levels[level].records->organizer_table[i].organizer, sizeof(NG_ORGANIZER));
		}
		for (int i = 0; i < ng_levels[level].records->item_group_count; i++) {
			unsigned int id = ng_levels[level].records->item_group_table[i].record_id;

			memcpy(&current_item_groups[id], &ng_levels[level].records->item_group_table[i].item_group, sizeof(NG_ITEM_GROUP));
		}
	}
}

extern void NGScriptCleanup() {
	for (int i = 0; i < MAX_NG_LEVELS; i++) {
		NGFreeLevel(ng_levels[i]);
	}
}

void NGReadNGGameflowInfo(char* gfScriptFile, unsigned int offset, unsigned int len) {
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

	//offset += 3;

	if (ng_header_found) {
		unsigned int options_header_block_start_position = offset;

		unsigned short options_header_block_size = NG_READ_16(gfScriptFile, offset);
		unsigned int options_header_block_end_pos = options_header_block_start_position + (options_header_block_size * sizeof(short));
		unsigned short options_block_unknown_variable = NG_READ_16(gfScriptFile, offset);

		while (1) {
			unsigned int data_block_start_start_position = offset;
			unsigned char current_data_block_size_wide = NG_READ_8(gfScriptFile, offset);

			unsigned char block_type = NG_READ_8(gfScriptFile, offset);

			if (offset >= options_header_block_end_pos) {
				if (offset != options_header_block_end_pos) {
					printf("Options header block size mismatch!\n");
				}
				break;
			}

			switch (block_type) {
				case 0xc8: {
					unsigned char flags = NG_READ_16(gfScriptFile, offset);
					get_game_mod_global_info().show_lara_in_title = flags & 0x40;
					break;
				}
				default: {
					printf("Unimplemented NG option data block type: %u\n", block_type);
					offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
					break;
				}
			}

			int command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
			if (offset != command_block_end_position) {
				printf("Command block size mismatch!\n");
			}
			offset = command_block_end_position;
		}
		
		offset = options_header_block_end_pos;

		unsigned short second_header_block_size = NG_READ_16(gfScriptFile, offset);
		offset += (second_header_block_size - 1) * sizeof(short);

		int current_level = 0;

		// Allocate the trigger table for each level.
		NG_GLOBAL_TRIGGER_RECORD *level_global_triggers_table = (NG_GLOBAL_TRIGGER_RECORD *)malloc(sizeof(NG_GLOBAL_TRIGGER_RECORD) * MAX_NG_GLOBAL_TRIGGERS);
		NG_TRIGGER_GROUP_RECORD *level_trigger_group_table = (NG_TRIGGER_GROUP_RECORD *)malloc(sizeof(NG_TRIGGER_GROUP_RECORD) * MAX_NG_TRIGGER_GROUPS);
		NG_ORGANIZER_RECORD *level_organizer_table = (NG_ORGANIZER_RECORD *)malloc(sizeof(NG_ORGANIZER_RECORD) * MAX_NG_ORGANIZERS);
		NG_ITEM_GROUP_RECORD *level_item_group_table = (NG_ITEM_GROUP_RECORD*)malloc(sizeof(NG_ITEM_GROUP_RECORD) * MAX_NG_ITEM_GROUPS);

		// Do the levels
		while (1) {
			memset(level_global_triggers_table, 0x00, sizeof(NG_GLOBAL_TRIGGER_RECORD) * MAX_NG_GLOBAL_TRIGGERS);
			memset(level_trigger_group_table, 0x00, sizeof(NG_TRIGGER_GROUP_RECORD) * MAX_NG_TRIGGER_GROUPS);
			memset(level_organizer_table, 0x00, sizeof(NG_ORGANIZER_RECORD) * MAX_NG_ORGANIZERS);
			memset(level_item_group_table, 0x00, sizeof(NG_ITEM_GROUP_RECORD) * MAX_NG_ITEM_GROUPS);

			unsigned int level_global_trigger_count = 0;
			unsigned int level_trigger_group_count = 0;
			unsigned int level_organizer_count = 0;
			unsigned int level_item_group_count = 0;

			unsigned int level_block_start_position = offset;
			unsigned short level_block_size = NG_READ_16(gfScriptFile, offset);
			unsigned short level_block_unknown_variable = NG_READ_16(gfScriptFile, offset);

			if (level_block_size == 0) {
				return;
			}

			unsigned int level_block_end_pos = level_block_start_position + level_block_size * sizeof(short);

			int command_blocks_parsed = 0;
			int command_blocks_failed = 0;

			// Do the commands
			while (1) {
				unsigned int data_block_start_start_position = offset;
				unsigned char current_data_block_size_wide = NG_READ_8(gfScriptFile, offset);

				unsigned char block_type = NG_READ_8(gfScriptFile, offset);

				int command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));

				if (offset >= level_block_end_pos) {
					if (offset != level_block_end_pos) {
						printf("Level block size mismatch!\n");
					}
					offset = level_block_end_pos;
					break;
				}

				command_blocks_parsed++;

				switch (block_type) {
					case 0x01: {
						// AssignSlot (WIP)
						unsigned int slot_a = NG_READ_16(gfScriptFile, offset);
						unsigned int slot_b = 0;
						if (get_game_mod_global_info().trng_version_minor < 3) {
							slot_b = NG_READ_16(gfScriptFile, offset);
						} else {
							slot_b = NG_READ_32(gfScriptFile, offset);
						}

						break;
					}
					case 0x02: {
						// Snow
						printf("Snow is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x03: {
						// LevelFarView (WIP)
						unsigned short level_far_view = NG_READ_16(gfScriptFile, offset);
						break;
					}
					case 0x04: {
						// FogRange (WIP)
						unsigned short fog_start = NG_READ_16(gfScriptFile, offset);
						unsigned short fog_end = NG_READ_16(gfScriptFile, offset);

						break;
					}
					case 0x06: {
						// Rain
						printf("Rain is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x08: {
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
						// Enemy (WIP)
						unsigned short slot = NG_READ_16(gfScriptFile, offset);
						unsigned short hp = NG_READ_16(gfScriptFile, offset);
						unsigned short nef_flags = NG_READ_16(gfScriptFile, offset);
						unsigned short tomb_flags = NG_READ_16(gfScriptFile, offset);
						unsigned short extra_flags = NG_READ_16(gfScriptFile, offset);
						unsigned short damage_1 = NG_READ_16(gfScriptFile, offset);
						unsigned short damage_2 = NG_READ_16(gfScriptFile, offset);
						unsigned short damage_3 = NG_READ_16(gfScriptFile, offset);

						break;
					}
					case 0x0a: {
						// Animation (WIP)
						unsigned short anim_index = NG_READ_16(gfScriptFile, offset);
						unsigned short key_1 = NG_READ_16(gfScriptFile, offset);
						unsigned short key_2 = NG_READ_16(gfScriptFile, offset);
						unsigned short fan_flags = NG_READ_16(gfScriptFile, offset);
						unsigned short env_environment = NG_READ_16(gfScriptFile, offset);
						unsigned short distance_for_env = NG_READ_16(gfScriptFile, offset);
						unsigned short extra = NG_READ_16(gfScriptFile, offset);

						// TODO: Figure out the array format
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));

						break;
					}
					case 0x0b: {
						// MirrorEffect (WIP)
						unsigned short in_front_room = NG_READ_16(gfScriptFile, offset);
						unsigned short hidden_room = NG_READ_16(gfScriptFile, offset);
						unsigned short mirror_type = NG_READ_16(gfScriptFile, offset);

						// The rest of it should be an array of animatings:
						while ((offset != command_block_end_position)) {
							unsigned short animating_index = NG_READ_16(gfScriptFile, offset);
						}
						break;
					}
					case 0x0c: {
						// Elevator
						printf("Elevator is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x0d: {
						// Keypad
						printf("Keypad is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x0e: {
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
						printf("Detector is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x11: {
						// TextureSequence
						printf("TextureSequence is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x12: {
						// Equipment (WIP)
						unsigned short slot_item = NG_READ_16(gfScriptFile, offset);
						unsigned short amount = NG_READ_16(gfScriptFile, offset);

						break;
					}
					case 0x13: {
						// MultiEnvCondition (WIP)
						printf("MultiEnvCondition is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x14: {
						// Customize (WIP)
						unsigned int customization_category = 0;
						if (get_game_mod_global_info().trng_version_minor < 3) {
							customization_category = NG_READ_16(gfScriptFile, offset);
						} else {
							customization_category = NG_READ_32(gfScriptFile, offset);
						}
						switch (customization_category) {
							// CUST_DISABLE_SCREAMING_HEAD	
							case 0x0001: {
								printf("CUST_DISABLE_SCREAMING_HEAD unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SET_SECRET_NUMBER	
							case 0x0002: {
								unsigned short secret_count = NG_READ_16(gfScriptFile, offset);
								if (current_level == 0) {
									for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
										get_game_mod_level_stat_info(i).secret_count = secret_count;
									}
								} else {
									get_game_mod_level_stat_info(current_level).secret_count = secret_count;
								}
								break;
							}
							// CUST_SET_CREDITS_LEVEL
							case 0x0003: {
								printf("CUST_SET_CREDITS_LEVEL unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_DISABLE_FORCING_ANIM_96
							case 0x0004: {
								printf("CUST_DISABLE_FORCING_ANIM_96 unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_ROLLINGBALL_PUSHING
							case 0x0005: {
								printf("CUST_ROLLINGBALL_PUSHING unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_NEW_SOUND_ENGINE
							case 0x0006: {
								unsigned short new_sound_engine_flags = NG_READ_16(gfScriptFile, offset);
								unsigned short sound_extension = NG_READ_16(gfScriptFile, offset); // Obsolete
								unsigned short long_fade_out = NG_READ_16(gfScriptFile, offset);
								unsigned short short_fade_out = NG_READ_16(gfScriptFile, offset);

								printf("CUST_NEW_SOUND_ENGINE unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SPEED_MOVING
							case 0x0007: {
								printf("CUST_SPEED_MOVING unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SHATTER_RANGE
							case 0x0008: {
								unsigned short first_static_as_shatter = NG_READ_16(gfScriptFile, offset);
								unsigned short last_static_as_shatter = NG_READ_16(gfScriptFile, offset);
								printf("CUST_SHATTER_RANGE unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_WEAPON
							case 0x0009: {
								printf("CUST_WEAPON unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_AMMO
							case 0x000a: {
								unsigned short ammo_slot = NG_READ_16(gfScriptFile, offset);
								unsigned short ammo_flags = NG_READ_16(gfScriptFile, offset);
								unsigned short damage = NG_READ_16(gfScriptFile, offset);
								unsigned short shots_for_box = NG_READ_16(gfScriptFile, offset);
								unsigned short shots_with_weapon = NG_READ_16(gfScriptFile, offset);
								unsigned short extra = NG_READ_16(gfScriptFile, offset);
								unsigned short trigger_group_when_hit_enemy = NG_READ_16(gfScriptFile, offset);
								unsigned short damage_for_explosion = NG_READ_16(gfScriptFile, offset);
								unsigned short speed = NG_READ_16(gfScriptFile, offset);
								unsigned short gravity = NG_READ_16(gfScriptFile, offset);
								unsigned short id_add_effect_to_ammo = NG_READ_16(gfScriptFile, offset);
								unsigned short id_trigger_group_at_end = NG_READ_16(gfScriptFile, offset);
								printf("CUST_AMMO unimplemented!\n");
								break;
							}
							// CUST_SHOW_AMMO_COUNTER
							case 0x000b: {
								unsigned short color = NG_READ_16(gfScriptFile, offset);
								unsigned short format_flags = NG_READ_16(gfScriptFile, offset); // Obsolete
								unsigned short blink_time = NG_READ_16(gfScriptFile, offset);
								unsigned short size_character = NG_READ_16(gfScriptFile, offset);
								unsigned short show_counter_flags = NG_READ_16(gfScriptFile, offset);

								printf("CUST_SHOW_AMMO_COUNTER unimplemented!\n");
								break;
							}
							// CUST_SET_INV_ITEM
							case 0x000c: {
								printf("CUST_SET_INV_ITEM unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SET_JEEP_KEY_SLOT
							case 0x000d: {
								printf("CUST_SET_JEEP_KEY_SLOT unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_STATIC_TRANSPARENCY
							case 0x000e: {
								printf("CUST_STATIC_TRANSPARENCY unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SET_STATIC_DAMAGE
							case 0x000f: {
								printf("CUST_SET_STATIC_DAMAGE unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_LOOK_TRASPARENT
							case 0x0010: {
								unsigned short is_enabled = NG_READ_16(gfScriptFile, offset);

								printf("CUST_LOOK_TRASPARENT unimplemented!\n");
								break;
							}
							// CUST_HAIR_TYPE
							case 0x0011: {
								printf("CUST_HAIR_TYPE unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_KEEP_DEAD_ENEMIES
							case 0x0012: {
								printf("CUST_KEEP_DEAD_ENEMIES unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SET_OLD_CD_TRIGGER
							case 0x0013: {
								printf("CUST_SET_OLD_CD_TRIGGER unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_ESCAPE_FLY_CAMERA
							case 0x0014: {
								printf("CUST_ESCAPE_FLY_CAMERA unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_PAUSE_FLY_CAMERA
							case 0x0015: {
								printf("CUST_PAUSE_FLY_CAMERA unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_TEXT_ON_FLY_SCREEN
							case 0x0016: {
								printf("CUST_TEXT_ON_FLY_SCREEN unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_CD_SINGLE_PLAYBACK
							case 0x0017: {
								printf("CUST_CD_SINGLE_PLAYBACK unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_ADD_DEATH_ANIMATION
							case 0x0018: {
								// Death animation for immortal creatures.
								printf("CUST_ADD_DEATH_ANIMATION unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_BAR 
							case 0x0019: {
								printf("CUST_BAR unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_NO_TIME_IN_SAVELIST
							case 0x001a: {
								printf("CUST_NO_TIME_IN_SAVELIST unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_PARALLEL_BARS
							case 0x001b: {
								printf("CUST_PARALLEL_BARS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_CAMERA
							case 0x001c: {
								printf("CUST_CAMERA unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_DISABLE_MISSING_SOUNDS
							case 0x001d: {
								printf("CUST_DISABLE_MISSING_SOUNDS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_INNER_SCREENSHOT
							case 0x001e: {
								printf("CUST_INNER_SCREENSHOT unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_FMV_CUTSCENE
							case 0x001f: {
								printf("CUST_FMV_CUTSCENE unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_FIX_WATER_FOG_BUG 
							case 0x0020: {
								printf("CUST_FIX_WATER_FOG_BUG unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SAVE_LOCUST 
							case 0x0021: {
								printf("CUST_SAVE_LOCUST unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_LIGHT_OBJECT 
							case 0x0022: {
								printf("CUST_LIGHT_OBJECT unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_HARPOON
							case 0x0023: {
								printf("CUST_HARPOON unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SCREENSHOT_CAPTURE
							case 0x0024: {
								printf("CUST_SCREENSHOT_CAPTURE unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_RAIN
							case 0x0025: {
								printf("CUST_RAIN unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_TR5_UNDERWATER_COLLISIONS 
							case 0x0026: {
								printf("CUST_TR5_UNDERWATER_COLLISIONS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_DARTS 
							case 0x0027: {
								printf("CUST_DARTS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_FLARE 
							case 0x0028: {
								unsigned short flare_flags = NG_READ_16(gfScriptFile, offset);
								unsigned short flare_lifetime_in_seconds = NG_READ_16(gfScriptFile, offset);
								unsigned char flare_light_r = NG_READ_16(gfScriptFile, offset);
								unsigned char flare_light_g = NG_READ_16(gfScriptFile, offset);
								unsigned char flare_light_b = NG_READ_16(gfScriptFile, offset);
								unsigned char flare_light_intensity = NG_READ_16(gfScriptFile, offset);

								if (current_level == 0) {
									for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
										if (flare_light_r != 0xff && flare_light_g != 0xff && flare_light_b != 0xff) {
											get_game_mod_level_flare_info(i).light_color_r = flare_light_r;
											get_game_mod_level_flare_info(i).light_color_g = flare_light_g;
											get_game_mod_level_flare_info(i).light_color_b = flare_light_b;
										}
										if (flare_light_intensity != 0xff)
											get_game_mod_level_flare_info(i).light_intensity = flare_light_intensity;
										if (flare_lifetime_in_seconds != 0xffff)
											get_game_mod_level_flare_info(i).flare_lifetime_in_ticks = flare_lifetime_in_seconds * 30;
										if (flare_flags != 0xffff) {
											get_game_mod_level_flare_info(i).has_sparks = flare_flags & 0x0001;
											get_game_mod_level_flare_info(i).has_fire = flare_flags & 0x0002; // Unsupported
											get_game_mod_level_flare_info(i).sparks_include_smoke = flare_flags & 0x0004;
											get_game_mod_level_flare_info(i).has_glow = flare_flags & 0x0008; // Unsupported
											get_game_mod_level_flare_info(i).flat_light = flare_flags & 0x0010;
										}
									}
								} else {
									if (flare_light_r != 0xff && flare_light_g != 0xff && flare_light_b != 0xff) {
										get_game_mod_level_flare_info(current_level).light_color_r = flare_light_r;
										get_game_mod_level_flare_info(current_level).light_color_g = flare_light_g;
										get_game_mod_level_flare_info(current_level).light_color_b = flare_light_b;
									}
									if (flare_light_intensity != 0xff)
										get_game_mod_level_flare_info(current_level).light_intensity = flare_light_intensity;
									if (flare_lifetime_in_seconds != 0xffff)
										get_game_mod_level_flare_info(current_level).flare_lifetime_in_ticks = flare_lifetime_in_seconds * 30;
									if (flare_flags != 0xffff) {
										get_game_mod_level_flare_info(current_level).has_sparks = flare_flags & 0x0001;
										get_game_mod_level_flare_info(current_level).has_fire = flare_flags & 0x0002; // Unsupported
										get_game_mod_level_flare_info(current_level).sparks_include_smoke = flare_flags & 0x0004;
										get_game_mod_level_flare_info(current_level).has_glow = flare_flags & 0x0008;
										get_game_mod_level_flare_info(current_level).flat_light = flare_flags & 0x0010;
									}
								}

								break;
							}
							 // CUST_SET_TEXT_COLOR 
							case 0x0029: {
								printf("CUST_SET_TEXT_COLOR unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							 // CUST_SET_STILL_COLLISION 
							case 0x002a: {
								printf("CUST_SET_STILL_COLLISION unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							 // CUST_WATERFALL_SPEED 
							case 0x002b: {
								printf("CUST_WATERFALL_SPEED unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							 // CUST_ROLLING_BOAT 
							case 0x002c: {
								printf("CUST_ROLLING_BOAT unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							 // CUST_SFX 
							case 0x002d: {
								printf("CUST_SFX unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							 // CUST_TITLE_FMV 
							case 0x002e: {
								printf("CUST_TITLE_FMV unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							 // CUST_KEEP_LARA_HP 
							case 0x002f: {
								printf("CUST_KEEP_LARA_HP unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_BINOCULARS
							case 0x0030: {
								printf("CUST_BINOCULARS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_BACKGROUND
							case 0x0031: {
								printf("CUST_BACKGROUND unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_DISABLE_PUSH_AWAY_ANIMATION
							case 0x0032: {
								printf("CUST_DISABLE_PUSH_AWAY_ANIMATION unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SLOT_FLAGS
							case 0x0034: {
								printf("CUST_SLOT_FLAGS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_FIX_BUGS
							case 0x0035: {
								printf("CUST_FIX_BUGS unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							// CUST_SHATTER_SPECIFIC
							case 0x0036: {
								printf("CUST_SHATTER_SPECIFIC unimplemented!\n");
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
							default: {
								printf("Unimplemented NG customization category: %u\n", customization_category);
								offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
								break;
							}
						}

						int command_block_end_position = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));

						if (offset != command_block_end_position) {
							printf("Customize block size mismatch!\n");
						}

						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x15:
						// TriggerGroup (legacy?)
						// Older builds of TRNG seem to use this opcode for TriggerGroups. Not sure why it changed though.
						if (get_game_mod_global_info().trng_version_minor < 3) {
							// TriggerGroup (WIP)
							unsigned short id = NG_READ_16(gfScriptFile, offset);

							if (id > MAX_NG_TRIGGER_GROUPS) {
								printf("TriggerGroup id is not valid!\n");
								return;
								// Broken
							}

							level_trigger_group_table[level_trigger_group_count].record_id = id;

							unsigned char data_index = 0;
							while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
								unsigned short first_field = NG_READ_16(gfScriptFile, offset);
								// I assume this indicates the end of the command.
								if (first_field == 0x0000) {
									break;
								}
								unsigned short second_field = NG_READ_16(gfScriptFile, offset);
								unsigned short third_field = NG_READ_16(gfScriptFile, offset);

								level_trigger_group_table[level_trigger_group_count].trigger_group.data[data_index].first_field = first_field;
								level_trigger_group_table[level_trigger_group_count].trigger_group.data[data_index].second_field = second_field;
								level_trigger_group_table[level_trigger_group_count].trigger_group.data[data_index].third_field = third_field;

								data_index++;
								if (data_index > NG_TRIGGER_GROUP_DATA_SIZE) {
									printf("TriggerGroup size overflow!\n");
									return;
								}
							}
							level_trigger_group_count++;
						} else {
							printf("Encountered a legacy(?) TriggerGroup opcode in an TRNG 1.3+ level.\n");
							offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						}
						break;
					case 0x16: {
						// Global Trigger
						unsigned short id = NG_READ_16(gfScriptFile, offset);

						if (id > MAX_NG_GLOBAL_TRIGGERS) {
							printf("Global Trigger id is not valid!\n");
							return;
							// Broken
						}

						level_global_triggers_table[level_global_trigger_count].record_id = id;

						unsigned short flags = NG_READ_16(gfScriptFile, offset);
						if (flags == -1)
							flags = 0;

						level_global_triggers_table[level_global_trigger_count].global_trigger.flags = flags;
						level_global_triggers_table[level_global_trigger_count].global_trigger.type = NG_READ_16(gfScriptFile, offset);
						level_global_triggers_table[level_global_trigger_count].global_trigger.parameter = NG_READ_32(gfScriptFile, offset);
						level_global_triggers_table[level_global_trigger_count].global_trigger.condition_trigger_group = NG_READ_16(gfScriptFile, offset);
						level_global_triggers_table[level_global_trigger_count].global_trigger.perform_trigger_group = NG_READ_16(gfScriptFile, offset);
						// The block may end here on older version of TRNG
						if (offset < command_block_end_position) {
							level_global_triggers_table[level_global_trigger_count].global_trigger.on_false_trigger_group = NG_READ_16(gfScriptFile, offset);
						} else {
							level_global_triggers_table[level_global_trigger_count].global_trigger.on_false_trigger_group = 0xffff;
						}

						level_global_trigger_count++;

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

						if (id > MAX_NG_ORGANIZERS) {
							printf("Organizer id is not valid!\n");
							return;
							// Broken
						}

						level_organizer_table[level_organizer_count].record_id = id;

						unsigned short flags = NG_READ_16(gfScriptFile, offset);;
						if (flags == -1)
							flags = 0;

						level_organizer_table[level_organizer_count].organizer.flags = flags;
						level_organizer_table[level_organizer_count].organizer.parameters = NG_READ_16(gfScriptFile, offset);
						if (level_organizer_table[level_organizer_count].organizer.parameters != -1) {
							printf("Organizer parameters are not supported!\n");
						}

						level_organizer_table[level_organizer_count].organizer.appointment_count = 0;

						unsigned int index = 0;
						while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
							level_organizer_table[level_organizer_count].organizer.appointments[index].time = NG_READ_16(gfScriptFile, offset);
							// !FO_TICK_TIME
							if (!(flags & 0x04)) {
								level_organizer_table[level_organizer_count].organizer.appointments[index].time *= 30;
							}

							level_organizer_table[level_organizer_count].organizer.appointments[index].trigger_group = NG_READ_16(gfScriptFile, offset);

							index++;

							level_organizer_table[level_organizer_count].organizer.appointment_count = index;

							if (index > NG_ORGANIZER_MAX_APPOINTMENTS) {
								printf("Organizer appointment size overflow!\n");
								return;
							}
						}
						level_organizer_count++;
						break;
					}
					case 0x19: {
						// Item Groups
						unsigned short id = NG_READ_16(gfScriptFile, offset);

						if (id > MAX_NG_ITEM_GROUPS) {
							printf("ItemGroup id is not valid!\n");
							return;
							// Broken
						}

						level_item_group_table[level_item_group_count].record_id = id;

						unsigned char index = 0;
						while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
							level_item_group_table[level_item_group_count].item_group.item_list[index] = NG_READ_16(gfScriptFile, offset);

							index++;
							if (index > NG_ITEM_GROUP_MAX_LIST) {
								printf("ItemGroup record size overflow!\n");
								return;
							}
						}
						level_item_group_count++;
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
						printf("Parameters is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x1c: {
						// Turbo (WIP)
						printf("Turbo is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x1d: {
						// WindowTitle (WIP)
						printf("WindowTitle is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x1e: {
						// TestPosition (WIP)
						printf("TestPosition is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x21: {
						// Diary
						printf("Diary is not implemented!\n");
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
						printf("SavegamePanel is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x25: {
						// Switch
						printf("Switch is not implemented!\n");
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
						printf("Standby is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x28: {
						// AnimationSlot
						printf("AnimationSlot is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x2a: {
						// Demo
						printf("Demo is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x2c: {
						// LaraStartPos
						printf("LaraStartPos is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x2d: {
						// StaticMIP
						printf("StaticMIP is not implemented!\n");
						// Skip to the end
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						break;
					}
					case 0x2e: {
						// TriggerGroup (WIP)
						unsigned short id = NG_READ_16(gfScriptFile, offset);

						if (id > MAX_NG_TRIGGER_GROUPS) {
							printf("TriggerGroup id is not valid!\n");
							return;
							// Broken
						}

						level_trigger_group_table[level_trigger_group_count].record_id = id;

						unsigned char data_index = 0;
						while (offset < data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short))) {
							unsigned short first_field = NG_READ_16(gfScriptFile, offset);
							// I assume this indicates the end of the command.
							if (first_field == 0x0000) {
								break;
							}
							unsigned short second_field = NG_READ_16(gfScriptFile, offset);
							unsigned short third_field = NG_READ_16(gfScriptFile, offset);

							level_trigger_group_table[level_trigger_group_count].trigger_group.data[data_index].first_field = first_field;
							level_trigger_group_table[level_trigger_group_count].trigger_group.data[data_index].second_field = second_field;
							level_trigger_group_table[level_trigger_group_count].trigger_group.data[data_index].third_field = third_field;

							data_index++;
							if (data_index > NG_TRIGGER_GROUP_DATA_SIZE) {
								printf("TriggerGroup size overflow!\n");
								return;
							}
						}
						level_trigger_group_count++;
						break;
					}
					case 0xc9: {
						// Level flags (?)
						unsigned short flags = NG_READ_16(gfScriptFile, offset);
						break;
					}
					default: {
						offset = data_block_start_start_position + (current_data_block_size_wide * sizeof(short) + sizeof(short));
						command_blocks_failed++;
						printf("Unimplemented NG level data block type: %u\n", block_type);
						break;
					}
				}
				if (offset != command_block_end_position) {
					printf("Command block size mismatch!\n");
				}
				offset = command_block_end_position;
			}
			// Now save the tables
			NGReallocateLevel(ng_levels[current_level], level_global_trigger_count, level_trigger_group_count, level_organizer_count, level_item_group_count);
			memcpy(ng_levels[current_level].records->global_triggers_table, level_global_triggers_table, sizeof(NG_GLOBAL_TRIGGER_RECORD) * level_global_trigger_count);
			memcpy(ng_levels[current_level].records->trigger_group_table, level_trigger_group_table, sizeof(NG_TRIGGER_GROUP_RECORD) * level_trigger_group_count);
			memcpy(ng_levels[current_level].records->organizer_table, level_organizer_table, sizeof(NG_ORGANIZER_RECORD) * level_organizer_count);
			memcpy(ng_levels[current_level].records->item_group_table, level_item_group_table, sizeof(NG_ITEM_GROUP_RECORD) * level_item_group_count);

			current_level++;
		}

		// Cleanup
		free(level_global_triggers_table);
		free(level_trigger_group_table);
		free(level_organizer_table);
		free(level_item_group_table);
	}
}