#include "../tomb4/pch.h"

#include "objects/general/clockworkbeetle.h"

#include "objects/vehicles/bike.h"
#include "objects/vehicles/jeep.h"

#include "savegame.h"
#include "objects.h"
#include "traps.h"
#include "items.h"
#include "laramisc.h"
#include "control.h"
#include "lot.h"
#include "pickup.h"
#include "spotcam.h"
#include "../specific/function_stubs.h"
#include "camera.h"

#include "objects/creatures/voncroy.h"

#include "objects/effects/scarabs.h"

#include "lara.h"
#include "newinv.h"
#include "senet.h"
#include "switch.h"
#include "rope.h"
#include "gameflow.h"
#include "../specific/file.h"
#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_objects.h"
#include "trng/trng_savegame.h"
#include "../specific/audio.h"
#include "moveblok.h"

LEGACY_SAVEGAME_INFO savegame;

static char* SGpoint = nullptr;
static int32_t SGcount = 0;

int32_t CheckSumValid(char* buffer) {
	int8_t checksum;

	checksum = 0;

	for (int32_t i = 0; i < sizeof(LEGACY_SAVEGAME_INFO); i++)
		checksum += *buffer++;

	return !checksum;
}

void sgInitialiseHub(int32_t dont_save_lara) {
	for (int32_t i = 0; i < MAX_HUB_LEVELS; i++) {
		savegame.HubLevels[i] = 0;
		savegame.HubOffsets[i] = 0;
		savegame.HubSizes[i] = 0;
	}

	if (dont_save_lara)
		savegame.HubSavedLara = 0;
	else {
		SaveLaraData();
		savegame.HubSavedLara = 1;
	}

	memset(savegame.Lara.puzzleitems, 0, 12);
	savegame.Lara.puzzleitemscombo = 0;
	savegame.Lara.keyitems = 0;
	savegame.Lara.keyitemscombo = 0;
	savegame.Lara.pickupitems = 0;
	savegame.Lara.pickupitemscombo = 0;
	savegame.Lara.examine1 = 0;
	savegame.Lara.examine2 = 0;
	savegame.Lara.examine3 = 0;
	savegame.Lara.mechanical_scarab = 0;
	savegame.Lara.location = -1;
	savegame.Lara.highest_location = -1;
	savegame.Lara.vehicle = NO_ITEM;
}

void SaveLaraData() {
	ITEM_INFO* item;

	savegame.Lara.item_number = lara.item_number;
	savegame.Lara.gun_status = lara.gun_status;
	savegame.Lara.gun_type = lara.gun_type;
	savegame.Lara.request_gun_type = lara.request_gun_type;
	savegame.Lara.last_gun_type = lara.last_gun_type;
	savegame.Lara.calc_fallspeed = lara.calc_fallspeed;
	savegame.Lara.water_status = lara.water_status;
	savegame.Lara.climb_status = lara.climb_status;
	savegame.Lara.pose_count = lara.pose_count;
	savegame.Lara.hit_frame = lara.hit_frame;
	savegame.Lara.hit_direction = lara.hit_direction;
	savegame.Lara.air = lara.air;
	savegame.Lara.dive_count = lara.dive_count;
	savegame.Lara.death_count = lara.death_count;
	savegame.Lara.current_active = lara.current_active;
	savegame.Lara.current_xvel = lara.current_xvel;
	savegame.Lara.current_yvel = lara.current_yvel;
	savegame.Lara.current_zvel = lara.current_zvel;
	savegame.Lara.spaz_effect_count = lara.spaz_effect_count;
	savegame.Lara.flare_age = lara.flare_age;
	savegame.Lara.vehicle = lara.vehicle;
	savegame.Lara.weapon_item = lara.weapon_item;
	savegame.Lara.back_gun = lara.back_gun;
	savegame.Lara.flare_frame = lara.flare_frame;
	savegame.Lara.poisoned = lara.poisoned;
	savegame.Lara.dpoisoned = lara.dpoisoned;
	savegame.Lara.electric = lara.electric;

	for (int32_t i = 0; i < WET_COUNT; i++) {
		savegame.Lara.wet[i] = lara.wet[i];
	}

	savegame.Lara.flare_control_left = lara.flare_control_left;
	savegame.Lara.Unused1 = lara.Unused1;
	savegame.Lara.look = lara.look;
	savegame.Lara.burn = lara.burn;
	savegame.Lara.keep_ducked = lara.keep_ducked;
	savegame.Lara.IsMoving = lara.IsMoving;
	savegame.Lara.CanMonkeySwing = lara.CanMonkeySwing;
	savegame.Lara.Unused2 = lara.Unused2;
	savegame.Lara.OnBeetleFloor = lara.OnBeetleFloor;
	savegame.Lara.BurnGreen = lara.BurnGreen;
	savegame.Lara.IsDucked = lara.IsDucked;
	savegame.Lara.has_fired = lara.has_fired;
	savegame.Lara.Busy = lara.Busy;
	savegame.Lara.LitTorch = lara.LitTorch;
	savegame.Lara.IsClimbing = lara.IsClimbing;
	savegame.Lara.Fired = lara.Fired;
	savegame.Lara.water_surface_dist = lara.water_surface_dist;
	savegame.Lara.last_pos.x = lara.last_pos.x;
	savegame.Lara.last_pos.y = lara.last_pos.y;
	savegame.Lara.last_pos.z = lara.last_pos.z;
	savegame.Lara.spaz_effect = 0; // Pointer
	savegame.Lara.mesh_effects = lara.mesh_effects;

	for (int32_t i = 0; i < LARA_MESH_PTR_COUNT; i++) {
		size_t base = (size_t)mesh_base;
		size_t offset = ((size_t)lara.mesh_ptrs[i]) - base;

		savegame.Lara.mesh_ptrs[i] = 0;
		for (size_t j = 0; j < mesh_mapping_table_count; j++) {
			if (mesh_mapping_table[j].mesh_native_ptr == offset) {
				savegame.Lara.mesh_ptrs[i] = (X32_POINTER)(mesh_mapping_table[j].mesh_x32_ptr);
			}
		}
	}

	savegame.Lara.target = 0; // Pointer

	for (int32_t i = 0; i < 2; i++) {
		savegame.Lara.target_angles[i] = lara.target_angles[i];
	}

	savegame.Lara.turn_rate = lara.turn_rate;
	savegame.Lara.move_angle = lara.move_angle;
	savegame.Lara.head_y_rot = lara.head_y_rot;
	savegame.Lara.head_x_rot = lara.head_x_rot;
	savegame.Lara.head_z_rot = lara.head_z_rot;
	savegame.Lara.torso_y_rot = lara.torso_y_rot;
	savegame.Lara.torso_x_rot = lara.torso_x_rot;
	savegame.Lara.torso_z_rot = lara.torso_z_rot;

	// Left Arm
	savegame.Lara.left_arm.frame_base = (X32_POINTER)((size_t)lara.left_arm.frame_base - (size_t)objects[T4PlusGetPistolsAnimSlotID()].frame_base);
	savegame.Lara.left_arm.frame_number = lara.left_arm.frame_number;
	savegame.Lara.left_arm.anim_number = lara.left_arm.anim_number;
	savegame.Lara.left_arm.lock = lara.left_arm.lock;
	savegame.Lara.left_arm.y_rot = lara.left_arm.y_rot;
	savegame.Lara.left_arm.x_rot = lara.left_arm.x_rot;
	savegame.Lara.left_arm.z_rot = lara.left_arm.z_rot;
	savegame.Lara.left_arm.flash_gun = lara.left_arm.flash_gun;

	// Right Arm
	savegame.Lara.right_arm.frame_base = (X32_POINTER)((size_t)lara.right_arm.frame_base - (size_t)objects[T4PlusGetPistolsAnimSlotID()].frame_base);
	savegame.Lara.right_arm.frame_number = lara.right_arm.frame_number;
	savegame.Lara.right_arm.anim_number = lara.right_arm.anim_number;
	savegame.Lara.right_arm.lock = lara.right_arm.lock;
	savegame.Lara.right_arm.y_rot = lara.right_arm.y_rot;
	savegame.Lara.right_arm.x_rot = lara.right_arm.x_rot;
	savegame.Lara.right_arm.z_rot = lara.right_arm.z_rot;
	savegame.Lara.right_arm.flash_gun = lara.right_arm.flash_gun;

	savegame.Lara.holster = lara.holster;

	savegame.Lara.creature = 0; // Pointer
	savegame.Lara.CornerX = 0; // Pointer
	savegame.Lara.CornerZ = 0; // Pointer

	savegame.Lara.RopeSegment = lara.RopeSegment;
	savegame.Lara.RopeDirection = lara.RopeDirection;
	savegame.Lara.RopeArcFront = lara.RopeArcFront;
	savegame.Lara.RopeArcBack = lara.RopeArcBack;
	savegame.Lara.RopeLastX = lara.RopeLastX;
	savegame.Lara.RopeMaxXForward = lara.RopeMaxXForward;
	savegame.Lara.RopeMaxXBackward = lara.RopeMaxXBackward;
	savegame.Lara.RopeDFrame = lara.RopeDFrame;
	savegame.Lara.RopeFrame = lara.RopeFrame;
	savegame.Lara.RopeFrameRate = lara.RopeFrameRate;
	savegame.Lara.RopeY = lara.RopeY;
	savegame.Lara.RopePtr = lara.RopePtr;
	savegame.Lara.GeneralPtr = lara.GeneralPtr;
	savegame.Lara.RopeOffset = lara.RopeOffset;
	savegame.Lara.RopeDownVel = lara.RopeDownVel;
	savegame.Lara.RopeFlag = lara.RopeFlag;
	savegame.Lara.MoveCount = lara.MoveCount;
	savegame.Lara.RopeCount = lara.RopeCount;
	savegame.Lara.pistols_type_carried = lara.pistols_type_carried;
	savegame.Lara.uzis_type_carried = lara.uzis_type_carried;
	savegame.Lara.shotgun_type_carried = lara.shotgun_type_carried;
	savegame.Lara.crossbow_type_carried = lara.crossbow_type_carried;
	savegame.Lara.grenade_type_carried = lara.grenade_type_carried;
	savegame.Lara.sixshooter_type_carried = lara.sixshooter_type_carried;
	savegame.Lara.lasersight = lara.lasersight;
	savegame.Lara.binoculars = lara.binoculars;
	savegame.Lara.crowbar = lara.crowbar;
	savegame.Lara.mechanical_scarab = lara.mechanical_scarab;
	savegame.Lara.small_water_skin = lara.small_water_skin;
	savegame.Lara.big_water_skin = lara.big_water_skin;
	savegame.Lara.examine1 = lara.examine1;
	savegame.Lara.examine2 = lara.examine2;
	savegame.Lara.examine3 = lara.examine3;

	for (int32_t i = 0; i < 12; i++) {
		savegame.Lara.puzzleitems[i] = lara.puzzleitems[i];
	}

	savegame.Lara.puzzleitemscombo = lara.puzzleitemscombo;
	savegame.Lara.keyitems = lara.keyitems;
	savegame.Lara.keyitemscombo = lara.keyitemscombo;
	savegame.Lara.pickupitems = lara.pickupitems;
	savegame.Lara.pickupitemscombo = lara.pickupitemscombo;
	savegame.Lara.questitems = lara.questitems;
	savegame.Lara.num_small_medipack = lara.num_small_medipack;
	savegame.Lara.num_large_medipack = lara.num_large_medipack;
	savegame.Lara.num_flares = lara.num_flares;
	savegame.Lara.num_pistols_ammo = lara.num_pistols_ammo;
	savegame.Lara.num_uzi_ammo = lara.num_uzi_ammo;
	savegame.Lara.num_revolver_ammo = lara.num_revolver_ammo;
	savegame.Lara.num_shotgun_ammo1 = lara.num_shotgun_ammo1;
	savegame.Lara.num_shotgun_ammo2 = lara.num_shotgun_ammo2;
	savegame.Lara.num_grenade_ammo1 = lara.num_grenade_ammo1;
	savegame.Lara.num_grenade_ammo2 = lara.num_grenade_ammo2;
	savegame.Lara.num_grenade_ammo3 = lara.num_grenade_ammo3;
	savegame.Lara.num_crossbow_ammo1 = lara.num_crossbow_ammo1;
	savegame.Lara.num_crossbow_ammo2 = lara.num_crossbow_ammo2;
	savegame.Lara.num_crossbow_ammo3 = lara.num_crossbow_ammo3;
	savegame.Lara.beetle_uses = lara.beetle_uses;
	savegame.Lara.blindTimer = lara.blindTimer;
	savegame.Lara.location = lara.location;
	savegame.Lara.highest_location = lara.highest_location;
	savegame.Lara.locationPad = lara.locationPad;

	if (lara.weapon_item != NO_ITEM) {
		item = &items[lara.weapon_item];
		savegame.WeaponObject = item->object_number;
		savegame.WeaponAnim = item->anim_number;
		savegame.WeaponFrame = item->frame_number;
		savegame.WeaponCurrent = item->current_anim_state;
		savegame.WeaponGoal = item->goal_anim_state;
	}

	savegame.cutscene_triggered = CutSceneTriggered;
}

void WriteSG(void* pointer, int32_t size) {
	char* data;

	SGcount += size;

	for (data = (char*)pointer; size > 0; size--)
		*SGpoint++ = *data++;
}

void ReadSG(void* pointer, int32_t size) {
	char* data;

	SGcount += size;

	for (data = (char*)pointer; size > 0; size--)
		*data++ = *SGpoint++;
}

void SaveHubData(int32_t index) {
	savegame.HubSizes[index] = uint16_t(SGcount - savegame.HubOffsets[index]);

	if (index < MAX_HUB_LEVELS)
		savegame.HubSizes[index - (MAX_HUB_LEVELS-1)] = savegame.HubSizes[index] + savegame.HubOffsets[index];
}

void RestoreLaraData(bool full_save) {
	ITEM_INFO* item;

	if (!full_save)
		savegame.Lara.item_number = lara.item_number;

	lara.item_number = savegame.Lara.item_number;
	lara.gun_status = savegame.Lara.gun_status;
	lara.gun_type = savegame.Lara.gun_type;
	lara.request_gun_type = savegame.Lara.request_gun_type;
	lara.last_gun_type = savegame.Lara.last_gun_type;
	lara.calc_fallspeed = savegame.Lara.calc_fallspeed;
	lara.water_status = savegame.Lara.water_status;
	lara.climb_status = savegame.Lara.climb_status;
	lara.pose_count = savegame.Lara.pose_count;
	lara.hit_frame = savegame.Lara.hit_frame;
	lara.hit_direction = savegame.Lara.hit_direction;
	lara.air = savegame.Lara.air;
	lara.dive_count = savegame.Lara.dive_count;
	lara.death_count = savegame.Lara.death_count;
	lara.current_active = savegame.Lara.current_active;
	lara.current_xvel = savegame.Lara.current_xvel;
	lara.current_yvel = savegame.Lara.current_yvel;
	lara.current_zvel = savegame.Lara.current_zvel;
	lara.spaz_effect_count = savegame.Lara.spaz_effect_count;
	lara.flare_age = savegame.Lara.flare_age;
	lara.vehicle = savegame.Lara.vehicle;
	lara.weapon_item = savegame.Lara.weapon_item;
	lara.back_gun = savegame.Lara.back_gun;
	lara.flare_frame = savegame.Lara.flare_frame;
	lara.poisoned = savegame.Lara.poisoned;
	lara.dpoisoned = savegame.Lara.dpoisoned;
	lara.electric = savegame.Lara.electric;

	for (int32_t i = 0; i < WET_COUNT; i++) {
		lara.wet[i] = savegame.Lara.wet[i];
	}

	lara.flare_control_left = savegame.Lara.flare_control_left;
	lara.Unused1 = savegame.Lara.Unused1;
	lara.look = savegame.Lara.look;
	lara.burn = savegame.Lara.burn;
	lara.keep_ducked = savegame.Lara.keep_ducked;
	lara.IsMoving = savegame.Lara.IsMoving;
	lara.CanMonkeySwing = savegame.Lara.CanMonkeySwing;
	lara.Unused2 = savegame.Lara.Unused2;
	lara.OnBeetleFloor = savegame.Lara.OnBeetleFloor;
	lara.BurnGreen = savegame.Lara.BurnGreen;
	lara.IsDucked = savegame.Lara.IsDucked;
	lara.has_fired = savegame.Lara.has_fired;
	lara.Busy = savegame.Lara.Busy;
	lara.LitTorch = savegame.Lara.LitTorch;
	lara.IsClimbing = savegame.Lara.IsClimbing;
	lara.Fired = savegame.Lara.Fired;
	lara.water_surface_dist = savegame.Lara.water_surface_dist;
	lara.last_pos.x = savegame.Lara.last_pos.x;
	lara.last_pos.y = savegame.Lara.last_pos.y;
	lara.last_pos.z = savegame.Lara.last_pos.z;
	lara.spaz_effect = nullptr; // Pointer
	lara.mesh_effects = savegame.Lara.mesh_effects;

	if (!savegame.HubSavedLara) {
		for (int32_t i = 0; i < LARA_MESH_PTR_COUNT; i++) {
			size_t base = (size_t)mesh_base;
			size_t offset = (size_t)(savegame.Lara.mesh_ptrs[i]);

			lara.mesh_ptrs[i] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + i * 2];

			size_t offset_original = (size_t)(lara.mesh_ptrs[i]) - base;
			for (size_t j = 0; j < mesh_mapping_table_count; j++) {
				if (mesh_mapping_table[j].mesh_x32_ptr == offset) {
					lara.mesh_ptrs[i] = (int16_t*)(base + mesh_mapping_table[j].mesh_native_ptr);
				}
			}
		}
	}

	lara.target = nullptr; // Pointer

	for (int32_t i = 0; i < 2; i++) {
		lara.target_angles[i] = savegame.Lara.target_angles[i];
	}

	lara.turn_rate = savegame.Lara.turn_rate;
	lara.move_angle = savegame.Lara.move_angle;
	lara.head_y_rot = savegame.Lara.head_y_rot;
	lara.head_x_rot = savegame.Lara.head_x_rot;
	lara.head_z_rot = savegame.Lara.head_z_rot;
	lara.torso_y_rot = savegame.Lara.torso_y_rot;
	lara.torso_x_rot = savegame.Lara.torso_x_rot;
	lara.torso_z_rot = savegame.Lara.torso_z_rot;

	// Left Arm
	lara.left_arm.frame_base = (int16_t*)((size_t)lara.left_arm.frame_base + (size_t)objects[T4PlusGetPistolsAnimSlotID()].frame_base);
	lara.left_arm.frame_number = savegame.Lara.left_arm.frame_number;
	lara.left_arm.anim_number = savegame.Lara.left_arm.anim_number;
	lara.left_arm.lock = savegame.Lara.left_arm.lock;
	lara.left_arm.y_rot = savegame.Lara.left_arm.y_rot;
	lara.left_arm.x_rot = savegame.Lara.left_arm.x_rot;
	lara.left_arm.z_rot = savegame.Lara.left_arm.z_rot;
	lara.left_arm.flash_gun = savegame.Lara.left_arm.flash_gun;

	// Right Arm
	lara.right_arm.frame_base = (int16_t*)((size_t)lara.right_arm.frame_base + (size_t)objects[T4PlusGetPistolsAnimSlotID()].frame_base);
	lara.right_arm.frame_number = savegame.Lara.right_arm.frame_number;
	lara.right_arm.anim_number = savegame.Lara.right_arm.anim_number;
	lara.right_arm.lock = savegame.Lara.right_arm.lock;
	lara.right_arm.y_rot = savegame.Lara.right_arm.y_rot;
	lara.right_arm.x_rot = savegame.Lara.right_arm.x_rot;
	lara.right_arm.z_rot = savegame.Lara.right_arm.z_rot;
	lara.right_arm.flash_gun = savegame.Lara.right_arm.flash_gun;

	lara.holster = savegame.Lara.holster;

	lara.creature = 0; // Pointer
	lara.CornerX = 0; // Pointer
	lara.CornerZ = 0; // Pointer

	lara.RopeSegment = savegame.Lara.RopeSegment;
	lara.RopeDirection = savegame.Lara.RopeDirection;
	lara.RopeArcFront = savegame.Lara.RopeArcFront;
	lara.RopeArcBack = savegame.Lara.RopeArcBack;
	lara.RopeLastX = savegame.Lara.RopeLastX;
	lara.RopeMaxXForward = savegame.Lara.RopeMaxXForward;
	lara.RopeMaxXBackward = savegame.Lara.RopeMaxXBackward;
	lara.RopeDFrame = savegame.Lara.RopeDFrame;
	lara.RopeFrame = savegame.Lara.RopeFrame;
	lara.RopeFrameRate = savegame.Lara.RopeFrameRate;
	lara.RopeY = savegame.Lara.RopeY;
	lara.RopePtr = savegame.Lara.RopePtr;
	lara.GeneralPtr = savegame.Lara.GeneralPtr;
	lara.RopeOffset = savegame.Lara.RopeOffset;
	lara.RopeDownVel = savegame.Lara.RopeDownVel;
	lara.RopeFlag = savegame.Lara.RopeFlag;
	lara.MoveCount = savegame.Lara.MoveCount;
	lara.RopeCount = savegame.Lara.RopeCount;
	lara.pistols_type_carried = savegame.Lara.pistols_type_carried;
	lara.uzis_type_carried = savegame.Lara.uzis_type_carried;
	lara.shotgun_type_carried = savegame.Lara.shotgun_type_carried;
	lara.crossbow_type_carried = savegame.Lara.crossbow_type_carried;
	lara.grenade_type_carried = savegame.Lara.grenade_type_carried;
	lara.sixshooter_type_carried = savegame.Lara.sixshooter_type_carried;
	lara.lasersight = savegame.Lara.lasersight;
	lara.binoculars = savegame.Lara.binoculars;
	lara.crowbar = savegame.Lara.crowbar;
	lara.mechanical_scarab = savegame.Lara.mechanical_scarab;
	lara.small_water_skin = savegame.Lara.small_water_skin;
	lara.big_water_skin = savegame.Lara.big_water_skin;
	lara.examine1 = savegame.Lara.examine1;
	lara.examine2 = savegame.Lara.examine2;
	lara.examine3 = savegame.Lara.examine3;

	for (int32_t i = 0; i < 12; i++) {
		lara.puzzleitems[i] = savegame.Lara.puzzleitems[i];
	}

	lara.puzzleitemscombo = savegame.Lara.puzzleitemscombo;
	lara.keyitems = savegame.Lara.keyitems;
	lara.keyitemscombo = savegame.Lara.keyitemscombo;
	lara.pickupitems = savegame.Lara.pickupitems;
	lara.pickupitemscombo = savegame.Lara.pickupitemscombo;
	lara.questitems = savegame.Lara.questitems;
	lara.num_small_medipack = savegame.Lara.num_small_medipack;
	lara.num_large_medipack = savegame.Lara.num_large_medipack;
	lara.num_flares = savegame.Lara.num_flares;
	lara.num_pistols_ammo = savegame.Lara.num_pistols_ammo;
	lara.num_uzi_ammo = savegame.Lara.num_uzi_ammo;
	lara.num_revolver_ammo = savegame.Lara.num_revolver_ammo;
	lara.num_shotgun_ammo1 = savegame.Lara.num_shotgun_ammo1;
	lara.num_shotgun_ammo2 = savegame.Lara.num_shotgun_ammo2;
	lara.num_grenade_ammo1 = savegame.Lara.num_grenade_ammo1;
	lara.num_grenade_ammo2 = savegame.Lara.num_grenade_ammo2;
	lara.num_grenade_ammo3 = savegame.Lara.num_grenade_ammo3;
	lara.num_crossbow_ammo1 = savegame.Lara.num_crossbow_ammo1;
	lara.num_crossbow_ammo2 = savegame.Lara.num_crossbow_ammo2;
	lara.num_crossbow_ammo3 = savegame.Lara.num_crossbow_ammo3;
	lara.beetle_uses = savegame.Lara.beetle_uses;
	lara.blindTimer = savegame.Lara.blindTimer;
	lara.location = savegame.Lara.location;
	lara.highest_location = savegame.Lara.highest_location;
	lara.locationPad = savegame.Lara.locationPad;

	// Tomb4Plus - we've modified the GeneralPtr to be an index rather than a memory address since it was buggy and barely used as memory address directly.
	// Added a check to make sure the item is within range to prevent crashing with old savegames.
	if (lara.GeneralPtr >= VANILLA_ITEM_COUNT) {
		Log(0, "RestoreLaraData: GeneralPtr overflow!");
		lara.GeneralPtr = 0;
	}

	if (lara.burn) {
		lara.burn = 0;
		LaraBurn();
	}

	if (lara.weapon_item != NO_ITEM) {
		lara.weapon_item = CreateItem();
		item = &items[lara.weapon_item];
		item->object_number = savegame.WeaponObject;
		item->anim_number = savegame.WeaponAnim;
		item->frame_number = savegame.WeaponFrame;
		item->current_anim_state = savegame.WeaponCurrent;
		item->goal_anim_state = savegame.WeaponGoal;
		item->status = ITEM_ACTIVE;
		item->room_number = 255;
	}

	if (savegame.HubSavedLara) {
		LaraInitialiseMeshes();
		savegame.HubSavedLara &= ~1;
		lara.last_gun_type = WEAPON_PISTOLS;
		lara.gun_type = WEAPON_PISTOLS;
		lara.request_gun_type = WEAPON_PISTOLS;

		if (lara.weapon_item != NO_ITEM) {
			KillItem(lara.weapon_item);
			lara.weapon_item = NO_ITEM;
		}
	}

	CutSceneTriggered = savegame.cutscene_triggered;
}

void sgRestoreLevel() {
	AIOBJECT* lsp;
	ITEM_INFO* item;
	FLOOR_INFO* floor;

	if (OpenSaveGame(gfCurrentLevel, 0) >= 0) {
		RestoreLevelData(false, get_game_mod_global_info()->trng_savegames);

		// T4Plus
		T4PlusEnterLevel(gfCurrentLevel, false);
	} else {
		// T4Plus
		T4PlusEnterLevel(gfCurrentLevel, true);
	}

	RestoreLaraData(0);

	if (gfRequiredStartPos) {
		lsp = &AIObjects[gfRequiredStartPos - 1];
		lara_item->pos.x_pos = lsp->x;
		lara_item->pos.y_pos = lsp->y;
		lara_item->pos.z_pos = lsp->z;
		lara_item->pos.y_rot = lsp->y_rot;

		if (lara_item->room_number != lsp->room_number)
			ItemNewRoom(lara.item_number, lsp->room_number);
	}

	InitialiseLaraAnims(lara_item);

	if (savegame.Lara.vehicle != NO_ITEM) {
		for (int32_t i = 0; i < level_items; i++) {
			item = &items[i];

			if (item->object_number == T4PlusGetMotorbikeSlotID() || item->object_number == T4PlusGetJeepSlotID()) {
				item->pos.x_pos = lara_item->pos.x_pos;
				item->pos.y_pos = lara_item->pos.y_pos;
				item->pos.z_pos = lara_item->pos.z_pos;
				item->pos.y_rot = lara_item->pos.y_rot;

				if (item->room_number != lara_item->room_number)
					ItemNewRoom(i, lara_item->room_number);

				floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &item->room_number);
				item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
				lara.vehicle = i;

				if (item->object_number == T4PlusGetMotorbikeSlotID())
					BikeStart(item, lara_item);
				else if (item->object_number == T4PlusGetJeepSlotID())
					JeepStart(item, lara_item);

				break;
			}
		}
	}
}

void CreateCheckSum() {
	char* ptr;
	int8_t checksum;

	savegame.Checksum = 0;
	ptr = (char*)&savegame;
	checksum = 0;

	for (int32_t i = 0; i < sizeof(LEGACY_SAVEGAME_INFO); i++)
		checksum += *ptr++;

	savegame.Checksum = -checksum;
}

void sgSaveLevel() {
	int32_t level_index;

	level_index = OpenSaveGame(gfCurrentLevel, 1);
	SaveLevelData(0, get_game_mod_global_info()->trng_savegames);
	SaveLaraData();
	SaveHubData(level_index);
}

void sgSaveGame() {
	int32_t level_index;

	level_index = OpenSaveGame(gfCurrentLevel, 1);
	savegame.Game.Timer = GameTimer;
	savegame.CurrentLevel ^= (gfCurrentLevel ^ savegame.CurrentLevel) & 0x7F;
	savegame.fog_colour.r = gfVolumetricFog.r;
	savegame.fog_colour.g = gfVolumetricFog.g;
	savegame.fog_colour.b = gfVolumetricFog.b;
	SaveLevelData(1, get_game_mod_global_info()->trng_savegames);
	SaveLaraData();
	SaveHubData(level_index);
	CreateCheckSum();

	// TRNG
	if (get_game_mod_global_info()->trng_savegames) {
		NGWriteNGSavegameInfo();
	}
}

void sgRestoreGame() {
	OpenSaveGame(savegame.CurrentLevel & 0x7F, 0);
	GameTimer = savegame.Game.Timer;
	gfCurrentLevel = savegame.CurrentLevel & 0x7F;
	RestoreLevelData(true, NGIsNGSavegame());
	RestoreLaraData(true);

	// TRNG
	NGReadNGSavegameInfo();
}

int32_t OpenSaveGame(uint8_t current_level, int32_t saving) {
	uint16_t* curOffset;
	uint16_t* nexOffset;
	int32_t index, i, j;

	index = 0;

	while (index < MAX_HUB_LEVELS && savegame.HubLevels[index] != current_level)
		index++;

	if (saving == 1) {
		j = index + 1;
		i = index;

		if (index < MAX_HUB_LEVELS) {
			while (j < MAX_HUB_LEVELS) {
				curOffset = &savegame.HubOffsets[i];
				nexOffset = &savegame.HubOffsets[j];

				if (!savegame.HubLevels[j])
					break;

				memcpy(&savegame.buffer[curOffset[0]], &savegame.buffer[nexOffset[0]], nexOffset[MAX_HUB_LEVELS]);
				curOffset[MAX_HUB_LEVELS] = nexOffset[MAX_HUB_LEVELS];
				nexOffset[0] = curOffset[0] + curOffset[MAX_HUB_LEVELS];
				savegame.HubLevels[i] = savegame.HubLevels[j];
				i++;
				j++;
			}

			savegame.HubLevels[i] = 0;
		}

		for (index = 0; index < MAX_HUB_LEVELS; index++) {
			if (!savegame.HubLevels[index])
				break;
		}

		savegame.HubLevels[index] = current_level;
		SGcount = savegame.HubOffsets[index];
		SGpoint = &savegame.buffer[SGcount];
		return index;
	}

	if (index < MAX_HUB_LEVELS) {
		SGcount = savegame.HubOffsets[index];
		SGpoint = &savegame.buffer[SGcount];
		return index;
	}

	return -1;
}

void SaveLevelData(bool full_save, bool use_full_flipmask) {
	ITEM_INFO* item;
	ROOM_INFO* r;
	OBJECT_INFO* obj;
	MESH_INFO* mesh;
	CREATURE_INFO* creature;
	uint32_t flags;
	int32_t flare_age;
	uint16_t packed;
	int16_t pos, word;
	uint8_t byte;
	int8_t lflags;
	uint32_t flipmap_mask = 0;
	size_t flipmap_bitcount = 0;

	WriteSG(&FmvSceneTriggered, sizeof(int32_t));
	WriteSG(&GLOBAL_lastinvitem, sizeof(int32_t));
	word = 0;

	if (use_full_flipmask) {
		flipmap_bitcount = 32;
	} else {
		flipmap_bitcount = 10;
	}

	for (size_t i = 0; i < flipmap_bitcount; i++) {
		if (flip_stats[i])
			flipmap_mask |= (1 << i);
	}

	if (use_full_flipmask) {
		WriteSG(&flipmap_mask, sizeof(uint32_t));
	} else {
		uint16_t flipmap_mask_16 = (uint16_t)(flipmap_mask & 0xffff);
		WriteSG(&flipmap_mask_16, sizeof(uint16_t));
	}

	for (size_t i = 0; i < flipmap_bitcount; i++) {
		word = int16_t(flipmap[i] >> 8);
		WriteSG(&word, sizeof(int16_t));
	}

	WriteSG(&flipeffect, sizeof(int32_t));
	WriteSG(&fliptimer, sizeof(int32_t));
	WriteSG(&flip_status, sizeof(int32_t));
	WriteSG(cd_flags, 128);
	WriteSG(&CurrentAtmosphere, sizeof(uint8_t));

	word = 0;
	int32_t shatter_bit_idx = 0;

	for (int32_t i = 0; i < number_rooms; i++) {
		r = &room[i];

		for (int32_t j = 0; j < r->num_meshes; j++) {
			mesh = &r->mesh[j];

			if (get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number].record_shatter_state_in_savegames) {
				word |= ((mesh->Flags & 1) << shatter_bit_idx);
				shatter_bit_idx++;

				if (shatter_bit_idx == 16) {
					WriteSG(&word, sizeof(int16_t));
					shatter_bit_idx = 0;
					word = 0;
				}
			}
		}
	}

	if (shatter_bit_idx)
		WriteSG(&word, sizeof(int16_t));

	byte = 0;

	for (int32_t i = 0; i < MAX_LIBRARY_TABS; i++)
		byte |= LibraryTab[i] << i;

	WriteSG(&byte, sizeof(uint8_t));
	WriteSG(&CurrentSequence, sizeof(uint8_t));
	byte = 0;

	for (int32_t i = 0; i < MAX_USED_SEQUENCES; i++)
		byte |= SequenceUsed[i] << i;

	WriteSG(&byte, sizeof(uint8_t));
	WriteSG(Sequences, MAX_SEQUENCES);

	for (int32_t i = 0; i < number_cameras; i++)
		WriteSG(&camera.fixed[i].flags, sizeof(int16_t));

	for (int32_t i = 0; i < number_spotcams; i++)
		WriteSG(&SpotCam[i].flags, sizeof(int16_t));

	for (int32_t i = 0; i < level_items; i++) {
		item = &items[i];
		obj = &objects[item->object_number];
		packed = 0;

		if (item->flags & IFL_CLEARBODY || (item->after_death && (item->object_number < GAME_PIECE1 || item->object_number > ENEMY_PIECE))) {
			packed = 0x2000;
			WriteSG(&packed, sizeof(uint16_t));
		} else {
			if (item->flags & (IFL_CODEBITS | IFL_INVISIBLE | IFL_TRIGGERED) || item->object_number == T4PlusGetLaraSlotID() && full_save) {
				packed = 0x8000;

				if (item->pos.x_rot)
					packed |= 1;

				if (item->pos.z_rot)
					packed |= 2;

				if (item->pos.x_pos & 1)
					packed |= 4;

				if (item->pos.y_pos & 1)
					packed |= 8;

				if (item->pos.z_pos & 1)
					packed |= 0x10;

				if (item->speed)
					packed |= 0x20;

				if (item->fallspeed)
					packed |= 0x40;

				if (item->item_flags[0])
					packed |= 0x80;

				if (item->item_flags[1])
					packed |= 0x100;

				if (item->item_flags[2])
					packed |= 0x200;

				if (item->item_flags[3])
					packed |= 0x400;

				if (item->timer)
					packed |= 0x800;

				if (item->trigger_flags)
					packed |= 0x1000;

				if (obj->save_hitpoints && item->hit_points != obj->hit_points)
					packed |= 0x4000;

				WriteSG(&packed, sizeof(uint16_t));

				if (obj->save_position) {
					pos = int16_t(item->pos.x_pos >> 1);
					WriteSG(&pos, sizeof(int16_t));

					pos = int16_t(item->pos.y_pos >> 1);
					WriteSG(&pos, sizeof(int16_t));

					pos = int16_t(item->pos.z_pos >> 1);
					WriteSG(&pos, sizeof(int16_t));

					byte = (uint8_t)item->room_number;
					WriteSG(&byte, sizeof(uint8_t));

					WriteSG(&item->pos.y_rot, sizeof(int16_t));

					if (packed & 1)
						WriteSG(&item->pos.x_rot, sizeof(int16_t));

					if (packed & 2)
						WriteSG(&item->pos.z_rot, sizeof(int16_t));

					if (packed & 0x20)
						WriteSG(&item->speed, sizeof(int16_t));

					if (packed & 0x40)
						WriteSG(&item->fallspeed, sizeof(int16_t));
				}

				if (obj->save_anim) {
					byte = (uint8_t)item->current_anim_state;
					WriteSG(&byte, sizeof(uint8_t));

					byte = (uint8_t)item->goal_anim_state;
					WriteSG(&byte, sizeof(uint8_t));

					byte = (uint8_t)item->required_anim_state;
					WriteSG(&byte, sizeof(uint8_t));

					if (item->object_number != T4PlusGetLaraSlotID()) {
						byte = item->anim_number - obj->anim_index;
						WriteSG(&byte, sizeof(uint8_t));
					} else
						WriteSG(&item->anim_number, sizeof(int16_t));

					WriteSG(&item->frame_number, sizeof(int16_t));
				}

				if (packed & 0x4000)
					WriteSG(&item->hit_points, sizeof(int16_t));

				if (obj->save_flags) {
					flags = item->flags;
					flags |= item->active << 16;
					flags |= item->status << 17;
					flags |= item->gravity_status << 19;
					flags |= item->hit_status << 20;
					flags |= item->collidable << 21;
					flags |= item->looked_at << 22;
					flags |= item->dynamic_light << 23;
					flags |= item->poisoned << 24;
					flags |= item->ai_bits << 25;
					flags |= item->really_active << 30;

					if (obj->intelligent && item->data)
						flags |= 0x80000000;

					WriteSG(&flags, sizeof(uint32_t));

					if (packed & 0x80)
						WriteSG(&item->item_flags[0], sizeof(int16_t));

					if (packed & 0x100)
						WriteSG(&item->item_flags[1], sizeof(int16_t));

					if (packed & 0x200)
						WriteSG(&item->item_flags[2], sizeof(int16_t));

					if (packed & 0x400)
						WriteSG(&item->item_flags[3], sizeof(int16_t));

					if (packed & 0x800)
						WriteSG(&item->timer, sizeof(int16_t));

					if (packed & 0x1000)
						WriteSG(&item->trigger_flags, sizeof(int16_t));

					if (obj->intelligent)
						WriteSG(&item->carried_item, sizeof(int16_t));

					if (flags & 0x80000000) {
						creature = (CREATURE_INFO*)item->data;

						WriteSG(item->data, 18);
						int32_t enemy_ptr = -1;
						if (creature->enemy) {
							for (int32_t j = 0; j < VANILLA_ITEM_COUNT; j++) {
								if (creature->enemy == &items[j]) {
									enemy_ptr = int32_t((vanilla_item_malloc_offset + (j * TR4_VANILLA_ITEM_STRUCT_SIZE)) & 0xffffffff);
									break;
								}
							}
						}

						WriteSG(&enemy_ptr, sizeof(enemy_ptr));

						WriteSG(&creature->ai_target.object_number, sizeof(int16_t));
						WriteSG(&creature->ai_target.room_number, sizeof(int16_t));
						WriteSG(&creature->ai_target.box_number, sizeof(uint16_t));
						WriteSG(&creature->ai_target.flags, sizeof(int16_t));
						WriteSG(&creature->ai_target.trigger_flags, sizeof(int16_t));
						WriteSG(&creature->ai_target.pos, sizeof(PHD_3DPOS));

						lflags = creature->LOT.can_jump;
						lflags |= creature->LOT.can_monkey << 1;
						lflags |= creature->LOT.is_amphibious << 2;
						lflags |= creature->LOT.is_jumping << 3;
						lflags |= creature->LOT.is_monkeying << 4;
						WriteSG(&lflags, sizeof(int8_t));
					}
				}

				if (obj->save_mesh) {
					WriteSG(&item->mesh_bits, sizeof(uint32_t));
					WriteSG(&item->meshswap_meshbits, sizeof(uint32_t));
				}

				if (item->object_number == T4PlusGetMotorbikeSlotID())
					WriteSG(item->data, sizeof(BIKEINFO));
				else if (item->object_number == T4PlusGetJeepSlotID())
					WriteSG(item->data, sizeof(JEEPINFO));
			} else
				WriteSG(&packed, sizeof(uint16_t));
		}
	}

	if (objects[WHEEL_OF_FORTUNE].loaded) {
		WriteSG(senet_item, sizeof(int16_t) * 6);
		WriteSG(senet_piece, sizeof(int8_t) * 6);
		WriteSG(senet_board, sizeof(int8_t) * 17);
		WriteSG(&last_throw, sizeof(int8_t));
		WriteSG(&SenetTargetX, sizeof(int32_t));
		WriteSG(&SenetTargetZ, sizeof(int32_t));
		WriteSG(&piece_moving, sizeof(int8_t));
	}

	if (full_save) {
		byte = 0;
		item = &items[level_items];

		for (int32_t i = level_items; i < VANILLA_ITEM_COUNT; i++) {
			if (item->active && (item->object_number == FLARE_ITEM || item->object_number == BURNING_TORCH_ITEM))
				byte++;

			item++;
		}

		WriteSG(&byte, sizeof(uint8_t));
		item = &items[level_items];

		for (int32_t i = level_items; i < VANILLA_ITEM_COUNT; i++) {
			if (item->active && (item->object_number == FLARE_ITEM || item->object_number == BURNING_TORCH_ITEM)) {
				if (item->object_number == FLARE_ITEM)
					byte = 0;
				else if (item->object_number == BURNING_TORCH_ITEM)
					byte = 1;

				WriteSG(&byte, sizeof(int8_t));
				WriteSG(&item->pos, sizeof(PHD_3DPOS));
				WriteSG(&item->room_number, sizeof(int16_t));
				WriteSG(&item->speed, sizeof(int16_t));
				WriteSG(&item->fallspeed, sizeof(int16_t));

				if (item->object_number == FLARE_ITEM) {
					flare_age = int32_t(size_t(item->data) & 0x7fff);
					WriteSG(&flare_age, sizeof(int32_t));
				} else
					WriteSG(&item->item_flags[3], sizeof(int16_t));
			}

			item++;
		}

		if (objects[LITTLE_BEETLE].loaded) {
			byte = 0;

			for (int32_t j = 0; j < 128; j++) {
				if (Scarabs[j].On)
					byte++;
			}

			WriteSG(&byte, sizeof(uint8_t));

			for (int32_t j = 0; j < 128; j++) {
				if (Scarabs[j].On) {
					word = Scarabs[j].room_number << 8;

					if (Scarabs[j].pos.x_pos & 1)
						word |= 1;

					if (Scarabs[j].pos.y_pos & 1)
						word |= 2;

					if (Scarabs[j].pos.z_pos & 1)
						word |= 4;

					if (Scarabs[j].pos.x_rot)
						word |= 8;

					WriteSG(&word, sizeof(int16_t));

					pos = int16_t(Scarabs[j].pos.x_pos >> 1);
					WriteSG(&pos, sizeof(int16_t));

					pos = int16_t(Scarabs[j].pos.y_pos >> 1);
					WriteSG(&pos, sizeof(int16_t));

					pos = int16_t(Scarabs[j].pos.z_pos >> 1);
					WriteSG(&pos, sizeof(int16_t));

					WriteSG(&Scarabs[j].pos.y_rot, sizeof(int16_t));

					if (word & 8)
						WriteSG(&Scarabs[j].pos.x_rot, sizeof(int16_t));
				}
			}
		}

		byte = 0;
		item = &items[level_items];

		for (int32_t i = level_items; i < 256; i++) {
			if (item->active && item->object_number == CLOCKWORK_BEETLE) {
				byte = 1;
				break;
			}

			item++;
		}

		WriteSG(&byte, sizeof(uint8_t));

		if (byte) {
			WriteSG(&item->pos, sizeof(PHD_3DPOS));
			WriteSG(item->item_flags, sizeof(int16_t) * 4);
		}

		if (gfCurrentLevel == 1) {
			flags = 0;

			for (int32_t i = 0; i < MAX_VONCROY_FLAGS; i++) {
				if (VonCroyCutFlags[i])
					flags |= 1 << (i & 0xF);

				if ((i & 0xF) == 0xF) {
					WriteSG(&flags, 2);
					flags = 0;
				}
			}
		}

		if (lara.RopePtr != -1) {
			WriteSG(&RopeList[lara.RopePtr], sizeof(ROPE_STRUCT));
			CurrentPendulum.Rope = (ROPE_STRUCT*)((int8_t*)CurrentPendulum.Rope - (size_t)RopeList);

			WriteSG(&CurrentPendulum, sizeof(PENDULUM));
			CurrentPendulum.Rope = (ROPE_STRUCT*)((int8_t*)CurrentPendulum.Rope + (size_t)RopeList);
		}
	}
}

void RestoreLevelData(bool full_save, bool use_full_flipmask) {
	ROOM_INFO* r;
	ITEM_INFO* item;
	CREATURE_INFO* creature;
	FLOOR_INFO* floor;
	OBJECT_INFO* obj;
	MESH_INFO* mesh;
	uint32_t flags;
	int32_t flare_age;
	uint16_t word, packed, uroom_number, uword;
	int16_t sword, item_number, room_number, req, goal, current;
	uint8_t numberof;
	int8_t byte, anim, lflags;
	uint32_t flipmap_mask = 0;
	int32_t flipmap_bitcount = 0;

	ReadSG(&FmvSceneTriggered, sizeof(int32_t));
	ReadSG(&GLOBAL_lastinvitem, sizeof(int32_t));

	if (use_full_flipmask) {
		ReadSG(&flipmap_mask, sizeof(uint32_t));
		flipmap_bitcount = 32;
	} else {
		ReadSG(&flipmap_mask, sizeof(uint16_t));
		flipmap_bitcount = 10;
	}

	for (int32_t i = 0; i < flipmap_bitcount; i++) {
		if (flipmap_mask & (1 << i))
			FlipMap(i);

		ReadSG(&uword, sizeof(uint16_t));
		flipmap[i] = uword << 8;
	}

	ReadSG(&flipeffect, sizeof(int32_t));
	ReadSG(&fliptimer, sizeof(int32_t));
	ReadSG(&flip_status, sizeof(int32_t));
	ReadSG(cd_flags, 128);
	ReadSG(&CurrentAtmosphere, sizeof(uint8_t));
	int32_t shatter_bit_idx = 16;

	for (int32_t i = 0; i < number_rooms; i++) {
		r = &room[i];

		for (int32_t j = 0; j < r->num_meshes; j++) {
			mesh = &r->mesh[j];

			if (get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number].record_shatter_state_in_savegames) {
				if (shatter_bit_idx == 16) {
					ReadSG(&uword, sizeof(uint16_t));
					shatter_bit_idx = 0;
				}

				mesh->Flags ^= (uword ^ mesh->Flags) & 1;

				if (!mesh->Flags) {
					room_number = i;
					floor = GetFloor(mesh->x, mesh->y, mesh->z, &room_number);
					GetHeight(floor, mesh->x, mesh->y, mesh->z);
					TestTriggers(trigger_index, true, 0);
					floor->stopper = 0;
				}

				uword >>= 1;
				shatter_bit_idx++;
			}
		}
	}

	ReadSG(&byte, sizeof(int8_t));

	for (int32_t i = 0; i < MAX_LIBRARY_TABS; i++) {
		LibraryTab[i] = byte & 1;
		byte >>= 1;
	}

	ReadSG(&CurrentSequence, sizeof(uint8_t));
	ReadSG(&byte, sizeof(int8_t));

	for (int32_t i = 0; i < MAX_USED_SEQUENCES; i++) {
		SequenceUsed[i] = byte & 1;
		byte >>= 1;
	}

	ReadSG(Sequences, MAX_SEQUENCES);

	for (int32_t i = 0; i < number_cameras; i++)
		ReadSG(&camera.fixed[i].flags, sizeof(int16_t));

	for (int32_t i = 0; i < number_spotcams; i++)
		ReadSG(&SpotCam[i].flags, sizeof(int16_t));

	for (int32_t i = 0; i < level_items; i++) {
		item = &items[i];
		obj = &objects[item->object_number];
		ReadSG(&packed, sizeof(uint16_t));

		if (packed & 0x2000) {
			KillItem(i);
			item->status = ITEM_DEACTIVATED;
			item->flags |= IFL_INVISIBLE;
		} else if (packed & 0x8000) {
			PHD_3DPOS original_pos = item->pos;
			item_status original_status = (item_status)item->status;

			if (obj->save_position) {
				uroom_number = 0;

				ReadSG(&word, sizeof(uint16_t));
				item->pos.x_pos = (word << 1) | (packed >> 2) & 1;

				ReadSG(&sword, sizeof(int16_t));
				item->pos.y_pos = (sword << 1) | (packed >> 3) & 1;

				ReadSG(&word, sizeof(uint16_t));
				item->pos.z_pos = (word << 1) | (packed >> 4) & 1;

				ReadSG(&uroom_number, sizeof(uint8_t));
				ReadSG(&item->pos.y_rot, sizeof(int16_t));

				if (packed & 1)
					ReadSG(&item->pos.x_rot, sizeof(int16_t));

				if (packed & 2)
					ReadSG(&item->pos.z_rot, sizeof(int16_t));

				if (packed & 0x20)
					ReadSG(&item->speed, sizeof(int16_t));

				if (packed & 0x40)
					ReadSG(&item->fallspeed, sizeof(int16_t));

				if (item->room_number != uroom_number)
					ItemNewRoom(i, uroom_number);

				if (obj->shadow_size) {
					floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (int16_t*)&uroom_number);
					item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
				}
			}

			if (obj->save_anim) {
				current = 0;
				goal = 0;
				req = 0;
				ReadSG(&current, sizeof(int8_t));
				ReadSG(&goal, sizeof(int8_t));
				ReadSG(&req, sizeof(int8_t));
				item->current_anim_state = current;
				item->goal_anim_state = goal;
				item->required_anim_state = req;

				if (item->object_number != T4PlusGetLaraSlotID()) {
					ReadSG(&anim, sizeof(int8_t));
					item->anim_number = obj->anim_index + anim;
				} else
					ReadSG(&item->anim_number, sizeof(int16_t));

				ReadSG(&item->frame_number, sizeof(int16_t));
			}

			if (packed & 0x4000)
				ReadSG(&item->hit_points, sizeof(int16_t));

			if (obj->save_flags) {
				ReadSG(&flags, sizeof(uint32_t));
				item->flags = (int16_t)flags;

				if (packed & 0x80)
					ReadSG(&item->item_flags[0], sizeof(int16_t));

				if (packed & 0x100)
					ReadSG(&item->item_flags[1], sizeof(int16_t));

				if (packed & 0x200)
					ReadSG(&item->item_flags[2], sizeof(int16_t));

				if (packed & 0x400)
					ReadSG(&item->item_flags[3], sizeof(int16_t));

				if (packed & 0x800)
					ReadSG(&item->timer, sizeof(int16_t));

				if (packed & 0x1000)
					ReadSG(&item->trigger_flags, sizeof(int16_t));

				if (obj->intelligent)
					ReadSG(&item->carried_item, sizeof(int16_t));

				if (flags & 0x10000 && !item->active)
					AddActiveItem(i);

				item->active = (flags >> 16) & 1;
				item->status = (flags >> 17) & 3;
				item->gravity_status = (flags >> 19) & 1;
				item->hit_status = (flags >> 20) & 1;
				item->collidable = (flags >> 21) & 1;
				item->looked_at = (flags >> 22) & 1;
				item->dynamic_light = (flags >> 23) & 1;
				item->poisoned = (flags >> 24) & 1;
				item->ai_bits = (flags >> 25) & 31;
				item->really_active = (flags >> 30) & 1;

				if (flags & 0x80000000) {
					EnableBaddieAI(i, 1);
					creature = (CREATURE_INFO*)item->data;

					if (creature) {
						ReadSG(creature, 18);
						int32_t enemy_ptr = 0;
						ReadSG(&enemy_ptr, sizeof(enemy_ptr));
						creature->enemy = nullptr;
						if (enemy_ptr >= 0) {
							size_t base_item_ptr = ((size_t)enemy_ptr - (size_t)vanilla_item_malloc_offset);
							int32_t enemy_id = (int32_t)(base_item_ptr / TR4_VANILLA_ITEM_STRUCT_SIZE);
							int32_t remainder = base_item_ptr % TR4_VANILLA_ITEM_STRUCT_SIZE;
							if (remainder == 0) {
								if (enemy_id >= 0 && enemy_id < VANILLA_ITEM_COUNT) {
									creature->enemy = &items[enemy_id];
								}
							}
						}

						ReadSG(&creature->ai_target.object_number, sizeof(int16_t));
						ReadSG(&creature->ai_target.room_number, sizeof(int16_t));
						ReadSG(&creature->ai_target.box_number, sizeof(uint16_t));
						ReadSG(&creature->ai_target.flags, sizeof(int16_t));
						ReadSG(&creature->ai_target.trigger_flags, sizeof(int16_t));
						ReadSG(&creature->ai_target.pos, sizeof(PHD_3DPOS));
						ReadSG(&lflags, sizeof(int8_t));
						creature->LOT.can_jump = (lflags & 1) == 1;
						creature->LOT.can_monkey = (lflags & 2) == 2;
						creature->LOT.is_amphibious = (lflags & 4) == 4;
						creature->LOT.is_jumping = (lflags & 8) == 8;
						creature->LOT.is_monkeying = (lflags & 16) == 16;
					} else
						SGpoint += 51;
				}
			}

			if (obj->save_mesh) {
				ReadSG(&item->mesh_bits, sizeof(uint32_t));
				ReadSG(&item->meshswap_meshbits, sizeof(uint32_t));
			}

			if (item->object_number == T4PlusGetMotorbikeSlotID())
				ReadSG(item->data, sizeof(BIKEINFO));
			else if (item->object_number == T4PlusGetJeepSlotID())
				ReadSG(item->data, sizeof(JEEPINFO));

			if (obj->collision == PuzzleHoleCollision) {
				if (item->status == ITEM_DEACTIVATED || item->status == ITEM_ACTIVE) {
					item->object_number += 12;
					item->anim_number = objects[item->object_number].anim_index + anim;
				}
			}

			if (item->object_number >= SMASH_OBJECT1 && item->object_number <= SMASH_OBJECT8 && item->flags & IFL_INVISIBLE) {
				item->mesh_bits = 0x100;
			}

			if (item->object_number == RAISING_BLOCK1 && item->item_flags[1] ||
				item->object_number == EXPANDING_PLATFORM && item->item_flags[2]) {
				AlterFloorHeight(item, -BLOCK_SIZE);
			}

			if (item->object_number == RAISING_BLOCK2 && item->item_flags[1]) {
				AlterFloorHeight(item, -(BLOCK_SIZE * 2));
			}

			// Revert MoveableBlock standing collision
			int32_t climbable_block_height = GetMoveableBlockHeight(i);

			if (item->object_number >= PUSHABLE_OBJECT1 && item->object_number <= PUSHABLE_OBJECT5) {
				if (climbable_block_height) {
					if (original_status == ITEM_INACTIVE) {
						// Clear the original floor modification.
						PHD_3DPOS new_pos = item->pos;
						item->pos = original_pos;
						AlterFloorHeight(item, climbable_block_height * CLICK_SIZE);

						item->pos = new_pos;
					}

					if (item->status == ITEM_INACTIVE) {
						// Apply the new floor modification.
						AlterFloorHeight(item, -climbable_block_height* CLICK_SIZE);
					}
				}
			}
		}
	}

	if (objects[WHEEL_OF_FORTUNE].loaded) {
		ReadSG(senet_item, sizeof(int16_t) * 6);
		ReadSG(senet_piece, sizeof(int8_t) * 6);
		ReadSG(senet_board, sizeof(int8_t) * 17);
		ReadSG(&last_throw, sizeof(int8_t));
		ReadSG(&SenetTargetX, sizeof(int32_t));
		ReadSG(&SenetTargetZ, sizeof(int32_t));
		ReadSG(&piece_moving, sizeof(int8_t));
	}

	if (full_save) {
		ReadSG(&numberof, sizeof(uint8_t));

		for (int32_t i = 0; i < numberof; i++) {
			item_number = CreateItem();
			item = &items[item_number];
			ReadSG(&byte, sizeof(int8_t));

			if (!byte)
				item->object_number = FLARE_ITEM;
			else
				item->object_number = BURNING_TORCH_ITEM;

			ReadSG(&item->pos, sizeof(PHD_3DPOS));
			ReadSG(&item->room_number, sizeof(int16_t));
			ReadSG(&item->speed, sizeof(int16_t));
			ReadSG(&item->fallspeed, sizeof(int16_t));
			InitialiseItem(item_number);
			AddActiveItem(item_number);

			switch (item->object_number) {
				case BURNING_TORCH_ITEM:
					ReadSG(&item->item_flags[3], 2);
					break;

				case FLARE_ITEM:
					ReadSG(&flare_age, sizeof(int32_t));
					item->data = (void*)size_t(flare_age);
					break;
			}
		}

		if (objects[LITTLE_BEETLE].loaded) {
			ReadSG(&byte, sizeof(int8_t));

			for (int32_t i = 0; i < byte; i++) {
				ReadSG(&sword, sizeof(int16_t));

				ReadSG(&uword, sizeof(uint16_t));
				Scarabs[i].pos.x_pos = uword << 1;
				Scarabs[i].pos.x_pos |= sword & 1;

				ReadSG(&req, sizeof(int16_t));
				Scarabs[i].pos.y_pos = req << 1;
				Scarabs[i].pos.y_pos |= (sword >> 1) & 1;

				ReadSG(&uword, sizeof(uint16_t));
				Scarabs[i].pos.z_pos = uword << 1;
				Scarabs[i].pos.z_pos |= (sword >> 2) & 1;

				ReadSG(&Scarabs[i].pos.y_rot, sizeof(int16_t));

				if (sword & 8)
					ReadSG(&Scarabs[i].pos.x_rot, sizeof(int16_t));

				Scarabs[i].On = 1;
				Scarabs[i].room_number = (sword >> 8) & 0xFF;
			}
		}

		ReadSG(&byte, sizeof(int8_t));

		if (byte) {
			item = TriggerClockworkBeetle(1);
			ReadSG(&item->pos, sizeof(PHD_3DPOS));
			ReadSG(item->item_flags, sizeof(int16_t) * 4);
		}

		if (gfCurrentLevel == 1) {
			for (int32_t i = 0; i < MAX_VONCROY_FLAGS; i++) {
				if (!(i & 0xF))
					ReadSG(&uword, sizeof(uint16_t));

				if (uword & 1 << (i & 0xF))
					VonCroyCutFlags[i] = 1;
			}
		}

		if (lara.RopePtr != -1) {
			ReadSG(&RopeList[lara.RopePtr], sizeof(ROPE_STRUCT));
			ReadSG(&CurrentPendulum, sizeof(PENDULUM));
			CurrentPendulum.Rope = (ROPE_STRUCT*)((int8_t*)CurrentPendulum.Rope + (size_t)RopeList);
		}
	}

	JustLoaded = true;
}
