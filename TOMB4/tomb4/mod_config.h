#pragma once

#include "../game/objects.h"
#include "../game/text.h"
#include "../game/sound.h"

#include "tomb4plus/t4plus_plugin.h"
#include "tomb4plus/t4plus_weather.h"
#include "tomb4plus/t4plus_mirror.h"
#include "tomb4plus/t4plus_environment.h"

#define MOD_LEVEL_COUNT 64

// 1 - Change object customization ID format.
#define ENGINE_MANIFEST_VERSION 1

#define DEFAULT_FOG_START_VALUE 12288
#define DEFAULT_FOG_END_VALUE 20480
#define DEFAULT_FAR_VIEW_VALUE 20480

// This may need increasing...
#define MAXIMUM_JSON_ALLOCATION_BLOCKS 32768 

#define MAX_EQUIPMENT_MODIFIERS 32
#define MAX_VAPOR_OCB_CUSTOMIZATIONS 16

#define MAX_T4PLUS_STRINGS 4096
extern int global_string_table_size;
extern char **global_string_table;

struct MOD_GLOBAL_PLUGIN {
	char *plugin_name = nullptr;
	char *plugin_type = nullptr;
	char *plugin_source = nullptr;
};

struct MOD_LEVEL_ENVIRONMENT_INFO {
	int fog_start_range = DEFAULT_FOG_START_VALUE;
	int fog_end_range = DEFAULT_FOG_END_VALUE;
	int far_view = DEFAULT_FAR_VIEW_VALUE;
	bool force_train_fog = false;
	bool disable_distance_limit = false;
	bool enable_multi_color_fog_bulbs = false;

	unsigned int room_swamp_flag = 0;
	unsigned int room_cold_flag = ROOM_COLD;
	unsigned int room_damage_flag = 0;
};

struct MOD_LEVEL_FONT_INFO {
	int custom_glyph_scale_width = DEFAULT_GLYPH_SCALE_WIDTH;
	int custom_glyph_scale_height = DEFAULT_GLYPH_SCALE_HEIGHT;
	float custom_vertical_spacing = 0.075F;

	int main_font_main_color = 0xff808080;
	int main_font_fade_color = 0xff808080;

	int options_title_font_main_color = 0xffc08040;
	int options_title_font_fade_color = 0xff401000;

	int inventory_title_font_main_color = 0xffe0c000;
	int inventory_title_font_fade_color = 0xff402000;

	int inventory_item_font_main_color = 0xff808080;
	int inventory_item_font_fade_color = 0xff101010;

	CHARDEF custom_font_table[CHAR_TABLE_COUNT] = {};
};

struct MOD_LEVEL_CAMERA_INFO {
	int chase_camera_distance = 1536;
	int chase_camera_vertical_orientation = -1820;
	int chase_camera_horizontal_orientation = 0;

	int combat_camera_distance = 1536;
	int combat_camera_vertical_orientation = -2730;

	int look_camera_distance = -1024;
	int look_camera_height = 16;

	int camera_speed = 10;
	int add_on_battle_camera_top = 256;
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
	unsigned char bone_id = -1;
	unsigned char bone_data = 0;
};

struct MOD_LEVEL_OBJECT_CUSTOMIZATION {
	const char *initialise_routine;
	const char *collision_routine;
	const char *control_routine;
	const char *draw_routine;
	const char *draw_routine_extra;
	const char *ceiling_routine;
	const char *floor_routine;

	ushort is_amphibious : 1;

	ushort is_agent : 1;
	ushort is_undead : 1;

	ushort save_position : 1;
	ushort save_hitpoints : 1;
	ushort save_flags : 1;
	ushort save_anim : 1;
	ushort save_mesh : 1;

	ushort override_hit_points : 1;
	ushort override_hit_type : 1;

	ushort explode_immediately : 1;
	ushort explode_after_death_animation : 1;
	ushort explosive_death_only : 1;
	ushort hit_type : 2;
	
	int pivot_length = 0;
	int radius = 10;
	int shadow_size = 0;

	int explodable_meshbits = 0;

	short hit_points = 0;
	short damage_1 = 0;
	short damage_2 = 0;
	short damage_3 = 0;

	ushort linked_object_1 = 0;
	ushort linked_object_2 = 0;
	ushort linked_object_3 = 0;

	int object_mip = 0;

	MOD_LEVEL_OBJECT_BONE_CUSTOMIZATION bone_customization[MAX_BONE_CUSTOMIZATIONS];
};

struct MOD_LEVEL_OBJECTS_INFO {
	// Metadata slots
	int16_t lara_slot = LARA;

	int16_t pistols_anim = PISTOLS_ANIM;
	int16_t uzi_anim = UZI_ANIM;
	int16_t shotgun_anim = SHOTGUN_ANIM;
	int16_t crossbow_anim = CROSSBOW_ANIM;
	int16_t grenade_gun_anim = GRENADE_GUN_ANIM;
	int16_t revolver_anim = SIXSHOOTER_ANIM;
	int16_t flare_anim = FLARE_ANIM;

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

	int16_t nitrous_oxide_feeder_slot = PUZZLE_ITEM1;
	int16_t jeep_key_slot = PUZZLE_ITEM1;
	
	int16_t rubber_boat_slot = RUBBER_BOAT;
	int16_t rubber_boat_extra_slot = RUBBER_BOAT_LARA;
	int16_t motor_boat_slot = MOTOR_BOAT;
	int16_t motor_boat_extra_slot = MOTOR_BOAT_LARA;

	int16_t laser_head_slot = LASER_HEAD;
	int16_t laser_head_base_slot = LASER_HEAD_BASE;
	int16_t laser_head_tentacle_slot = LASER_HEAD_TENTACLE;

	int16_t darts_interval = 24;
	int16_t darts_speed = 256;
	int16_t falling_block_timer = 60;
	int16_t falling_block_tremble = 1023;

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
	int small_scorpion_poison_strength = 512;
	
	bool remove_knights_templar_sparks = false;
	
	bool remove_ahmet_death_flames = false;
	bool remove_ahmet_death_loop = false;
	bool disable_ahmet_heavy_trigger = false;

	bool remove_mummy_stun_animations = false;

	bool disable_sentry_flame_attack = false;
};

struct MOD_LEVEL_MIRROR_CUSTOMIZATION {
	short room_number = -1;
	int plane_position = 0;
	T4PlusMirrorDirection plane_direction = T4PlusMirrorDirection::T4_MIR_PLANE_X;
};

struct MOD_LEVEL_VAPOR_CUSTOMIZATION {

};

struct MOD_LEVEL_GFX_INFO {
	T4PColdBreath cold_breath = COLD_BREATH_DISABLED;
	
	short default_envmap_sprite_index = 11;
	short pickup_envmap_sprite_index = 11;

	MOD_LEVEL_MIRROR_CUSTOMIZATION mirror_customization[MAX_MIRRORS-1];

	MOD_LEVEL_VAPOR_CUSTOMIZATION white_smoke_emitter_customization_for_OCB[MAX_VAPOR_OCB_CUSTOMIZATIONS];
	MOD_LEVEL_VAPOR_CUSTOMIZATION black_smoke_emitter_customization_for_OCB[MAX_VAPOR_OCB_CUSTOMIZATIONS];
	MOD_LEVEL_VAPOR_CUSTOMIZATION vapor_emitter_customization_for_OCB[MAX_VAPOR_OCB_CUSTOMIZATIONS];
};

struct MOD_LEVEL_AUDIO_INFO {
	bool new_audio_system = false;
	bool old_cd_trigger_system = true;

	int sample_rate = 22050;

	short first_looped_audio_track = 105;
	short last_looped_audio_track = 111;

	short boat_track = 12;
	short inside_jeep_track = 98;
	short outside_jeep_track = 110;
	short secret_track = 5;

	// SFX
	short pour_sfx_id = SFX_POUR;
	short loop_for_small_fires_sfx_id = SFX_LOOP_FOR_SMALL_FIRES;
	short flame_emitter_sfx_id = SFX_FLAME_EMITTER;
	short underwater_sfx_id = SFX_UNDERWATER;
	short explosion_1_sfx_id = SFX_EXPLOSION1;
	short explosion_2_sfx_id = SFX_EXPLOSION2;
	short lara_bubbles_sfx_id = SFX_LARA_BUBBLES;

	short lara_no_sfx_id = SFX_LARA_NO;
	short lara_richochet_sfx_id = SFX_LARA_RICOCHET;
	short lara_pistols_overlay_sfx_id = SFX_EXPLOSION1;

	short lara_pistol_shell_sfx_id = SFX_LARA_SHOTGUN_SHELL; 
	short lara_shotgun_shell_sfx_id = SFX_LARA_SHOTGUN_SHELL;
	short lara_rope_creak_sfx_id = SFX_LARA_ROPE_CREAK;
	short sample_test_sfx_id = SFX_LARA_BREATH;
	short menu_select_sfx_id = SFX_MENU_SELECT;
	short menu_choose_sfx_id = SFX_MENU_CHOOSE;
	short menu_combine_sfx_id = SFX_MENU_COMBINE;
	short menu_large_medipack_sfx_id = SFX_MENU_MEDI;
	short menu_small_medipack_sfx_id = SFX_MENU_MEDI;

	short bike_idle_sfx_id = SFX_BIKE_IDLE;
	short bike_moving_sfx_id = SFX_BIKE_MOVING;
	short jeep_idle_sfx_id = SFX_JEEP_IDLE;
	short jeep_moving_sfx_id = SFX_JEEP_MOVE;

	short motorboat_idle_sfx_id = 308;
	short motorboat_moving_sfx_id = 307;
	short rubber_boat_idle_sfx_id = 308;
	short rubber_boat_moving_sfx_id = 307;

	short god_head_charge_sfx_id = SFX_GENERIC_NRG_CHARGE;
	short god_head_laser_loop_sfx_id = SFX_BAZOOKA_FIRE;
	short god_head_blast_sfx_id = SFX_DEMIGOD_FALCON_PLAS;
	short god_head_smash_sfx_id = SFX_EXPLOSION2;
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
	long hair_gravity = 10;

	long braid_x = -4;
	long braid_y = -4;
	long braid_z = -48;

	long pigtail_left_x = -52;
	long pigtail_left_y = -48;
	long pigtail_left_z = -50;

	long pigtail_right_x = 44;
	long pigtail_right_y = -48;
	long pigtail_right_z = -50;

	int crawlspace_jump_animation = 421;
	int crawlspace_jump_pit_deepness_threshold = 768;

	bool use_tr5_swimming_collision = false;
	bool disable_hardcoded_breath_sound = false;

	int ledge_to_jump_state = -1;
	int ledge_to_down_state = -1;

	bool use_look_transparency = true;
};

struct TRNG_ENGINE_VERSION {
	unsigned char trng_version_major = 1;
	unsigned char trng_version_minor = 3;
	unsigned char trng_version_maintainence = 0;
	unsigned char trng_version_build = 7;
};


struct MOD_GLOBAL_INFO {
	int manifest_compatibility_version = -1;
	char *game_name = nullptr;
	char *authors = nullptr;
	char *game_user_dir_name = nullptr;

	unsigned int tr_engine_version = 4;
	bool tr_level_editor = true;
	bool tr_times_exclusive = false;
	bool tr_use_adpcm_audio = false;

	// Default to latest known version.
	TRNG_ENGINE_VERSION trng_engine_version;

	// TRNG Stuff
	bool trng_extended_flipmap_bitmask = false;
	bool trng_new_triggers = false; // TRNG (special TRNG flipeffects. Disable if conflicting with FURR)
	bool trng_anim_commands_enabled = false;
	bool trng_timerfields_enabled = false;
	bool trng_rollingball_extended_ocb = false; // TRNG (moveable and regular trigger activation)
	bool trng_statics_extended_ocb = false; // TRNG (touch inflicting poison)
	// Climable pushables (both implementations have different quirks. Will attempt to more accurately recreate them later)
	bool trng_pushable_extended_ocb = false; // TRNG (climable pushables)
	bool trng_switch_extended_ocb = false; // TRNG (custom switch animations)
	bool trng_hack_allow_meshes_with_exactly_256_vertices = false; // TRNG seems to have a special hack which allows meshes of 256 verticies
	bool trng_advanced_block_raising_behaviour = false;
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
	unsigned short max_particles = 256;
};

struct MOD_EQUIPMENT_MODIFIER {
	int object_id = -1;
	int amount = -1;
};

struct MOD_LEVEL_STAT_INFO {
	unsigned int secret_count = 70;

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
	int localized_strings_info_count = 0;
	MOD_LEVEL_LOCALIZED_STRINGS_OVERWRITE_INFO *localized_strings_info;
};

struct MOD_LEVEL_FLARE_INFO {
	unsigned char light_color_r = 128;
	unsigned char light_color_g = 192;
	unsigned char light_color_b = 0;
	int flare_lifetime_in_ticks = 30 * 30;
	int light_intensity = 16;
	bool has_sparks = false;
	bool has_fire = false; // Unimplemented
	bool sparks_include_smoke = false;
	bool has_glow = false; // Unimplemented
	bool flat_light = false;
};

struct MOD_LEVEL_AMMO_INFO {
	bool disable_explosion_sfx = false;

	short damage = 0;
	short poison_damage = 0;
	short explosion_damage = 0;
	
	short speed = 0;
	short gravity = 0;

	short shots = 1;
	short fire_rate = 0;
	short dispertion = 0;
	short flash_duration = 0;

	short weapon_pickup_amount = 0;
	short ammo_pickup_amount = 0;
	
	short grenade_timer = 0;

	bool add_pistol_shell = false;
	bool add_shotgun_shell = false;

	bool creates_explosion = false;
	bool creates_super_explosion = false;

	short push_lara_amount = 0;
	short push_target_amount = 0;

	// TRNG-specific
	short trng_trigger_id_when_enemy_hit = -1;
	short trng_trigger_id_at_end = -1;
	short trng_effect = -1;
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
	unsigned int legend_timer = 150;
	bool lara_impales_on_spikes = false;
	bool enable_ricochet_sound_effect = false;
	bool enable_smashing_and_killing_rolling_balls = false;
	bool enable_standing_pushables = false;
	bool enemy_gun_hit_underwater_sfx_fix = false;
	bool darts_poison_fix = false;
	bool disable_motorbike_headlights = false;
	bool always_exit_from_statistics_screen = false;
	// Fix for a fallthrough bug which will vertically warp Lara to the top of a water room
	// if she jumps into it from below. Disabled by default here since certain custom levels
	// depend on this bug to be able to functional correctly.
	bool fix_vertical_water_warp = false;
	// TREP
	bool trep_switch_maker = false;
	int trep_switch_on_ocb_1_anim = 0;
	int trep_switch_off_ocb_1_anim = 0;
	int trep_switch_on_ocb_2_anim = 0;
	int trep_switch_off_ocb_2_anim = 0;
	int trep_switch_on_ocb_3_anim = 0;
	int trep_switch_off_ocb_3_anim = 0;
	int trep_switch_on_ocb_4_anim = 0;
	int trep_switch_off_ocb_4_anim = 0;
	int trep_switch_on_ocb_5_anim = 0;
	int trep_switch_off_ocb_5_anim = 0;
	int trep_switch_on_ocb_6_anim = 0;
	int trep_switch_off_ocb_6_anim = 0;

	bool enable_teeth_spikes_kill_enemies = false;

	short static_transparency_glass = 128;
	short static_transparency_ice = 208;
	short damage_static_interaction = 10;
	short posion_static_interaction = 256;
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

extern void setup_custom_slots_for_level(int level, OBJECT_INFO* current_object_info_array);
extern void assign_slot_for_level(int level, int dest_slot, int src_slot);

extern void T4PlusSetupObjectsForLevel(int level, OBJECT_INFO* current_object_info_array);

extern MOD_GLOBAL_INFO *get_game_mod_global_info();

extern MOD_LEVEL_AUDIO_INFO *get_game_mod_level_audio_info(int level);
extern MOD_LEVEL_BARS_INFO *get_game_mod_level_bars_info(int level);
extern MOD_LEVEL_ENVIRONMENT_INFO *get_game_mod_level_environment_info(int level);
extern MOD_LEVEL_FONT_INFO *get_game_mod_level_font_info(int level);
extern MOD_LEVEL_CAMERA_INFO *get_game_mod_level_camera_info(int level);
extern MOD_LEVEL_CREATURE_INFO *get_game_mod_level_creature_info(int level);
extern MOD_LEVEL_GFX_INFO *get_game_mod_level_gfx_info(int level);
extern MOD_LEVEL_LARA_INFO *get_game_mod_level_lara_info(int level);
extern MOD_LEVEL_OBJECTS_INFO *get_game_mod_level_objects_info(int level);
extern MOD_LEVEL_STAT_INFO *get_game_mod_level_stat_info(int level);
extern MOD_LEVEL_FLARE_INFO *get_game_mod_level_flare_info(int level);
extern MOD_LEVEL_WEAPON_INFO *get_game_mod_level_weapon_info(int level);
extern MOD_LEVEL_MISC_INFO *get_game_mod_level_misc_info(int level);
extern MOD_LEVEL_STATICS_INFO* get_game_mod_level_statics_info(int level);

extern MOD_LEVEL_AMMO_INFO *get_game_mod_current_lara_ammo_info(MOD_LEVEL_WEAPON_INFO *weapon_info);

extern MOD_LEVEL_OBJECT_CUSTOMIZATION *get_game_mod_level_object_customization_for_slot(int level, int slot);

extern bool LoadGameModConfigFirstPass();
extern void LoadGameModConfigSecondPass();

extern void T4PlusLevelReset();
extern void T4PlusLevelSetup(int current_level);
extern void T4PlusEnterLevel(int current_level, bool initial_entry);
extern void T4PlusInitializeLara();
extern void T4PlusCleanup();
extern void T4PlusInit();

extern bool is_mod_trng_version_equal_or_greater_than_target(
	unsigned char target_major_version,
	unsigned char target_minor_version,
	unsigned char target_maintainence_version,
	unsigned char target_build_version);