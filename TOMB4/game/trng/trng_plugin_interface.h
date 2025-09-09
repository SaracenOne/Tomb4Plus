#pragma once

#define CB_INIT_PROGRAM 0
#define CB_SAVING_GAME 1
#define CB_LOADING_GAME 2
#define CB_INIT_GAME 3
#define CB_INIT_LOAD_NEW_LEVEL 4
#define CB_FLIPEFFECT_MINE 5
#define CB_ACTION_MINE 6
#define CB_CONDITION_MINE 7
#define CB_CUSTOMIZE_MINE 8
#define CB_PARAMETER_MINE 9
#define CB_CYCLE_BEGIN 10
#define CB_CYCLE_END 11
#define CB_GLOBAL_TRIGGER 12
#define CB_INIT_OBJECTS 13
#define CB_PROGR_ACTION_MINE 14
#define CB_PROGR_ACTION_DRAW_MINE 15
#define CB_INIT_LEVEL 16
#define CB_COMPLETED_PROGR_ACTION 17
#define CB_VEHICLE 18
#define CB_ASSIGN_SLOT_MINE 19
#define CB_FMV_MANAGER 20
#define CB_INPUT_MANAGER 21
#define CB_SAVEGAME_MANAGER 22
#define CB_PAUSE_MANAGER 23
#define CB_STATISTICS_MANAGER 24
#define CB_TITLE_MENU_MANAGER 25
#define CB_WINDOWS_FONT_CREATE 26
#define CB_WINDOWS_UNICODE_CONVERT 27
#define CB_WINDOWS_TEXT_PRINT 28
#define CB_DIAGNOSTIC 29
#define CB_LARA_CONTROL 33
#define CB_LARA_DRAW 34
#define CB_LARA_HAIR_DRAW 35
#define CB_LARA_HAIR_CONTROL 36
#define CB_INVENTORY_MAIN 37
#define	CB_INVENT_BACKGROUND_CREATE 38
#define CB_INVENT_BACKGROUND_DRAW 39
#define CB_INVENT_BACKGROUND_QUIT 40
#define CB_ANIMATE_LARA 41
#define CB_OPTIONS_MANAGER 42

#define CB_FLIPEFFECT 100
#define CB_ACTION 101
#define CB_CONDITION 102
#define CB_VEHICLE_CONTROL 103
#define CB_PROGR_ACTION 105
#define CB_NUMERIC_TRNG_PATCH 106
#define CB_SLOT_INITIALISE 107
#define CB_SLOT_CONTROL 108
#define CB_SLOT_COLLISION 109
#define CB_SLOT_DRAW 110
#define CB_SLOT_FLOOR 111
#define CB_SLOT_CEILING 112
#define CB_SLOT_DRAW_EXTRA 113
#define CB_STATE_ID_LARA_CTRL 114
#define CB_STATE_ID_LARA_COLLISION 115

struct TRNGPlugin {
	//void InitLevel(int32_t newLevel, int32_t oldLevel, uint32_t fil_flags);
	//void PreloadLevel(); // Called before beginning loading of new level

	//int32_t PluginFlipEffect();
	//int32_t PluginAction();
	//int32_t PluginCondition();
	//int32_t PluginCustomize();
	//int32_t PluginParameters();
	//int32_t PluginAssignSlot();

	//void CycleBegin();
};

void RegisterVirtualPlugin(const char *pluginName, TRNGPlugin *pluginStructure) {

};