#pragma once

#include "../sound.h"

#define NG_MAX_FLOORSTATE_ACTIONS 128

#define NG_MAX_TRIGGERED_ITEMS 128
extern bool ng_loaded_savegame;

extern short ng_camera_target_id;

struct NG_LARA_EXTRASTATE {
	uchar TightRopeOnCount;
	uchar TightRopeOff;
	uchar TightRopeFall;
};

extern NG_LARA_EXTRASTATE ng_lara_extrastate;

enum NGTimerTrackerType {
	TTT_ONLY_SHOW_SECONDS = 0,
	TTT_SECONDS_AND_ONE_DECIMAL_POINT_SEPERATOR,
	TTT_SECONDS_AND_TWO_DECIMAL_POINT_SEPERATOR,
	TTT_SECONDS_AND_ONE_DECIMAL_COLON_SEPERATOR,
	TTT_SECONDS_AND_TWO_DECIMAL_COLON_SEPERATOR,
	TTT_SECONDS_WITH_THREE_NOUGHTS
};

#define MAX_NG_GLOBAL_TRIGGERS 9999
#define MAX_NG_TRIGGER_GROUPS 9999
#define MAX_NG_ORGANIZERS 4999
#define MAX_NG_ITEM_GROUPS 999
#define MAX_NG_ANIMATIONS 9999
#define MAX_NG_MULTI_ENV_CONDITIONS 9999
#define MAX_NG_TEST_POSITIONS 9999

struct NG_GLOBAL_TRIGGER_STATE {
	bool is_disabled = false;
	bool is_halted = false;
};

struct NG_TRIGGER_GROUP_STATE {
	bool continuous = false;
	bool one_shot = false;
};

struct NG_ORGANIZER_STATE {
	bool is_enabled = false;
	int current_tick = 0;
};

extern NG_GLOBAL_TRIGGER_STATE ng_global_trigger_states[MAX_NG_GLOBAL_TRIGGERS];
extern NG_TRIGGER_GROUP_STATE ng_trigger_group_states[MAX_NG_TRIGGER_GROUPS];
extern NG_ORGANIZER_STATE ng_organizer_states[MAX_NG_ORGANIZERS];

extern int ng_looped_sound_state[NumSamples];

extern bool ng_lara_infinite_air;

// Timers
enum NGTimerPosition {
	NG_TIMER_POSITION_INVISIBLE = 0,
	NG_TIMER_POSITION_BOTTOM_CENTER,
	NG_TIMER_POSITION_TOP_CENTER,
	NG_TIMER_POSITION_CENTER_CENTER,
	NG_TIMER_POSITION_TOP_LEFT,
	NG_TIMER_POSITION_TOP_RIGHT,
	NG_TIMER_POSITION_BOTTOM_LEFT,
	NG_TIMER_POSITION_BOTTOM_RIGHT,
	NG_TIMER_POSITION_DOWN_DAMAGE_BAR,
	NG_TIMER_POSITION_DOWN_COLD_BAR,
	NG_TIMER_POSITION_DOWN_LEFT_BARS,
	NG_TIMER_POSITION_DOWN_RIGHT_BARS,
};

extern int ng_animation_current_animation;
extern short ng_animation_prev_hands_state;
extern int ng_animation_target_item;
extern int ng_animation_target_test_position;

extern int ng_global_timer;
extern char ng_global_timer_frame_increment;
extern NGTimerPosition ng_global_timer_position;
extern int ng_global_timer_time_until_hide;

extern int ng_local_timer;
extern char ng_local_timer_frame_increment;
extern NGTimerPosition ng_local_timer_position;
extern int ng_local_timer_time_until_hide;

// Level
extern int pending_level_load_timer;
extern int pending_level_load_id;

// Variables
extern int ng_current_value;
extern int ng_global_alfa;
extern int ng_global_beta;
extern int ng_global_delta;
extern int ng_local_alfa;
extern int ng_local_beta;
extern int ng_local_delta;
extern int ng_last_input_number;

// Inventory
extern unsigned char ng_selected_inventory_item_memory;
extern int ng_used_inventory_object_for_frame;
extern bool ng_used_large_medipack;
extern bool ng_used_small_medipack;

extern void NGSetupLevelExtraState();

extern void NGFrameStartExtraState();
extern void NGFrameFinishExtraState();

extern int NGValidateInputAgainstLockTimers(int32_t input);
extern int NGApplySimulatedInput(int32_t input);
extern bool NGValidateInputSavegame();
extern bool NGValidateInputLoadgame();
extern bool NGValidateInputWeaponHotkeys();

extern void NGDisableInputForTime(unsigned char input, int ticks);
extern void NGSimulateInputForTime(unsigned char input, int ticks);
extern void NGEnableInput(unsigned char input);

extern bool NGIsItemFrozen(unsigned int item_num);
extern void NGSetItemFreezeTimer(unsigned int item_num, int ticks);

// Moveables

extern bool NGIsItemPerformingContinousAction(unsigned int item_num);
extern bool NGIsItemPerformingRotation(unsigned int item_num);
extern bool NGIsItemPerformingMovement(unsigned int item_num);

extern short NGGetItemHorizontalRotationSpeed(unsigned int item_num);
extern void NGSetItemHorizontalRotationSpeed(unsigned int item_num, short speed);
extern short NGGetItemVerticalRotationSpeed(unsigned int item_num);
extern void NGSetItemVerticalRotationSpeed(unsigned int item_num, short speed);

extern int NGGetItemHorizontalRotationRemaining(unsigned int item_num);
extern void NGSetItemHorizontalRotationRemaining(unsigned int item_num, int remaining);
extern int NGGetItemVerticalRotationRemaining(unsigned int item_num);
extern void NGSetItemVerticalRotationRemaining(unsigned int item_num, int remaining);

extern void NGSetItemHorizontalMovementAngle(unsigned int item_num, short angle);
extern short NGGetItemHorizontalMovementAngle(unsigned int item_num);

extern int NGGetItemHorizontalMovementRemainingUnits(unsigned int item_num);
extern void NGSetItemHorizontalMovementRemainingUnits(unsigned int item_num, int units);

extern int NGGetItemVerticalMovementRemainingUnits(unsigned int item_num);
extern void NGSetItemVerticalMovementRemainingUnits(unsigned int item_num, int units);

extern int NGGetItemHorizontalMovementRepeatUnits(unsigned int item_num);
extern void NGSetItemHorizontalMovementRepeatUnits(unsigned int item_num, int units);

extern int NGGetItemVerticalMovementRepeatUnits(unsigned int item_num);
extern void NGSetItemVerticalMovementRepeatUnits(unsigned int item_num, int units);

extern int NGGetItemHorizontalMovementSpeed(unsigned int item_num);
extern void NGSetItemHorizontalMovementSpeed(unsigned int item_num, unsigned int movement_speed);
extern int NGGetItemVerticalMovementSpeed(unsigned int item_num);
extern void NGSetItemVerticalMovementSpeed(unsigned int item_num, unsigned int movement_speed);

extern int NGGetItemMovementInProgressSound(unsigned int item_num);
extern void NGSetItemMovementInProgressSound(unsigned int item_num, int sound_effect_id);
extern void NGSetItemMovementFinishedSound(unsigned int item_num, int sound_effect_id);
extern int NGGetItemMovementFinishedSound(unsigned int item_num);

extern bool NGGetItemMovementTriggerHeavyAtEnd(unsigned int item_num);
extern void NGSetItemMovementTriggerHeavyAtEnd(unsigned int item_num, bool trigger_heavy_at_end);
extern bool NGGetItemMovementTriggerNormalWhenMoving(unsigned int item_num);
extern void NGSetItemMovementTriggerNormalWhenMoving(unsigned int item_num, bool trigger_normal_when_moving);
extern bool NGGetItemMovementTriggerHeavyWhenMoving(unsigned int item_num);
extern void NGSetItemMovementTriggerHeavyWhenMoving(unsigned int item_num, bool trigger_heavy_when_moving);

// Statics

extern bool NGIsStaticPerformingContinousAction(unsigned int static_num);
extern bool NGIsStaticPerformingRotation(unsigned int static_num);
extern bool NGIsStaticPerformingMovement(unsigned int static_num);

extern short NGGetStaticHorizontalRotationSpeed(unsigned int static_num);
extern void NGSetStaticHorizontalRotationSpeed(unsigned int static_num, short speed);
extern short NGGetStaticVerticalRotationSpeed(unsigned int static_num);
extern void NGSetStaticVerticalRotationSpeed(unsigned int static_num, short speed);

extern int NGGetStaticHorizontalRotationRemaining(unsigned int static_num);
extern void NGSetStaticHorizontalRotationRemaining(unsigned int static_num, int remaining);
extern int NGGetStaticVerticalRotationRemaining(unsigned int static_num);
extern void NGSetStaticVerticalRotationRemaining(unsigned int static_num, int remaining);

extern void NGSetStaticHorizontalMovementAngle(unsigned int static_num, short angle);
extern short NGGetStaticHorizontalMovementAngle(unsigned int static_num);

extern int NGGetStaticHorizontalMovementRemainingUnits(unsigned int static_num);
extern void NGSetStaticHorizontalMovementRemainingUnits(unsigned int static_num, int units);

extern int NGGetStaticVerticalMovementRemainingUnits(unsigned int static_num);
extern void NGSetStaticVerticalMovementRemainingUnits(unsigned int static_num, int units);

extern int NGGetStaticHorizontalMovementRepeatUnits(unsigned int static_num);
extern void NGSetStaticHorizontalMovementRepeatUnits(unsigned int static_num, int units);

extern int NGGetStaticVerticalMovementRepeatUnits(unsigned int static_num);
extern void NGSetStaticVerticalMovementRepeatUnits(unsigned int static_num, int units);

extern int NGGetStaticHorizontalMovementSpeed(unsigned int static_num);
extern void NGSetStaticHorizontalMovementSpeed(unsigned int static_num, unsigned int movement_speed);
extern int NGGetStaticVerticalMovementSpeed(unsigned int static_num);
extern void NGSetStaticVerticalMovementSpeed(unsigned int static_num, unsigned int movement_speed);

extern int NGGetStaticMovementInProgressSound(unsigned int static_num);
extern void NGSetStaticMovementInProgressSound(unsigned int static_num, int sound_effect_id);
extern void NGSetStaticMovementFinishedSound(unsigned int static_num, int sound_effect_id);
extern int NGGetStaticMovementFinishedSound(unsigned int static_num);

extern bool NGGetStaticMovementTriggerHeavyAtEnd(unsigned int static_num);
extern void NGSetStaticMovementTriggerHeavyAtEnd(unsigned int static_num, bool trigger_heavy_at_end);
extern bool NGGetStaticMovementTriggerNormalWhenMoving(unsigned int static_num);
extern void NGSetStaticMovementTriggerNormalWhenMoving(unsigned int static_num, bool trigger_normal_when_moving);
extern bool NGGetStaticMovementTriggerHeavyWhenMoving(unsigned int static_num);
extern void NGSetStaticMovementTriggerHeavyWhenMoving(unsigned int static_num, bool trigger_heavy_when_moving);

//

struct NGItemCollision {
	ITEM_INFO* item_info;
	int32_t flags;
};


#define MAX_LARA_COLLISONS 20
#define MAX_LARA_STATIC_COLLISONS 20

#define NG_COLLISION_TYPE_BOUNDS 1
#define NG_COLLISION_TYPE_PUSH 2

extern void NGAddLaraItemCollision(ITEM_INFO *item_info, int32_t flags);
extern void NGAddLaraStaticCollision(int32_t room_number, int32_t mesh_number);
extern void NGClearLaraCollisions();
extern ITEM_INFO *NGIsLaraCollidingWithItem(ITEM_INFO *item_info, int32_t mask);
extern ITEM_INFO *NGIsLaraCollidingWithMoveableSlot(int32_t slot_number, int32_t mask);

enum NGCreatureType {
	NG_CREATURE_TYPE_ANY,
	NG_CREATURE_TYPE_MORTAL,
	NG_CREATURE_TYPE_IMMORTAL,
	NG_CREATURE_TYPE_FRIEND
};

ITEM_INFO *NGIsLaraCollidingWithCreature(NGCreatureType creature_type, int32_t mask);
extern int NGIsLaraCollidingWithStaticID(int id);
extern int NGIsLaraCollidingWithStaticSlot(int slot);

extern bool NGIsItemCollisionDisabled(unsigned int item_num);
extern void NGDisableItemCollision(unsigned int item_num);
extern void NGEnableItemCollision(unsigned int item_num);

extern void NGToggleItemMeshVisibilityMaskBit(unsigned int item_num, unsigned int mask_bit, bool enabled);
extern unsigned int NGGetItemMeshVisibilityMask(unsigned int item_num);

extern void NGSetFadeOverride(int item_id, short fade_override);
extern short NGGetFadeOverride(int item_id);

extern void NGSetFullscreenCurtainTimer(int ticks);
extern void NGSetCinemaTypeAndTimer(int type, int ticks);

extern bool NGIsTriggerGroupContinuous(int trigger_group_id);
extern void NGSetTriggerGroupContinuous(int trigger_group_id, bool is_continuous);

extern void NGToggleOrganizer(int organizer_id, bool is_enabled);
extern bool NGIsOrganizerEnabled(int organizer_id);
extern void NGResetOrganizer(int organizer_id);

extern void NGSetDisplayTimerForMoveableWithType(int item_id, NGTimerTrackerType timer_type);

extern bool NGProcessGlobalTriggers(int selected_inventory_object_id);

extern void NGDrawPhase();