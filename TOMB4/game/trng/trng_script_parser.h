#pragma once

#include "trng_extra_state.h"

#include "../objects.h"

#define MAX_NG_STRINGS 16384
#define MAX_NG_LEVELS 64
#define MAX_NG_PLUGINS 256

#define NG_DEFINE_RECORD(CLASS_NAME) struct CLASS_NAME##_RECORD { \
	uint16_t record_id = 0; \
	CLASS_NAME record; \
};

#define NG_DEFINE_RECORD_DATA_ENTRY(CLASS_NAME, VAR_NAME) int32_t VAR_NAME##_count = 0; \
	CLASS_NAME##_RECORD* VAR_NAME##_table = NULL; \

struct NG_PLUGIN {
	bool is_enabled = false;
	int32_t ng_plugin_string_id = 0;
	int32_t t4plus_plugin = -1;
};

enum NG_PARAM_ENUMS {
	PARAM_MOVE_ITEM = 0x02,
	PARAM_ROTATE_ITEM = 0x03,
	PARAM_COLOR_ITEM = 0x04,
	PARAM_PRINT_TEXT = 0x05,
	PARAM_SET_CAMERA = 0x06,
	PARAM_BIG_NUMBERS = 0x07,
	PARAM_SCALE_ITEM = 0x08,
	PARAM_SHOW_SPRITE = 0x09,
	PARAM_TRIANGLE = 0x0a,
	PARAM_QUADRILATERAL = 0x0b,
	PARAM_CIRCLE = 0x0c,
	PARAM_LIGHTNING = 0x0d,
	PARAM_WTEXT = 0x0e,
	PARAM_RECT = 0x0f,
	PARAM_SWAP_ANIMATIONS = 0x10,
	PARAM_ACTOR_SPEECH = 0x11,
	PARAM_INPUT_BOX = 0x12,
};

enum NG_FMOV_ENUMS {
	FMOV_INFINITE_LOOP = 0x1,
	FMOV_HEAVY_AT_END = 0x2,
	FMOV_TRIGGERS_ALL = 0x4,
	FMOV_HEAVY_ALL = 0x8,
	FMOV_USE_LEADING_ACTOR_INDEX = 0x10,
	FMOV_USE_EXTRA_ACTOR_INDEX = 0x20,
	FMOV_IGNORE_FLOOR_COLLISION = 0x40,
	FMOV_WAIT_STAND_ON_FLOOR = 0x80,
	FMOV_FROG_JUMP_GRAVITY = 0x100,
	FMOV_LEAF_GRAVITY = 0x200,
	FMOV_EXPLOSION_GRAVITY = 0x300,
	FMOV_APPLE_GRAVITY = 0x400,
	FMOV_APOLLO_GRAVITY = 0x500,
	FMOV_MAN_GRAVITY = 0x600,
	FMOV_ANVIL_GRAVITY = 0x700,
	FMOV_EXPLOSION_SPEED = 0x1000,
	FMOV_MAGNET_SPEED = 0x2000,
	FMOV_CAR_SPEED = 0x3000
};

enum NG_DIR_ENUMS {
	DIR_NORTH = 0x0,
	DIR_EAST = 0x1,
	DIR_SOUTH = 0x2,
	DIR_WEST = 0x3,
	DIR_UP = 0x10,
	DIR_DOWN = 0x11,
	DIR_FORWARD = 0x12,
	DIR_TURNING_LEFT_90 = 0x13,
	DIR_TURNING_LEFT_45 = 0x14,
	DIR_LU_TURNING_180 = 0x15,
	DIR_TURNING_RIGHT_45 = 0x16,
	DIR_TURNING_RIGHT_90 = 0x17,
	DIR_RU_TURNING_180 = 0x18,
	DIR_LARA_FACING = 0x19,
	DIR_LEADING_ACTOR_FACING = 0x1a,
	DIR_DIRECTION_LARA_LEADING_ACTOR = 0x1b,
	DIR_HEAD_FOR_LARA = 0x1c,
	DIR_HEAD_FOR_LEADING_ACTOR = 0x1d,
	DIR_HEAD_FOR_EXTRA_ACTOR = 0x1e,

	DIR_INVERT_DIRECTION = 0x100,
};

enum NG_CUST_ENUMS {
	CUST_DISABLE_SCREAMING_HEAD = 0x01,
	CUST_SET_SECRET_NUMBER = 0x02,
	CUST_SET_CREDITS_LEVEL = 0x03,
	CUST_DISABLE_FORCING_ANIM_96 = 0x04,
	CUST_ROLLINGBALL_PUSHING = 0x05,
	CUST_NEW_SOUND_ENGINE = 0x06,
	CUST_SPEED_MOVING = 0x07,
	CUST_SHATTER_RANGE = 0x08,
	CUST_WEAPON = 0x09,
	CUST_AMMO = 0x0a,
	CUST_SHOW_AMMO_COUNTER = 0x0b,
	CUST_SET_INV_ITEM = 0x0c,
	CUST_SET_JEEP_KEY_SLOT = 0x0d,
	CUST_STATIC_TRANSPARENCY = 0x0e,
	CUST_SET_STATIC_DAMAGE = 0x0f,
	CUST_LOOK_TRANSPARENT = 0x10,
	CUST_HAIR_TYPE = 0x11,
	CUST_KEEP_DEAD_ENEMIES = 0x12,
	CUST_SET_OLD_CD_TRIGGER = 0x13,
	CUST_ESCAPE_FLY_CAMERA = 0x14,
	CUST_PAUSE_FLY_CAMERA = 0x15,
	CUST_TEXT_ON_FLY_SCREEN = 0x16,
	CUST_CD_SINGLE_PLAYBACK = 0x17,
	CUST_ADD_DEATH_ANIMATION = 0x18,
	CUST_BAR = 0x19,
	CUST_NO_TIME_IN_SAVELIST = 0x1a,
	CUST_PARALLEL_BARS = 0x1b,
	CUST_CAMERA = 0x1c,
	CUST_DISABLE_MISSING_SOUNDS = 0x1d,
	CUST_INNER_SCREENSHOT = 0x1e,
	CUST_FMV_CUTSCENE = 0x1f,
	CUST_FIX_WATER_FOG_BUG = 0x20,
	CUST_SAVE_LOCUST = 0x21,
	CUST_LIGHT_OBJECT = 0x22,
	CUST_HARPOON = 0x23,
	CUST_SCREENSHOT_CAPTURE = 0x24,
	CUST_RAIN = 0x25,
	CUST_TR5_UNDERWATER_COLLISIONS = 0x26,
	CUST_DARTS = 0x27,
	CUST_FLARE = 0x28,
	CUST_SET_TEXT_COLOR = 0x29,
	CUST_SET_STILL_COLLISION = 0x2a,
	CUST_WATERFALL_SPEED = 0x2b,
	CUST_ROLLING_BOAT = 0x2c,
	CUST_SFX = 0x2d,
	CUST_TITLE_FMV = 0x2e,
	CUST_KEEP_LARA_HP = 0x2f,
	CUST_BINOCULARS = 0x30,
	CUST_BACKGROUND = 0x31,
	CUST_DISABLE_PUSH_AWAY_ANIMATION = 0x32,
	CUST_SLOT_FLAGS = 0x34,
	CUST_FIX_BUGS = 0x35,
	CUST_SHATTER_SPECIFIC = 0x36
};

enum NG_BUGF_ENUMS {
	BUGF_TRANSPARENT_WHITE_ON_FOG = 0x01,
	BUGF_DART_NO_POISON_LARA = 0x02,
	BUGF_LAND_WATER_SFX_ENEMIES = 0x04
};

enum NG_TGROUP_FLAGS {
	TGROUP_USE_FOUND_ITEM_INDEX = 0x01,
	TGROUP_USE_TRIGGER_ITEM_INDEX = 0x02,
	TGROUP_COMMAND = 0x03,
	TGROUP_USE_OWNER_ANIM_ITEM_INDEX = 0x04,
	TGROUP_SINGLE_SHOT_RESUMED = 0x08, // Post 1.2.2.7
	TGROUP_AND = 0x08, // Pre 1.2.2.7
	TGROUP_OR = 0x10,
	TGROUP_NOT = 0x20,
	TGROUP_ELSE = 0x40,
	TGROUP_USE_EXECUTOR_ITEM_INDEX = 0x100,
	TGROUP_SINGLE_SHOT = 0x400,
	TGROUP_USE_ITEM_USED_BY_LARA_INDEX = 0x800,

	TGROUP_MOVEABLE = 0x1000,
	TGROUP_FLIPEFFECT = 0x2000,
	TGROUP_ACTION = 0x4000,
	TGROUP_CONDITION_TRIGGER = 0x8000,

};

enum NG_AMMO_FLAGS {
	AMMO_PUSH_TARGET = 0x02,
	AMMO_AMMO_PUSH_LARA = 0x04,
	AMMO_SET_GRENADE_TIMER = 0x08,
	AMMO_ADD_GUN_SHELL = 0x10,
	AMMO_ADD_SHOTGUN_SHELL = 0x20,
	AMMO_REMOVE_SHOTGUN_SHELL = 0x40
};

struct NG_MULTI_ENV_TRIPLET {
	uint16_t env_condition;
	int16_t distance_for_env;
	uint16_t extra;
};

struct NG_GLOBAL_TRIGGER {
	uint16_t flags = 0x0;
	uint16_t type = 0x00;
	int32_t parameter = 0x00;
	uint16_t condition_trigger_group = 0x00;
	uint16_t perform_trigger_group = 0x00;
	uint16_t on_false_trigger_group = 0x00;
};

struct NG_TRIGGER_GROUP_DATA {
	uint16_t plugin_id = 0;
	uint16_t flags = 0x00;
	uint16_t object = 0x00;
	uint16_t timer = 0x00;
};

#define NG_TRIGGER_GROUP_DATA_SIZE 0xff
struct NG_TRIGGER_GROUP {
	int32_t data_size = 0;
	NG_TRIGGER_GROUP_DATA data[NG_TRIGGER_GROUP_DATA_SIZE] = {};

	bool oneshot_triggered = false;
	bool was_executed = false;
};

struct NG_ORGANIZER_APPOINTMENT {
	int32_t time = 0;
	int16_t trigger_group = 0;
};

#define NG_ORGANIZER_MAX_APPOINTMENTS 4096
struct NG_ORGANIZER {
	int16_t flags = 0;
	int16_t parameters = 0; // Unused
	uint32_t appointment_count = 0;
	NG_ORGANIZER_APPOINTMENT appointments[NG_ORGANIZER_MAX_APPOINTMENTS] = {};
};

#define NG_ITEM_GROUP_MAX_LIST 4096
struct NG_ITEM_GROUP {
	int16_t item_count = 0;
	int16_t item_list[NG_ITEM_GROUP_MAX_LIST] = {};
};

#define NG_ANIMATION_CONDTION_MAX_SIZE 4096
struct NG_ANIMATION {
	uint16_t animation_index = 0;
	uint16_t key_1 = 0;
	uint16_t key_2 = 0;
	uint16_t fan_flags = 0;
	NG_MULTI_ENV_TRIPLET environment = {};
	uint16_t state_or_animation_condition_count = 0;
	int16_t state_or_animation_condition_array[NG_ANIMATION_CONDTION_MAX_SIZE] = {};
};

#define NG_MULTI_ENV_CONDITION_MAX_TRIPLETS 128
struct NG_MULTI_ENV_CONDITION {
	int32_t env_condition_triplet_count = 0;
	NG_MULTI_ENV_TRIPLET env_condition_triplet_array[NG_MULTI_ENV_CONDITION_MAX_TRIPLETS] = {};
};

struct NG_TEST_POSITION {
	uint16_t flags = 0;
	uint16_t moveable_slot = 0;
	int16_t x_distance_min = 0;
	int16_t x_distance_max = 0;
	int16_t y_distance_min = 0;
	int16_t y_distance_max = 0;
	int16_t z_distance_min = 0;
	int16_t z_distance_max = 0;
	int16_t h_orient_diff_min = 0;
	int16_t h_orient_diff_max = 0;
	int16_t v_orient_diff_min = 0;
	int16_t v_orient_diff_max = 0;
	int16_t r_orient_diff_min = 0;
	int16_t r_orient_diff_max = 0;
};

// Params
struct NG_MOVE_ITEM {
	uint16_t flags = 0;
	uint16_t index_item = 0;
	uint16_t direction = 0;
	uint16_t distance = 0;
	uint16_t speed = 0;
	int16_t moving_sound = 0;
	int16_t final_sound = 0;
	int16_t extra = 0;
};

struct NG_ROTATE_ITEM {
	uint16_t flags = 0;
	uint16_t index_item = 0;

	uint16_t dir_h_rotation = 0;
	uint16_t h_rotation_angle = 0;
	uint16_t speed_h_rotation = 0;

	uint16_t dir_v_rotation = 0;
	uint16_t v_rotation_angle = 0;
	uint16_t speed_v_rotation = 0;

	int16_t moving_sound = 0;
	int16_t final_sound = 0;
};

struct NG_BIG_NUMBER {
	uint16_t big_number = 0;
};

NG_DEFINE_RECORD(NG_GLOBAL_TRIGGER);
NG_DEFINE_RECORD(NG_TRIGGER_GROUP);
NG_DEFINE_RECORD(NG_ORGANIZER);
NG_DEFINE_RECORD(NG_ITEM_GROUP);
NG_DEFINE_RECORD(NG_ANIMATION);
NG_DEFINE_RECORD(NG_MULTI_ENV_CONDITION);
NG_DEFINE_RECORD(NG_TEST_POSITION);

// Params
NG_DEFINE_RECORD(NG_MOVE_ITEM);
NG_DEFINE_RECORD(NG_ROTATE_ITEM);
NG_DEFINE_RECORD(NG_BIG_NUMBER);

struct NG_LEVEL_RECORD_DATA {
	NG_DEFINE_RECORD_DATA_ENTRY(NG_GLOBAL_TRIGGER, global_trigger);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_TRIGGER_GROUP, trigger_group);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_ORGANIZER, organizer);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_ITEM_GROUP, item_group);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_ANIMATION, animation);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_MULTI_ENV_CONDITION, multi_env_condition);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_TEST_POSITION, test_position);

	// Params
	NG_DEFINE_RECORD_DATA_ENTRY(NG_MOVE_ITEM, move_item);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_ROTATE_ITEM, rotate_item);
	NG_DEFINE_RECORD_DATA_ENTRY(NG_BIG_NUMBER, big_number);
};

struct NG_LEVEL_RECORD_TABLES {
	uint32_t level_global_trigger_count = 0;
	uint32_t level_trigger_group_count = 0;
	uint32_t level_organizer_count = 0;
	uint32_t level_item_group_count = 0;
	uint32_t level_animation_count = 0;
	uint32_t level_multi_env_condition_count = 0;
	uint32_t level_test_position_count = 0;

	// Params
	uint32_t level_move_item_count = 0;
	uint32_t level_rotate_item_count = 0;
	uint32_t level_big_number_count = 0;

	NG_GLOBAL_TRIGGER_RECORD* level_global_triggers_table = NULL;
	NG_TRIGGER_GROUP_RECORD* level_trigger_group_table = NULL;
	NG_ORGANIZER_RECORD* level_organizer_table = NULL;
	NG_ITEM_GROUP_RECORD* level_item_group_table = NULL;
	NG_ANIMATION_RECORD* level_animation_table = NULL;
	NG_MULTI_ENV_CONDITION_RECORD* level_multi_env_condition_table = NULL;
	NG_TEST_POSITION_RECORD* level_test_position_table = NULL;

	// Params
	NG_MOVE_ITEM_RECORD* level_move_item_table = NULL;
	NG_ROTATE_ITEM_RECORD* level_rotate_item_table = NULL;
	NG_BIG_NUMBER_RECORD* level_big_number_table = NULL;
};

struct NG_LEVEL {
	NG_LEVEL_RECORD_DATA *records = NULL;

	int32_t first_shatter_id = SHATTER0;
	int32_t last_shatter_id = SHATTER9;
};

extern NG_LEVEL ng_levels[MAX_NG_LEVELS];

enum NG_ADD_EFFECT_TYPE {
	NG_ADD_NOTHING,
	NG_ADD_FLAME,
	NG_ADD_SMOKE,
	NG_ADD_BLOOD,
	NG_ADD_MIST,
	NG_ADD_LIGHT_FLAT,
	NG_ADD_LIGHT_BLINK,
	NG_ADD_LIGHT_SPOT,
	NG_ADD_LIGHT_GLOVE,
};

extern NG_GLOBAL_TRIGGER current_global_triggers[MAX_NG_GLOBAL_TRIGGERS];
extern NG_TRIGGER_GROUP current_trigger_groups[MAX_NG_TRIGGER_GROUPS];
extern NG_ORGANIZER current_organizers[MAX_NG_ORGANIZERS];
extern NG_ITEM_GROUP current_item_groups[MAX_NG_ITEM_GROUPS];
extern NG_ANIMATION current_animations[MAX_NG_ANIMATIONS];
extern NG_MULTI_ENV_CONDITION current_multi_env_conditions[MAX_NG_MULTI_ENV_CONDITIONS];
extern NG_TEST_POSITION current_test_positions[MAX_NG_TEST_POSITIONS];

#define MAX_NG_MOVE_ITEMS 9999
#define MAX_NG_ROTATE_ITEMS 9999
#define MAX_NG_BIG_NUMBERS 9999

// Params
extern NG_MOVE_ITEM current_move_items[MAX_NG_MOVE_ITEMS];
extern NG_ROTATE_ITEM current_rotate_items[MAX_NG_ROTATE_ITEMS];
extern NG_BIG_NUMBER current_big_numbers[MAX_NG_BIG_NUMBERS];

extern char *NGGetString(int16_t string_id);
extern char *NGGetPluginString(int16_t plugin_id);
extern int32_t NGGetT4PluginID(int16_t plugin_id);

extern void NGScriptInit(char* gfScriptFile, size_t offset, size_t len);
extern void NGScriptCleanup();
extern void NGLoadTablesForLevel(uint32_t level);
extern void NGReadNGGameflowInfo(char* gfScriptFile, size_t offset, size_t len);
extern void NGReadNGExtraStrings(char* gfLanguageFile, size_t offset, size_t len);