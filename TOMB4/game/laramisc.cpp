#include "../tomb4/pch.h"
#include "laramisc.h"
#include "objects.h"
#include "laraswim.h"
#include "lara_states.h"
#include "control.h"
#include "collide.h"
#include "sound.h"
#include "effects.h"
#include "rope.h"
#include "../specific/audio.h"
#include "lara.h"
#include "larasurf.h"
#include "../specific/3dmath.h"
#include "spotcam.h"
#include "camera.h"
#include "newinv.h"
#include "../specific/dxshell.h"
#include "../specific/input.h"
#include "savegame.h"
#include "gameflow.h"
#include "../specific/file.h"
#include "../tomb4/tomb4.h"
#include "../specific/dxsound.h"

#include "trng/trng.h"
#include "trng/trng_flipeffect.h"
#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4plus/t4plus_objects.h"
#include "../tomb4/tomb4plus/t4plus_environment.h"

COLL_INFO mycoll;

static COLL_INFO* lara_coll = &mycoll;

void LaraCheatyBits() {
	if (!tomb4.cheats)
		return;

#ifdef DEBUG
	if (keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_F1)])
#else
	if (
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_S)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_T)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_U)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_F)])
#endif
	{
		lara.num_large_medipack = -1;
		lara.num_small_medipack = -1;
		lara.num_revolver_ammo = -1;
		lara.num_uzi_ammo = -1;
		lara.num_crossbow_ammo1 = -1;
		lara.num_crossbow_ammo2 = -1;
		lara.num_crossbow_ammo3 = -1;
		lara.num_grenade_ammo1 = -1;
		lara.num_grenade_ammo2 = -1;
		lara.num_grenade_ammo3 = -1;
		lara.num_flares = -1;
		lara.num_shotgun_ammo1 = -1;
		lara.num_shotgun_ammo2 = -1;
		savegame.HaveBikeBooster = 1;

		if (objects[LASERSIGHT_ITEM].loaded)
			lara.lasersight = 1;

		if (!(gfLevelFlags & GF_YOUNGLARA)) {
			lara.pistols_type_carried |= W_PRESENT;
			lara.uzis_type_carried |= W_PRESENT;
			lara.shotgun_type_carried |= W_PRESENT;
			lara.crossbow_type_carried |= W_PRESENT;
			lara.grenade_type_carried |= W_PRESENT;
			lara.sixshooter_type_carried |= W_PRESENT;
		}

		dels_give_lara_items_cheat();
		lara_item->hit_points = DEFAULT_LARA_MAX_HEALTH;
	}

#ifdef DEBUG
	if (keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_F2)])
#else
	if (
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_D)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_O)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_Z)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_Y)])
#endif
	{
		if (lara.vehicle != NO_ITEM)
			return;

		lara_item->pos.y_pos -= 128;

		if (lara.water_status != LW_FLYCHEAT) {
			lara.water_status = LW_FLYCHEAT;
			lara_item->frame_number = anims[ANIM_SWIMCHEAT].frame_base;
			lara_item->anim_number = ANIM_SWIMCHEAT;
			lara_item->current_anim_state = AS_SWIM;
			lara_item->goal_anim_state = AS_SWIM;
			lara_item->gravity_status = 0;
			lara_item->pos.x_rot = 0;
			lara.air = DEFAULT_LARA_MAX_OXYGEN;
			lara.death_count = 0;
			lara.torso_y_rot = 0;
			lara.torso_x_rot = 0;
			lara.head_y_rot = 0;
			lara.head_x_rot = 0;
		}
	}

#ifdef DEBUG
	if (keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_F3)])
#else
	if (
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_M)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_O)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_V)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_E)])
#endif
	{
		if (gfCurrentLevel == 2 || gfCurrentLevel == 6 || gfCurrentLevel == 13 || gfCurrentLevel == 21 || gfCurrentLevel == 27)
			skipped_level = 1;

		gfRequiredStartPos = 0;
		gfLevelComplete = gfCurrentLevel + 1;
	}

#ifdef DEBUG
	if (keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_F7)])
#else
	if (
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_F)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_R)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_E)] &&
	    keymap[convert_tomb_keycode_to_sdl_scancode(T4P_KEY_Z)])
#endif
	{
		if (!freeze_camera_button_pressed) {
			camera_frozen = !camera_frozen;
		}
		freeze_camera_button_pressed = true;
	} else {
		freeze_camera_button_pressed = false;
	}
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll) {
	lara_item->hit_points = DEFAULT_LARA_MAX_HEALTH;
	LaraUnderWater(item, coll);

	if (input & IN_WALK && !(input & IN_LOOK)) {
		lara.water_status = LW_ABOVE_WATER;
		item->frame_number = anims[ANIM_STOP].frame_base;
		item->anim_number = ANIM_STOP;
		item->pos.z_rot = 0;
		item->pos.x_rot = 0;
		lara.torso_y_rot = 0;
		lara.torso_x_rot = 0;
		lara.head_y_rot = 0;
		lara.head_x_rot = 0;
		lara.gun_status = 0;
		LaraInitialiseMeshes();
		lara.mesh_effects = 0;
	}
}

void InitialiseLaraLoad(int16_t item_number) {
	lara.item_number = item_number;
	lara_item = &items[item_number];
}

void InitialiseLaraAnims(ITEM_INFO* item) {
	if (room[item->room_number].flags & ROOM_UNDERWATER) {
		item->anim_number = ANIM_TREAD;
		item->frame_number = anims[ANIM_TREAD].frame_base;
		item->current_anim_state = AS_TREAD;
		item->goal_anim_state = AS_TREAD;
		lara.water_status = LW_UNDERWATER;
		item->fallspeed = 0;
	} else {
		item->anim_number = ANIM_STOP;
		item->frame_number = anims[ANIM_STOP].frame_base;
		item->current_anim_state = AS_STOP;
		item->goal_anim_state = AS_STOP;
		lara.water_status = LW_ABOVE_WATER;
	}
}

void LaraInitialiseMeshes() {
	for (int i = 0; i < LARA_MESH_PTR_COUNT; i++) {
		meshes[objects[T4PlusGetLaraSlotID()].mesh_index + i * 2] = meshes[objects[T4PlusGetLaraSkinSlotID()].mesh_index + i * 2];
		lara.mesh_ptrs[i] = meshes[objects[T4PlusGetLaraSlotID()].mesh_index + i * 2];
	}

	if (lara.gun_type == WEAPON_GRENADE)
		lara.back_gun = GRENADE_GUN_ANIM;
	else if (lara.shotgun_type_carried)
		lara.back_gun = SHOTGUN_ANIM;
	else if (lara.grenade_type_carried)
		lara.back_gun = GRENADE_GUN_ANIM;

	lara.gun_status = LG_NO_ARMS;
	lara.left_arm.frame_number = 0;
	lara.right_arm.frame_number = 0;
	lara.target = 0;
	lara.right_arm.lock = 0;
	lara.left_arm.lock = 0;
}

void AnimateLara(ITEM_INFO* item) {
	ANIM_STRUCT* anim;
	int16_t* cmd;
	int32_t speed;
	uint16_t type;

	item->frame_number++;
	anim = &anims[item->anim_number];

	if (anim->number_changes > 0) {
		if (GetChange(item, anim)) {
			anim = &anims[item->anim_number];
			item->current_anim_state = anim->current_anim_state;
		}
	}

	if (item->frame_number > anim->frame_end) {
		if (anim->number_commands > 0) {
			cmd = &commands[anim->command_index];

			for (int i = anim->number_commands; i > 0; i--) {
				switch (*cmd++) {
					case ACMD_SETPOS:
						// TRNG
						if (NGIsUsingNGAnimCommands()) {
							uint8_t command_id = (cmd[0] & 0xff00) >> 8;
							if (command_id == 0xa0) {
								cmd += 3;
								break;
							}
						}

						TranslateItem(item, cmd[0], cmd[1], cmd[2]);
						UpdateLaraRoom(item, -381);
						cmd += 3;
						break;

					// TRNG feature - make Lara respect the kill command
					case ACMD_KILL:
						break;

					case ACMD_JUMPVEL:
						item->fallspeed = cmd[0];
						item->speed = cmd[1];
						item->gravity_status = 1;

						if (lara.calc_fallspeed) {
							item->fallspeed = lara.calc_fallspeed;
							lara.calc_fallspeed = 0;
						}

						cmd += 2;
						break;

					case ACMD_FREEHANDS:

						if (lara.gun_status != LG_FLARE)
							lara.gun_status = LG_NO_ARMS;

						break;

					case ACMD_PLAYSFX:
					case ACMD_FLIPEFFECT:
						cmd += 2;
						break;
				}
			}
		}

		item->anim_number = anim->jump_anim_num;
		item->frame_number = anim->jump_frame_num;
		anim = &anims[item->anim_number];
		item->current_anim_state = anim->current_anim_state;
	}

	if (anim->number_commands > 0) {
		cmd = &commands[anim->command_index];

		for (int i = anim->number_commands; i > 0; i--) {
			switch (*cmd++) {
				case ACMD_SETPOS: {
					int offset_frame = item->frame_number - anim->frame_base;

					// TRNG
					if (NGIsUsingNGAnimCommands()) {
						uint8_t command_frame = (cmd[0] & 0xff);
						uint8_t command_id = (cmd[0] & 0xff00) >> 8;
						if (command_id == 0xa0 && (offset_frame == command_frame || command_frame == 0xff)) {
							NGExecuteFlipEffect(0, cmd[1], cmd[2], SCANF_ANIM_COMMAND);
						}
					}
					cmd += 3;
					break;
				}

				// TRNG feature - make Lara respect the kill command
				case ACMD_KILL:
					break;

				case ACMD_JUMPVEL:
					cmd += 2;
					break;

				case ACMD_PLAYSFX:

					if (item->frame_number == cmd[0]) {
						type = cmd[1] & 0xC000;

						if (type == SFX_LANDANDWATER || (type == SFX_LANDONLY && (lara.water_surface_dist >= 0 || lara.water_surface_dist == NO_HEIGHT)) ||
						        (type == SFX_WATERONLY && lara.water_surface_dist < 0 && lara.water_surface_dist != NO_HEIGHT && !T4PlusIsRoomSwamp(&room[item->room_number])))
							SoundEffect(cmd[1] & 0x3FFF, &item->pos, SFX_ALWAYS);
					}

					cmd += 2;
					break;

				case ACMD_FLIPEFFECT:

					if (item->frame_number == *cmd) {
						FXType = cmd[1] & 0xC000;
						effect_routines[cmd[1] & 0x3FFF](item);
					}

					cmd += 2;
					break;
			}
		}
	}

	if (item->gravity_status) {
		if (lara.water_status == LW_WADE && T4PlusIsRoomSwamp(&room[item->room_number])) {
			item->speed -= item->speed >> 3;

			if (abs(item->speed) < 8) {
				item->speed = 0;
				item->gravity_status = 0;
			}

			if (item->fallspeed > 128)
				item->fallspeed >>= 1;

			item->fallspeed -= item->fallspeed >> 2;

			if (item->fallspeed < 4)
				item->fallspeed = 4;
		} else {
			speed = anim->velocity + anim->acceleration * (item->frame_number - anim->frame_base - 1);
			item->speed -= speed >> 16;
			speed += anim->acceleration;
			item->speed += speed >> 16;
			item->fallspeed += item->fallspeed < 128 ? 6 : 1;
		}
		item->pos.y_pos += item->fallspeed;
	} else {
		if (lara.water_status == LW_WADE && T4PlusIsRoomSwamp(&room[item->room_number])) {
			speed = anim->velocity >> 1;

			if (anim->acceleration)
				speed += (anim->acceleration * (item->frame_number - anim->frame_base)) >> 2;
		} else {
			speed = anim->velocity;

			if (anim->acceleration)
				speed += anim->acceleration * (item->frame_number - anim->frame_base);
		}
		item->speed = speed >> 16;
	}

	if (lara.RopePtr != -1)
		AlignLaraToRope(item);

	if (!lara.IsMoving) {
		item->pos.x_pos += item->speed * phd_sin(lara.move_angle) >> W2V_SHIFT;
		item->pos.z_pos += item->speed * phd_cos(lara.move_angle) >> W2V_SHIFT;
	}
}

void LaraControl(int16_t item_number) {
	ITEM_INFO* l;
	int32_t oldx, oldy, oldz;
	int32_t wh, wd, hfw, room_water_state;

	l = lara_item;

	if (lara.IsMoving && lara.MoveCount++ > 90) {
		lara.IsMoving = 0;
		lara.gun_status = LG_NO_ARMS;
	}

	LaraCheatyBits();

	if (!bDisableLaraControl)
		lara.locationPad = -128;

	oldx = l->pos.x_pos;
	oldy = l->pos.y_pos;
	oldz = l->pos.z_pos;

	if (lara.gun_status == LG_HANDS_BUSY && l->current_anim_state == AS_STOP && l->goal_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && !l->gravity_status)
		lara.gun_status = LG_NO_ARMS;

	if (l->current_anim_state != AS_DASH && DashTimer < 120)
		DashTimer++;

	lara.IsDucked = 0;
	room_water_state = room[l->room_number].flags & ROOM_UNDERWATER;
	if (T4PlusIsRoomSwamp(&room[l->room_number]))
		room_water_state |= ROOM_SWAMP;

	wd = GetWaterDepth(l->pos.x_pos, l->pos.y_pos, l->pos.z_pos, l->room_number);
	wh = GetWaterHeight(l->pos.x_pos, l->pos.y_pos, l->pos.z_pos, l->room_number);

	if (wh != NO_HEIGHT)
		hfw = l->pos.y_pos - wh;
	else
		hfw = NO_HEIGHT;

	lara.water_surface_dist = -hfw;

	if (lara.vehicle == NO_ITEM) {
		WadeSplash(l, wh, wd);

		switch (lara.water_status) {
			case LW_ABOVE_WATER:

				if (hfw != NO_HEIGHT && hfw >= 265) {
					if (wd <= 474 || room_water_state & ROOM_SWAMP) {
						if (hfw > CLICK_SIZE) {
							lara.water_status = LW_WADE;

							if (!l->gravity_status)
								l->goal_anim_state = AS_STOP;
							else if (room_water_state & ROOM_SWAMP) {
								if (lara_item->current_anim_state == AS_SWANDIVE || lara_item->current_anim_state == AS_FASTDIVE)
									lara_item->pos.y_pos = wh + 1000;

								lara_item->anim_number = ANIM_WADE;
								lara_item->frame_number = anims[ANIM_WADE].frame_base;
								lara_item->current_anim_state = AS_WADE;
								lara_item->goal_anim_state = AS_WADE;
							}
						}
					} else if (room_water_state) {
						lara.air = DEFAULT_LARA_MAX_OXYGEN;
						lara.water_status = LW_UNDERWATER;
						l->gravity_status = 0;
						l->pos.y_pos += 100;
						UpdateLaraRoom(l, 0);
						StopSoundEffect(SFX_LARA_FALL);

						if (l->current_anim_state == AS_SWANDIVE) {
							l->pos.x_rot = -DEGREES_TO_ROTATION(45);
							l->goal_anim_state = AS_DIVE;
							AnimateLara(l);
							l->fallspeed *= 2;
						} else if (l->current_anim_state == AS_FASTDIVE) {
							l->pos.x_rot = -DEGREES_TO_ROTATION(85);
							l->goal_anim_state = AS_DIVE;
							AnimateLara(l);
							l->fallspeed *= 2;
						} else {
							l->pos.x_rot = -DEGREES_TO_ROTATION(45);
							l->anim_number = 112;
							l->frame_number = anims[112].frame_base;
							l->current_anim_state = AS_DIVE;
							l->goal_anim_state = AS_SWIM;
							l->fallspeed = 3 * l->fallspeed / 2;
						}

						lara.torso_x_rot = 0;
						lara.torso_y_rot = 0;
						lara.head_x_rot = 0;
						lara.head_y_rot = 0;
						Splash(l);
					}
				} else
					break;
				// OG bug, needed for certain custom levels.
				if (get_game_mod_level_misc_info(gfCurrentLevel)->fix_vertical_water_warp) {
					break;
				}
				[[fallthrough]];
			case LW_WADE:
				camera.target_elevation = -DEGREES_TO_ROTATION(22);

				if (hfw <= CLICK_SIZE) {
					lara.water_status = LW_ABOVE_WATER;

					if (l->current_anim_state == AS_WADE)
						l->goal_anim_state = AS_RUN;
				} else if (hfw > 730 && !(room_water_state & ROOM_SWAMP)) {
					lara.water_status = LW_SURFACE;
					l->pos.y_pos += 1 - hfw;

					switch (l->current_anim_state) {
						case AS_BACK:
							l->anim_number = 140;
							l->frame_number = anims[140].frame_base;
							l->current_anim_state = AS_SURFBACK;
							l->goal_anim_state = AS_SURFBACK;
							break;

						case AS_STEPRIGHT:
							l->anim_number = 144;
							l->frame_number = anims[144].frame_base;
							l->current_anim_state = AS_SURFRIGHT;
							l->goal_anim_state = AS_SURFRIGHT;
							break;

						case AS_STEPLEFT:
							l->anim_number = 143;
							l->frame_number = anims[143].frame_base;
							l->current_anim_state = AS_SURFLEFT;
							l->goal_anim_state = AS_SURFLEFT;
							break;

						default:
							l->anim_number = 116;
							l->frame_number = anims[116].frame_base;
							l->current_anim_state = AS_SURFSWIM;
							l->goal_anim_state = AS_SURFSWIM;
							break;
					}

					l->gravity_status = 0;
					l->fallspeed = 0;
					lara.dive_count = 0;
					l->pos.x_rot = 0;
					l->pos.z_rot = 0;
					lara.torso_x_rot = 0;
					lara.torso_y_rot = 0;
					lara.head_x_rot = 0;
					lara.head_y_rot = 0;
					UpdateLaraRoom(l, 0);
				}

				break;

			case LW_UNDERWATER:

				if (!room_water_state) {
					if (wd != NO_HEIGHT && abs(hfw) < CLICK_SIZE) {
						lara.water_status = LW_SURFACE;
						l->pos.y_pos = wh;
						l->anim_number = 114;
						l->frame_number = anims[114].frame_base;
						l->current_anim_state = AS_SURFTREAD;
						l->goal_anim_state = AS_SURFTREAD;
						l->fallspeed = 0;
						lara.dive_count = 11;
						l->pos.x_rot = 0;
						l->pos.z_rot = 0;
						lara.torso_x_rot = 0;
						lara.torso_y_rot = 0;
						lara.head_x_rot = 0;
						lara.head_y_rot = 0;
						UpdateLaraRoom(l, -381);
						if (!get_game_mod_level_lara_info(gfCurrentLevel)->disable_hardcoded_breath_sound)
							SoundEffect(SFX_LARA_BREATH, &l->pos, SFX_ALWAYS);
					} else {
						lara.water_status = LW_ABOVE_WATER;
						l->anim_number = ANIM_FALLDOWN;
						l->frame_number = anims[ANIM_FALLDOWN].frame_base;
						l->current_anim_state = AS_FORWARDJUMP;
						l->goal_anim_state = AS_FORWARDJUMP;
						l->speed = l->fallspeed / 4;
						l->gravity_status = 1;
						l->fallspeed = 0;
						l->pos.x_rot = 0;
						l->pos.z_rot = 0;
						lara.torso_x_rot = 0;
						lara.torso_y_rot = 0;
						lara.head_x_rot = 0;
						lara.head_y_rot = 0;
					}
				}

				break;

			case LW_SURFACE:

				if (!room_water_state) {
					if (hfw <= CLICK_SIZE) {
						lara.water_status = LW_ABOVE_WATER;
						l->anim_number = ANIM_FALLDOWN;
						l->frame_number = anims[ANIM_FALLDOWN].frame_base;
						l->goal_anim_state = AS_FORWARDJUMP;
						l->current_anim_state = AS_FORWARDJUMP;
						l->speed = l->fallspeed / 4;
						l->gravity_status = 1;
					} else {
						lara.water_status = LW_WADE;
						l->anim_number = ANIM_BREATH;
						l->frame_number = anims[ANIM_BREATH].frame_base;
						l->current_anim_state = AS_STOP;
						l->goal_anim_state = AS_WADE;
						AnimateItem(l);
					}

					l->fallspeed = 0;
					l->pos.x_rot = 0;
					l->pos.z_rot = 0;
					lara.torso_x_rot = 0;
					lara.torso_y_rot = 0;
					lara.head_x_rot = 0;
					lara.head_y_rot = 0;
				}

				break;
		}
	}

	if (tomb4.reverb == REVERB_LARA_ROOM)
		S_SetReverbType(room[l->room_number].ReverbType);
	else
		S_SetReverbType(room[camera.pos.room_number].ReverbType);

	if (l->hit_points <= 0) {
		l->hit_points = -1;

		if (!lara.death_count)
			S_CDStop();

		lara.death_count++;

		if (l->flags & IFL_INVISIBLE) {
			lara.death_count++;
			return;
		}
	}

	switch (lara.water_status) {
		case LW_ABOVE_WATER:
		case LW_WADE:

			if (room[lara_item->room_number].flags & ROOM_SWAMP && lara.water_surface_dist < -775) {
				if (lara_item->hit_points >= 0) {
					lara.air -= 6;

					if (lara.air < 0) {
						lara.air = -1;
						lara_item->hit_points -= 10;
					}
				}
			} else if (lara.vehicle == NO_ITEM) {
				lara.air = DEFAULT_LARA_MAX_OXYGEN;
			}

			LaraAboveWater(l, lara_coll);
			break;

		case LW_UNDERWATER:

			if (l->hit_points >= 0) {
				// NGLE
				if (!NGLaraHasInfiniteAir()) {
					lara.air--;

					if (lara.air < 0) {
						lara.air = -1;
						l->hit_points -= 5;
					}
				}
			}

			LaraUnderWater(l, lara_coll);
			break;

		case LW_SURFACE:

			if (l->hit_points >= 0) {
				lara.air += 10;

				if (lara.air > DEFAULT_LARA_MAX_OXYGEN)
					lara.air = DEFAULT_LARA_MAX_OXYGEN;
			}

			LaraSurface(l, lara_coll);
			break;

		case LW_FLYCHEAT:
			LaraCheat(l, lara_coll);
			break;
	}

	savegame.Game.Distance += phd_sqrt(SQUARE(l->pos.x_pos - oldx) + SQUARE(l->pos.y_pos - oldy) + SQUARE(l->pos.z_pos - oldz));
}
