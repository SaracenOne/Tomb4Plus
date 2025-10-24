#pragma once

#include "../game/objects.h"
#include "../game/text.h"
#include "../game/sound.h"

#include "tomb4plus/t4plus_plugin.h"
#include "tomb4plus/t4plus_weather.h"
#include "tomb4plus/t4plus_mirror.h"
#include "tomb4plus/t4plus_environment.h"
#include "../game/lara_states.h"

#define MOD_LEVEL_COUNT 64

// 1 - Change object customization ID format.
#define ENGINE_MANIFEST_VERSION 1

#define DEFAULT_FOG_START_BLOCKS 12
#define DEFAULT_FOG_END_BLOCKS 20
#define DEFAULT_CLIP_RANGE_BLOCKS 20

#define DEFAULT_FOG_START_VALUE BLOCK_SIZE * DEFAULT_FOG_START_BLOCKS
#define DEFAULT_FOG_END_VALUE BLOCK_SIZE * DEFAULT_FOG_END_BLOCKS
#define DEFAULT_FAR_VIEW_VALUE BLOCK_SIZE * DEFAULT_FOG_END_BLOCKS

// This may need increasing...
#define MAXIMUM_JSON_ALLOCATION_BLOCKS 32768

#define MAX_EQUIPMENT_MODIFIERS 32
#define MAX_VAPOR_OCB_CUSTOMIZATIONS 16

#define MAX_T4PLUS_STRINGS 4096
extern int32_t global_string_table_size;
extern char **global_string_table;

struct MOD_GLOBAL_PLUGIN {
	char *plugin_name = nullptr;
	char *plugin_type = nullptr;
	char *plugin_source = nullptr;
};

struct MOD_LEVEL_ENVIRONMENT_INFO {
	int32_t fog_start_range = DEFAULT_FOG_START_VALUE;
	int32_t fog_end_range = DEFAULT_FOG_END_VALUE;
	int32_t far_view = DEFAULT_FAR_VIEW_VALUE;
	bool force_train_fog = false;
	bool disable_distance_limit = false;
	bool enable_multi_color_fog_bulbs = false;

	uint32_t room_swamp_flag = 0;
	uint32_t room_cold_flag = ROOM_COLD;
	uint32_t room_damage_flag = 0;
};

struct MOD_LEVEL_FONT_INFO {
	int32_t custom_glyph_scale_width = DEFAULT_GLYPH_SCALE_WIDTH;
	int32_t custom_glyph_scale_height = DEFAULT_GLYPH_SCALE_HEIGHT;
	float custom_vertical_spacing = 0.075F;

	int32_t main_font_main_color = 0xff808080;
	int32_t main_font_fade_color = 0xff808080;

	int32_t options_title_font_main_color = 0xffc08040;
	int32_t options_title_font_fade_color = 0xff401000;

	int32_t inventory_title_font_main_color = 0xffe0c000;
	int32_t inventory_title_font_fade_color = 0xff402000;

	int32_t inventory_item_font_main_color = 0xff808080;
	int32_t inventory_item_font_fade_color = 0xff101010;

	CHARDEF custom_font_table[CHAR_TABLE_COUNT] = {};
};

struct MOD_LEVEL_CAMERA_INFO {
	int32_t chase_camera_distance = BLOCK_SIZE + HALF_BLOCK_SIZE;
	int32_t chase_camera_vertical_orientation = -DEGREES_TO_ROTATION(10);
	int32_t chase_camera_horizontal_orientation = 0;

	int32_t combat_camera_distance = BLOCK_SIZE + HALF_BLOCK_SIZE;
	int32_t combat_camera_vertical_orientation = -DEGREES_TO_ROTATION(15);

	int32_t look_camera_distance = -BLOCK_SIZE;
	int32_t look_camera_height = (QUARTER_CLICK_SIZE / 4);

	int32_t camera_speed = 10;
	int32_t add_on_battle_camera_top = CLICK_SIZE;
	bool disable_battle_camera = false;
};

enum CREATURE_HIT_TYPE {
	CREATURE_HIT_SMOKE,
	CREATURE_HIT_BLOOD,
	CREATURE_HIT_NO_ACTION,
	CREATURE_HIT_RICHOCHET,
	CREATURE_HIT_CLEAR_DAMAGE
};

enum SLOT_EXPLODE_TYPE {
	NO_EXPLOSION,
	EXPLODE_IMMEDIATELY,
	EXPLODE_AFTER_DEATH_ANIMATION
};

enum SLOT_HIT_TYPE {
	HIT_NONE,
	HIT_BLOOD,
	HIT_FRAGMENTS,
	HIT_SMOKE
};

#define MAX_BONE_CUSTOMIZATIONS 4

struct MOD_LEVEL_OBJECT_BONE_CUSTOMIZATION {
	uint8_t bone_id = -1;
	uint8_t bone_data = 0;
};

struct MOD_LEVEL_OBJECT_CUSTOMIZATION {
	const char *initialise_routine;
	const char *collision_routine;
	const char *control_routine;
	const char *draw_routine;
	const char *draw_routine_extra;
	const char *ceiling_routine;
	const char *floor_routine;

	uint16_t is_amphibious : 1;

	uint16_t is_agent : 1;
	uint16_t is_undead : 1;

	uint16_t save_position : 1;
	uint16_t save_hitpoints : 1;
	uint16_t save_flags : 1;
	uint16_t save_animation : 1;
	uint16_t save_mesh : 1;

	uint16_t override_hit_points : 1;
	uint16_t override_hit_type : 1;

	uint16_t explode_immediately : 1;
	uint16_t explode_after_death_animation : 1;
	uint16_t explosive_death_only : 1;
	uint16_t hit_type : 2;

	int32_t pivot_length = 0;
	int32_t radius = 10;
	int32_t shadow_size = 0;

	int32_t explodable_meshbits = 0;

	int16_t hit_points = 0;
	int16_t damage_1 = 0;
	int16_t damage_2 = 0;
	int16_t damage_3 = 0;

	uint16_t linked_object_1 = 0;
	uint16_t linked_object_2 = 0;
	uint16_t linked_object_3 = 0;

	int32_t object_mip = 0;

	bool pathfinding_can_jump;
	bool pathfinding_can_monkey;
	int32_t pathfinding_max_step = CLICK_SIZE;
	int32_t pathfinding_max_drop = -HALF_BLOCK_SIZE;
	int32_t pathfinding_zone = BASIC_ZONE;

	MOD_LEVEL_OBJECT_BONE_CUSTOMIZATION bone_customization[MAX_BONE_CUSTOMIZATIONS];
};

struct MOD_LEVEL_OBJECTS_INFO {
	// Metadata slots
	int16_t lara_slot = LARA;

	int16_t lara_pistols_animations_slot = PISTOLS_ANIM;
	int16_t lara_uzi_animations_slot = UZI_ANIM;
	int16_t lara_shotgun_animations_slot = SHOTGUN_ANIM;
	int16_t lara_crossbow_animations_slot = CROSSBOW_ANIM;
	int16_t lara_grenade_gun_animations_slot = GRENADE_GUN_ANIM;
	int16_t lara_revolver_animations_slot = SIXSHOOTER_ANIM;
	int16_t lara_flare_animations_slot = FLARE_ANIM;

	int16_t lara_skin_slot = LARA_SKIN;
	int16_t lara_skin_joints_slot = LARA_SKIN_JOINTS;
	int16_t lara_scream_slot = LARA_SCREAM;
	int16_t lara_crossbow_laser = LARA_CROSSBOW_LASER;
	int16_t lara_revolver_laser = LARA_REVOLVER_LASER;

	int16_t lara_holsters = LARA_HOLSTERS;
	int16_t lara_holsters_pistols = LARA_HOLSTERS_PISTOLS;
	int16_t lara_holsters_uzis = LARA_HOLSTERS_UZIS;
	int16_t lara_holsters_revolver = LARA_HOLSTERS_SIXSHOOTER;

	int16_t lara_hair_slot = HAIR;

	int16_t motorbike_slot = MOTORBIKE;
	int16_t jeep_slot = JEEP;
	int16_t motorbike_extra_slot = VEHICLE_EXTRA;
	int16_t jeep_extra_slot = VEHICLE_EXTRA;

	int16_t meshswap_1_slot = MESHSWAP1;
	int16_t meshswap_2_slot = MESHSWAP2;
	int16_t meshswap_3_slot = MESHSWAP3;
	int16_t bubbles_slot = BUBBLES;
	int16_t default_sprites_slot = DEFAULT_SPRITES;

	int16_t horizon_slot = HORIZON;
	int16_t sky_graphics_slot = SKY_GRAPHICS;
	int16_t binocular_graphics_slot = BINOCULAR_GRAPHICS;
	int16_t target_graphics_slot = TARGET_GRAPHICS;

	int16_t lightning_conductor_target_slot = ANIMATING8;

	int16_t nitrous_oxide_feeder_slot = PUZZLE_ITEM1;
	int16_t jeep_key_slot = PUZZLE_ITEM1;

	int16_t rubber_boat_slot = RUBBER_BOAT;
	int16_t rubber_boat_extra_slot = RUBBER_BOAT_LARA;
	int16_t motor_boat_slot = MOTOR_BOAT;
	int16_t motor_boat_extra_slot = MOTOR_BOAT_LARA;

	int16_t laser_head_slot = LASER_HEAD;
	int16_t laser_head_base_slot = LASER_HEAD_BASE;
	int16_t laser_head_tentacle_slot = LASER_HEAD_TENTACLE;

	int16_t hydra_slot = HYDRA;
	int16_t hydra_missile_slot = HYDRA_MISSLE;

	int16_t lara_double_slot = LARA_DOUBLE;
	int16_t enemy_jeep_slot = ENEMY_JEEP;

	int16_t chef_slot = -1;

	int16_t darts_interval = 24;
	int16_t darts_speed = 256;
	int16_t falling_block_timer = 60;
	int16_t falling_block_tremble = 1023;

	uint32_t raising_block_1_height = BLOCK_SIZE;
	uint32_t raising_block_2_height = BLOCK_SIZE * 2;

	int16_t whitelight_teleport_ocb = -1;

	int16_t slot_override[NUMBER_OBJECTS];
	MOD_LEVEL_OBJECT_CUSTOMIZATION object_customization[NUMBER_OBJECTS];
};

struct MOD_LEVEL_STATIC_INFO {
	bool lara_guns_can_shatter = false;
	bool large_objects_can_shatter = false;
	bool creatures_can_shatter = false;
	bool record_shatter_state_in_savegames = false;
	bool hard_collision = false;
	int16_t shatter_sound_id = 0;
};

struct MOD_LEVEL_STATICS_INFO {
	MOD_LEVEL_STATIC_INFO static_info[NUMBER_STATIC_OBJECTS];
};

struct MOD_LEVEL_CREATURE_INFO {
	bool fade_dead_enemies = true;
	bool small_scorpion_is_poisonous = true;
	int32_t small_scorpion_poison_strength = 512;

	bool use_voncroy_racing_behaviour = false;

	bool remove_knights_templar_sparks = false;

	bool remove_ahmet_death_flames = false;
	bool remove_ahmet_death_loop = false;
	bool disable_ahmet_heavy_trigger = false;

	bool remove_mummy_stun_animations = false;

	bool disable_sentry_flame_attack = false;
};

struct MOD_LEVEL_MIRROR_CUSTOMIZATION {
	int16_t room_number = -1;
	int32_t plane_position = 0;
	T4PlusMirrorDirection plane_direction = T4PlusMirrorDirection::T4_MIR_PLANE_X;
};

struct MOD_LEVEL_VAPOR_CUSTOMIZATION {

};

struct MOD_LEVEL_GFX_INFO {
	T4PColdBreath cold_breath = COLD_BREATH_DISABLED;

	int16_t default_envmap_sprite_index = 11;
	int16_t pickup_envmap_sprite_index = 11;

	MOD_LEVEL_MIRROR_CUSTOMIZATION mirror_customization[MAX_MIRRORS-1];

	MOD_LEVEL_VAPOR_CUSTOMIZATION white_smoke_emitter_customization_for_OCB[MAX_VAPOR_OCB_CUSTOMIZATIONS];
	MOD_LEVEL_VAPOR_CUSTOMIZATION black_smoke_emitter_customization_for_OCB[MAX_VAPOR_OCB_CUSTOMIZATIONS];
	MOD_LEVEL_VAPOR_CUSTOMIZATION vapor_emitter_customization_for_OCB[MAX_VAPOR_OCB_CUSTOMIZATIONS];
};

struct MOD_LEVEL_AUDIO_INFO {
	bool new_audio_system = false;
	bool old_cd_trigger_system = true;

	int32_t sample_rate = 22050;

	int16_t first_looped_audio_track = 105;
	int16_t last_looped_audio_track = 127;

	int16_t boat_track = 12;
	int16_t inside_jeep_track = 98;
	int16_t outside_jeep_track = 110;
	int16_t secret_track = 5;

	// SFX
	int16_t lara_hit_sfx_id = SFX_UNDERWATER_DOOR;
	int16_t no_ammo_sfx_id = SFX_SARLID_PALACES;
	int16_t pour_sfx_id = SFX_POUR;
	int16_t loop_for_small_fires_sfx_id = SFX_LOOP_FOR_SMALL_FIRES;
	int16_t flame_emitter_sfx_id = SFX_FLAME_EMITTER;
	int16_t underwater_sfx_id = SFX_UNDERWATER;
	int16_t explosion_1_sfx_id = SFX_EXPLOSION1;
	int16_t explosion_2_sfx_id = SFX_EXPLOSION2;
	int16_t lara_bubbles_sfx_id = SFX_LARA_BUBBLES;

	int16_t lara_no_sfx_id = SFX_LARA_NO;
	int16_t lara_richochet_sfx_id = SFX_LARA_RICOCHET;
	int16_t lara_pistols_overlay_sfx_id = SFX_EXPLOSION1;

	int16_t lara_pistol_shell_sfx_id = SFX_LARA_SHOTGUN_SHELL;
	int16_t lara_shotgun_shell_sfx_id = SFX_LARA_SHOTGUN_SHELL;
	int16_t lara_rope_creak_sfx_id = SFX_LARA_ROPE_CREAK;
	int16_t sample_test_sfx_id = SFX_LARA_BREATH;
	int16_t menu_select_sfx_id = SFX_MENU_SELECT;
	int16_t menu_choose_sfx_id = SFX_MENU_CHOOSE;
	int16_t menu_combine_sfx_id = SFX_MENU_COMBINE;
	int16_t menu_large_medipack_sfx_id = SFX_MENU_MEDI;
	int16_t menu_small_medipack_sfx_id = SFX_MENU_MEDI;

	int16_t bike_idle_sfx_id = SFX_BIKE_IDLE;
	int16_t bike_moving_sfx_id = SFX_BIKE_MOVING;
	int16_t jeep_idle_sfx_id = SFX_JEEP_IDLE;
	int16_t jeep_moving_sfx_id = SFX_JEEP_MOVE;

	int16_t motorboat_idle_sfx_id = 308;
	int16_t motorboat_moving_sfx_id = 307;
	int16_t rubber_boat_idle_sfx_id = 308;
	int16_t rubber_boat_moving_sfx_id = 307;

	int16_t god_head_charge_sfx_id = SFX_GENERIC_NRG_CHARGE;
	int16_t god_head_laser_loop_sfx_id = SFX_BAZOOKA_FIRE;
	int16_t god_head_blast_sfx_id = SFX_DEMIGOD_FALCON_PLAS;
	int16_t god_head_smash_sfx_id = SFX_EXPLOSION2;

	int16_t hydra_smash_sfx_id = SFX_THUNDER_CRACK;
};

struct MOD_LEVEL_RECT_COLOR_INFO {
	uint32_t upper_left_color;
	uint32_t upper_right_color;
	uint32_t lower_right_color;
	uint32_t lower_left_color;
};

struct MOD_LEVEL_BAR_INFO {
	MOD_LEVEL_RECT_COLOR_INFO lower_rect;
	MOD_LEVEL_RECT_COLOR_INFO upper_rect;
	MOD_LEVEL_RECT_COLOR_INFO border_rect;
	MOD_LEVEL_RECT_COLOR_INFO background_rect;
	uint32_t border_size;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct MOD_LEVEL_BARS_INFO {
	MOD_LEVEL_BAR_INFO health_bar;
	MOD_LEVEL_BAR_INFO poison_bar;
	MOD_LEVEL_BAR_INFO air_bar;
	MOD_LEVEL_BAR_INFO sprint_bar;
	MOD_LEVEL_BAR_INFO loading_bar;
	MOD_LEVEL_BAR_INFO enemy_bar;
};

enum LARA_HAIR_TYPE {
	LARA_HAIR_TYPE_DEFAULT = 0,
	LARA_HAIR_TYPE_BRAID,
	LARA_HAIR_TYPE_PIGTAILS,
	LARA_HAIR_TYPE_NONE,
};

struct MOD_LEVEL_LARA_INFO {
	LARA_HAIR_TYPE hair_type = LARA_HAIR_TYPE_DEFAULT;
	int32_t hair_gravity = 10;

	int32_t braid_x = -4;
	int32_t braid_y = -4;
	int32_t braid_z = -48;

	int32_t pigtail_left_x = -52;
	int32_t pigtail_left_y = -48;
	int32_t pigtail_left_z = -50;

	int32_t pigtail_right_x = 44;
	int32_t pigtail_right_y = -48;
	int32_t pigtail_right_z = -50;

	int32_t crawlspace_jump_animation = 421;
	int32_t crawlspace_jump_pit_deepness_threshold = (HALF_BLOCK_SIZE + CLICK_SIZE);

	bool use_tr5_swimming_collision = false;
	bool disable_hardcoded_breath_sound = false;

	int32_t ledge_to_jump_state = -1;
	int32_t ledge_to_down_state = -1;

	bool use_look_transparency = true;
};

struct TRNG_ENGINE_VERSION {
	uint8_t trng_version_major = 1;
	uint8_t trng_version_minor = 3;
	uint8_t trng_version_maintainence = 0;
	uint8_t trng_version_build = 7;
};


struct MOD_GLOBAL_INFO {
	int32_t manifest_compatibility_version = -1;
	char *game_name = nullptr;
	char *authors = nullptr;
	char *game_user_dir_name = nullptr;

	uint32_t tr_engine_version = 4;
	bool tr_level_editor = true;
	bool tr_times_exclusive = false;
	bool tr_use_adpcm_audio = false;

	// Default to latest known version.
	TRNG_ENGINE_VERSION trng_engine_version;

	// TRNG Stuff
	bool trng_savegames = false;
	bool trng_new_triggers = false; // TRNG (special TRNG flipeffects. Disable if conflicting with FURR)
	bool trng_anim_commands_enabled = false;
	bool trng_timerfields_enabled = false;
	bool trng_rollingball_extended_ocb = false; // TRNG (moveable and regular trigger activation)
	bool trng_statics_extended_ocb = false; // TRNG (touch inflicting poison)
	// Climable pushables (both implementations have different quirks. Will attempt to more accurately recreate them later)
	bool trng_pushable_extended_ocb = false; // TRNG (climable pushables)
	bool trng_switch_extended_ocb = false; // TRNG (custom switch animations)
	bool trng_hack_allow_meshes_with_exactly_256_vertices = false; // TRNG seems to have a special hack which allows meshes of 256 verticies
	bool trng_advanced_block_raising_behaviour = true;
	bool trng_pushables_have_gravity = false;
	bool trng_legacy_ng_trigger_behaviour = false;

	bool trep_using_extended_saves = false;

	// Tomo stuff
	bool tomo_enable_weather_flipeffect = false;
	bool tomo_swap_whitelight_for_teleporter = false;

	// Patcher Bug Fixes
	bool grenades_damage_lara = true;
	bool spinning_debris = true;
	bool fix_rope_glitch = true;
	bool fix_lara_small_switch_rotation = true;
	bool fix_lara_hands_free_flipeffect_bugs = true;

	bool show_logo_in_title = true;
	bool show_lara_in_title = false;
	uint16_t max_particles = 256;
};

struct MOD_EQUIPMENT_MODIFIER {
	int32_t object_id = -1;
	int32_t amount = -1;
};

struct MOD_LEVEL_STAT_INFO {
	uint32_t secret_count = 70;

	MOD_EQUIPMENT_MODIFIER equipment_modifiers[MAX_EQUIPMENT_MODIFIERS];
};

struct MOD_LEVEL_LOCALIZED_STRING_OVERWRITE {
	const char *id = "";
	const char *string = "";
};

struct MOD_LEVEL_LOCALIZED_STRINGS_OVERWRITE_INFO {
	const char *locale_name = "";
	MOD_LEVEL_LOCALIZED_STRING_OVERWRITE *string_remap_table;
};

struct MOD_LEVEL_STRINGS_OVERWRITE_INFO {
	int32_t localized_strings_info_count = 0;
	MOD_LEVEL_LOCALIZED_STRINGS_OVERWRITE_INFO *localized_strings_info;
};

struct MOD_LEVEL_FLARE_INFO {
	uint8_t light_color_r = 128;
	uint8_t light_color_g = 192;
	uint8_t light_color_b = 0;
	int32_t flare_lifetime_in_ticks = 30 * 30;
	int32_t light_intensity = 16;
	bool has_sparks = false;
	bool has_fire = false; // Unimplemented
	bool sparks_include_smoke = false;
	bool has_glow = false; // Unimplemented
	bool flat_light = false;
};

struct MOD_LEVEL_AMMO_INFO {
	bool disable_explosion_sfx = false;

	int16_t damage = 0;
	int16_t poison_damage = 0;
	int16_t explosion_damage = 0;

	int16_t speed = 0;
	int16_t gravity = 0;

	int16_t shots = 1;
	int16_t fire_rate = 0;
	int16_t dispertion = 0;
	int16_t flash_duration = 0;

	int16_t weapon_pickup_amount = 0;
	int16_t ammo_pickup_amount = 0;

	int16_t grenade_timer = 0;

	bool add_pistol_shell = false;
	bool add_shotgun_shell = false;

	bool creates_explosion = false;
	bool creates_super_explosion = false;

	int16_t push_lara_amount = 0;
	int16_t push_target_amount = 0;

	// TRNG-specific
	int16_t trng_trigger_id_when_enemy_hit = -1;
	int16_t trng_trigger_id_at_end = -1;
	int16_t trng_effect = -1;
};

struct MOD_LEVEL_WEAPON_INFO {
	MOD_LEVEL_AMMO_INFO pistol_ammo_info;
	MOD_LEVEL_AMMO_INFO uzi_ammo_info;

	MOD_LEVEL_AMMO_INFO shotgun_1_ammo_info;
	MOD_LEVEL_AMMO_INFO shotgun_2_ammo_info;

	MOD_LEVEL_AMMO_INFO crossbow_1_ammo_info;
	MOD_LEVEL_AMMO_INFO crossbow_2_ammo_info;
	MOD_LEVEL_AMMO_INFO crossbow_3_ammo_info;

	MOD_LEVEL_AMMO_INFO grenade_1_ammo_info;
	MOD_LEVEL_AMMO_INFO grenade_2_ammo_info;
	MOD_LEVEL_AMMO_INFO grenade_3_ammo_info;

	MOD_LEVEL_AMMO_INFO six_shooter_ammo_info;
};

struct MOD_LEVEL_MISC_INFO {
	T4POverrideFogMode override_fog_mode = T4P_FOG_DEFAULT;

	T4PWeatherType rain_type = T4P_WEATHER_DISABLED;
	T4PWeatherType snow_type = T4P_WEATHER_DISABLED;

	bool draw_legend_on_flyby = false;
	int32_t legend_timer = 150;
	bool lara_impales_on_spikes = false;
	bool enable_ricochet_sound_effect = false;
	bool enable_smashing_and_killing_rolling_balls = false;
	bool enable_standing_pushables = false;
	bool darts_poison_fix = false;
	bool disable_motorbike_headlights = false;
	bool always_exit_from_statistics_screen = false;
	// Fix for a fallthrough bug which will vertically warp Lara to the top of a water room
	// if she jumps into it from below. Disabled by default here since certain custom levels
	// depend on this bug to be able to functional correctly.
	bool fix_vertical_water_warp = false;
	// TREP
	bool trep_switch_maker = false;
	int32_t trep_switch_on_ocb_1_anim = LARA_ANIM_HIDDENPICKUP;
	int32_t trep_switch_off_ocb_1_anim = LARA_ANIM_HIDDENPICKUP;
	int32_t trep_switch_on_ocb_2_anim = LARA_ANIM_HIDDENPICKUP;
	int32_t trep_switch_off_ocb_2_anim = LARA_ANIM_HIDDENPICKUP;
	int32_t trep_switch_on_ocb_3_anim = LARA_ANIM_SMALLSWITCH;
	int32_t trep_switch_off_ocb_3_anim = LARA_ANIM_SMALLSWITCH;
	int32_t trep_switch_on_ocb_4_anim = LARA_ANIM_GENERATORSW_ON;
	int32_t trep_switch_off_ocb_4_anim = LARA_ANIM_GENERATORSW_OFF;
	int32_t trep_switch_on_ocb_5_anim = LARA_ANIM_ONEHANDPUSHSW;
	int32_t trep_switch_off_ocb_5_anim = LARA_ANIM_ONEHANDPUSHSW;
	int32_t trep_switch_on_ocb_6_anim = 470;
	int32_t trep_switch_off_ocb_6_anim = 471;

	bool enable_teeth_spikes_kill_enemies = false;

	int16_t static_transparency_glass = 128;
	int16_t static_transparency_ice = 208;
	int16_t damage_static_interaction = 10;
	int16_t poison_static_interaction = 256;
};

struct MOD_LEVEL_INFO {
	MOD_LEVEL_BARS_INFO bars_info;
	MOD_LEVEL_ENVIRONMENT_INFO environment_info;
	MOD_LEVEL_FONT_INFO font_info;
	MOD_LEVEL_CAMERA_INFO camera_info;
	MOD_LEVEL_CREATURE_INFO creature_info;
	MOD_LEVEL_GFX_INFO gfx_info;
	MOD_LEVEL_STAT_INFO stat_info;
	MOD_LEVEL_LARA_INFO lara_info;
	MOD_LEVEL_AUDIO_INFO audio_info;
	MOD_LEVEL_FLARE_INFO flare_info;
	MOD_LEVEL_WEAPON_INFO weapon_info;
	MOD_LEVEL_MISC_INFO misc_info;
	MOD_LEVEL_OBJECTS_INFO objects_info;
	MOD_LEVEL_STATICS_INFO statics_info;
};

struct GAME_MOD_CONFIG {
	MOD_GLOBAL_INFO global_info;

	MOD_LEVEL_INFO level_info[MOD_LEVEL_COUNT];
};

extern GAME_MOD_CONFIG game_mod_config;

extern void setup_custom_slots_for_level(int32_t level, OBJECT_INFO* current_object_info_array);
extern void assign_slot_for_level(int32_t level, int32_t dest_slot, int32_t src_slot);

extern void T4PlusSetupObjectsForLevel(int32_t level, OBJECT_INFO* current_object_info_array);

extern MOD_GLOBAL_INFO *get_game_mod_global_info();

extern MOD_LEVEL_AUDIO_INFO *get_game_mod_level_audio_info(int32_t level);
extern MOD_LEVEL_BARS_INFO *get_game_mod_level_bars_info(int32_t level);
extern MOD_LEVEL_ENVIRONMENT_INFO *get_game_mod_level_environment_info(int32_t level);
extern MOD_LEVEL_FONT_INFO *get_game_mod_level_font_info(int32_t level);
extern MOD_LEVEL_CAMERA_INFO *get_game_mod_level_camera_info(int32_t level);
extern MOD_LEVEL_CREATURE_INFO *get_game_mod_level_creature_info(int32_t level);
extern MOD_LEVEL_GFX_INFO *get_game_mod_level_gfx_info(int32_t level);
extern MOD_LEVEL_LARA_INFO *get_game_mod_level_lara_info(int32_t level);
extern MOD_LEVEL_OBJECTS_INFO *get_game_mod_level_objects_info(int32_t level);
extern MOD_LEVEL_STAT_INFO *get_game_mod_level_stat_info(int32_t level);
extern MOD_LEVEL_FLARE_INFO *get_game_mod_level_flare_info(int32_t level);
extern MOD_LEVEL_WEAPON_INFO *get_game_mod_level_weapon_info(int32_t level);
extern MOD_LEVEL_MISC_INFO *get_game_mod_level_misc_info(int32_t level);
extern MOD_LEVEL_STATICS_INFO* get_game_mod_level_statics_info(int32_t level);

extern MOD_LEVEL_AMMO_INFO *get_game_mod_current_lara_ammo_info(MOD_LEVEL_WEAPON_INFO *weapon_info);

extern MOD_LEVEL_OBJECT_CUSTOMIZATION *get_game_mod_level_object_customization_for_slot(int32_t level, int32_t slot);

extern bool LoadGameModConfigFirstPass();
extern void LoadGameModConfigSecondPass();

extern void T4PlusLevelReset();
extern void T4PlusLevelSetup(int32_t current_level);
extern void T4PlusEnterLevel(int32_t current_level, bool initial_entry);
extern void T4PlusInitializeLara();
extern void T4PlusCleanup();
extern void T4PlusInit();

extern bool is_mod_trng_version_equal_or_greater_than_target(
    uint8_t target_major_version,
    uint8_t target_minor_version,
    uint8_t target_maintainence_version,
    uint8_t target_build_version);