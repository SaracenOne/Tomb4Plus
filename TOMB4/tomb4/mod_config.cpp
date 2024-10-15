#include "../tomb4/pch.h"
#include "mod_config.h"
#include "../specific/file.h"

#include "libs/tiny-json/tiny-json.h"
#include "../specific/function_stubs.h"
#include "../game/gameflow.h"
#include "../game/trng/trng.h"
#include "tomb4plus/t4plus_mirror.h"
#include "tomb4plus/t4plus_inventory.h"
#include "../game/lara.h"

#include "../game/trep/furr.h"

#include "../specific/audio.h"
#include "../specific/cmdline.h"
#include "../specific/function_table.h"
#include "../specific/output.h"
#include "../specific/platform.h"
#include "../specific/registry.h"

#include <string>
#include "../game/sound.h"

int global_string_table_size = 0;
char** global_string_table = nullptr;

#define READ_JSON_INTEGER_CAST(value_name, json, my_struct, my_type) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (my_type)json_getInteger(value_name); } \
    }

#define READ_JSON_SINT8(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (signed char)json_getInteger(value_name); } \
    }

#define READ_JSON_SINT16(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (signed short)json_getInteger(value_name); } \
    }

#define READ_JSON_SINT32(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (signed int)json_getInteger(value_name); } \
    }

#define READ_JSON_UINT8(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (unsigned char)json_getInteger(value_name); } \
    }

#define READ_JSON_UINT16(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (unsigned short)json_getInteger(value_name); } \
    }

#define READ_JSON_UINT32(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_INTEGER == json_getType(value_name)) { \
        (my_struct)->value_name = (unsigned int)json_getInteger(value_name); } \
    }

#define READ_JSON_BOOL(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_BOOLEAN == json_getType(value_name)) { \
        (my_struct)->value_name = (bool)json_getBoolean(value_name); } \
    }

#define READ_JSON_BOOL_AND_SET_FLAG(value_name, json, my_struct, flag) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_BOOLEAN == json_getType(value_name)) { \
        (my_struct)->value_name = (bool)json_getBoolean(value_name); } \
    } \
    flag = true;

#define READ_JSON_FLOAT32(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_REAL == json_getType(value_name)) { \
        (my_struct)->value_name = (float)json_getReal(value_name); } \
    }

#define READ_JSON_ARGB(value_name, json, my_struct) { \
    const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_OBJ == json_getType(value_name)) {\
    const json_t* r = json_getProperty(value_name, "r"); \
    if (r && JSON_INTEGER == json_getType(r)) { \
        unsigned char r8 = (unsigned char)json_getInteger(r); \
        (my_struct)->value_name &= ~(0x00ff0000); \
        (my_struct)->value_name |= (((unsigned int)r8) << 16) & 0x00ff0000; \
    } \
    const json_t* g = json_getProperty(value_name, "g"); \
    if (g && JSON_INTEGER == json_getType(g)) { \
        unsigned char g8 = (unsigned char)json_getInteger(g); \
        (my_struct)->value_name &= ~(0x0000ff00); \
        (my_struct)->value_name |= (((unsigned int)g8) << 8) & 0x0000ff00; \
    } \
    const json_t* b = json_getProperty(value_name, "b"); \
    if (b && JSON_INTEGER == json_getType(b)) { \
        unsigned char b8 = (unsigned char)json_getInteger(b); \
        (my_struct)->value_name &= ~(0x000000ff); \
        (my_struct)->value_name |= (((unsigned int)b8)) & 0x000000ff; \
    } \
    const json_t* a = json_getProperty(value_name, "a"); \
    if (a && JSON_INTEGER == json_getType(a)) { \
        unsigned char a8 = (unsigned char)json_getInteger(a); \
        (my_struct)->value_name &= ~(0xff000000); \
        (my_struct)->value_name |= (((unsigned int)a8) << 24) & 0xff000000; \
    } \
    } \
    }
    

#define READ_JSON_STRING(value_name, json, my_struct) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_TEXT == json_getType(value_name)) { \
        char *temp = (char *)json_getValue(value_name); \
        if (temp) { \
        (my_struct)->value_name = T4PlusAllocateString(temp); } \
        else { \
            (my_struct)->value_name = nullptr; \
        } \
    } \
};

#define READ_JSON_STRING_TEMP(value_name, json, ptr) { const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_TEXT == json_getType(value_name)) { \
        ptr = (char *)json_getValue(value_name); \
    } \
};

bool SetupUserDirectories() {
    if (game_mod_config.global_info.game_user_dir_name) {
        std::string new_game_user_dir_path = platform_get_userdata_path() + "game_data" + PATH_SEPARATOR + game_mod_config.global_info.game_user_dir_name + PATH_SEPARATOR;
        if (platform_create_directory(new_game_user_dir_path.c_str())) {
            game_user_dir_path = new_game_user_dir_path;

            // Saves
            std::string game_user_saves_dir_name = game_user_dir_path + "saves" + PATH_SEPARATOR;
            if (platform_create_directory(game_user_saves_dir_name.c_str())) {
                savegame_dir_path = game_user_saves_dir_name;
            } else {
                platform_fatal_error("Failed to create game user saves directory at path '%s'.", game_user_saves_dir_name);
                return false;
            }

            // Screenshots
            std::string game_user_screenshots_dir_name = game_user_dir_path + "screenshots" + PATH_SEPARATOR;
            if (platform_create_directory(game_user_screenshots_dir_name.c_str())) {
                screenshots_dir_path = game_user_screenshots_dir_name;
            } else {
                platform_fatal_error("Failed to create game user screenshots directory at path '%s'.", game_user_screenshots_dir_name);
                return false;
            }
        } else {
            platform_fatal_error("Failed to create game user directory at path '%s'.", game_user_dir_path);
            return false;
        }
    } else {
        platform_fatal_error("game_mod_config.json manifest is missing field 'game_user_dir_name'.");
        return false;
    }

    GlobalLog("Starting Tomb4Plus game: %s...", game_mod_config.global_info.game_user_dir_name);
    return true;
}

char *T4PlusAllocateString(char* str) {
    if (global_string_table_size < MAX_T4PLUS_STRINGS) {
        for (int i = 0; i < global_string_table_size; i++) {
            if (strcmp(str, global_string_table[i]) == 0) {
                return global_string_table[i];
            }
        }

        char *new_string = (char*)SYSTEM_MALLOC(strlen(str) + 1);
        if (!new_string) {
            platform_fatal_error("Memory allocation failed!");
            return nullptr;
        }

        memcpy(new_string, str, strlen(str) + 1);
        global_string_table[global_string_table_size] = new_string;
        global_string_table_size++;

        return new_string;
    } else {
        Log(2, "Maximum T4Plus Strings allocated!");
    }

    return nullptr;
}

void T4PlusFreeAllStrings() {
    for (int i = 0; i < global_string_table_size; i++) {
        SYSTEM_FREE(global_string_table[i]);
    }
    SYSTEM_FREE(global_string_table);
}

// Full overrides
bool scorpion_poison_override_found = false;

GAME_MOD_CONFIG game_mod_config;

void setup_custom_slots_for_level(int level, OBJECT_INFO* current_object_info_array) {
    OBJECT_INFO* backup_object_info_array = (OBJECT_INFO* )SYSTEM_MALLOC(sizeof(OBJECT_INFO) * NUMBER_OBJECTS);

    if (backup_object_info_array) {
        memcpy(backup_object_info_array, current_object_info_array, sizeof(OBJECT_INFO) * NUMBER_OBJECTS);

        for (int i = 0; i < NUMBER_OBJECTS; i++) {
            if (game_mod_config.level_info[level].objects_info.slot_override[i] != i) {
                short original_nmeshes = current_object_info_array[i].nmeshes;
                short original_mesh_index = current_object_info_array[i].mesh_index;
                long original_bone_index = current_object_info_array[i].bone_index;
                short *original_frame_base = current_object_info_array[i].frame_base;
                short original_anim_index = current_object_info_array[i].anim_index;

                memcpy(&current_object_info_array[i], &backup_object_info_array[game_mod_config.level_info[level].objects_info.slot_override[i]], sizeof(OBJECT_INFO));

                current_object_info_array[i].nmeshes = original_nmeshes;
                current_object_info_array[i].mesh_index = original_mesh_index;
                current_object_info_array[i].bone_index = original_bone_index;
                current_object_info_array[i].frame_base = original_frame_base;
                current_object_info_array[i].anim_index = original_anim_index;
            }
        }
        SYSTEM_FREE(backup_object_info_array);
    }
}

void assign_slot_for_level(int level, int dest_slot, int src_slot) {
    if (src_slot < NUMBER_OBJECTS && dest_slot < NUMBER_OBJECTS && level < MOD_LEVEL_COUNT) {
        game_mod_config.level_info[level].objects_info.slot_override[dest_slot] = src_slot;
    } else {
        printf("Invalid slot assignment!\n");
    }
}

void T4PlusSetupObjectsForLevel(int level, OBJECT_INFO* current_object_info_array) {
    setup_custom_slots_for_level(level, current_object_info_array);

    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        if (game_mod_config.level_info[level].objects_info.object_customization[i].override_hit_points) {
            current_object_info_array[i].hit_points = game_mod_config.level_info[level].objects_info.object_customization[i].hit_points;
        }
    }
}

extern MOD_GLOBAL_INFO *get_game_mod_global_info() {
    return &game_mod_config.global_info;
}

MOD_LEVEL_AUDIO_INFO *get_game_mod_level_audio_info(int level) {
    return &game_mod_config.level_info[level].audio_info;
}

MOD_LEVEL_BARS_INFO *get_game_mod_level_bars_info(int level) {
    return &game_mod_config.level_info[level].bars_info;
}

MOD_LEVEL_ENVIRONMENT_INFO *get_game_mod_level_environment_info(int level) {
    return &game_mod_config.level_info[level].environment_info;
}

extern MOD_LEVEL_FONT_INFO *get_game_mod_level_font_info(int level) {
    return &game_mod_config.level_info[level].font_info;
}

extern MOD_LEVEL_CAMERA_INFO *get_game_mod_level_camera_info(int level) {
    return &game_mod_config.level_info[level].camera_info;
}

MOD_LEVEL_CREATURE_INFO *get_game_mod_level_creature_info(int level) {
    return &game_mod_config.level_info[level].creature_info;
}

MOD_LEVEL_GFX_INFO *get_game_mod_level_gfx_info(int level) {
    return &game_mod_config.level_info[level].gfx_info;
}

MOD_LEVEL_LARA_INFO *get_game_mod_level_lara_info(int level) {
	return &game_mod_config.level_info[level].lara_info;
}

MOD_LEVEL_STAT_INFO *get_game_mod_level_stat_info(int level) {
    return &game_mod_config.level_info[level].stat_info;
}

MOD_LEVEL_FLARE_INFO *get_game_mod_level_flare_info(int level) {
    return &game_mod_config.level_info[level].flare_info;
}

MOD_LEVEL_WEAPON_INFO *get_game_mod_level_weapon_info(int level) {
    return &game_mod_config.level_info[level].weapon_info;
}

MOD_LEVEL_AMMO_INFO *get_game_mod_current_lara_ammo_info(MOD_LEVEL_WEAPON_INFO *weapon_info) {
    switch (lara.weapon_item)
    {
    case WEAPON_REVOLVER:
        return &weapon_info->six_shooter_ammo_info;
    case WEAPON_UZI:
        return &weapon_info->uzi_ammo_info;
    case WEAPON_SHOTGUN:
        if (lara.shotgun_type_carried & W_AMMO1)
            return &weapon_info->shotgun_1_ammo_info;
        else
            return &weapon_info->shotgun_2_ammo_info;
    case WEAPON_GRENADE:
        if (lara.grenade_type_carried & W_AMMO1)
            return &weapon_info->grenade_1_ammo_info;
        else if (lara.grenade_type_carried & W_AMMO2)
            return &weapon_info->grenade_2_ammo_info;
        else
            return &weapon_info->grenade_3_ammo_info;

        break;
    case WEAPON_CROSSBOW:
        if (lara.crossbow_type_carried & W_AMMO1)
            return &weapon_info->crossbow_1_ammo_info;
        else if (lara.crossbow_type_carried & W_AMMO2)
            return &weapon_info->crossbow_2_ammo_info;
        else
            return &weapon_info->crossbow_3_ammo_info;
    default:
        return &weapon_info->pistol_ammo_info;
    }
}

MOD_LEVEL_OBJECTS_INFO *get_game_mod_level_objects_info(int level) {
    return &game_mod_config.level_info[level].objects_info;
}

MOD_LEVEL_OBJECT_CUSTOMIZATION *get_game_mod_level_object_customization_for_slot(int level, int slot) {
    if (slot < NUMBER_OBJECTS) {
        return &get_game_mod_level_objects_info(level)->object_customization[slot];
    } else {
        return nullptr;
    }
}

MOD_LEVEL_MISC_INFO *get_game_mod_level_misc_info(int level) {
    return &game_mod_config.level_info[level].misc_info;
}

MOD_LEVEL_STATICS_INFO* get_game_mod_level_statics_info(int level) {
    return &game_mod_config.level_info[level].statics_info;
}

void LoadGameModLevelAudioInfo(const json_t* audio, MOD_LEVEL_AUDIO_INFO* audio_info) {
    READ_JSON_BOOL(new_audio_system, audio, audio_info);
    READ_JSON_BOOL(old_cd_trigger_system, audio, audio_info);

    READ_JSON_SINT32(sample_rate, audio, audio_info);

    READ_JSON_SINT16(first_looped_audio_track, audio, audio_info);
    READ_JSON_SINT16(last_looped_audio_track, audio, audio_info);

    READ_JSON_SINT16(boat_track, audio, audio_info);
    READ_JSON_SINT16(inside_jeep_track, audio, audio_info);
    READ_JSON_SINT16(outside_jeep_track, audio, audio_info);
    READ_JSON_SINT16(secret_track, audio, audio_info);
}

void LoadGameModLevelRectColorInfo(const json_t* rect_color, MOD_LEVEL_RECT_COLOR_INFO *rect_color_info) {
    READ_JSON_ARGB(upper_left_color, rect_color, rect_color_info);
    READ_JSON_ARGB(upper_right_color, rect_color, rect_color_info);
    READ_JSON_ARGB(lower_right_color, rect_color, rect_color_info);
    READ_JSON_ARGB(lower_left_color, rect_color, rect_color_info);
}

void LoadGameModLevelBarInfo(const json_t* bar, MOD_LEVEL_BAR_INFO* bar_info) {
    const json_t* lower_rect = json_getProperty(bar, "lower_rect");
    if (lower_rect && JSON_OBJ == json_getType(lower_rect)) {
        LoadGameModLevelRectColorInfo(lower_rect, &bar_info->lower_rect);
    }

    const json_t* upper_rect = json_getProperty(bar, "upper_rect");
    if (upper_rect && JSON_OBJ == json_getType(upper_rect)) {
        LoadGameModLevelRectColorInfo(upper_rect, &bar_info->upper_rect);
    }

    const json_t* border_rect = json_getProperty(bar, "border_rect");
    if (border_rect && JSON_OBJ == json_getType(border_rect)) {
        LoadGameModLevelRectColorInfo(border_rect, &bar_info->border_rect);
    }

    const json_t* background_rect = json_getProperty(bar, "background_rect");
    if (background_rect && JSON_OBJ == json_getType(background_rect)) {
        LoadGameModLevelRectColorInfo(background_rect, &bar_info->background_rect);
    }

    READ_JSON_UINT32(border_size, bar, bar_info);
    READ_JSON_UINT32(x, bar, bar_info);
    READ_JSON_UINT32(y, bar, bar_info);
    READ_JSON_UINT32(width, bar, bar_info);
    READ_JSON_UINT32(height, bar, bar_info);
}

void LoadGameModLevelBarsInfo(const json_t* bars, MOD_LEVEL_BARS_INFO *bars_info) {
    const json_t* health_bar_info = json_getProperty(bars, "health_bar");
    if (health_bar_info && JSON_OBJ == json_getType(health_bar_info)) {
        LoadGameModLevelBarInfo(health_bar_info, &bars_info->health_bar);
    }

    const json_t* poison_bar_info = json_getProperty(bars, "poison_bar");
    if (poison_bar_info && JSON_OBJ == json_getType(poison_bar_info)) {
        LoadGameModLevelBarInfo(poison_bar_info, &bars_info->poison_bar);
    }

    const json_t* air_bar_info = json_getProperty(bars, "air_bar");
    if (air_bar_info && JSON_OBJ == json_getType(air_bar_info)) {
        LoadGameModLevelBarInfo(air_bar_info, &bars_info->air_bar);
    }

    const json_t* sprint_bar_info = json_getProperty(bars, "sprint_bar");
    if (sprint_bar_info && JSON_OBJ == json_getType(sprint_bar_info)) {
        LoadGameModLevelBarInfo(sprint_bar_info, &bars_info->sprint_bar);
    }

    const json_t* loading_bar_info = json_getProperty(bars, "loading_bar");
    if (loading_bar_info && JSON_OBJ == json_getType(loading_bar_info)) {
        LoadGameModLevelBarInfo(loading_bar_info, &bars_info->loading_bar);
    }

    const json_t* enemy_bar_info = json_getProperty(bars, "enemy_bar");
    if (enemy_bar_info && JSON_OBJ == json_getType(enemy_bar_info)) {
        LoadGameModLevelBarInfo(enemy_bar_info, &bars_info->enemy_bar);
    }
}

void LoadGameModLevelEnvironmentInfo(const json_t* environment, MOD_LEVEL_ENVIRONMENT_INFO* environment_info) {
    READ_JSON_UINT32(fog_start_range, environment, environment_info);
    READ_JSON_UINT32(fog_end_range, environment, environment_info);
    READ_JSON_UINT32(far_view, environment, environment_info);
    READ_JSON_BOOL(force_train_fog, environment, environment_info);
    READ_JSON_BOOL(disable_distance_limit, environment, environment_info);
    READ_JSON_BOOL(enable_multi_color_fog_bulbs, environment, environment_info);

    READ_JSON_UINT32(room_swamp_flag, environment, environment_info);
    READ_JSON_UINT32(room_cold_flag, environment, environment_info);
    READ_JSON_UINT32(room_damage_flag, environment, environment_info);
}

void LoadGameModLevelFontInfo(const json_t* font, MOD_LEVEL_FONT_INFO* font_info) {
    READ_JSON_SINT32(custom_glyph_scale_width, font, font_info);
    READ_JSON_SINT32(custom_glyph_scale_height, font, font_info);
    READ_JSON_FLOAT32(custom_vertical_spacing, font, font_info);

    READ_JSON_ARGB(main_font_main_color, font, font_info);
    READ_JSON_ARGB(main_font_fade_color, font, font_info);
    READ_JSON_ARGB(options_title_font_main_color, font, font_info);
    READ_JSON_ARGB(options_title_font_fade_color, font, font_info);
    READ_JSON_ARGB(inventory_title_font_main_color, font, font_info);
    READ_JSON_ARGB(inventory_title_font_fade_color, font, font_info);
    READ_JSON_ARGB(inventory_item_font_main_color, font, font_info);
    READ_JSON_ARGB(inventory_item_font_fade_color, font, font_info);

    const json_t* char_table = json_getProperty(font, "custom_font_table");
    if (char_table && JSON_ARRAY == json_getType(char_table)) {
        json_t const *char_table_entry;
        int char_table_index = 0;
        for (char_table_entry = json_getChild(char_table); char_table_entry != 0; char_table_entry = json_getSibling(char_table_entry)) {
            if (char_table_index >= CHAR_TABLE_COUNT)
                break;

            READ_JSON_FLOAT32(u, char_table_entry, &font_info->custom_font_table[char_table_index]);
            READ_JSON_FLOAT32(v, char_table_entry, &font_info->custom_font_table[char_table_index]);

            READ_JSON_SINT16(w, char_table_entry, &font_info->custom_font_table[char_table_index]);
            READ_JSON_SINT16(h, char_table_entry, &font_info->custom_font_table[char_table_index]);
            READ_JSON_SINT16(y_offset, char_table_entry, &font_info->custom_font_table[char_table_index]);

            READ_JSON_SINT8(top_shade, char_table_entry, &font_info->custom_font_table[char_table_index]);
            READ_JSON_SINT8(bottom_shade, char_table_entry, &font_info->custom_font_table[char_table_index]);

            char_table_index++;
        }
    }
}

void LoadGameModLevelCameraInfo(const json_t* camera, MOD_LEVEL_CAMERA_INFO* camera_info) {
    READ_JSON_SINT32(chase_camera_distance, camera, camera_info);
    READ_JSON_SINT32(chase_camera_vertical_orientation, camera, camera_info);
    READ_JSON_SINT32(chase_camera_horizontal_orientation, camera, camera_info);

    READ_JSON_SINT32(combat_camera_distance, camera, camera_info);
    READ_JSON_SINT32(combat_camera_vertical_orientation, camera, camera_info);

    READ_JSON_SINT32(look_camera_distance, camera, camera_info);
    READ_JSON_SINT32(look_camera_height, camera, camera_info);

    READ_JSON_SINT32(camera_speed, camera, camera_info);
    READ_JSON_SINT32(add_on_battle_camera_top, camera, camera_info);

    READ_JSON_BOOL(disable_battle_camera, camera, camera_info);
}

typedef struct {
    const char* name;
    int value;
} StringEnumPair;

// Function to map string to enum
int MapStringToEnum(const char* str, const StringEnumPair* table, int default_value) {
    for (int i = 0; table[i].name != NULL; i++) {
        if (strcmp(str, table[i].name) == 0) {
            return table[i].value;
        }
    }
    return default_value;
}

#define READ_JSON_STRING_TO_ENUM(value_name, json, ptr, table, default_value) { \
    const json_t* value_name = json_getProperty(json, #value_name); \
    if (value_name && JSON_TEXT == json_getType(value_name)) { \
        const char* temp_ptr = json_getValue(value_name); \
        ptr = MapStringToEnum(temp_ptr, table, default_value); \
    } \
}

#define READ_JSON_STRING_TO_ENUM_GENERIC(enum_type, value_name, json, ptr, table, default_value) { \
    int temp = default_value; \
    READ_JSON_STRING_TO_ENUM(value_name, json, temp, table, default_value) \
    ptr = (enum_type) temp; \
}

void LoadGameModLevelLaraInfo(const json_t* level, MOD_LEVEL_LARA_INFO *lara_info) {
    static const StringEnumPair hair_type_table[] = {
        {"braid", LARA_HAIR_TYPE_BRAID},
        {"pigtails", LARA_HAIR_TYPE_PIGTAILS},
        {"none", LARA_HAIR_TYPE_NONE},
    };

    READ_JSON_STRING_TO_ENUM_GENERIC(LARA_HAIR_TYPE, hair_type, level, lara_info->hair_type, hair_type_table, lara_info->hair_type);

    READ_JSON_SINT32(hair_gravity, level, lara_info);

    READ_JSON_SINT32(braid_x, level, lara_info);
    READ_JSON_SINT32(braid_y, level, lara_info);
    READ_JSON_SINT32(braid_z, level, lara_info);

    READ_JSON_SINT32(pigtail_left_x, level, lara_info);
    READ_JSON_SINT32(pigtail_left_y, level, lara_info);
    READ_JSON_SINT32(pigtail_left_z, level, lara_info);

    READ_JSON_SINT32(pigtail_right_x, level, lara_info);
    READ_JSON_SINT32(pigtail_right_y, level, lara_info);
    READ_JSON_SINT32(pigtail_right_z, level, lara_info);

    READ_JSON_SINT32(crawlspace_jump_animation, level, lara_info);
    READ_JSON_SINT32(crawlspace_jump_pit_deepness_threshold, level, lara_info);

    READ_JSON_BOOL(use_tr5_swimming_collision, level, lara_info);
    READ_JSON_BOOL(disable_hardcoded_breath_sound, level, lara_info);

    READ_JSON_SINT32(ledge_to_jump_state, level, lara_info);
    READ_JSON_SINT32(ledge_to_down_state, level, lara_info);
}

void LoadGameModLevelCreatureInfo(const json_t* creature, MOD_LEVEL_CREATURE_INFO *creature_info) {
    READ_JSON_BOOL_AND_SET_FLAG(small_scorpion_is_poisonous, creature, creature_info, scorpion_poison_override_found);
    READ_JSON_SINT32(small_scorpion_poison_strength, creature, creature_info);

    READ_JSON_BOOL(remove_knights_templar_sparks, creature, creature_info);

    READ_JSON_BOOL(remove_ahmet_death_flames, creature, creature_info);
    READ_JSON_BOOL(remove_ahmet_death_loop, creature, creature_info);
    READ_JSON_BOOL(disable_ahmet_heavy_trigger, creature, creature_info);

    READ_JSON_BOOL(remove_mummy_stun_animations, creature, creature_info);

    READ_JSON_BOOL(disable_sentry_flame_attack, creature, creature_info);
}

void LoadGameModLevelGFXInfo(const json_t* gfx, MOD_LEVEL_GFX_INFO *gfx_info) {
    static const StringEnumPair cold_breath_table[] = {
        {"disabled", COLD_BREATH_DISABLED},
        {"enabled_in_cold_rooms_only", COLD_BREATH_ENABLED_IN_COLD_ROOMS},
        {"enabled_outside_and_in_cold_rooms", COLD_BREATH_ENABLED_OUTSIDE_AND_IN_COLD_ROOMS},
    };

    READ_JSON_STRING_TO_ENUM_GENERIC(
        T4PColdBreath,
        cold_breath,
        gfx,
        gfx_info->cold_breath,
        cold_breath_table,
        gfx_info->cold_breath);

    READ_JSON_SINT16(default_envmap_sprite_index, gfx, gfx_info);
    READ_JSON_SINT16(pickup_envmap_sprite_index, gfx, gfx_info);

    const json_t* mirror_customization = json_getProperty(gfx, "mirror_customization");
    if (mirror_customization && JSON_ARRAY == json_getType(mirror_customization)) {
        json_t const* mirror_customization_json;
        int mirror_customization_index = 0;
        for (mirror_customization_json = json_getChild(mirror_customization); mirror_customization_json != 0; mirror_customization_json = json_getSibling(mirror_customization_json)) {
            if (mirror_customization_index >= MAX_MIRRORS - 1)
                break;


            READ_JSON_SINT16(room_number, mirror_customization_json, &gfx_info->mirror_customization[mirror_customization_index]);
            READ_JSON_SINT32(plane_position, mirror_customization_json, &gfx_info->mirror_customization[mirror_customization_index]);
            
            static const StringEnumPair plane_direction_table[] = {
                {"x", T4_MIR_PLANE_X},
                {"y", T4_MIR_PLANE_Y},
                {"z", T4_MIR_PLANE_Z},
            };
            READ_JSON_STRING_TO_ENUM_GENERIC(
                T4PlusMirrorDirection,
                plane_direction,
                mirror_customization_json,
                gfx_info->mirror_customization[mirror_customization_index].plane_direction,
                plane_direction_table,
                gfx_info->mirror_customization[mirror_customization_index].plane_direction);

            mirror_customization_index++;
        }
    }
}

void LoadGameModObjectCustomizationInfo(const json_t* object_customization_json, MOD_LEVEL_OBJECT_CUSTOMIZATION *object_customization) {
    {
        const json_t* prop = json_getProperty(object_customization_json, "hit_points");
        if (prop && JSON_INTEGER == json_getType(prop)) {
            (object_customization)->override_hit_points = true;
            (object_customization)->hit_points = (signed int)json_getInteger(prop);
        }
    };

    READ_JSON_SINT32(damage_1, object_customization_json, object_customization);
    READ_JSON_SINT32(damage_2, object_customization_json, object_customization);
    READ_JSON_SINT32(damage_3, object_customization_json, object_customization);
}

void LoadGameModLevelObjectsInfo(const json_t* objects, MOD_LEVEL_OBJECTS_INFO* objects_info) {
    READ_JSON_SINT16(lara_slot, objects, objects_info);

    READ_JSON_SINT16(pistols_anim, objects, objects_info);
    READ_JSON_SINT16(uzi_anim, objects, objects_info);
    READ_JSON_SINT16(shotgun_anim, objects, objects_info);
    READ_JSON_SINT16(crossbow_anim, objects, objects_info);
    READ_JSON_SINT16(grenade_gun_anim, objects, objects_info);
    READ_JSON_SINT16(revolver_anim, objects, objects_info);
    READ_JSON_SINT16(flare_anim, objects, objects_info);

    READ_JSON_SINT16(lara_skin_slot, objects, objects_info);
    READ_JSON_SINT16(lara_skin_joints_slot, objects, objects_info);
    READ_JSON_SINT16(lara_scream_slot, objects, objects_info);
    READ_JSON_SINT16(lara_crossbow_laser, objects, objects_info);
    READ_JSON_SINT16(lara_revolver_laser, objects, objects_info);

    READ_JSON_SINT16(lara_holsters, objects, objects_info);
    READ_JSON_SINT16(lara_holsters_pistols, objects, objects_info);
    READ_JSON_SINT16(lara_holsters_uzis, objects, objects_info);
    READ_JSON_SINT16(lara_holsters_revolver, objects, objects_info);

    READ_JSON_SINT16(lara_hair_slot, objects, objects_info);

    READ_JSON_SINT16(motorbike_slot, objects, objects_info);
    READ_JSON_SINT16(jeep_slot, objects, objects_info);
    READ_JSON_SINT16(motorbike_extra_slot, objects, objects_info);
    READ_JSON_SINT16(jeep_extra_slot, objects, objects_info);

    READ_JSON_SINT16(meshswap_1_slot, objects, objects_info);
    READ_JSON_SINT16(meshswap_2_slot, objects, objects_info);
    READ_JSON_SINT16(meshswap_3_slot, objects, objects_info);
    READ_JSON_SINT16(bubbles_slot, objects, objects_info);
    READ_JSON_SINT16(default_sprites_slot, objects, objects_info);

    READ_JSON_SINT16(horizon_slot, objects, objects_info);
    READ_JSON_SINT16(sky_graphics_slot, objects, objects_info);
    READ_JSON_SINT16(binocular_graphics_slot, objects, objects_info);
    READ_JSON_SINT16(target_graphics_slot, objects, objects_info);

    READ_JSON_SINT16(nitrous_oxide_feeder_slot, objects, objects_info);
    READ_JSON_SINT16(jeep_key_slot, objects, objects_info);

    READ_JSON_SINT16(darts_interval, objects, objects_info);
    READ_JSON_SINT16(darts_speed, objects, objects_info);
    READ_JSON_SINT16(falling_block_timer, objects, objects_info);
    READ_JSON_SINT16(falling_block_tremble, objects, objects_info);

    const json_t *object_customization = json_getProperty(objects, "object_customization");
    if (object_customization && JSON_ARRAY == json_getType(object_customization)) {
        json_t const* object_customization_json;
        for (object_customization_json = json_getChild(object_customization); object_customization_json != 0; object_customization_json = json_getSibling(object_customization_json)) {
            int object_customization_index = -1;
            const json_t* prop = json_getProperty(object_customization_json, "object_id");
            if (prop && JSON_INTEGER == json_getType(prop)) {
                object_customization_index = (signed int)json_getInteger(prop);
            }
            
            if (object_customization_index >= NUMBER_OBJECTS || object_customization_index < 0)
                break;

            LoadGameModObjectCustomizationInfo(object_customization_json, &objects_info->object_customization[object_customization_index]);

            object_customization_index++;
        }
    }
}

void LoadGameModLevelMiscInfo(const json_t *misc, MOD_LEVEL_MISC_INFO *misc_info) {
    READ_JSON_INTEGER_CAST(override_fog_mode, misc, misc_info, T4POverrideFogMode);
    READ_JSON_INTEGER_CAST(rain_type, misc, misc_info, T4PWeatherType);
    READ_JSON_INTEGER_CAST(snow_type, misc, misc_info, T4PWeatherType);

    READ_JSON_BOOL(draw_legend_on_flyby, misc, misc_info);
    READ_JSON_UINT32(legend_timer, misc, misc_info);
    READ_JSON_BOOL(lara_impales_on_spikes, misc, misc_info);
    READ_JSON_BOOL(enable_ricochet_sound_effect, misc, misc_info);
    READ_JSON_BOOL(enable_smashing_and_killing_rolling_balls, misc, misc_info);
    READ_JSON_BOOL(enable_standing_pushables, misc, misc_info);
    READ_JSON_BOOL(enemy_gun_hit_underwater_sfx_fix, misc, misc_info);
    READ_JSON_BOOL(darts_poison_fix, misc, misc_info);
    READ_JSON_BOOL(disable_motorbike_headlights, misc, misc_info);
    READ_JSON_BOOL(always_exit_from_statistics_screen, misc, misc_info);

    // TREP
    READ_JSON_BOOL(trep_switch_maker, misc, misc_info);

    READ_JSON_SINT32(trep_switch_on_ocb_1_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_off_ocb_1_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_on_ocb_2_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_off_ocb_2_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_on_ocb_3_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_off_ocb_3_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_on_ocb_4_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_off_ocb_4_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_on_ocb_5_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_off_ocb_5_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_on_ocb_6_anim, misc, misc_info);
    READ_JSON_SINT32(trep_switch_off_ocb_6_anim, misc, misc_info);

    READ_JSON_BOOL(enable_teeth_spikes_kill_enemies, misc, misc_info);
}

void LoadGameModLevelStatInfo(const json_t* stats, MOD_LEVEL_STAT_INFO* stat_info) {
    READ_JSON_UINT32(secret_count, stats, stat_info);

    int equipment_modifier_index = 0;
    const json_t* equipment_modifiers = json_getProperty(stats, "equipment_modifiers");
    if (equipment_modifiers && JSON_ARRAY == json_getType(equipment_modifiers)) {
        const json_t* equipment_modifier;
        for (equipment_modifier = json_getChild(equipment_modifiers); equipment_modifier != nullptr; equipment_modifier = json_getSibling(equipment_modifier)) {
            if (equipment_modifier_index >= MAX_EQUIPMENT_MODIFIERS) {
                break;
            }

            MOD_EQUIPMENT_MODIFIER* mod_equipment_modifier = &stat_info->equipment_modifiers[equipment_modifier_index];
            
            READ_JSON_SINT32(object_id, equipment_modifier, mod_equipment_modifier);
            READ_JSON_SINT32(amount, equipment_modifier, mod_equipment_modifier);

            equipment_modifier_index++;
        }
    }
}

void LoadGameModLevelWeaponInfo(const json_t* stats, MOD_LEVEL_WEAPON_INFO* weapon_info) {

}

void LoadGameModLevel(const json_t *level, MOD_LEVEL_INFO *level_info) {
    const json_t* audio_info = json_getProperty(level, "audio_info");
    if (audio_info && JSON_OBJ == json_getType(audio_info)) {
        LoadGameModLevelAudioInfo(audio_info, &level_info->audio_info);
    }

    const json_t* bars_info = json_getProperty(level, "bars_info");
    if (bars_info && JSON_OBJ == json_getType(bars_info)) {
        LoadGameModLevelBarsInfo(bars_info, &level_info->bars_info);
    }

    const json_t* camera_info = json_getProperty(level, "camera_info");
    if (camera_info && JSON_OBJ == json_getType(camera_info)) {
        LoadGameModLevelCameraInfo(camera_info, &level_info->camera_info);
    }

    const json_t* environment_info = json_getProperty(level, "environment_info");
    if (environment_info && JSON_OBJ == json_getType(environment_info)) {
        LoadGameModLevelEnvironmentInfo(environment_info, &level_info->environment_info);
    }

    const json_t* font_info = json_getProperty(level, "font_info");
    if (font_info && JSON_OBJ == json_getType(font_info)) {
        LoadGameModLevelFontInfo(font_info, &level_info->font_info);
    }

    const json_t* lara_info = json_getProperty(level, "lara_info");
    if (lara_info && JSON_OBJ == json_getType(lara_info)) {
        LoadGameModLevelLaraInfo(lara_info, &level_info->lara_info);
    }

    const json_t* creature_info = json_getProperty(level, "creature_info");
    if (creature_info && JSON_OBJ == json_getType(creature_info)) {
        LoadGameModLevelCreatureInfo(creature_info, &level_info->creature_info);
    }

    const json_t* gfx_info = json_getProperty(level, "gfx_info");
    if (gfx_info && JSON_OBJ == json_getType(gfx_info)) {
        LoadGameModLevelGFXInfo(gfx_info, &level_info->gfx_info);
    }

    const json_t* objects_info = json_getProperty(level, "objects_info");
    if (objects_info && JSON_OBJ == json_getType(objects_info)) {
        LoadGameModLevelObjectsInfo(objects_info, &level_info->objects_info);
    }

    const json_t* stat_info = json_getProperty(level, "stat_info");
    if (stat_info && JSON_OBJ == json_getType(stat_info)) {
        LoadGameModLevelStatInfo(stat_info, &level_info->stat_info);
    }

    const json_t* weapon_info = json_getProperty(level, "weapon_info");
    if (weapon_info && JSON_OBJ == json_getType(weapon_info)) {
        LoadGameModLevelWeaponInfo(weapon_info, &level_info->weapon_info);
    }

    const json_t* misc_info = json_getProperty(level, "misc_info");
    if (misc_info && JSON_OBJ == json_getType(misc_info)) {
        LoadGameModLevelMiscInfo(misc_info, &level_info->misc_info);
    }
}

void SetupDefaultFontInfoForLevel(MOD_LEVEL_INFO* level_info) {
    level_info->font_info.custom_glyph_scale_width = 512;
    level_info->font_info.custom_glyph_scale_height = 240;
    level_info->font_info.custom_vertical_spacing = 0.075f;

    level_info->font_info.main_font_main_color = 0xff808080;
    level_info->font_info.main_font_fade_color = 0xff808080;

    level_info->font_info.options_title_font_main_color = 0xffc08040;
    level_info->font_info.options_title_font_fade_color = 0xff401000;

    level_info->font_info.inventory_title_font_main_color = 0xffe0c000;
    level_info->font_info.inventory_title_font_fade_color = 0xff402000;

    level_info->font_info.inventory_item_font_main_color = 0xff808080;
    level_info->font_info.inventory_item_font_fade_color = 0xff101010;

    memcpy(level_info->font_info.custom_font_table, DEFAULT_CHAR_TABLE, sizeof(CHARDEF) * CHAR_TABLE_COUNT);
}

void SetupDefaultSlotInfoForLevel(MOD_LEVEL_INFO* level_info) {
    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        level_info->objects_info.slot_override[i] = i;
    }
}

void SetupDefaultStaticsInfoForLevel(MOD_LEVEL_INFO* level_info) {
    for (int i = 0; i < NUMBER_STATIC_OBJECTS; i++) {
        if (i >= SHATTER0) {
            if (i <= SHATTER9) {
                level_info->statics_info.static_info[i].large_objects_can_shatter = true;
            }
            if (i < SHATTER8) {
                level_info->statics_info.static_info[i].lara_guns_can_shatter = true;
            }

            level_info->statics_info.static_info[i].creatures_can_shatter = true;
            level_info->statics_info.static_info[i].record_shatter_state_in_savegames = true;
            level_info->statics_info.static_info[i].hard_collision = false;
            level_info->statics_info.static_info[i].shatter_sound_id = SFX_HIT_ROCK;
        }
    }
}

void SetupDefaultWeaponInfoForLevel(MOD_LEVEL_INFO* level_info) {
    // Pistols
    level_info->weapon_info.pistol_ammo_info.damage = 1;
    level_info->weapon_info.pistol_ammo_info.fire_rate = 9;
    level_info->weapon_info.pistol_ammo_info.dispertion = 1456;
    level_info->weapon_info.pistol_ammo_info.flash_duration = 3;

    level_info->weapon_info.pistol_ammo_info.weapon_pickup_amount = 40;
    level_info->weapon_info.pistol_ammo_info.ammo_pickup_amount = 40;

    level_info->weapon_info.pistol_ammo_info.add_pistol_shell = true;

    // Uzis
    level_info->weapon_info.uzi_ammo_info.damage = 1;
    level_info->weapon_info.uzi_ammo_info.fire_rate = 3;
    level_info->weapon_info.uzi_ammo_info.dispertion = 1456;
    level_info->weapon_info.uzi_ammo_info.flash_duration = 3;

    level_info->weapon_info.uzi_ammo_info.weapon_pickup_amount = 30;
    level_info->weapon_info.uzi_ammo_info.ammo_pickup_amount = 30;

    level_info->weapon_info.uzi_ammo_info.add_pistol_shell = true;

    // Six-Shooter
    level_info->weapon_info.six_shooter_ammo_info.damage = 21;
    level_info->weapon_info.six_shooter_ammo_info.fire_rate = 16;
    level_info->weapon_info.six_shooter_ammo_info.dispertion = 728;
    level_info->weapon_info.six_shooter_ammo_info.flash_duration = 3;

    level_info->weapon_info.six_shooter_ammo_info.weapon_pickup_amount = 6;
    level_info->weapon_info.six_shooter_ammo_info.ammo_pickup_amount = 6;

    // Shotgun (Normal ammo)
    level_info->weapon_info.shotgun_1_ammo_info.shots = 6;
    level_info->weapon_info.shotgun_1_ammo_info.damage = 3;
    level_info->weapon_info.shotgun_1_ammo_info.fire_rate = 32;
    level_info->weapon_info.shotgun_1_ammo_info.dispertion = 0; // TODO: softcode wideshot vs normal support
    level_info->weapon_info.shotgun_1_ammo_info.flash_duration = 3;

    level_info->weapon_info.shotgun_1_ammo_info.weapon_pickup_amount = 6;
    level_info->weapon_info.shotgun_1_ammo_info.ammo_pickup_amount = 6;

    level_info->weapon_info.shotgun_1_ammo_info.add_shotgun_shell = true;

    // Shotgun (Wideshot ammo)
    level_info->weapon_info.shotgun_2_ammo_info.shots = 6;
    level_info->weapon_info.shotgun_2_ammo_info.damage = 3;
    level_info->weapon_info.shotgun_2_ammo_info.fire_rate = 32;
    level_info->weapon_info.shotgun_2_ammo_info.dispertion = 0; // TODO: softcode wideshot vs normal support
    level_info->weapon_info.shotgun_2_ammo_info.flash_duration = 3;

    level_info->weapon_info.shotgun_2_ammo_info.weapon_pickup_amount = 6;
    level_info->weapon_info.shotgun_2_ammo_info.ammo_pickup_amount = 6;

    level_info->weapon_info.shotgun_2_ammo_info.add_shotgun_shell = true;

    // Grenade gun
    // Normal
    level_info->weapon_info.grenade_1_ammo_info.shots = 1;
    level_info->weapon_info.grenade_1_ammo_info.damage = 20;

    level_info->weapon_info.grenade_1_ammo_info.dispertion = 1456;

    // Super
    level_info->weapon_info.grenade_2_ammo_info.shots = 1;
    level_info->weapon_info.grenade_2_ammo_info.damage = 20;

    level_info->weapon_info.grenade_2_ammo_info.dispertion = 1456;

    // Flash
    level_info->weapon_info.grenade_3_ammo_info.shots = 1;
    level_info->weapon_info.grenade_3_ammo_info.damage = 20;

    level_info->weapon_info.grenade_3_ammo_info.dispertion = 1456;

    // Crossbow
    // Normal
    level_info->weapon_info.crossbow_1_ammo_info.shots = 1;
    level_info->weapon_info.crossbow_1_ammo_info.damage = 5;

    level_info->weapon_info.crossbow_1_ammo_info.dispertion = 1456;

    // Poison
    level_info->weapon_info.crossbow_2_ammo_info.shots = 1;
    level_info->weapon_info.crossbow_2_ammo_info.damage = 5;

    level_info->weapon_info.crossbow_2_ammo_info.dispertion = 1456;

    // Explosive
    level_info->weapon_info.crossbow_3_ammo_info.shots = 1;
    level_info->weapon_info.crossbow_3_ammo_info.damage = 5;

    level_info->weapon_info.crossbow_3_ammo_info.dispertion = 1456;


}

void SetupDefaultObjectInfoForLevel(MOD_LEVEL_INFO* level_info) {

    MOD_LEVEL_OBJECT_CUSTOMIZATION *level_object_customization = level_info->objects_info.object_customization;

    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        MOD_LEVEL_OBJECT_CUSTOMIZATION* cust = &level_object_customization[i];
        cust->hit_points = -16384;
        cust->damage_1 = 0;
        cust->damage_2 = 0;
        cust->damage_3 = 0;
        cust->override_hit_points = false;
        cust->override_hit_type = false;
        cust->explode_immediately = false;
        cust->explode_after_death_animation = false;
        cust->hit_type = HIT_NONE;
        cust->explosive_death_only = false;
    }

    level_object_customization[JEEP].damage_1 = 1000; // TODO
    
    level_object_customization[SKELETON].damage_1 = 80;
    
    level_object_customization[BADDY_1].damage_1 = 15;
    level_object_customization[BADDY_1].damage_2 = 120;

    level_object_customization[BADDY_2].damage_1 = 15;
    level_object_customization[BADDY_2].damage_2 = 120;

    level_object_customization[SETHA].damage_1 = 200;
    level_object_customization[SETHA].damage_2 = 250;

    level_object_customization[MUMMY].damage_1 = 100;

    level_object_customization[SPHINX].damage_1 = 200;

    level_object_customization[CROCODILE].damage_1 = 120;

    level_object_customization[HORSEMAN].damage_1 = 10; //TODO
    level_object_customization[HORSEMAN].damage_2 = 20; // TODO
    level_object_customization[HORSEMAN].damage_3 = 150; // TODO

    level_object_customization[SCORPION].damage_1 = 120;

    level_object_customization[TROOPS].damage_1 = 23;
    level_object_customization[TROOPS].damage_2 = 15;

    level_object_customization[KNIGHTS_TEMPLAR].damage_1 = 120;

    level_object_customization[HORSE].damage_1 = 150; // TODO
    level_object_customization[HORSE].damage_2 = 250; // TODO
    level_object_customization[HORSE].damage_3 = 100; // TODO

    level_object_customization[BABOON_NORMAL].damage_1 = 70;
    level_object_customization[WILD_BOAR].damage_1 = 30;

    level_object_customization[HARPY].damage_1 = 10;
    level_object_customization[HARPY].damage_2 = 100;

    level_object_customization[LITTLE_BEETLE].damage_1 = 1;

    level_object_customization[BIG_BEETLE].damage_1 = 50;

    //level_object_customization[WRAITH1].damage_1 = ?;
    //level_object_customization[WRAITH2].damage_1 = ?;
    //level_object_customization[WRAITH3].damage_1 = ?;
    //level_object_customization[WRAITH4].damage_1 = ?;

    level_object_customization[BAT].damage_1 = 2;

    level_object_customization[DOG].damage_1 = 10;
    level_object_customization[DOG].damage_2 = 20;

    level_object_customization[HAMMERHEAD].damage_1 = 120;

    level_object_customization[SAS].damage_1 = 15;
    level_object_customization[SAS].damage_2 = 50; // TODO

    level_object_customization[AHMET].damage_1 = 80;
    level_object_customization[AHMET].damage_2 = 120;

    level_object_customization[LARA_DOUBLE].damage_1 = 1000;

    level_object_customization[SMALL_SCORPION].damage_1 = 20;

    level_object_customization[FISH].damage_1 = 3;

    level_object_customization[DARTS].damage_1 = 25;

    level_object_customization[FALLING_CEILING].damage_1 = 300;

    level_object_customization[ROLLINGBALL].damage_1 = 8;

    level_object_customization[TEETH_SPIKES].damage_1 = 8;

    // Fill in the rest...

    level_object_customization[SENTRY_GUN].damage_1 = 5;
}

void SetupDefaultBarsInfoForLevel(MOD_LEVEL_INFO* level_info) {
    const uint32_t BORDER_COLOR = 0xffffffff;
    const uint32_t BACKGROUND_COLOR = 0xff000000;
    const uint32_t BORDER_SIZE = 1;

    // Health
    const uint32_t HEALTH_BAR_COLOR = 0xffff0000;
    const uint32_t HEALTH_BAR_FADE = 0xff000000;

    level_info->bars_info.health_bar.x = 8;
    level_info->bars_info.health_bar.y = 8;
    level_info->bars_info.health_bar.width = 150;
    level_info->bars_info.health_bar.height = 12;

    level_info->bars_info.health_bar.border_rect.upper_left_color = BORDER_COLOR;
    level_info->bars_info.health_bar.border_rect.upper_right_color = BORDER_COLOR;
    level_info->bars_info.health_bar.border_rect.lower_right_color = BORDER_COLOR;
    level_info->bars_info.health_bar.border_rect.lower_left_color = BORDER_COLOR;

    level_info->bars_info.health_bar.background_rect.upper_left_color = BACKGROUND_COLOR;
    level_info->bars_info.health_bar.background_rect.upper_right_color = BACKGROUND_COLOR;
    level_info->bars_info.health_bar.background_rect.lower_right_color = BACKGROUND_COLOR;
    level_info->bars_info.health_bar.background_rect.lower_left_color = BACKGROUND_COLOR;

    level_info->bars_info.health_bar.border_size = BORDER_SIZE;

    level_info->bars_info.health_bar.lower_rect.upper_left_color = HEALTH_BAR_COLOR;
    level_info->bars_info.health_bar.lower_rect.upper_right_color = HEALTH_BAR_COLOR;
    level_info->bars_info.health_bar.lower_rect.lower_right_color = HEALTH_BAR_FADE;
    level_info->bars_info.health_bar.lower_rect.lower_left_color = HEALTH_BAR_FADE;

    level_info->bars_info.health_bar.upper_rect.upper_left_color = HEALTH_BAR_FADE;
    level_info->bars_info.health_bar.upper_rect.upper_right_color = HEALTH_BAR_FADE;
    level_info->bars_info.health_bar.upper_rect.lower_right_color = HEALTH_BAR_COLOR;
    level_info->bars_info.health_bar.upper_rect.lower_left_color = HEALTH_BAR_COLOR;

    // Poisoned
    const uint32_t POISON_BAR_COLOR = 0xffffff00;
    const uint32_t POISON_BAR_FADE = 0xff000000;

    level_info->bars_info.poison_bar.x = 8;
    level_info->bars_info.poison_bar.y = 8;
    level_info->bars_info.poison_bar.width = 150;
    level_info->bars_info.poison_bar.height = 12;

    level_info->bars_info.poison_bar.border_rect.upper_left_color = BORDER_COLOR;
    level_info->bars_info.poison_bar.border_rect.upper_right_color = BORDER_COLOR;
    level_info->bars_info.poison_bar.border_rect.lower_right_color = BORDER_COLOR;
    level_info->bars_info.poison_bar.border_rect.lower_left_color = BORDER_COLOR;

    level_info->bars_info.poison_bar.background_rect.upper_left_color = BACKGROUND_COLOR;
    level_info->bars_info.poison_bar.background_rect.upper_right_color = BACKGROUND_COLOR;
    level_info->bars_info.poison_bar.background_rect.lower_right_color = BACKGROUND_COLOR;
    level_info->bars_info.poison_bar.background_rect.lower_left_color = BACKGROUND_COLOR;

    level_info->bars_info.poison_bar.border_size = BORDER_SIZE;

    level_info->bars_info.poison_bar.lower_rect.upper_left_color = POISON_BAR_COLOR;
    level_info->bars_info.poison_bar.lower_rect.upper_right_color = POISON_BAR_COLOR;
    level_info->bars_info.poison_bar.lower_rect.lower_right_color = POISON_BAR_FADE;
    level_info->bars_info.poison_bar.lower_rect.lower_left_color = POISON_BAR_FADE;

    level_info->bars_info.poison_bar.upper_rect.upper_left_color = POISON_BAR_FADE;
    level_info->bars_info.poison_bar.upper_rect.upper_right_color = POISON_BAR_FADE;
    level_info->bars_info.poison_bar.upper_rect.lower_right_color = POISON_BAR_COLOR;
    level_info->bars_info.poison_bar.upper_rect.lower_left_color = POISON_BAR_COLOR;

    // Air
    const uint32_t AIR_BAR_COLOR = 0xff0000ff;
    const uint32_t AIR_BAR_FADE = 0xff000000;

    level_info->bars_info.air_bar.x = 8;
    level_info->bars_info.air_bar.y = 25;
    level_info->bars_info.air_bar.width = 150;
    level_info->bars_info.air_bar.height = 12;

    level_info->bars_info.air_bar.border_rect.upper_left_color = BORDER_COLOR;
    level_info->bars_info.air_bar.border_rect.upper_right_color = BORDER_COLOR;
    level_info->bars_info.air_bar.border_rect.lower_right_color = BORDER_COLOR;
    level_info->bars_info.air_bar.border_rect.lower_left_color = BORDER_COLOR;

    level_info->bars_info.air_bar.background_rect.upper_left_color = BACKGROUND_COLOR;
    level_info->bars_info.air_bar.background_rect.upper_right_color = BACKGROUND_COLOR;
    level_info->bars_info.air_bar.background_rect.lower_right_color = BACKGROUND_COLOR;
    level_info->bars_info.air_bar.background_rect.lower_left_color = BACKGROUND_COLOR;

    level_info->bars_info.air_bar.border_size = BORDER_SIZE;

    level_info->bars_info.air_bar.lower_rect.upper_left_color = AIR_BAR_COLOR;
    level_info->bars_info.air_bar.lower_rect.upper_right_color = AIR_BAR_COLOR;
    level_info->bars_info.air_bar.lower_rect.lower_right_color = AIR_BAR_FADE;
    level_info->bars_info.air_bar.lower_rect.lower_left_color = AIR_BAR_FADE;

    level_info->bars_info.air_bar.upper_rect.upper_left_color = AIR_BAR_FADE;
    level_info->bars_info.air_bar.upper_rect.upper_right_color = AIR_BAR_FADE;
    level_info->bars_info.air_bar.upper_rect.lower_right_color = AIR_BAR_COLOR;
    level_info->bars_info.air_bar.upper_rect.lower_left_color = AIR_BAR_COLOR;

    // Sprint
    const uint32_t SPRINT_BAR_COLOR = 0xff00ff00;
    const uint32_t SPRINT_BAR_FADE = 0xff000000;

    level_info->bars_info.sprint_bar.x = 8;
    level_info->bars_info.sprint_bar.y = 8;
    level_info->bars_info.sprint_bar.width = 150;
    level_info->bars_info.sprint_bar.height = 12;

    level_info->bars_info.sprint_bar.border_rect.upper_left_color = BORDER_COLOR;
    level_info->bars_info.sprint_bar.border_rect.upper_right_color = BORDER_COLOR;
    level_info->bars_info.sprint_bar.border_rect.lower_right_color = BORDER_COLOR;
    level_info->bars_info.sprint_bar.border_rect.lower_left_color = BORDER_COLOR;

    level_info->bars_info.sprint_bar.background_rect.upper_left_color = BACKGROUND_COLOR;
    level_info->bars_info.sprint_bar.background_rect.upper_right_color = BACKGROUND_COLOR;
    level_info->bars_info.sprint_bar.background_rect.lower_right_color = BACKGROUND_COLOR;
    level_info->bars_info.sprint_bar.background_rect.lower_left_color = BACKGROUND_COLOR;

    level_info->bars_info.sprint_bar.border_size = BORDER_SIZE;

    level_info->bars_info.sprint_bar.lower_rect.upper_left_color = SPRINT_BAR_COLOR;
    level_info->bars_info.sprint_bar.lower_rect.upper_right_color = SPRINT_BAR_COLOR;
    level_info->bars_info.sprint_bar.lower_rect.lower_right_color = SPRINT_BAR_FADE;
    level_info->bars_info.sprint_bar.lower_rect.lower_left_color = SPRINT_BAR_FADE;

    level_info->bars_info.sprint_bar.upper_rect.upper_left_color = SPRINT_BAR_FADE;
    level_info->bars_info.sprint_bar.upper_rect.upper_right_color = SPRINT_BAR_FADE;
    level_info->bars_info.sprint_bar.upper_rect.lower_right_color = SPRINT_BAR_COLOR;
    level_info->bars_info.sprint_bar.upper_rect.lower_left_color = SPRINT_BAR_COLOR;

    // Loading
    const uint32_t LOADING_BAR_COLOR = 0xff9f1f80;
    const uint32_t LOADING_BAR_FADE = 0xff000000;

    level_info->bars_info.loading_bar.border_rect.upper_left_color = BORDER_COLOR;
    level_info->bars_info.loading_bar.border_rect.upper_right_color = BORDER_COLOR;
    level_info->bars_info.loading_bar.border_rect.lower_right_color = BORDER_COLOR;
    level_info->bars_info.loading_bar.border_rect.lower_left_color = BORDER_COLOR;

    level_info->bars_info.loading_bar.background_rect.upper_left_color = BACKGROUND_COLOR;
    level_info->bars_info.loading_bar.background_rect.upper_right_color = BACKGROUND_COLOR;
    level_info->bars_info.loading_bar.background_rect.lower_right_color = BACKGROUND_COLOR;
    level_info->bars_info.loading_bar.background_rect.lower_left_color = BACKGROUND_COLOR;

    level_info->bars_info.loading_bar.border_size = BORDER_SIZE;

    level_info->bars_info.loading_bar.lower_rect.upper_left_color = LOADING_BAR_COLOR;
    level_info->bars_info.loading_bar.lower_rect.upper_right_color = LOADING_BAR_COLOR;
    level_info->bars_info.loading_bar.lower_rect.lower_right_color = LOADING_BAR_FADE;
    level_info->bars_info.loading_bar.lower_rect.lower_left_color = LOADING_BAR_FADE;

    level_info->bars_info.loading_bar.upper_rect.upper_left_color = LOADING_BAR_FADE;
    level_info->bars_info.loading_bar.upper_rect.upper_right_color = LOADING_BAR_FADE;
    level_info->bars_info.loading_bar.upper_rect.lower_right_color = LOADING_BAR_COLOR;
    level_info->bars_info.loading_bar.upper_rect.lower_left_color = LOADING_BAR_COLOR;

    // Enemy
    const uint32_t ENEMY_BAR_COLOR = 0xffffa000;
    const uint32_t ENEMY_BAR_FADE = 0xff000000;

    level_info->bars_info.enemy_bar.x = 8;
    level_info->bars_info.enemy_bar.y = 25;
    level_info->bars_info.enemy_bar.width = 150;
    level_info->bars_info.enemy_bar.height = 12;

    level_info->bars_info.enemy_bar.border_rect.upper_left_color = BORDER_COLOR;
    level_info->bars_info.enemy_bar.border_rect.upper_right_color = BORDER_COLOR;
    level_info->bars_info.enemy_bar.border_rect.lower_right_color = BORDER_COLOR;
    level_info->bars_info.enemy_bar.border_rect.lower_left_color = BORDER_COLOR;

    level_info->bars_info.enemy_bar.background_rect.upper_left_color = BACKGROUND_COLOR;
    level_info->bars_info.enemy_bar.background_rect.upper_right_color = BACKGROUND_COLOR;
    level_info->bars_info.enemy_bar.background_rect.lower_right_color = BACKGROUND_COLOR;
    level_info->bars_info.enemy_bar.background_rect.lower_left_color = BACKGROUND_COLOR;

    level_info->bars_info.enemy_bar.border_size = BORDER_SIZE;

    level_info->bars_info.enemy_bar.lower_rect.upper_left_color = ENEMY_BAR_COLOR;
    level_info->bars_info.enemy_bar.lower_rect.upper_right_color = ENEMY_BAR_COLOR;
    level_info->bars_info.enemy_bar.lower_rect.lower_right_color = ENEMY_BAR_FADE;
    level_info->bars_info.enemy_bar.lower_rect.lower_left_color = ENEMY_BAR_FADE;

    level_info->bars_info.enemy_bar.upper_rect.upper_left_color = ENEMY_BAR_FADE;
    level_info->bars_info.enemy_bar.upper_rect.upper_right_color = ENEMY_BAR_FADE;
    level_info->bars_info.enemy_bar.upper_rect.lower_right_color = ENEMY_BAR_COLOR;
    level_info->bars_info.enemy_bar.upper_rect.lower_left_color = ENEMY_BAR_COLOR;
}

void SetupDefaultStatInfoForLevel(MOD_LEVEL_INFO* level_info) {
    level_info->stat_info.secret_count = 70;
    if (get_game_mod_global_info()->tr_times_exclusive) {
        level_info->stat_info.secret_count = 2;
    }
}

void SetupLevelDefaults() {
    for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
        if (i <= 3) {
            if (!scorpion_poison_override_found)
                game_mod_config.level_info[i].creature_info.small_scorpion_is_poisonous = false;
        }

        if (!get_game_mod_global_info()->tr_level_editor) {
            if (i == 5 || i == 6) {
                game_mod_config.level_info[i].environment_info.force_train_fog = true;
            }
        }
    }
}

void SetupGlobalDefaults() {
    MOD_GLOBAL_INFO* mod_global_info = &game_mod_config.global_info;

#ifdef TIMES_LEVEL
    mod_global_info->tr_times_exclusive = true;
    mod_global_info->tr_level_editor = false;
    mod_global_info->tr_use_adpcm_audio = true;
#endif

#ifndef LEVEL_EDITOR
    mod_global_info->tr_level_editor = false;
    mod_global_info->tr_use_adpcm_audio = true;
#endif
}

#define CLEANUP_JSON_MEMORY if (json_buf) {SYSTEM_FREE(json_buf);} if (mem) {SYSTEM_FREE(mem);}

bool LoadGameModConfigFirstPass() {
    char* json_buf = NULL;
    if (LoadFile("game_mod_config.json", &json_buf) <= 0) {
        platform_fatal_error("Missing 'game_mod_config.json' game manifest from working directory \"%s\". Please examine README.md file for further information about how to create a game manifest.", working_dir_path.c_str());
        return false;
    }

    SetupGlobalDefaults();

    const json_t* level = nullptr;

    FURROpcode *furr_command_list = (FURROpcode * )SYSTEM_MALLOC(MAX_FURR_COMMANDS * sizeof(FURROpcode));
   
    json_t* mem = NULL;
    if (json_buf) {
        mem = (json_t*)SYSTEM_MALLOC(MAXIMUM_JSON_ALLOCATION_BLOCKS * sizeof(json_t));
        if (mem) {
            const json_t* root_json = json_create(json_buf, mem, MAXIMUM_JSON_ALLOCATION_BLOCKS);
            if (root_json) {
                const json_t* global = json_getProperty(root_json, "global_info");
                if (global && JSON_OBJ == json_getType(global)) {
                    MOD_GLOBAL_INFO* mod_global_info = &game_mod_config.global_info;

                    READ_JSON_SINT32(manifest_compatibility_version, global, mod_global_info);
                    if (mod_global_info->manifest_compatibility_version < ENGINE_MANIFEST_VERSION) {
                        CLEANUP_JSON_MEMORY
                        platform_fatal_error("game_mod_config manifest version (%i) is incompatible with the ENGINE_MANIFEST_VERSION (%i). Update the manifest and try running again.", mod_global_info->manifest_compatibility_version, ENGINE_MANIFEST_VERSION);
                        return false;
                    } else if (mod_global_info->manifest_compatibility_version > ENGINE_MANIFEST_VERSION) {
                        CLEANUP_JSON_MEMORY
                        platform_fatal_error("game_mod_config manifest version (%i) is incompatible with the ENGINE_MANIFEST_VERSION (%i). Update the engine to a newer version and try running again.", mod_global_info->manifest_compatibility_version, ENGINE_MANIFEST_VERSION);
                        return false;
                    }

                    READ_JSON_STRING(game_name, global, mod_global_info);
                    READ_JSON_STRING(authors, global, mod_global_info);
                    READ_JSON_STRING(game_user_dir_name, global, mod_global_info);

                    READ_JSON_UINT32(tr_engine_version, global, mod_global_info);
                    READ_JSON_BOOL(tr_level_editor, global, mod_global_info);
                    READ_JSON_BOOL(tr_times_exclusive, global, mod_global_info);
                    READ_JSON_BOOL(tr_use_adpcm_audio, global, mod_global_info);

                    // TRNG
                    READ_JSON_UINT8(trng_version_major, global, &mod_global_info->trng_engine_version);
                    READ_JSON_UINT8(trng_version_minor, global, &mod_global_info->trng_engine_version);
                    READ_JSON_UINT8(trng_version_maintainence, global, &mod_global_info->trng_engine_version);
                    READ_JSON_UINT8(trng_version_build, global, &mod_global_info->trng_engine_version);

                    const json_t* furr_data = json_getProperty(global, "furr_data");
                    if (furr_data && JSON_OBJ == json_getType(furr_data)) {
                        const json_t* furr_flipeffects = json_getProperty(furr_data, "furr_flipeffects");
                        if (furr_flipeffects && JSON_ARRAY == json_getType(furr_flipeffects)) {
                            json_t const* furr_flipeffect;
                            int furr_flipeffect_index = 47;
                            for (furr_flipeffect = json_getChild(furr_flipeffects); furr_flipeffect != nullptr; furr_flipeffect = json_getSibling(furr_flipeffect)) {
                                if (furr_flipeffect_index > LAST_FURR_FLIPEFFECT) {
                                    Log(1, "LoadGameModConfigFirstPass: FURR flipeffect overflow!");
                                    break;
                                }
                                if (furr_flipeffect && JSON_ARRAY == json_getType(furr_flipeffect)) {
                                    json_t const* furr_flipeffect_block;
                                    int furr_flipeffect_block_index = 0;

                                    //
                                    int furr_command_count = 0;
                                    int furr_command_buffer_size = 0;

                                    // First pass to determine the required size of the bytecode buffer
                                    for (furr_flipeffect_block = json_getChild(furr_flipeffect); furr_flipeffect_block != nullptr; furr_flipeffect_block = json_getSibling(furr_flipeffect_block)) {
                                        if (furr_flipeffect_block && JSON_ARRAY == json_getType(furr_flipeffect_block)) {
                                            json_t const* furr_flipeffect_data;
                                            int furr_flipeffect_data_index = 0;
                                            // Count the data
                                            for (furr_flipeffect_data = json_getChild(furr_flipeffect_block); furr_flipeffect_data != nullptr; furr_flipeffect_data = json_getSibling(furr_flipeffect_data)) {
                                                if (furr_flipeffect_data_index == 0) {
                                                    if (furr_flipeffect_data && JSON_TEXT == json_getType(furr_flipeffect_data)) {
                                                        const char *name = furr_flipeffect_data->u.value;
                                                        int opcode_id = furr_get_opcode_for_command_string(name);
                                                        if (opcode_id >= 0) {
                                                            int furr_command_arg_count = furr_get_arg_count_for_opcode((FURROpcode)opcode_id);

                                                            furr_command_list[furr_command_count] = (FURROpcode)opcode_id;

                                                            furr_command_count++;
                                                            if (furr_command_count >= MAX_FURR_COMMANDS) {
                                                                platform_fatal_error("LoadGameModConfigFirstPass: Exceed maximum allowed FURR commands in flipeffect!");
                                                                SYSTEM_FREE(furr_command_list);
                                                                CLEANUP_JSON_MEMORY
                                                                return false;
                                                            }
                                                            furr_command_buffer_size += (1 + furr_command_arg_count);
                                                        } else {
                                                            platform_fatal_error("LoadGameModConfigFirstPass: unknown FURR opcode detected!");
                                                            SYSTEM_FREE(furr_command_list);
                                                            CLEANUP_JSON_MEMORY
                                                            return false;
                                                        }
                                                    }
                                                } else {
                                                    break;
                                                }
                                                furr_flipeffect_data_index++;
                                            }

                                        }
                                        furr_flipeffect_block_index++;
                                    }

                                    furr_flipeffect_block_index = 0;
                                    furr_allocate_flipeffect_buffer(furr_flipeffect_index, furr_command_buffer_size);


                                    //
                                    // Second pass to populate the bytecode buffer
                                    for (furr_flipeffect_block = json_getChild(furr_flipeffect); furr_flipeffect_block != nullptr; furr_flipeffect_block = json_getSibling(furr_flipeffect_block)) {
                                        if (furr_flipeffect_block && JSON_ARRAY == json_getType(furr_flipeffect_block)) {
                                            json_t const* furr_flipeffect_data;
                                            int furr_flipeffect_data_index = 0;
                                            int furr_command_arg_count = 0;

                                            // Count the data
                                            for (furr_flipeffect_data = json_getChild(furr_flipeffect_block); furr_flipeffect_data != nullptr; furr_flipeffect_data = json_getSibling(furr_flipeffect_data)) {
                                                if (furr_flipeffect_data_index != 0) {
                                                    if (furr_flipeffect_data_index <= furr_command_arg_count) {
                                                        if (furr_flipeffect_data && JSON_INTEGER == json_getType(furr_flipeffect_data)) {
                                                            int argument_value = (int)json_getInteger(furr_flipeffect_data);
                                                            furr_add_flipeffect_token(furr_flipeffect_index, argument_value);
                                                        }
                                                        else {
                                                            furr_add_flipeffect_token(furr_flipeffect_index, 0);
                                                        }
                                                    }
                                                } else {
                                                    furr_command_arg_count = furr_get_arg_count_for_opcode(furr_command_list[furr_flipeffect_block_index]);
                                                    furr_add_flipeffect_token(furr_flipeffect_index, furr_command_list[furr_flipeffect_block_index]);
                                                }
                                                furr_flipeffect_data_index++;
                                            }
                                            
                                            // In case we're missing args
                                            if (furr_flipeffect_data_index < furr_command_arg_count) {
                                                furr_add_flipeffect_token(furr_flipeffect_index, 0);
                                                furr_flipeffect_data_index++;
                                            }

                                        }
                                        furr_flipeffect_block_index++;
                                    }
                                }
                                furr_flipeffect_index++;
                            }
                        }
                    }

                    const json_t* plugins = json_getProperty(global, "plugins");
                    if (plugins && JSON_ARRAY == json_getType(plugins)) {
                        json_t const* plugin;
                        for (plugin = json_getChild(plugins); plugin != 0; plugin = json_getSibling(plugin)) {
                            if (JSON_OBJ == json_getType(plugin)) {
                                MOD_GLOBAL_PLUGIN mod_global_plugin {};

                                READ_JSON_STRING(plugin_name, plugin, &mod_global_plugin);
                                READ_JSON_STRING(plugin_type, plugin, &mod_global_plugin);
                                READ_JSON_STRING(plugin_source, plugin, &mod_global_plugin);

                                if (mod_global_plugin.plugin_type && mod_global_plugin.plugin_name && mod_global_plugin.plugin_source) {
                                    if (strcmp(mod_global_plugin.plugin_type, "builtin") == 0) {
                                        T4PlusRegisterBuiltinPlugin(mod_global_plugin.plugin_name, mod_global_plugin.plugin_source);
                                    }
                                }
                            }
                        }
                   }
                }
                level = json_getProperty(root_json, "global_level_info");
            } else {
                platform_fatal_error("Not enough memory allocated for JSON buffer!");
                return false;
            }
        } else {
            platform_fatal_error("Failed to allocate memory for JSON buffer!");
            return false;
        }
    }

    SYSTEM_FREE(furr_command_list);

    MOD_LEVEL_INFO global_level_info;

    SetupDefaultFontInfoForLevel(&global_level_info);
    SetupDefaultSlotInfoForLevel(&global_level_info);
    SetupDefaultStaticsInfoForLevel(&global_level_info);
    SetupDefaultObjectInfoForLevel(&global_level_info);
    SetupDefaultBarsInfoForLevel(&global_level_info);
    SetupDefaultStatInfoForLevel(&global_level_info);

    if (level && JSON_OBJ == json_getType(level)) {
        LoadGameModLevel(level, &global_level_info);
    }
    for (int i = 0; i < MOD_LEVEL_COUNT; i++) {
        memcpy(&game_mod_config.level_info[i], &global_level_info, sizeof(MOD_LEVEL_INFO));

        // Reset equipment modifiers.
        if (i > 0) {
            for (int j = 0; j < MAX_EQUIPMENT_MODIFIERS; j++) {
                game_mod_config.level_info[i].stat_info.equipment_modifiers[j].object_id = -1;
                game_mod_config.level_info[i].stat_info.equipment_modifiers[j].amount = 0;
            }
        }
    }

    CLEANUP_JSON_MEMORY

    SetupLevelDefaults();
    if (!SetupUserDirectories()) {
        return false;
    }

    return true;
}

void LoadGameModConfigSecondPass() {
    char* json_buf = NULL;
    if (LoadFile("game_mod_config.json", &json_buf) <= 0) {
        platform_fatal_error("Missing 'game_mod_config.json' game manifest from working directory \"%s\". Please examine README.md file for further information about how to create a game manifest.", working_dir_path.c_str());
        return;
    }

    json_t *mem = NULL;

    if (json_buf) {
        mem = (json_t*)SYSTEM_MALLOC(MAXIMUM_JSON_ALLOCATION_BLOCKS * sizeof(json_t));
        if (mem) {
            const json_t* root_json = json_create(json_buf, mem, MAXIMUM_JSON_ALLOCATION_BLOCKS);
            if (root_json) {
                const json_t* global = json_getProperty(root_json, "global_info");
                if (global && JSON_OBJ == json_getType(global)) {
                    MOD_GLOBAL_INFO* mod_global_info = &game_mod_config.global_info;

                    READ_JSON_UINT32(tr_engine_version, global, mod_global_info);
                    READ_JSON_BOOL(tr_level_editor, global, mod_global_info);
                    READ_JSON_BOOL(tr_times_exclusive, global, mod_global_info);
                    READ_JSON_BOOL(tr_use_adpcm_audio, global, mod_global_info);

                    // TRNG
                    READ_JSON_UINT8(trng_version_major, global, &mod_global_info->trng_engine_version);
                    READ_JSON_UINT8(trng_version_minor, global, &mod_global_info->trng_engine_version);
                    READ_JSON_UINT8(trng_version_maintainence, global, &mod_global_info->trng_engine_version);
                    READ_JSON_UINT8(trng_version_build, global, &mod_global_info->trng_engine_version);

                    READ_JSON_BOOL(trng_extended_flipmap_bitmask, global, mod_global_info);
                    READ_JSON_BOOL(trng_new_triggers, global, mod_global_info);
                    READ_JSON_BOOL(trng_anim_commands_enabled, global, mod_global_info);
                    READ_JSON_BOOL(trng_rollingball_extended_ocb, global, mod_global_info);
                    READ_JSON_BOOL(trng_statics_extended_ocb, global, mod_global_info);
                    READ_JSON_BOOL(trng_pushable_extended_ocb, global, mod_global_info);
                    READ_JSON_BOOL(trng_switch_extended_ocb, global, mod_global_info);
                    READ_JSON_BOOL(trng_hack_allow_meshes_with_exactly_256_vertices, global, mod_global_info);
                    READ_JSON_BOOL(trng_advanced_block_raising_behaviour, global, mod_global_info);
                    READ_JSON_BOOL(trng_pushables_have_gravity, global, mod_global_info);
                    READ_JSON_BOOL(trng_legacy_ng_trigger_behaviour, global, mod_global_info);

                    READ_JSON_BOOL(trep_using_extended_saves, global, mod_global_info);

                    // Tomo
                    READ_JSON_BOOL(tomo_enable_weather_flipeffect, global, mod_global_info);
                    READ_JSON_BOOL(tomo_swap_whitelight_for_teleporter, global, mod_global_info);

                    // Patcher Bug Fixes
                    READ_JSON_BOOL(grenades_damage_lara, global, mod_global_info);
                    READ_JSON_BOOL(spinning_debris, global, mod_global_info);
                    READ_JSON_BOOL(fix_rope_glitch, global, mod_global_info);
                    READ_JSON_BOOL(fix_lara_small_switch_rotation, global, mod_global_info);

                    // Misc
                    READ_JSON_BOOL(show_logo_in_title, global, mod_global_info);
                    READ_JSON_BOOL(show_lara_in_title, global, mod_global_info);
                    READ_JSON_UINT16(max_particles, global, mod_global_info);
                }

                const json_t* levels = json_getProperty(root_json, "levels");
                if (levels && JSON_ARRAY == json_getType(levels)) {
                    json_t const* level;
                    int level_index = 0;
                    for (level = json_getChild(levels); level != nullptr; level = json_getSibling(level)) {
                        if (JSON_OBJ == json_getType(level)) {
                            LoadGameModLevel(level, &game_mod_config.level_info[level_index]);
                        }
                        level_index++;
                    }
                }
            } else {
                Log(1, "LoadGameModConfigSecondPass: Not enough memory allocated for JSON buffer!");
            }
        } else {
            Log(1, "LoadGameModConfigSecondPass: Failed to allocate memory for JSON buffer!");
        }
    }

    CLEANUP_JSON_MEMORY
}

void T4PlusLevelReset() {
    furr_clear_oneshot_buffer();
    ClearWeatherFX();
    InitWeatherFX();
}

void T4PlusLevelSetup(int current_level) {
    S_Reset(); // Reset audio channels.

    MOD_LEVEL_AUDIO_INFO *audio_info = get_game_mod_level_audio_info(current_level);

    SetUsingNewAudioSystem(audio_info->new_audio_system);
    SetUsingOldTriggerMode(audio_info->old_cd_trigger_system);

    MOD_LEVEL_ENVIRONMENT_INFO *environment_info = get_game_mod_level_environment_info(current_level);

    using_multi_color_fog_bulbs = environment_info->enable_multi_color_fog_bulbs;

    NGLevelSetup();

    T4PResetMirrors();
    if (gfMirrorRoom >= 0) {
        T4PInsertMirror(gfMirrorRoom, gfMirrorZPlane, T4_MIR_PLANE_Z);
    }

    for (int i = 0; i < MAX_MIRRORS - 1; i++) {
        MOD_LEVEL_MIRROR_CUSTOMIZATION* mirror_customization = &get_game_mod_level_gfx_info(current_level)->mirror_customization[i];

        if (mirror_customization->room_number >= 0) {
            T4PInsertMirror(mirror_customization->room_number, mirror_customization->plane_position, mirror_customization->plane_direction);
        }
    }
}

// TODO: check if the equipment commands are valid on hub re-entry.
void T4PlusEnterLevel(int current_level, bool initial_entry) {
    if (initial_entry) {
        t4_override_fog_mode = get_game_mod_level_misc_info(current_level)->override_fog_mode;
        UpdateDistanceFogColor();

        t4p_rain_type = get_game_mod_level_misc_info(current_level)->rain_type;
        t4p_snow_type = get_game_mod_level_misc_info(current_level)->snow_type;

        MOD_EQUIPMENT_MODIFIER *equipment_modifiers = get_game_mod_level_stat_info(current_level)->equipment_modifiers;
        for (int i = 0; i < MAX_EQUIPMENT_MODIFIERS; i++) {
            if (equipment_modifiers[i].object_id != -1) {
                T4PlusSetInventoryCount(equipment_modifiers[i].object_id, equipment_modifiers[i].amount, true);
            } else {
                break;
            }
        }
    }
}

void T4PlusInitializeLara() {
    MOD_EQUIPMENT_MODIFIER *equipment_modifiers = get_game_mod_level_stat_info(0)->equipment_modifiers;
    for (int i = 0; i < MAX_EQUIPMENT_MODIFIERS; i++) {
        if (equipment_modifiers[i].object_id != -1) {
            T4PlusSetInventoryCount(equipment_modifiers[i].object_id, equipment_modifiers[i].amount, true);
        } else {
            break;
        }
    }
}

void T4PlusCleanup() {
    NGCleanup();

    furr_free_all_flipeffect_buffers();

    T4PlusFreeAllStrings();
}

extern void T4PlusInit() {
    global_string_table = (char **)SYSTEM_MALLOC(MAX_T4PLUS_STRINGS * sizeof(char*));

    NGInit();
}

bool is_source_trng_version_equal_or_greater_than_target(TRNG_ENGINE_VERSION source_version, TRNG_ENGINE_VERSION target_version) {
    if (source_version.trng_version_major < target_version.trng_version_major)
        return false;

    if (source_version.trng_version_minor < target_version.trng_version_minor)
        return false;

    if (source_version.trng_version_maintainence < target_version.trng_version_maintainence)
        return false;

    if (source_version.trng_version_build < target_version.trng_version_build)
        return false;

    return true;
}

bool is_mod_trng_version_equal_or_greater_than_target(
    unsigned char target_major_version,
    unsigned char target_minor_version,
    unsigned char target_maintainence_version,
    unsigned char target_build_version) {

    TRNG_ENGINE_VERSION target_version;
    target_version.trng_version_major = target_major_version;
    target_version.trng_version_minor = target_minor_version;
    target_version.trng_version_maintainence = target_maintainence_version;
    target_version.trng_version_build = target_build_version;

    return is_source_trng_version_equal_or_greater_than_target(get_game_mod_global_info()->trng_engine_version, target_version);
}