#pragma once
#include "../global/types.h"

int32_t LoadGame();
int32_t SaveGame();
void init_new_inventry();
void debounce_joystick();
void DrawInventoryItemMe(INVDRAWITEM* item, int32_t shade, int32_t overlay, int32_t shagflag);
void DrawThreeDeeObject2D(int32_t x, int32_t y, int32_t num, int32_t shade, int32_t xrot, int32_t yrot, int32_t zrot, int32_t bright, int32_t overlay);
int32_t go_and_load_game();
int32_t go_and_save_game();
void insert_object_into_list_v2(int16_t num);
void construct_combine_object_list();
void insert_object_into_list(int16_t num);
void construct_object_list();
void handle_object_changeover(int32_t ringnum);
void fade_ammo_selector();
void spinback(uint16_t* cock);
void update_laras_weapons_status();
int32_t have_i_got_item(int16_t obj);
void combine_revolver_lasersight(int32_t flag);
void combine_crossbow_lasersight(int32_t flag);
void combine_PuzzleItem1(int32_t flag);
void combine_PuzzleItem2(int32_t flag);
void combine_PuzzleItem3(int32_t flag);
void combine_PuzzleItem4(int32_t flag);
void combine_PuzzleItem5(int32_t flag);
void combine_PuzzleItem6(int32_t flag);
void combine_PuzzleItem7(int32_t flag);
void combine_PuzzleItem8(int32_t flag);
void combine_KeyItem1(int32_t flag);
void combine_KeyItem2(int32_t flag);
void combine_KeyItem3(int32_t flag);
void combine_KeyItem4(int32_t flag);
void combine_KeyItem5(int32_t flag);
void combine_KeyItem6(int32_t flag);
void combine_KeyItem7(int32_t flag);
void combine_KeyItem8(int32_t flag);
void combine_PickupItem1(int32_t flag);
void combine_PickupItem2(int32_t flag);
void combine_PickupItem3(int32_t flag);
void combine_PickupItem4(int32_t flag);
void combine_ClockWorkBeetle(int32_t flag);
int32_t do_special_waterskin_combine(int32_t flag);
void setup_objectlist_startposition(int16_t newobj);
void setup_objectlist_startposition2(int16_t newobj);
int32_t have_i_got_object(int16_t object_number);
void remove_inventory_item(int16_t object_number);
int32_t convert_obj_to_invobj(int16_t obj);
void draw_compass();
void do_examine_mode();
void dels_give_lara_items_cheat();
void use_current_item();
void DEL_picked_up_object(int16_t objnum);
int32_t is_item_currently_combinable(int16_t obj);
int32_t do_these_objects_combine(int32_t obj1, int32_t obj2);
void combine_these_two_objects(int16_t obj1, int16_t obj2);
void seperate_object(int16_t obj);
void draw_ammo_selector();
void handle_inventry_menu();
void draw_current_object_list(int32_t ringnum);
int32_t S_CallInventory2();

extern INVOBJ inventry_objects_list[];
extern int32_t GLOBAL_enterinventory;
extern int32_t GLOBAL_inventoryitemchosen;
extern int32_t GLOBAL_lastinvitem;
extern int32_t InventoryActive;

enum ring_types {
	RING_INVENTORY,
	RING_AMMO,
	RING_COUNT
};

enum option_types {
	OPT_UNUSED =		0x1,
	OPT_EQUIP =			0x2,
	OPT_USE =			0x4,
	OPT_COMBINE =		0x8,
	OPT_SEPARATE =		0x10,
	OPT_EXAMINE =		0x20,
	OPT_SHOTGUN =		0x40,
	OPT_CROSSBOW =		0x80,
	OPT_GRENADE =		0x100,
	OPT_UZI =			0x200,
	OPT_PISTOLS =		0x400,
	OPT_REVOLVER =		0x800,
	OPT_LOAD =			0x1000,
	OPT_SAVE =			0x2000
};

enum invobj_types {
	INV_UZI_ITEM = 0,
	INV_PISTOLS_ITEM,
	INV_SHOTGUN_ITEM,
	INV_REVOLVER_ITEM,
	INV_REVOLVER_LASER_ITEM,
	INV_CROSSBOW_ITEM,
	INV_CROSSBOW_LASER_ITEM,
	INV_GRENADEGUN_ITEM,
	INV_SHOTGUN_AMMO1_ITEM,
	INV_SHOTGUN_AMMO2_ITEM,
	INV_GRENADEGUN_AMMO1_ITEM,
	INV_GRENADEGUN_AMMO2_ITEM,
	INV_GRENADEGUN_AMMO3_ITEM,
	INV_CROSSBOW_AMMO1_ITEM,
	INV_CROSSBOW_AMMO2_ITEM,
	INV_CROSSBOW_AMMO3_ITEM,
	INV_REVOLVER_AMMO_ITEM,
	INV_UZI_AMMO_ITEM,
	INV_PISTOLS_AMMO_ITEM,
	INV_LASERSIGHT_ITEM,
	INV_BIGMEDI_ITEM,
	INV_SMALLMEDI_ITEM,
	INV_BINOCULARS_ITEM,
	INV_FLARE_INV_ITEM,
	INV_COMPASS_ITEM,
	INV_MEMCARD_LOAD_ITEM,
	INV_MEMCARD_SAVE_ITEM,
	INV_WATERSKIN1_EMPTY_ITEM,
	INV_WATERSKIN1_1_ITEM,
	INV_WATERSKIN1_2_ITEM,
	INV_WATERSKIN1_3_ITEM,
	INV_WATERSKIN2_EMPTY_ITEM,
	INV_WATERSKIN2_1_ITEM,
	INV_WATERSKIN2_2_ITEM,
	INV_WATERSKIN2_3_ITEM,
	INV_WATERSKIN2_4_ITEM,
	INV_WATERSKIN2_5_ITEM,
	INV_PUZZLE1_ITEM,
	INV_PUZZLE2_ITEM,
	INV_PUZZLE3_ITEM,
	INV_PUZZLE4_ITEM,
	INV_PUZZLE5_ITEM,
	INV_PUZZLE6_ITEM,
	INV_PUZZLE7_ITEM,
	INV_PUZZLE8_ITEM,
	INV_PUZZLE9_ITEM,
	INV_PUZZLE10_ITEM,
	INV_PUZZLE11_ITEM,
	INV_PUZZLE12_ITEM,
	INV_PUZZLE1_COMBO1_ITEM,
	INV_PUZZLE1_COMBO2_ITEM,
	INV_PUZZLE2_COMBO1_ITEM,
	INV_PUZZLE2_COMBO2_ITEM,
	INV_PUZZLE3_COMBO1_ITEM,
	INV_PUZZLE3_COMBO2_ITEM,
	INV_PUZZLE4_COMBO1_ITEM,
	INV_PUZZLE4_COMBO2_ITEM,
	INV_PUZZLE5_COMBO1_ITEM,
	INV_PUZZLE5_COMBO2_ITEM,
	INV_PUZZLE6_COMBO1_ITEM,
	INV_PUZZLE6_COMBO2_ITEM,
	INV_PUZZLE7_COMBO1_ITEM,
	INV_PUZZLE7_COMBO2_ITEM,
	INV_PUZZLE8_COMBO1_ITEM,
	INV_PUZZLE8_COMBO2_ITEM,
	INV_KEY1_ITEM,
	INV_KEY2_ITEM,
	INV_KEY3_ITEM,
	INV_KEY4_ITEM,
	INV_KEY5_ITEM,
	INV_KEY6_ITEM,
	INV_KEY7_ITEM,
	INV_KEY8_ITEM,
	INV_KEY9_ITEM,
	INV_KEY10_ITEM,
	INV_KEY11_ITEM,
	INV_KEY12_ITEM,
	INV_KEY1_COMBO1_ITEM,
	INV_KEY1_COMBO2_ITEM,
	INV_KEY2_COMBO1_ITEM,
	INV_KEY2_COMBO2_ITEM,
	INV_KEY3_COMBO1_ITEM,
	INV_KEY3_COMBO2_ITEM,
	INV_KEY4_COMBO1_ITEM,
	INV_KEY4_COMBO2_ITEM,
	INV_KEY5_COMBO1_ITEM,
	INV_KEY5_COMBO2_ITEM,
	INV_KEY6_COMBO1_ITEM,
	INV_KEY6_COMBO2_ITEM,
	INV_KEY7_COMBO1_ITEM,
	INV_KEY7_COMBO2_ITEM,
	INV_KEY8_COMBO1_ITEM,
	INV_KEY8_COMBO2_ITEM,
	INV_PICKUP1_ITEM,
	INV_PICKUP2_ITEM,
	INV_PICKUP3_ITEM,
	INV_PICKUP4_ITEM,
	INV_PICKUP1_COMBO1_ITEM,
	INV_PICKUP1_COMBO2_ITEM,
	INV_PICKUP2_COMBO1_ITEM,
	INV_PICKUP2_COMBO2_ITEM,
	INV_PICKUP3_COMBO1_ITEM,
	INV_PICKUP3_COMBO2_ITEM,
	INV_PICKUP4_COMBO1_ITEM,
	INV_PICKUP4_COMBO2_ITEM,
	INV_QUEST1_ITEM,
	INV_QUEST2_ITEM,
	INV_QUEST3_ITEM,
	INV_QUEST4_ITEM,
	INV_QUEST5_ITEM,
	INV_QUEST6_ITEM,
	INV_BURNING_TORCH_ITEM,
	INV_CROWBAR_ITEM,
	INV_CLOCKWORK_BEETLE_ITEM,
	INV_MECHANICAL_SCARAB_ITEM,
	INV_WINDING_KEY_ITEM,
	INV_EXAMINE1_ITEM,
	INV_EXAMINE2_ITEM,
	INV_EXAMINE3_ITEM,

	NUM_INVOBJ
};
