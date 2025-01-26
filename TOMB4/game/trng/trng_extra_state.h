#pragma once

#include "../sound.h"

#define NG_MAX_FLOORSTATE_ACTIONS 128

#define NG_MAX_TRIGGERED_ITEMS 128
extern bool ng_loaded_savegame;

extern int16_t ng_camera_target_id;

enum NGTimerTrackerType {
	TTT_ONLY_SHOW_SECONDS = 0,
	TTT_SECONDS_AND_ONE_DECIMAL_POINT_SEPERATOR,
	TTT_SECONDS_AND_TWO_DECIMAL_POINT_SEPERATOR,
	TTT_SECONDS_AND_ONE_DECIMAL_COLON_SEPERATOR,
	TTT_SECONDS_AND_TWO_DECIMAL_COLON_SEPERATOR,
	TTT_SECONDS_WITH_THREE_NOUGHTS
};

#define MAX_NG_GLOBAL_TRIGGERS 500
#define MAX_NG_TRIGGER_GROUPS 1000
#define MAX_NG_ORGANIZERS 4999
#define MAX_NG_ITEM_GROUPS 999
#define MAX_NG_ANIMATIONS 9999
#define MAX_NG_MULTI_ENV_CONDITIONS 9999
#define MAX_NG_TEST_POSITIONS 9999

extern int32_t ng_input_to_simulate;
extern int32_t ng_input_to_disable;
extern int32_t ng_single_input_to_simulate;

extern int32_t resumed_trigger_group_count;
extern uint16_t resumed_trigger_groups[MAX_NG_TRIGGER_GROUPS];
extern int32_t last_performed_trigger_group;
extern uint16_t performed_trigger_groups[MAX_NG_TRIGGER_GROUPS];

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
	int32_t current_tick = 0;
};

extern NG_GLOBAL_TRIGGER_STATE ng_global_trigger_states[MAX_NG_GLOBAL_TRIGGERS];
extern NG_TRIGGER_GROUP_STATE ng_trigger_group_states[MAX_NG_TRIGGER_GROUPS];
extern NG_ORGANIZER_STATE ng_organizer_states[MAX_NG_ORGANIZERS];

extern int32_t ng_looped_sound_state[NumSamples];

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

extern int32_t ng_animation_current_animation;
extern int16_t ng_animation_prev_hands_state;
extern int32_t ng_animation_target_item;
extern int32_t ng_animation_target_test_position;

extern int32_t ng_global_timer;
extern int8_t ng_global_timer_frame_increment;
extern NGTimerPosition ng_global_timer_position;
extern int32_t ng_global_timer_time_until_hide;

extern int32_t ng_local_timer;
extern int8_t ng_local_timer_frame_increment;
extern NGTimerPosition ng_local_timer_position;
extern int32_t ng_local_timer_time_until_hide;

// Level
extern int32_t pending_level_load_timer;
extern int32_t pending_level_load_id;

// Variables
extern int32_t ng_current_value;
extern int32_t ng_global_alfa;
extern int32_t ng_global_beta;
extern int32_t ng_global_delta;
extern int32_t ng_local_alfa;
extern int32_t ng_local_beta;
extern int32_t ng_local_delta;
extern int32_t ng_last_input_number;

#define STORE_VARIABLE_COUNT 16
extern int32_t ng_store_variables[STORE_VARIABLE_COUNT];

#define REGULAR_TEXT_BUFFER_SIZE 80
extern char ng_last_text_input[REGULAR_TEXT_BUFFER_SIZE];

extern char ng_string1[REGULAR_TEXT_BUFFER_SIZE];
extern char ng_string2[REGULAR_TEXT_BUFFER_SIZE];
extern char ng_string3[REGULAR_TEXT_BUFFER_SIZE];
extern char ng_string4[REGULAR_TEXT_BUFFER_SIZE];

#define BIG_TEXT_BUFFER_SIZE 320
extern char ng_text_big[BIG_TEXT_BUFFER_SIZE];

// Visual

enum NG_DRAW_STATE {
	NG_DRAW_STATE_ACTIVE,
	NG_DRAW_STATE_FROZEN,
	NG_DRAW_STATE_BLANK
};

extern NG_DRAW_STATE ng_drawing_state;

// Inventory
extern uint8_t ng_selected_inventory_item_memory;
extern int32_t ng_used_inventory_object_for_frame;
extern bool ng_used_large_medipack;
extern bool ng_used_small_medipack;

extern void NGSetupLevelExtraState();

extern void NGFrameStartExtraState();
extern void NGFrameFinishExtraState();

extern NG_DRAW_STATE NGGetDrawState();

extern int32_t NGValidateAgainstBlockedInput(int32_t input);
extern int32_t NGApplySimulatedInput(int32_t input);
extern bool NGValidateInputSavegame();
extern bool NGValidateInputLoadgame();
extern bool NGValidateInputWeaponHotkeys();

extern bool NGIsSimulatingInputSavegame();
extern bool NGIsSimulatingInputLoadgame();

extern void NGDisableInputForTime(uint32_t mask, int32_t ticks);
extern void NGSimulateInputForTime(uint32_t mask, int32_t ticks);
extern void NGEnableInput(uint32_t mask);
extern void NGClearSimulatedSingleInputForFrame();

extern bool NGIsItemFrozen(uint32_t item_num);
extern void NGSetItemFreezeTimer(uint32_t item_num, uint32_t ticks);

// Moveables

extern void NGSetItemHorizontalMovementAngle(uint32_t item_num, int16_t angle);
extern int16_t NGGetItemHorizontalMovementAngle(uint32_t item_num);

extern int32_t NGGetItemHorizontalMovementRemainingUnits(uint32_t item_num);
extern void NGSetItemHorizontalMovementRemainingUnits(uint32_t item_num, int32_t units);

extern int32_t NGGetItemVerticalMovementRemainingUnits(uint32_t item_num);
extern void NGSetItemVerticalMovementRemainingUnits(uint32_t item_num, int32_t units);

extern int32_t NGGetItemHorizontalMovementRepeatUnits(uint32_t item_num);
extern void NGSetItemHorizontalMovementRepeatUnits(uint32_t item_num, int32_t units);

extern int32_t NGGetItemVerticalMovementRepeatUnits(uint32_t item_num);
extern void NGSetItemVerticalMovementRepeatUnits(uint32_t item_num, int32_t units);

extern int32_t NGGetItemHorizontalMovementSpeed(uint32_t item_num);
extern void NGSetItemHorizontalMovementSpeed(uint32_t item_num, uint32_t movement_speed);
extern int32_t NGGetItemVerticalMovementSpeed(uint32_t item_num);
extern void NGSetItemVerticalMovementSpeed(uint32_t item_num, uint32_t movement_speed);

extern int32_t NGGetItemMovementInProgressSound(uint32_t item_num);
extern void NGSetItemMovementInProgressSound(uint32_t item_num, int32_t sound_effect_id);
extern void NGSetItemMovementFinishedSound(uint32_t item_num, int32_t sound_effect_id);
extern int32_t NGGetItemMovementFinishedSound(uint32_t item_num);

extern bool NGGetItemMovementTriggerHeavyAtEnd(uint32_t item_num);
extern void NGSetItemMovementTriggerHeavyAtEnd(uint32_t item_num, bool trigger_heavy_at_end);
extern bool NGGetItemMovementTriggerNormalWhenMoving(uint32_t item_num);
extern void NGSetItemMovementTriggerNormalWhenMoving(uint32_t item_num, bool trigger_normal_when_moving);
extern bool NGGetItemMovementTriggerHeavyWhenMoving(uint32_t item_num);
extern void NGSetItemMovementTriggerHeavyWhenMoving(uint32_t item_num, bool trigger_heavy_when_moving);

// Statics

extern void NGSetStaticHorizontalMovementAngle(uint32_t static_num, int16_t angle);
extern int16_t NGGetStaticHorizontalMovementAngle(uint32_t static_num);

extern int32_t NGGetStaticHorizontalMovementRemainingUnits(uint32_t static_num);
extern void NGSetStaticHorizontalMovementRemainingUnits(uint32_t static_num, int32_t units);

extern int32_t NGGetStaticVerticalMovementRemainingUnits(uint32_t static_num);
extern void NGSetStaticVerticalMovementRemainingUnits(uint32_t static_num, int32_t units);

extern int32_t NGGetStaticHorizontalMovementRepeatUnits(uint32_t static_num);
extern void NGSetStaticHorizontalMovementRepeatUnits(uint32_t static_num, int32_t units);

extern int32_t NGGetStaticVerticalMovementRepeatUnits(uint32_t static_num);
extern void NGSetStaticVerticalMovementRepeatUnits(uint32_t static_num, int32_t units);

extern int32_t NGGetStaticHorizontalMovementSpeed(uint32_t static_num);
extern void NGSetStaticHorizontalMovementSpeed(uint32_t static_num, uint32_t movement_speed);
extern int32_t NGGetStaticVerticalMovementSpeed(uint32_t static_num);
extern void NGSetStaticVerticalMovementSpeed(uint32_t static_num, uint32_t movement_speed);

extern int32_t NGGetStaticMovementInProgressSound(uint32_t static_num);
extern void NGSetStaticMovementInProgressSound(uint32_t static_num, int32_t sound_effect_id);
extern void NGSetStaticMovementFinishedSound(uint32_t static_num, int32_t sound_effect_id);
extern int32_t NGGetStaticMovementFinishedSound(uint32_t static_num);

extern bool NGGetStaticMovementTriggerHeavyAtEnd(uint32_t static_num);
extern void NGSetStaticMovementTriggerHeavyAtEnd(uint32_t static_num, bool trigger_heavy_at_end);
extern bool NGGetStaticMovementTriggerNormalWhenMoving(uint32_t static_num);
extern void NGSetStaticMovementTriggerNormalWhenMoving(uint32_t static_num, bool trigger_normal_when_moving);
extern bool NGGetStaticMovementTriggerHeavyWhenMoving(uint32_t static_num);
extern void NGSetStaticMovementTriggerHeavyWhenMoving(uint32_t static_num, bool trigger_heavy_when_moving);

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
extern int32_t NGIsLaraCollidingWithStaticID(int32_t id);
extern int32_t NGIsLaraCollidingWithStaticSlot(int32_t slot);

extern bool NGIsItemCollisionDisabled(uint32_t item_num);
extern void NGDisableItemCollision(uint32_t item_num);
extern void NGEnableItemCollision(uint32_t item_num);

extern void NGToggleItemMeshVisibilityMaskBit(uint32_t item_num, uint32_t mask_bit, bool enabled);
extern uint32_t NGGetItemMeshVisibilityMask(uint32_t item_num);

extern void NGSetFadeOverride(int32_t item_id, int16_t fade_override);
extern int16_t NGGetFadeOverride(int32_t item_id);

extern void NGSetFullscreenCurtainTimer(int32_t ticks);
extern void NGSetCinemaTypeAndTimer(int32_t type, int32_t ticks);

extern bool NGIsTriggerGroupContinuous(int32_t trigger_group_id);
extern void NGSetTriggerGroupContinuous(int32_t trigger_group_id, bool is_continuous);

extern void NGToggleOrganizer(int32_t organizer_id, bool is_enabled);
extern bool NGIsOrganizerEnabled(int32_t organizer_id);
extern void NGResetOrganizer(int32_t organizer_id);

extern void NGSetDisplayTimerForMoveableWithType(int32_t item_id, NGTimerTrackerType timer_type);

extern bool NGProcessGlobalTriggers(int32_t selected_inventory_object_id);

extern void NGDrawPhase();