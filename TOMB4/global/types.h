#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <cstdio>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "math_tbls.h"

#pragma pack(push, 1)

#if 1
#if defined(_MSC_VER)
#define TR_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define TR_FORCE_INLINE
#else
#define TR_FORCE_INLINE
#endif
#else
#if defined(_MSC_VER)
#define TR_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define TR_FORCE_INLINE inline __attribute__((always_inline))
#else
#define TR_FORCE_INLINE inline
#endif
#endif

#if defined(_MSC_VER)
#define TR_CDECL __cdecl
#define TR_GETCWD _getcwd
#else
#define TR_CDECL
#define TR_GETCWD getcwd
#endif

#include <SDL.h>

/*math*/
#define SQUARE(x)				((x)*(x))
#define	TRIGMULT2(a,b)			(((a) * (b)) >> W2V_SHIFT)
#define	TRIGMULT3(a,b,c)		(TRIGMULT2((TRIGMULT2(a, b)), c))
#define	FTRIGMULT2(a,b)			((a) * (b))
#define	FTRIGMULT3(a,b,c)		(FTRIGMULT2((FTRIGMULT2(a, b)), c))

/*color*/
#define RGBONLY(r, g, b)		((b) | (((g) | ((r) << 8)) << 8))
#define RGBA(r, g, b, a)		(RGBONLY(r, g, b) | ((a) << 24))
#define	CLRA(clr)				((clr >> 24) & 0xFF)	//shift r, g, and b out of the way and 0xFF
#define	CLRR(clr)				((clr >> 16) & 0xFF)	//shift g and b out of the way and 0xFF
#define	CLRG(clr)				((clr >> 8) & 0xFF)		//shift b out of the way and 0xFF
#define	CLRB(clr)				((clr) & 0xFF)			//and 0xFF

#define SetCutPlayed(num)		(CutSceneTriggered |= 1 << (num))
#define SetCutNotPlayed(num)	(CutSceneTriggered &= ~(1 << (num)))
#define CheckCutPlayed(num)		(CutSceneTriggered & (1 << (num)))


#define POP_BONE_FLAG						(1 << 0)
#define PUSH_BONE_FLAG						(1 << 1)
#define X_ROTATION_FLAG						(1 << 2)
#define Y_ROTATION_FLAG						(1 << 3)
#define Z_ROTATION_FLAG						(1 << 4)

#define QUARTER_CLICK_SIZE					64
#define HALF_CLICK_SIZE						(QUARTER_CLICK_SIZE * 2)
#define CLICK_SIZE							(HALF_CLICK_SIZE * 2)
#define HALF_BLOCK_SIZE						(CLICK_SIZE * 2)
#define BLOCK_SIZE							(HALF_BLOCK_SIZE * 2)

#define WALL_SHIFT							10

#define DEFAULT_FOV							80
#define ONE_DEGREE							182
#define HALF_DEGREE							91

#define DEGREES_TO_ROTATION(deg)			deg * ONE_DEGREE
#define HALF_DEGREES_TO_ROTATION(half_deg)	half_deg * HALF_DEGREE

#define NO_HEIGHT	-32512
#define NO_ITEM	-1
#define FVF (D3DFVF_TEX2 | D3DFVF_SPECULAR | D3DFVF_DIFFUSE | D3DFVF_XYZRHW)
#define W2V_SHIFT	14
#define MAX_SAMPLES	370
#define MAX_NGLE_SAMPLES	2048
#define MAX_DYNAMICS	64
#define MAX_BUCKETS		300 // TRLE: bumped from 20 to 300
#define BUCKET_VERT_COUNT	8224 // TRLE: increased size (256 * 32 + 32)
#ifdef LEVEL_EDITOR
#define MAX_SPARKS 8096 // TRLE: bumped from 256 to 8096 (WARNING: affects RNG)
#else
#define MAX_SPARKS 256
#endif
#define MALLOC_SIZE	64000000	// TRLE: bumped from 15MB to 64MB 
#define PARAMETER_MAX_LENGTH 1024

/*typedefs*/

// For legacy savegame backwards compatibility on 64-bit machines.
#define X32_POINTER uint32_t

enum DX_FLAGS {
	DXF_NONE = 0x0,
	DXF_FULLSCREEN = 0x1,
	DXF_WINDOWED = 0x2,
	DXF_ZBUFFER = 0x10,
	DXF_FPUSETUP = 0x20,
	DXF_NOFREE = 0x40,
	DXF_HWR = 0x80
};

enum carried_weapon_flags {
	W_NONE =		0x0,
	W_PRESENT =		0x1,
	W_FLASHLIGHT =	0x2,	//speculation, actually unused
	W_LASERSIGHT =	0x4,
	W_AMMO1 =		0x8,
	W_AMMO2 =		0x10,
	W_AMMO3 =		0x20
};

enum anim_commands {
	ACMD_NULL,
	ACMD_SETPOS,
	ACMD_JUMPVEL,
	ACMD_FREEHANDS,
	ACMD_KILL,
	ACMD_PLAYSFX,
	ACMD_FLIPEFFECT
};

enum ai_bits {
	GUARD =		1 << 0,
	AMBUSH =	1 << 1,
	PATROL1 =	1 << 2,
	MODIFY =	1 << 3,
	FOLLOW =	1 << 4
};

enum spark_flags {
	SF_NONE = 0x0,
	SF_FIRE = 0x1,	//burns Lara at contact
	SF_SCALE = 0x2,	//scale using sptr->Scalar
	SF_UNUSED = 0x4,
	SF_DEF = 0x8,	//use sptr->Def for the drawn sprite (otherwise do flat quad)
	SF_ROTATE = 0x10,	//rotate the drawn sprite (for regular sparks, SF_DEF is required)
	SF_NOKILL = 0x20,	//flag to avoid killing the spark in GetFreeSpark if no free slots are found
	SF_FX = 0x40,	//spark is attached to an effect
	SF_ITEM = 0x80,	//spark is attached to an item
	SF_OUTSIDE = 0x100,	//spark is affected by wind
	SF_UNUSED2 = 0x200,
	SF_DAMAGE = 0x400,	//damages lara at contact
	SF_UNWATER = 0x800,	//for underwater explosions to create bubbles etc.
	SF_ATTACHEDNODE = 0x1000,	//spark is attached to an item node, uses NodeOffsets
	SF_GREEN = 0x2000	//turns the spark into a green-ish blue (for explosions only)
};

enum languages {
	ENGLISH,
	FRENCH,
	GERMAN,
	ITALIAN,
	SPANISH,
	US,
	JAPAN,
	DUTCH,
	LANGUAGE_COUNT
};

enum font_flags {
	FF_SMALL =		0x1000,
	FF_BLINK =		0x2000,
	FF_RJUSTIFY =	0x4000,
	FF_CENTER =		0x8000
};

enum room_flags {
	ROOM_UNDERWATER =	0x1,
	ROOM_SWAMP =		0x4,
	ROOM_OUTSIDE =		0x8,
	ROOM_DAMAGE =		0x10,
	ROOM_NOT_INSIDE =	0x20,
	ROOM_INSIDE =		0x40,
	ROOM_NO_LENSFLARE = 0x80,
	ROOM_CAUSTICS =     0x100,
	ROOM_REFLECTIONS =  0x200,
	ROOM_SNOW =			0x400,
	ROOM_RAIN =			0x800,
	ROOM_COLD =			0x1000,
};

enum quadrant_names {
	NORTH,
	EAST,
	SOUTH,
	WEST
};

enum collision_types {
	CT_NONE =			0x0,
	CT_FRONT =			0x1,
	CT_LEFT =			0x2,
	CT_RIGHT =			0x4,
	CT_TOP =			0x8,
	CT_TOP_FRONT =		0x10,
	CT_CLAMP =			0x20
};

enum sfx_types {
	SFX_LANDANDWATER =	0,
	SFX_LANDONLY =		0x4000,
	SFX_WATERONLY =		0x8000
};

enum target_type {
	NO_TARGET,
	PRIME_TARGET,
	SECONDARY_TARGET
};

enum mood_type {
	BORED_MOOD,
	ATTACK_MOOD,
	ESCAPE_MOOD,
	STALK_MOOD,
};

enum zone_type {
	SKELLY_ZONE,
	BASIC_ZONE,
	CROC_ZONE,
	HUMAN_ZONE,
	FLYER_ZONE,
};

enum height_types {
	WALL,
	SMALL_SLOPE,
	BIG_SLOPE,
	DIAGONAL,
	SPLIT_TRI
};

enum item_status {
	ITEM_INACTIVE,
	ITEM_ACTIVE,
	ITEM_DEACTIVATED,
	ITEM_INVISIBLE
};

enum floor_types {
	FLOOR_TYPE,
	DOOR_TYPE,
	TILT_TYPE,
	ROOF_TYPE,
	TRIGGER_TYPE,
	LAVA_TYPE,
	CLIMB_TYPE,
	SPLIT1,
	SPLIT2,
	SPLIT3,
	SPLIT4,
	NOCOLF1T,
	NOCOLF1B,
	NOCOLF2T,
	NOCOLF2B,
	NOCOLC1T,
	NOCOLC1B,
	NOCOLC2T,
	NOCOLC2B,
	MONKEY_TYPE,
	TRIGTRIGGER_TYPE,
	MINER_TYPE
};

enum weapon_types {
	WEAPON_NONE,
	WEAPON_PISTOLS,
	WEAPON_REVOLVER,
	WEAPON_UZI,
	WEAPON_SHOTGUN,
	WEAPON_GRENADE,
	WEAPON_CROSSBOW,
	WEAPON_FLARE,
	WEAPON_TORCH
};

enum lara_water_status {
	LW_ABOVE_WATER,
	LW_UNDERWATER,
	LW_SURFACE,
	LW_FLYCHEAT,
	LW_WADE
};

// Imported from Tomb5
enum LMX {
	LMX_HIPS,
	LMX_THIGH_L,
	LMX_CALF_L,
	LMX_FOOT_L,
	LMX_THIGH_R,
	LMX_CALF_R,
	LMX_FOOT_R,
	LMX_TORSO,
	LMX_HEAD,
	LMX_UARM_R,
	LMX_LARM_R,
	LMX_HAND_R,
	LMX_UARM_L,
	LMX_LARM_L,
	LMX_HAND_L
};

enum lara_mesh {
	LM_HIPS,
	LM_LTHIGH,
	LM_LSHIN,
	LM_LFOOT,
	LM_RTHIGH,
	LM_RSHIN,
	LM_RFOOT,
	LM_TORSO,
	LM_RINARM,
	LM_ROUTARM,
	LM_RHAND,
	LM_LINARM,
	LM_LOUTARM,
	LM_LHAND,
	LM_HEAD,
	NUM_LARA_MESHES
};

enum trigger_types {
	TRIGGER,
	PAD,
	SWITCH,
	KEY,
	PICKUP,
	HEAVY,
	ANTIPAD,
	COMBAT,
	DUMMY,
	ANTITRIGGER,
	HEAVYSWITCH,
	HEAVYANTITRIGGER,
	MONKEY // TRNG - replaces this this with generic conditional triggers
};

enum trigobj_types {
	TO_OBJECT,
	TO_CAMERA,
	TO_SINK,
	TO_FLIPMAP,
	TO_FLIPON,
	TO_FLIPOFF,
	TO_TARGET,
	TO_FINISH,
	TO_CD,
	TO_FLIPEFFECT,
	TO_SECRET,
	TO_ACTION,
	TO_FLYBY,
	TO_CUTSCENE,
	TO_FMV,
	TO_TIMERFIELD
};

enum matrix_indices {
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23,

	indices_count
};

enum input_buttons {
	IN_NONE =				0x0,
	IN_FORWARD =			0x1,
	IN_BACK =				0x2,
	IN_LEFT =				0x4,
	IN_RIGHT =				0x8,
	IN_JUMP =				0x10,
	IN_DRAW =				0x20,
	IN_ACTION =				0x40,
	IN_WALK =				0x80,
	IN_OPTION =				0x100,
	IN_LOOK =				0x200,
	IN_LSTEP =				0x400,
	IN_RSTEP =				0x800,
	IN_ROLL =				0x1000,
	IN_PAUSE =				0x2000,
	IN_A =					0x4000,
	IN_B =					0x8000,
	IN_CHEAT =				0x10000,
	IN_D =					0x20000,
	IN_C =					0x30000,
	IN_E =					0x40000,
	IN_FLARE =				0x80000,
	IN_SELECT =				0x100000,
	IN_DESELECT =			0x200000,
	IN_SAVE =				0x400000,
	IN_LOAD =				0x800000,
	IN_STEPSHIFT =			0x1000000,
	IN_LOOKLEFT =			0x2000000,
	IN_LOOKRIGHT =			0x4000000,
	IN_LOOKFORWARD =		0x8000000,
	IN_LOOKBACK =			0x10000000,
	IN_DUCK =				0x20000000,
	IN_SPRINT =				0x40000000,
	IN_TARGET =				0x80000000,
	IN_ALL =				0xFFFFFFFF
};

enum ITEM_FLAGS {
	IFL_TRIGGERED =				0x20,
	IFL_SWITCH_ONESHOT =		0x40,	//oneshot for switch items
	IFL_ANTITRIGGER_ONESHOT =	0x80,	//oneshot for antitriggers
	IFL_INVISIBLE =				0x100,	//also used as oneshot for everything else
	IFL_CODEBITS =				0x3E00,
	IFL_REVERSE =				0x4000,
	IFL_CLEARBODY =				0x8000
};

enum lara_gun_status {
	LG_NO_ARMS,
	LG_HANDS_BUSY,
	LG_DRAW_GUNS,
	LG_UNDRAW_GUNS,
	LG_READY,
	LG_FLARE,
};

enum camera_type {
	CHASE_CAMERA,
	FIXED_CAMERA,
	LOOK_CAMERA,
	COMBAT_CAMERA,
	CINEMATIC_CAMERA,
	HEAVY_CAMERA,
};

enum LightTypes {
	LIGHT_SUN,
	LIGHT_POINT,
	LIGHT_SPOT,
	LIGHT_SHADOW,
	LIGHT_FOG
};

enum gf_level_options {
	GF_YOUNGLARA =		0x1,
	GF_WEATHER =		0x2,
	GF_HORIZON =		0x4,
	GF_LAYER1 =			0x8,
	GF_LAYER2 =			0x10,
	GF_STARFIELD =		0x20,
	GF_LIGHTNING =		0x40,
	GF_TRAIN =			0x80,
	GF_PULSE =			0x100,
	GF_HORIZONCOLADD =	0x200,
	GF_RESETHUB =		0x400,
	GF_LENSFLARE =		0x800,
	GF_TIMER =			0x1000,
	GF_MIRROR =			0x2000,
	GF_REMOVEAMULET =	0x4000,
	GF_NOLEVEL =		0x8000
};

struct CVECTOR {
	int8_t b;
	int8_t g;
	int8_t r;
	int8_t a;
};

struct SPHERE {
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t r;
};

struct PHD_VECTOR {
	int32_t x;
	int32_t y;
	int32_t z;
};

struct PHD_3DPOS {
	int32_t x_pos;
	int32_t y_pos;
	int32_t z_pos;
	int16_t x_rot;
	int16_t y_rot;
	int16_t z_rot;
};

struct GAME_VECTOR {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t room_number;
	int16_t box_number;
};

struct OBJECT_VECTOR {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t data;
	int16_t flags;
};

struct FVECTOR {
	float x;
	float y;
	float z;
};

struct SVECTOR {
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t pad;
};

struct PCLIGHT {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	int32_t shadow;
	float Inner;
	float Outer;
	float InnerAngle;
	float OuterAngle;
	float Cutoff;
	float nx;
	float ny;
	float nz;
	int32_t ix;
	int32_t iy;
	int32_t iz;
	int32_t inx;
	int32_t iny;
	int32_t inz;
	float tr;
	float tg;
	float tb;
	float rs;
	float gs;
	float bs;
	int32_t fcnt;
	uint8_t Type;
	uint8_t Active;
	PHD_VECTOR rlp;
	int32_t Range;
};

struct ITEM_LIGHT {
	int32_t r;
	int32_t g;
	int32_t b;
	int32_t ambient;
	int32_t rs;
	int32_t gs;
	int32_t bs;
	int32_t fcnt;
	PCLIGHT	CurrentLights[21];
	PCLIGHT	PrevLights[21];
	int32_t nCurrentLights;
	int32_t nPrevLights;
	int32_t room_number;
	int32_t RoomChange;
	PHD_VECTOR item_pos;
	void* pCurrentLights;
	void* pPrevLights;
};

#define TR4_VANILLA_ITEM_STRUCT_SIZE 5622

struct ITEM_INFO {
	int32_t floor;
	uint32_t touch_bits;
	uint32_t mesh_bits;
	int16_t object_number;
	int16_t current_anim_state;
	int16_t goal_anim_state;
	int16_t required_anim_state;
	int16_t anim_number;
	int16_t frame_number;
	int16_t room_number;
	int16_t next_item;
	int16_t next_active;
	int16_t speed;
	int16_t fallspeed;
	int16_t hit_points;
	uint16_t box_number;
	int16_t timer;
	int16_t flags;
	int16_t shade;
	int16_t trigger_flags;
	int16_t carried_item;
	int16_t after_death;
	uint16_t fired_weapon;
	int16_t item_flags[4];
	void* data;
	PHD_3DPOS pos;
	ITEM_LIGHT il;
	uint32_t active : 1; // 0x01
	uint32_t status : 2; // 0x02, 0x04
	uint32_t gravity_status : 1; // 0x08
	uint32_t hit_status : 1; // 0x10
	uint32_t collidable : 1; // 0x20
	uint32_t looked_at : 1; // 0x40
	uint32_t dynamic_light : 1; // 0x80
	uint32_t poisoned : 1; // 0x100
	uint32_t ai_bits : 5; // 0x200, 0x400, 0x800, 0x1000, 0x2000
	uint32_t really_active : 1; // 0x4000
	uint32_t meshswap_meshbits;
	int16_t draw_room;
	int16_t TOSSPAD;
};

struct BOX_NODE {
	int16_t exit_box;
	uint16_t search_number;
	int16_t next_expansion;
	int16_t box_number;
};

struct LOT_INFO {
	BOX_NODE* node;
	int16_t head;
	int16_t tail;
	uint16_t search_number;
	uint16_t block_mask;
	int16_t step;
	int16_t drop;
	int16_t zone_count;
	int16_t target_box;
	int16_t required_box;
	int16_t fly;
	uint16_t can_jump : 1;
	uint16_t can_monkey : 1;
	uint16_t is_amphibious : 1;
	uint16_t is_jumping : 1;
	uint16_t is_monkeying : 1;
	PHD_VECTOR target;
	zone_type zone;
};

#define CREATURE_JOINT_ROTATION_COUNT 4

struct CREATURE_INFO {
	int16_t joint_rotation[CREATURE_JOINT_ROTATION_COUNT];
	int16_t maximum_turn;
	int16_t flags;
	uint16_t alerted : 1;
	uint16_t head_left : 1;
	uint16_t head_right : 1;
	uint16_t reached_goal : 1;
	uint16_t hurt_by_lara : 1;
	uint16_t patrol2 : 1;
	uint16_t jump_ahead : 1;
	uint16_t monkey_ahead : 1;
	mood_type mood;
	ITEM_INFO* enemy;
	ITEM_INFO ai_target;
	int16_t pad;
	int16_t item_num;
	PHD_VECTOR target;
	LOT_INFO LOT;
};

struct FX_INFO {
	PHD_3DPOS pos;
	int16_t room_number;
	int16_t object_number;
	int16_t next_fx;
	int16_t next_active;
	int16_t speed;
	int16_t fallspeed;
	int16_t frame_number;
	int16_t counter;
	int16_t shade;
	int16_t flag1;
	int16_t flag2;
};

struct LARA_ARM {
	int16_t* frame_base;
	int16_t frame_number;
	int16_t anim_number;
	int16_t lock;
	int16_t y_rot;
	int16_t x_rot;
	int16_t z_rot;
	int16_t flash_gun;
};

#define LARA_MESH_PTR_COUNT 15
#define WET_COUNT 15
#define PUZZLE_ITEM_COUNT 12
#define KEY_ITEM_COUNT 12
#define PICKUP_ITEM_COUNT 4
#define PICKUP_COMBO_ITEM_COUNT 8
#define QUEST_ITEM_COUNT 6

struct LARA_INFO {
	int16_t item_number;
	int16_t gun_status;
	int16_t gun_type;
	int16_t request_gun_type;
	int16_t last_gun_type;
	int16_t calc_fallspeed;
	int16_t water_status;
	int16_t climb_status;
	int16_t pose_count;
	int16_t hit_frame;
	int16_t hit_direction;
	int16_t air;
	int16_t dive_count;
	int16_t death_count;
	int16_t current_active;
	int16_t current_xvel;
	int16_t current_yvel;
	int16_t current_zvel;
	int16_t spaz_effect_count;
	int16_t flare_age;
	int16_t vehicle;
	int16_t weapon_item;
	int16_t back_gun;
	int16_t flare_frame;
	int16_t poisoned;
	int16_t dpoisoned;
	uint8_t electric;
	uint8_t wet[WET_COUNT];
	uint16_t flare_control_left : 1; // 0x01
	uint16_t Unused1 : 1; // 0x02
	uint16_t look : 1; // 0x04
	uint16_t burn : 1; // 0x08
	uint16_t keep_ducked : 1; // 0x10
	uint16_t IsMoving : 1; // 0x20
	uint16_t CanMonkeySwing : 1; // 0x40
	uint16_t Unused2 : 1; // 0x80
	uint16_t OnBeetleFloor : 1; // 0x100
	uint16_t BurnGreen : 1; // 0x200
	uint16_t IsDucked : 1; // 0x400
	uint16_t has_fired : 1; // 0x800
	uint16_t Busy : 1; // 0x1000
	uint16_t LitTorch : 1; // 0x2000
	uint16_t IsClimbing : 1; // 0x4000
	uint16_t Fired : 1; // 0x8000
	int32_t water_surface_dist;
	PHD_VECTOR last_pos;
	FX_INFO* spaz_effect;
	int32_t mesh_effects;
	int16_t* mesh_ptrs[LARA_MESH_PTR_COUNT];
	ITEM_INFO* target;
	int16_t target_angles[2];
	int16_t turn_rate;
	int16_t move_angle;
	int16_t head_y_rot;
	int16_t head_x_rot;
	int16_t head_z_rot;
	int16_t torso_y_rot;
	int16_t torso_x_rot;
	int16_t torso_z_rot;
	LARA_ARM left_arm;
	LARA_ARM right_arm;
	uint16_t holster;
	CREATURE_INFO* creature;
	void *CornerX; // 32/64 bit
	void *CornerZ; // 32/64 bit
	int8_t RopeSegment;
	int8_t RopeDirection;
	int16_t RopeArcFront;
	int16_t RopeArcBack;
	int16_t RopeLastX;
	int16_t RopeMaxXForward;
	int16_t RopeMaxXBackward;
	int32_t RopeDFrame;
	int32_t RopeFrame;
	uint16_t RopeFrameRate;
	uint16_t RopeY;
	uint32_t RopePtr;
	uint32_t GeneralPtr;
	int32_t RopeOffset;
	uint32_t RopeDownVel;
	int8_t RopeFlag;
	int8_t MoveCount;
	int32_t RopeCount;
	int8_t pistols_type_carried;
	int8_t uzis_type_carried;
	int8_t shotgun_type_carried;
	int8_t crossbow_type_carried;
	int8_t grenade_type_carried;
	int8_t sixshooter_type_carried;
	int8_t lasersight;
	int8_t binoculars;
	int8_t crowbar;
	int8_t mechanical_scarab;
	uint8_t small_water_skin;
	uint8_t big_water_skin;
	int8_t examine1;
	int8_t examine2;
	int8_t examine3;
	int8_t puzzleitems[PUZZLE_ITEM_COUNT];
	uint16_t puzzleitemscombo;
	uint16_t keyitems;
	uint16_t keyitemscombo;
	uint16_t pickupitems;
	uint16_t pickupitemscombo;
	int16_t questitems;
	int16_t num_small_medipack;
	int16_t num_large_medipack;
	int16_t num_flares;
	int16_t num_pistols_ammo;
	int16_t num_uzi_ammo;
	int16_t num_revolver_ammo;
	int16_t num_shotgun_ammo1;
	int16_t num_shotgun_ammo2;
	int16_t num_grenade_ammo1;
	int16_t num_grenade_ammo2;
	int16_t num_grenade_ammo3;
	int16_t num_crossbow_ammo1;
	int16_t num_crossbow_ammo2;
	int16_t num_crossbow_ammo3;
	int8_t beetle_uses;
	int8_t blindTimer;
	int8_t location;
	int8_t highest_location;
	int8_t locationPad;
};

struct GAMEFLOW {
	uint32_t CheatEnabled : 1;
	uint32_t LoadSaveEnabled : 1;
	uint32_t TitleEnabled : 1;
	uint32_t PlayAnyLevel : 1;
	uint32_t Language : 3;
	uint32_t DemoDisc : 1;
	uint32_t Unused : 24;
	uint32_t InputTimeout;
	uint8_t SecurityTag;
	uint8_t nLevels;
	uint8_t nFileNames;
	uint8_t Pad;
	uint16_t FileNameLen;
	uint16_t ScriptLen;
};

static_assert(sizeof(GAMEFLOW)==16);

struct CAMERA_INFO {
	GAME_VECTOR pos;
	GAME_VECTOR target;
	camera_type type;
	camera_type old_type;
	int32_t shift;
	int32_t flags;
	int32_t fixed_camera;
	int32_t number_frames;
	int32_t bounce;
	int32_t underwater;
	int32_t target_distance;
	int16_t target_angle;
	int16_t target_elevation;
	int16_t actual_elevation;
	int16_t actual_angle;
	int16_t lara_node; // T4Plus
	int16_t number;
	int16_t last;
	int16_t timer;
	int16_t speed;
	ITEM_INFO* item;
	ITEM_INFO* last_item;
	OBJECT_VECTOR* fixed;
	int32_t mike_at_lara;
	PHD_VECTOR mike_pos;
};

struct COLL_INFO {
	int32_t mid_floor;
	int32_t mid_ceiling;
	int32_t mid_type;
	int32_t front_floor;
	int32_t front_ceiling;
	int32_t front_type;
	int32_t left_floor;
	int32_t left_ceiling;
	int32_t left_type;
	int32_t right_floor;
	int32_t right_ceiling;
	int32_t right_type;
	int32_t left_floor2;
	int32_t left_ceiling2;
	int32_t left_type2;
	int32_t right_floor2;
	int32_t right_ceiling2;
	int32_t right_type2;
	int32_t radius;
	int32_t bad_pos;
	int32_t bad_neg;
	int32_t bad_ceiling;
	PHD_VECTOR shift;
	PHD_VECTOR old;
	int16_t old_anim_state;
	int16_t old_anim_number;
	int16_t old_frame_number;
	int16_t facing;
	int16_t quadrant;
	int16_t coll_type;
	int16_t *trigger_index;
	int8_t tilt_x;
	int8_t tilt_z;
	int8_t hit_by_baddie;
	int8_t hit_static;
	uint16_t slopes_are_walls : 2;
	uint16_t slopes_are_pits : 1;
	uint16_t lava_is_pit : 1;
	uint16_t enable_baddie_push : 1;
	uint16_t enable_spaz : 1;
	uint16_t hit_ceiling : 1;
};

struct OBJECT_INFO {
	int16_t nmeshes;
	int16_t mesh_index;
	int32_t bone_index;
	int16_t* frame_base;
	void (*initialise)(int16_t item_number);
	void (*control)(int16_t item_number);
	void (*floor)(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
	void (*ceiling)(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
	void (*draw_routine)(ITEM_INFO* item);
	void (*collision)(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
	int16_t object_mip;
	int16_t anim_index;
	int16_t hit_points;
	int16_t pivot_length;
	int16_t radius;
	uint16_t aggression = 0xFFFF;
	int16_t shadow_size;
	uint16_t bite_offset;
	uint16_t loaded : 1;
	uint16_t intelligent : 1;
	uint16_t non_lot : 1;
	uint16_t save_position : 1;
	uint16_t save_hitpoints : 1;
	uint16_t save_flags : 1;
	uint16_t save_anim : 1;
	uint16_t semi_transparent : 1;
	uint16_t water_creature : 1;
	uint16_t using_drawanimating_item : 1;
	uint16_t HitEffect : 2;
	uint16_t undead : 1;
	uint16_t save_mesh : 1;
	void (*draw_routine_extra)(ITEM_INFO* item);
	uint32_t explodable_meshbits;
	uint32_t pad;
	uint16_t pad2;
};

struct FLOOR_INFO {
	uint16_t index;
	uint16_t fx : 4;
	uint16_t box : 11;
	uint16_t stopper : 1;
	uint8_t pit_room;
	int8_t floor;
	uint8_t sky_room;
	int8_t ceiling;
};

struct LIGHTINFO {
	int32_t x;
	int32_t y;
	int32_t z;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t Type;
	int16_t Intensity;
	float Inner;
	float Outer;
	float Length;
	float Cutoff;
	float nx;
	float ny;
	float nz;
};

struct FOGBULB_STRUCT {
	FVECTOR WorldPos;
	FVECTOR pos;
	FVECTOR vec;
	float rad;
	float sqrad;
	float inv_sqrad;
	float dist;
	int32_t density;
	int32_t inRange;
	int32_t timer;
	int32_t active;
	int32_t FXRad;
	int32_t room_number;
	int32_t r;
	int32_t g;
	int32_t b;
};

struct MESH_INFO {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t y_rot;
	int16_t shade;
	int16_t Flags;
	int16_t static_number;
};

struct PCLIGHT_INFO {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	int32_t shadow;
	float Inner;
	float Outer;
	float InnerAngle;
	float OuterAngle;
	float Cutoff;
	float nx;
	float ny;
	float nz;
	int32_t ix;
	int32_t iy;
	int32_t iz;
	int32_t inx;
	int32_t iny;
	int32_t inz;
	uint8_t Type;
	uint8_t Pad;
};

#define GFX_RGBA_SETALPHA(rgba, x) (((x) << 24) | ((rgba) & 0x00ffffff))

typedef uint32_t GFXCOLOR;
typedef float GFXVALUE;

struct GFXVECTOR {
	float x;
	float y;
	float z;
};

struct GFXMATRIX {
	float		_11, _12, _13, _14;
	float		_21, _22, _23, _24;
	float		_31, _32, _33, _34;
	float		_41, _42, _43, _44;
};

struct GFXVERTEX {
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float tu;
	float tv;
};

struct GFXTLVERTEX {
	GFXVALUE	sx;
	GFXVALUE	sy;
	GFXVALUE	sz;
	GFXVALUE	rhw;
	GFXCOLOR	color;
	GFXCOLOR	specular;
	GFXVALUE	tu;
	GFXVALUE	tv;
};

struct GFXTLBUMPVERTEX {
	GFXVALUE sx;
	GFXVALUE sy;
	GFXVALUE sz;
	GFXVALUE rhw;
	GFXCOLOR color;
	GFXCOLOR specular;
	GFXVALUE tu;
	GFXVALUE tv;
	GFXVALUE tx;
	GFXVALUE ty;
};

struct ROOM_INFO {
	int16_t* data;
	int16_t* door;
	FLOOR_INFO* floor;
	LIGHTINFO* light;
	MESH_INFO* mesh;
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t minfloor;
	int32_t maxceiling;
	int16_t x_size;
	int16_t y_size;
	int32_t ambient;
	int16_t num_lights;
	int16_t num_meshes;
	uint8_t ReverbType;
	uint8_t FlipNumber;
	int8_t MeshEffect;
	int8_t bound_active;
	int16_t left;
	int16_t right;
	int16_t top;
	int16_t bottom;
	int16_t test_left;
	int16_t test_right;
	int16_t test_top;
	int16_t test_bottom;
	int16_t item_number;
	int16_t fx_number;
	int16_t flipped_room;
	uint16_t flags;
	int32_t nVerts;
	int32_t nWaterVerts;
	int32_t nShoreVerts;
	GFXVERTEX *Buffer = nullptr;
	int16_t* FaceData;
	float posx;
	float posy;
	float posz;
	GFXVECTOR *vnormals;
	GFXVECTOR *fnormals;
	int32_t *prelight;
	int32_t *prelightwater;
	int32_t watercalc;
	GFXVECTOR *verts;
	int32_t gt3cnt;
	int32_t gt4cnt;
	PCLIGHT_INFO *pclight;
};

struct ANIM_STRUCT {
	int16_t* frame_ptr;
	int16_t interpolation;
	int16_t current_anim_state;
	int32_t velocity;
	int32_t acceleration;
	int32_t Xvelocity;
	int32_t Xacceleration;
	int16_t frame_base;
	int16_t frame_end;
	int16_t jump_anim_num;
	int16_t jump_frame_num;
	int16_t number_changes;
	int16_t change_index;
	int16_t number_commands;
	int16_t command_index;
};

#define MAX_ROPE_SEGMENTS 24
#define MAX_ROPE_COORDS 3

struct ROPE_STRUCT {
	PHD_VECTOR Segment[MAX_ROPE_SEGMENTS];
	PHD_VECTOR Velocity[MAX_ROPE_SEGMENTS];
	PHD_VECTOR NormalisedSegment[MAX_ROPE_SEGMENTS];
	PHD_VECTOR MeshSegment[MAX_ROPE_SEGMENTS];
	PHD_VECTOR Position;
	int32_t Coords[MAX_ROPE_SEGMENTS][MAX_ROPE_COORDS];
	int32_t SegmentLength;
	int32_t Active;
};

struct PENDULUM {
	PHD_VECTOR Position;
	PHD_VECTOR Velocity;
	int32_t node;
	ROPE_STRUCT* Rope;
};

struct STATS {
	uint32_t Timer;
	uint32_t Distance;
	uint32_t AmmoUsed;
	uint32_t AmmoHits;
	uint16_t Kills;
	uint8_t Secrets;
	uint8_t HealthUsed;
};

#define SAVEGAME_BUFFER_SIZE 15410

struct LEGACY_SAVEGAME_LARA_ARM {
	X32_POINTER frame_base; // Pointer
	int16_t frame_number;
	int16_t anim_number;
	int16_t lock;
	int16_t y_rot;
	int16_t x_rot;
	int16_t z_rot;
	int16_t flash_gun;
};

struct LEGACY_SAVEGAME_LARA_INFO {
	int16_t item_number;
	int16_t gun_status;
	int16_t gun_type;
	int16_t request_gun_type;
	int16_t last_gun_type;
	int16_t calc_fallspeed;
	int16_t water_status;
	int16_t climb_status;
	int16_t pose_count;
	int16_t hit_frame;
	int16_t hit_direction;
	int16_t air;
	int16_t dive_count;
	int16_t death_count;
	int16_t current_active;
	int16_t current_xvel;
	int16_t current_yvel;
	int16_t current_zvel;
	int16_t spaz_effect_count;
	int16_t flare_age;
	int16_t vehicle;
	int16_t weapon_item;
	int16_t back_gun;
	int16_t flare_frame;
	int16_t poisoned;
	int16_t dpoisoned;
	uint8_t electric;
	uint8_t wet[WET_COUNT];
	uint16_t flare_control_left : 1;
	uint16_t Unused1 : 1;
	uint16_t look : 1;
	uint16_t burn : 1;
	uint16_t keep_ducked : 1;
	uint16_t IsMoving : 1;
	uint16_t CanMonkeySwing : 1;
	uint16_t Unused2 : 1;
	uint16_t OnBeetleFloor : 1;
	uint16_t BurnGreen : 1;
	uint16_t IsDucked : 1;
	uint16_t has_fired : 1;
	uint16_t Busy : 1;
	uint16_t LitTorch : 1;
	uint16_t IsClimbing : 1;
	uint16_t Fired : 1;
	int32_t water_surface_dist;
	PHD_VECTOR last_pos;
	X32_POINTER spaz_effect;
	int32_t mesh_effects;
	X32_POINTER mesh_ptrs[LARA_MESH_PTR_COUNT];
	X32_POINTER target;
	int16_t target_angles[2];
	int16_t turn_rate;
	int16_t move_angle;
	int16_t head_y_rot;
	int16_t head_x_rot;
	int16_t head_z_rot;
	int16_t torso_y_rot;
	int16_t torso_x_rot;
	int16_t torso_z_rot;
	LEGACY_SAVEGAME_LARA_ARM left_arm;
	LEGACY_SAVEGAME_LARA_ARM right_arm;
	uint16_t holster;
	X32_POINTER creature;
	X32_POINTER CornerX; // 32/64 bit
	X32_POINTER CornerZ; // 32/64 bit
	int8_t RopeSegment;
	int8_t RopeDirection;
	int16_t RopeArcFront;
	int16_t RopeArcBack;
	int16_t RopeLastX;
	int16_t RopeMaxXForward;
	int16_t RopeMaxXBackward;
	int32_t RopeDFrame;
	int32_t RopeFrame;
	uint16_t RopeFrameRate;
	uint16_t RopeY;
	uint32_t RopePtr;
	uint32_t GeneralPtr;
	int32_t RopeOffset;
	uint32_t RopeDownVel;
	int8_t RopeFlag;
	int8_t MoveCount;
	int32_t RopeCount;
	int8_t pistols_type_carried;
	int8_t uzis_type_carried;
	int8_t shotgun_type_carried;
	int8_t crossbow_type_carried;
	int8_t grenade_type_carried;
	int8_t sixshooter_type_carried;
	int8_t lasersight;
	int8_t binoculars;
	int8_t crowbar;
	int8_t mechanical_scarab;
	uint8_t small_water_skin;
	uint8_t big_water_skin;
	int8_t examine1;
	int8_t examine2;
	int8_t examine3;
	int8_t puzzleitems[12];
	uint16_t puzzleitemscombo;
	uint16_t keyitems;
	uint16_t keyitemscombo;
	uint16_t pickupitems;
	uint16_t pickupitemscombo;
	int16_t questitems;
	int16_t num_small_medipack;
	int16_t num_large_medipack;
	int16_t num_flares;
	int16_t num_pistols_ammo;
	int16_t num_uzi_ammo;
	int16_t num_revolver_ammo;
	int16_t num_shotgun_ammo1;
	int16_t num_shotgun_ammo2;
	int16_t num_grenade_ammo1;
	int16_t num_grenade_ammo2;
	int16_t num_grenade_ammo3;
	int16_t num_crossbow_ammo1;
	int16_t num_crossbow_ammo2;
	int16_t num_crossbow_ammo3;
	int8_t beetle_uses;
	int8_t blindTimer;
	int8_t location;
	int8_t highest_location;
	int8_t locationPad;
};

#define MAX_HUB_LEVELS 10

struct LEGACY_SAVEGAME_INFO {
	LEGACY_SAVEGAME_LARA_INFO Lara;
	int32_t cutscene_triggered;
	uint8_t HubLevels[MAX_HUB_LEVELS];	//saved level indices. highest one that isn't 0 is the one we are currently in
	uint16_t HubOffsets[MAX_HUB_LEVELS];	//offset of each level's data inside the savegame buffer
	uint16_t HubSizes[MAX_HUB_LEVELS];	//size of each level's data inside the savegame buffer
	int8_t CurrentLevel;
	int8_t Checksum;
	STATS Game;
	STATS Level;
	int16_t WeaponObject;
	int16_t WeaponAnim;
	int16_t WeaponFrame;
	int16_t WeaponCurrent;
	int16_t WeaponGoal;
	CVECTOR fog_colour;
	uint8_t HubSavedLara : 1;	//flag that we saved Lara's data when we initialised hub, only set to 1 when InitialiseHub is called with 1
	uint8_t AutoTarget : 1;
	uint8_t HaveBikeBooster : 1;	//have the bike nitro thing
	char buffer[SAVEGAME_BUFFER_SIZE];
};

struct BIKEINFO {
	int16_t right_front_wheelrot;
	int16_t right_back_wheelrot;
	int32_t left_wheelrot;
	int32_t velocity;
	int32_t unused1;
	int32_t pitch1;
	int16_t move_angle;
	int16_t extra_rotation;
	int16_t rot_thing;
	int32_t bike_turn;
	int32_t pitch2;
	int16_t flags;
	int16_t light_intensity;
};

struct SPARKS {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t Xvel;
	int16_t Yvel;
	int16_t Zvel;
	int16_t Gravity;
	int16_t RotAng;
	int16_t Flags;
	uint8_t sSize;
	uint8_t dSize;
	uint8_t Size;
	uint8_t Friction;
	uint8_t Scalar;
	uint8_t Def;
	int8_t RotAdd;
	int8_t MaxYvel;
	uint8_t On;
	uint8_t sR;
	uint8_t sG;
	uint8_t sB;
	uint8_t dR;
	uint8_t dG;
	uint8_t dB;
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t ColFadeSpeed;
	uint8_t FadeToBlack;
	uint8_t sLife;
	uint8_t Life;
	uint8_t TransType;
	uint8_t extras;
	int8_t Dynamic;
	uint8_t FxObj;
	uint8_t RoomNumber;
	uint8_t NodeNumber;
};

struct STATIC_INFO {
	int16_t mesh_number;
	int16_t flags;
	int16_t x_minp;
	int16_t x_maxp;
	int16_t y_minp;
	int16_t y_maxp;
	int16_t z_minp;
	int16_t z_maxp;
	int16_t x_minc;
	int16_t x_maxc;
	int16_t y_minc;
	int16_t y_maxc;
	int16_t z_minc;
	int16_t z_maxc;
};

struct DXPTR {
#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
	LPDIRECTSOUND8 lpDS;
	IXAudio2* lpXA;
#endif
	uint32_t dwRenderWidth;
	uint32_t dwRenderHeight;
	int32_t Flags;

#ifdef _WIN32
	uint32_t WindowStyle;
#endif

#ifdef _WIN32
	HWND hWnd;
#endif
	volatile int32_t InScene;
	volatile int32_t WaitAtBeginScene;
	volatile int32_t DoneBlit;
};

struct DXDISPLAYMODE {
	int32_t w;
	int32_t h;
	int32_t bpp;
	int32_t RefreshRate;
	int32_t bPalette;
	uint8_t rbpp;
	uint8_t gbpp;
	uint8_t bbpp;
	uint8_t rshift;
	uint8_t gshift;
	uint8_t bshift;
};

struct DXTEXTUREINFO {
	uint32_t bpp;
	int32_t bPalette;
	int32_t bAlpha;
	uint8_t rbpp;
	uint8_t gbpp;
	uint8_t bbpp;
	uint8_t abpp;
	uint8_t rshift;
	uint8_t gshift;
	uint8_t bshift;
	uint8_t ashift;
};

struct DXZBUFFERINFO {
	uint32_t bpp;
};

struct DXD3DDEVICE {
	char Name[30];
	char About[80];
};

struct DXDIRECTDRAWINFO {
#ifdef UNICODE
	wchar_t Name[30];
	wchar_t About[80];
#else
	char Name[30];
	char About[80];
#endif
	int32_t nDisplayModes;
	DXDISPLAYMODE* DisplayModes;
	int32_t nD3DDevices;
	DXD3DDEVICE* D3DDevices;
};

struct DXDIRECTSOUNDINFO {
	char Name[30];
	char About[80];
#ifdef _WIN32
	LPGUID lpGuid;
	GUID Guid;
#endif
};

struct DXINFO {
	int32_t nDDInfo;
	int32_t nDSInfo;
	DXDIRECTDRAWINFO* DDInfo;
	DXDIRECTSOUNDINFO* DSInfo;
	int32_t nDD;
	int32_t nD3D;
	int32_t screenW;
	int32_t screenH;
	int32_t nTexture;
	int32_t nZBuffer;
	int32_t nDS;
	bool bHardware;
};

struct WINAPP {
#ifdef _WIN32
	HINSTANCE hInstance;
	HWND hWnd;
	WNDCLASS WindowClass;
#endif
	DXINFO DXInfo;
	DXPTR dx;
#ifdef _WIN32
	HANDLE mutex;
#endif
	float fps;

#ifdef _WIN32
	HACCEL hAccel;
#endif
	bool SetupComplete;
	bool BumpMapping;
	int32_t TextureSize;
	int32_t BumpMapSize;
	bool Filtering;
	bool Volumetric;
	bool SoundDisabled;
	int32_t StartFlags;
	volatile bool fmv;
	int32_t Desktopbpp;
	int32_t AutoTarget;
	int32_t VideoWidth;
	int32_t VideoHeight;
};

struct SPRITESTRUCT {
	uint16_t tpage;
	uint16_t offset;
	uint16_t width;
	uint16_t height;
	float x1;	//left
	float y1;	//top
	float x2;	//right
	float y2;	//bottom
};

struct MESH_DATA {
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t r;
	int16_t flags;
	int16_t nVerts;
	int16_t nNorms;
	uint16_t ngt4; // TRLE: Made unsigned, fixes some level loading
	int16_t* gt4;
	uint16_t ngt3; // TRLE: Made unsigned, fixes some level loading
	int16_t* gt3;
	int32_t* prelight;
	GFXVERTEX *Buffer = nullptr;
	GFXVECTOR* Normals;
};

struct TEXTURESTRUCT {
	uint16_t drawtype;
	uint16_t tpage;
	uint16_t flag;
	float u1;
	float v1;
	float u2;
	float v2;
	float u3;
	float v3;
	float u4;
	float v4;
};

struct LIGHTNING_STRUCT {
	PHD_VECTOR Point[4];
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t Life;
	int8_t Xvel1;
	int8_t Yvel1;
	int8_t Zvel1;
	int8_t Xvel2;
	int8_t Yvel2;
	int8_t Zvel2;
	int8_t Xvel3;
	int8_t Yvel3;
	int8_t Zvel3;
	uint8_t Size;
	uint8_t Flags;
	uint8_t Rand;
	uint8_t Segments;
	uint8_t Pad[3];
};

typedef struct SNOWFLAKE {
	int32_t x;
	int32_t y;
	int32_t z;
	int8_t xv;
	uint8_t yv;
	int8_t zv;
	uint8_t life;
	int16_t stopped;
	int16_t room_number;
} RAINDROPS, UWEFFECTS;

struct DYNAMIC {
	int32_t x;
	int32_t y;
	int32_t z;
	uint8_t on;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint16_t falloff;
	uint8_t used;
	uint8_t pad1[1];
	int32_t FalloffScale;
};

struct INVOBJ {
	int16_t object_number;
	int16_t yoff;
	int16_t scale1;
	int16_t yrot;
	int16_t xrot;
	int16_t zrot;
	int16_t flags;
	int16_t objname;
	uint32_t meshbits;
};

struct MENUTHANG {
	int32_t type;
	char* text;
};

struct AMMOLIST {
	int16_t invitem;
	int16_t amount;
	uint16_t yrot;
};

struct OBJLIST {
	int16_t invitem;
	uint16_t yrot;
	uint16_t bright;
};

struct INVDRAWITEM {
	int16_t xrot;
	int16_t yrot;
	int16_t zrot;
	int16_t object_number;
	uint32_t mesh_bits;
};

struct RINGME {
	OBJLIST current_object_list[119];
	int32_t ringactive;
	int32_t objlistmovement;
	int32_t curobjinlist;
	int32_t numobjectsinlist;
};

struct COMBINELIST {
	void(*combine_routine)(int32_t flag);
	int16_t item1;
	int16_t item2;
	int16_t combined_item;
};

struct CUTSEQ_ROUTINES {
	void(*init_func)();
	void(*control_func)();
	void(*end_func)();
};

struct ACTORME {
	int32_t offset;
	int16_t objslot;
	int16_t nodes;
};

struct NEW_CUTSCENE {
	int16_t numactors;
	int16_t numframes;
	int32_t orgx;
	int32_t orgy;
	int32_t orgz;
	int32_t audio_track;
	int32_t camera_offset;
	ACTORME actor_data[10];
};

struct RTDECODE {
	uint32_t length;
	uint32_t off;
	uint16_t counter;
	uint16_t data;
	uint8_t decodetype;
	uint8_t packmethod;
	uint16_t pad;
};

struct PACKNODE {
	int16_t xrot_run;
	int16_t yrot_run;
	int16_t zrot_run;
	int16_t xkey;
	int16_t ykey;
	int16_t zkey;
	RTDECODE decode_x;
	RTDECODE decode_y;
	RTDECODE decode_z;
	uint32_t xlength;
	uint32_t ylength;
	uint32_t zlength;
	char* xpacked;
	char* ypacked;
	char* zpacked;
};

struct NODELOADHEADER {
	int16_t xkey;
	int16_t ykey;
	int16_t zkey;
	int16_t packmethod;
	int16_t xlength;
	int16_t ylength;
	int16_t zlength;
};

struct HAIR_STRUCT {
	PHD_3DPOS pos;
	PHD_VECTOR vel;
};

struct SORTLIST {
	float zVal;
	int16_t drawtype;
	int16_t tpage;
	int16_t nVtx;
	int16_t polytype;
};

struct WATERTAB {
	int8_t shimmer;
	int8_t choppy;
	uint8_t random;
	uint8_t abs;
};

struct FOOTPRINT {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t YRot;
	int16_t Active;
};

struct DISPLAYPU {
	int16_t life;
	int16_t object_number;
};

struct GUNSHELL_STRUCT {
	PHD_3DPOS pos;
	int16_t fallspeed;
	int16_t room_number;
	int16_t speed;
	int16_t counter;
	int16_t DirXrot;
	int16_t object_number;
};

struct BITE_INFO {
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t mesh_num;
};

struct TEXTURE {
	bgfx::TextureHandle tex;
	uint32_t xoff;
	uint32_t yoff;
	uint32_t width;
	uint32_t height;
	int32_t tpage;
	bool bump;
	int32_t bumptpage;
};

struct TEXTUREBUCKET {
	int32_t tpage;
	int32_t nVtx;
	GFXTLBUMPVERTEX vtx[BUCKET_VERT_COUNT]; // TRLE: increased size (256 * 32 + 32)
	bgfx::DynamicVertexBufferHandle handle = BGFX_INVALID_HANDLE;
};

struct THREAD {
	volatile int32_t active;
	int32_t unk;
	volatile int32_t ended;
	SDL_Thread *handle;
};

struct DRIP_STRUCT {
	int32_t x;
	int32_t y;
	int32_t z;
	uint8_t On;
	uint8_t R;
	uint8_t G;
	uint8_t B;
	int16_t Yvel;
	uint8_t Gravity;
	uint8_t Life;
	int16_t RoomNumber;
	uint8_t Outside;
	uint8_t Pad;
};

struct AI_INFO {
	int16_t zone_number;
	int16_t enemy_zone;
	int32_t distance;
	int32_t ahead;
	int32_t bite;
	int16_t angle;
	int16_t x_angle;
	int16_t enemy_facing;
};

struct AIOBJECT {
	int16_t object_number;
	int16_t room_number;
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t trigger_flags;
	int16_t flags;
	int16_t y_rot;
	int16_t box_number;
};

struct OLD_CAMERA {
	int16_t current_anim_state;
	int16_t goal_anim_state;
	int32_t target_distance;
	int16_t target_angle;
	int16_t target_elevation;
	int16_t actual_elevation; // T4Plus
	PHD_3DPOS pos;
	PHD_3DPOS pos2;
	PHD_VECTOR t;
};

struct SHATTER_ITEM {
	SPHERE Sphere;
	ITEM_LIGHT* il;
	int16_t* meshp;
	int32_t Bit;
	int16_t YRot;
	int16_t Flags;
};

struct SPOTCAM {
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t tx;
	int32_t ty;
	int32_t tz;
	uint8_t sequence;
	uint8_t camera;
	int16_t fov;
	int16_t roll;
	int16_t timer;
	int16_t speed;
	int16_t flags;
	int16_t room_number;
	int16_t pad;
};

struct WRAITH_STRUCT {
	PHD_VECTOR pos;
	int16_t xv;
	int16_t yv;
	int16_t zv;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t pad[3];
};

struct LOCUST_STRUCT {
	PHD_3DPOS pos;
	int16_t room_number;
	int16_t speed;
	int16_t Counter;
	int16_t LaraTarget;
	int8_t XTarget;
	int8_t ZTarget;
	uint8_t On;
	uint8_t flags;
};

struct DOORPOS_DATA {
	FLOOR_INFO* floor;
	FLOOR_INFO data;
	int16_t block;
};

struct DOOR_DATA {
	DOORPOS_DATA d1;
	DOORPOS_DATA d1flip;
	DOORPOS_DATA d2;
	DOORPOS_DATA d2flip;
	int16_t Opened;
};

struct BOX_INFO {
	uint8_t left;
	uint8_t right;
	uint8_t top;
	uint8_t bottom;
	int16_t height;
	int16_t overlap_index;
};

struct SMOKE_SPARKS {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t Xvel;
	int16_t Yvel;
	int16_t Zvel;
	int16_t Gravity;
	int16_t RotAng;
	int16_t Flags;
	uint8_t sSize;
	uint8_t dSize;
	uint8_t Size;
	uint8_t Friction;
	uint8_t Scalar;
	uint8_t Def;
	int8_t RotAdd;
	int8_t MaxYvel;
	uint8_t On;
	uint8_t sShade;
	uint8_t dShade;
	uint8_t Shade;
	uint8_t ColFadeSpeed;
	uint8_t FadeToBlack;
	int8_t sLife;
	int8_t Life;
	uint8_t TransType;
	uint8_t FxObj;
	uint8_t NodeNumber;
	uint8_t mirror;
};

struct MONOSCREEN_STRUCT {
	bgfx::TextureHandle tex;
};

struct VonCroyCutData {
	PHD_VECTOR CameraPos;
	PHD_VECTOR CameraTarget;
	int32_t f;
};

struct DEBRIS_STRUCT {
	void* TextInfo;
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t XYZOffsets1[3];
	int16_t Dir;
	int16_t XYZOffsets2[3];
	int16_t Speed;
	int16_t XYZOffsets3[3];
	int16_t Yvel;
	int16_t Gravity;
	int16_t RoomNumber;
	uint8_t On;
	uint8_t XRot;
	uint8_t YRot;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t Pad[2];
	int32_t color1;
	int32_t color2;
	int32_t color3;
	int32_t ambient;
	int32_t flags;
};

struct JEEPINFO {
	int16_t right_front_wheelrot;
	int16_t left_front_wheelrot;
	int16_t left_back_wheelrot;
	int16_t right_back_wheelrot;
	int32_t velocity;
	int32_t unused1;
	int32_t pitch1;
	int32_t turn_rate;
	int32_t camera_angle;
	int16_t move_angle;
	int16_t extra_rotation;
	int16_t rot_thing;
	int32_t pitch2;
	int16_t flags;
	int16_t unused2;
	int16_t gear;
};

struct PISTOL_DEF {
	int16_t (*ObjectFunc)();
	int8_t Draw1Anim2;
	int8_t Draw1Anim;
	int8_t Draw2Anim;
	int8_t RecoilAnim;
};

struct BINK_STRUCT {
	int32_t first_pad;
	int32_t num;
	int8_t second_pad[8];
	int32_t num2;
};

struct LEGACY_SAVEFILE_INFO {
	char name[75];
	int8_t valid;
	int16_t hours;
	int16_t minutes;
	int16_t seconds;
	int16_t days;
	int32_t num;
};

struct COMMANDLINES {
	char command[20];
	bool needs_parameter;
	void (*code)(char*);
	char parameter[PARAMETER_MAX_LENGTH];
};

struct CHANGE_STRUCT {
	int16_t goal_anim_state;
	int16_t number_ranges;
	int16_t range_index;
};

struct RANGE_STRUCT {
	int16_t start_frame;
	int16_t end_frame;
	int16_t link_anim_num;
	int16_t link_frame_num;
};

struct PHDSPRITESTRUCT {
	uint16_t tpage;
	uint16_t offset;
	uint16_t width;
	uint16_t height;
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
};

struct PHDTEXTURESTRUCT {
	uint16_t drawtype;
	uint16_t tpage;
	uint16_t flag;
	uint16_t u1;
	uint16_t v1;
	uint16_t u2;
	uint16_t v2;
	uint16_t u3;
	uint16_t v3;
	uint16_t u4;
	uint16_t v4;
	uint32_t xoff;
	uint32_t yoff;
	uint32_t width;
	uint32_t height;
};

struct SAMPLE_INFO {
	int16_t number;
	uint8_t volume;
	uint8_t radius;
	uint8_t randomness;
	int8_t pitch;
	int16_t flags;
};

#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
struct DS_SAMPLE {
	LPDIRECTSOUNDBUFFER buffer;
	int32_t frequency;
	int32_t playing;
};
#endif

struct BUBBLE_STRUCT {
	PHD_VECTOR pos;
	int16_t room_number;
	int16_t speed;
	int16_t size;
	int16_t dsize;
	uint8_t shade;
	uint8_t vel;
	int16_t pad;
};

struct SHOCKWAVE_STRUCT {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t InnerRad;
	int16_t OuterRad;
	int16_t XRot;
	int16_t Flags;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t life;
	int16_t Speed;
	int16_t Temp;
};

struct SPLASH_STRUCT {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t InnerRad;
	int16_t InnerSize;
	int16_t InnerRadVel;
	int16_t InnerYVel;
	int16_t InnerY;
	int16_t MiddleRad;
	int16_t MiddleSize;
	int16_t MiddleRadVel;
	int16_t MiddleYVel;
	int16_t MiddleY;
	int16_t OuterRad;
	int16_t OuterSize;
	int16_t OuterRadVel;
	int8_t flags;
	uint8_t life;
};

struct RIPPLE_STRUCT {
	int32_t x;
	int32_t y;
	int32_t z;
	int8_t flags;
	uint8_t life;
	uint8_t size;
	uint8_t init;
};

struct FIRE_SPARKS {
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t Xvel;
	int16_t Yvel;
	int16_t Zvel;
	int16_t Gravity;
	int16_t RotAng;
	int16_t Flags;
	uint8_t sSize;
	uint8_t dSize;
	uint8_t Size;
	uint8_t Friction;
	uint8_t Scalar;
	uint8_t Def;
	int8_t RotAdd;
	int8_t MaxYvel;
	uint8_t On;
	uint8_t sR;
	uint8_t sG;
	uint8_t sB;
	uint8_t dR;
	uint8_t dG;
	uint8_t dB;
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t ColFadeSpeed;
	uint8_t FadeToBlack;
	uint8_t sLife;
	uint8_t Life;
};

struct BLOOD_STRUCT {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t Xvel;
	int16_t Yvel;
	int16_t Zvel;
	int16_t Gravity;
	int16_t RotAng;
	uint8_t sSize;
	uint8_t dSize;
	uint8_t Size;
	uint8_t Friction;
	int8_t RotAdd;
	uint8_t On;
	uint8_t sShade;
	uint8_t dShade;
	uint8_t Shade;
	uint8_t ColFadeSpeed;
	uint8_t FadeToBlack;
	int8_t sLife;
	int8_t Life;
	int8_t Pad;
};

struct WATER_DUST {
	PHD_VECTOR pos;
	int8_t xvel;
	uint8_t yvel;
	int8_t zvel;
	uint8_t life;
};

struct CHARDEF {
	float u;
	float v;
	int16_t w;
	int16_t h;
	int16_t y_offset;
	int8_t top_shade;
	int8_t bottom_shade;
};

struct STRINGHEADER {
	uint16_t nStrings;
	uint16_t nPSXStrings;
	uint16_t nPCStrings;
	uint16_t StringWadLen;
	uint16_t PSXStringWadLen;
	uint16_t PCStringWadLen;
};

struct GUNFLASH_STRUCT {
	float mx[12];
	int16_t on;
};

struct FIRE_LIST {
	int32_t x;
	int32_t y;
	int32_t z;
	int8_t on;
	int8_t size;
	int16_t room_number;
};

struct SoundSlot {
	int32_t OrigVolume;
	int32_t nVolume;
	int32_t nPan;
	int32_t nPitch;
	int32_t nSampleInfo;
	uint32_t distance;
	PHD_VECTOR pos;
};

struct WEAPON_INFO {
	int16_t lock_angles[4];
	int16_t left_angles[4];
	int16_t right_angles[4];
	int16_t aim_speed;
	int16_t shot_accuracy;
	int16_t gun_height;
	int16_t target_dist;
	int8_t damage;
	int8_t recoil_frame;
	int8_t flash_time;
	int8_t draw_frame;
	int16_t sample_num;
};

struct SCARAB_STRUCT {
	PHD_3DPOS pos;
	int16_t room_number;
	int16_t speed;
	int16_t fallspeed;
	uint8_t On;
	uint8_t flags;
};

struct SPLASH_SETUP {
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t InnerRad;
	int16_t InnerSize;
	int16_t InnerRadVel;
	int16_t InnerYVel;
	int16_t pad1;
	int16_t MiddleRad;
	int16_t MiddleSize;
	int16_t MiddleRadVel;
	int16_t MiddleYVel;
	int16_t pad2;
	int16_t OuterRad;
	int16_t OuterSize;
	int16_t OuterRadVel;
	int16_t pad3;
};

struct SP_DYNAMIC {
	uint8_t On;
	uint8_t Falloff;
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t Flags;
	uint8_t Pad[2];
};

struct NODEOFFSET_INFO {
	int16_t x;
	int16_t y;
	int16_t z;
	int8_t mesh_num;
	uint8_t GotIt;
};

struct TRAIN_STATIC {
	int16_t type;
	int16_t zoff;
};

struct ROOM_DYNAMIC {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float falloff;
	float sqr_falloff;
	float inv_falloff;
};

struct SUNLIGHT_STRUCT {
	FVECTOR vec;
	float r;
	float g;
	float b;
};

struct POINTLIGHT_STRUCT {
	FVECTOR vec;
	float r;
	float g;
	float b;
	float rad;
};

struct MESH_MAP_TABLE_ENTRY {
	uint32_t mesh_x32_ptr;
	uint32_t mesh_native_ptr;
};

struct GouraudBarColourSet {
	uint8_t abLeftRed[5];
	uint8_t abLeftGreen[5];
	uint8_t abLeftBlue[5];
	uint8_t abRightRed[5];
	uint8_t abRightGreen[5];
	uint8_t abRightBlue[5];
};

struct COLOR_BIT_MASKS {
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwRGBAlphaBitMask;
	uint32_t dwRBitDepth;
	uint32_t dwGBitDepth;
	uint32_t dwBBitDepth;
	uint32_t dwRGBAlphaBitDepth;
	uint32_t dwRBitOffset;
	uint32_t dwGBitOffset;
	uint32_t dwBBitOffset;
	uint32_t dwRGBAlphaBitOffset;
};

enum shadow_mode_enum {
	SHADOW_MODE_NULL = 0,
	SHADOW_MODE_ORIGINAL,
	SHADOW_MODE_CIRCLE,
	SHADOW_MODE_PSX_CIRCLE,
	SHADOW_MODE_PSX_SPRITE,
	SHADOW_MODE_ENUM_SIZE
};


enum bar_mode_enum {
	BAR_MODE_NULL,
	BAR_MODE_ORIGINAL,
	BAR_MODE_IMPROVED,
	BAR_MODE_PSX,
	BAR_MODE_CUSTOM,
	BAR_MODE_ENUM_SIZE
};

enum bars_pos_enum {
	BARS_POS_NULL,
	BARS_POS_ORIGINAL,
	BARS_POS_IMPROVED,
	BARS_POS_PSX,
	BARS_POS_CUSTOM,
	BARS_POS_ENUM_SIZE
};

enum inv_bg_mode_enum {
	INV_BG_MODE_NULL,
	INV_BG_MODE_ORIGINAL,
	INV_BG_MODE_TR5,
	INV_BG_MODE_CLEAR,
	INV_BG_MODE_ENUM_SIZE
};

enum look_transparency_enum {
	LOOK_TRANSPARENCY_NULL,
	LOOK_TRANSPARENCY_DEFAULT,
	LOOK_TRANSPARENCY_ON,
	LOOK_TRANSPARENCY_OFF,
	LOOK_TRANSPARENCY_ENUM_SIZE
};

enum reverb_enum {
	REVERB_NULL,
	REVERB_OFF,
	REVERB_LARA_ROOM,
	REVERB_CAMERA_ROOM,
	REVERB_ENUM_SIZE
};

enum pickup_lighting_enum {
	PICKUP_LIGHTING_NULL,
	PICKUP_LIGHTING_DEFAULT,
	PICKUP_LIGHTING_OFF,
	PICKUP_LIGHTING_ON,
	PICKUP_LIGHTING_ENUM_SIZE
};

enum volumetric_flash_grenades_enum {
	VOLUMETRIC_FLASH_GRENADES_NULL,
	VOLUMETRIC_FLASH_GRENADES_DEFAULT,
	VOLUMETRIC_FLASH_GRENADES_FLASH_ONLY,
	VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_ONLY,
	VOLUMETRIC_FLASH_GRENADES_VOLUMETRIC_AND_FLASH,
	VOLUMETRIC_FLASH_GRENADES_ENUM_SIZE
};

struct tomb4_options {	//keep this at the bottom of the file, please
	bool footprints;
	shadow_mode_enum shadow_mode;				//1-> original, 2-> circle, 3-> PSX like circle, 4-> PSX sprite, 5-> dynamic
	bool crawltilt;
	bool flexible_crawling;
	bool fix_climb_up_delay;
	bool gameover;
	bar_mode_enum bar_mode;						//1-> original, 2-> TR5, 3-> PSX, 4-> Custom
	bars_pos_enum bars_pos;						//1-> original, 2-> improved, 3-> PSX, 4-> Custom
	bool enemy_bars;
	bool cutseq_skipper;
	bool cheats;
	bool loadingtxt;
	inv_bg_mode_enum inv_bg_mode;				//1-> original, 2->TR5, 3-> clear
	bool tr5_loadbar;
	look_transparency_enum look_transparency;
	bool ammo_counter;
	bool ammotype_hotkeys;
	bool combat_cam_tilt;
	bool hpbar_inv;
	bool static_lighting;
	reverb_enum reverb;							//1-> off, 2-> Lara room, 3->camera room
	uint32_t minimum_clip_range;				//value in blocks
	float GUI_Scale;
	bool hang_game_thread;
	pickup_lighting_enum pickup_lighting;		// Chronicles-style shading for inventory objects.
	volumetric_flash_grenades_enum volumetric_flash_grenades;
};

#define VANILLA_ITEM_COUNT 256

#ifdef LEVEL_EDITOR
#define ITEM_COUNT 6000 // TRLE: bumped from 256
#else
#define ITEM_COUNT VANILLA_ITEM_COUNT
#endif

#pragma pack(pop)