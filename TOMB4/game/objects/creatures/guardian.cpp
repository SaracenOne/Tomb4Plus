#include "../../../tomb4/pch.h"
#include "../../../global/types.h"

#include "guardian.h"
#include "../../../specific/function_stubs.h"
#include "../../objects.h"
#include "../../control.h"
#include "../../items.h"
#include "../../sphere.h"
#include "../../effect2.h"
#include "../../tomb4fx.h"
#include "../../../specific/3dmath.h"
#include "../../sound.h"
#include "../../switch.h"
#include "../../draw.h"
#include "../../traps.h"
#include "../../gameflow.h"
#include "../../lara.h"
#include "../../../tomb4/mod_config.h"

BITE_INFO Base[5] = {
	{0, -640, 0, 0},
	{-188, -832, 440, 0},
	{188, -832, -440, 0},
	{440, -832, 188, 0},
	{-440, -832, -188, 0}
};

BITE_INFO Eye[2] = {
	{0, 0, 0, 1},
	{0, 0, 0, 2}
};

static GUARDIAN_TARGET gt;

void InitialiseGuardian(int16_t item_number) {
	ITEM_INFO* item;
	int16_t* aptr;
	int16_t* bptr;
	int16_t angle;

	MOD_LEVEL_OBJECTS_INFO *objects_info = get_game_mod_level_objects_info(gfCurrentLevel);

	item = &items[item_number];
	item->data = game_malloc(20);
	aptr = (int16_t*)item->data;

	for (int i = 0; i < level_items; i++) {
		if (items[i].object_number == objects_info->laser_head_base_slot) {
			aptr[0] = i;
			break;
		}
	}

	angle = 0;
	bptr = &aptr[1];

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < level_items; j++) {
			if (items[j].object_number == objects_info->laser_head_tentacle_slot && items[j].pos.y_rot == angle) {
				bptr[i] = j;
				break;
			}
		}

		angle += 0x2000;
	}

	for (int i = 0; i < level_items; i++) {
		if (items[i].object_number == PUZZLE_ITEM4) {
			aptr[9] = i;
			items[i].status = ITEM_INVISIBLE;
			break;
		}
	}

	item->pos.y_pos -= 640;
	item->item_flags[1] = (int16_t)item->pos.y_pos - (HALF_BLOCK_SIZE + HALF_CLICK_SIZE);
	item->current_anim_state = 0;
	item->item_flags[3] = 90;
	memset(&gt, 0, sizeof(GUARDIAN_TARGET));
}

void TriggerGuardianSparks(GAME_VECTOR* pos, int32_t size, int32_t rgb, int32_t power) {
	SPARKS* sptr;
	int32_t rnd;

	for (int i = 0; i < size; i++) {
		sptr = &spark[GetFreeSpark()];
		sptr->On = 1;
		sptr->sR = CLRB(rgb);	//BGR
		sptr->sG = CLRG(rgb);
		sptr->sB = CLRR(rgb);
		sptr->dR = 0;
		sptr->dG = 0;
		sptr->dB = 0;
		sptr->ColFadeSpeed = 9 << power;
		sptr->FadeToBlack = 0;
		sptr->Life = 9 << power;
		sptr->sLife = 9 << power;
		sptr->TransType = 2;

		rnd = GetRandomControl();
		sptr->x = pos->x;
		sptr->y = pos->y;
		sptr->z = pos->z;
		sptr->Xvel = ((GetRandomControl() & 0xFFF) - (BLOCK_SIZE * 2)) << power;
		sptr->Yvel = ((rnd & 0xFFF) - (BLOCK_SIZE * 2)) << power;
		sptr->Zvel = ((GetRandomControl() & 0xFFF) - (BLOCK_SIZE * 2)) << power;

		sptr->Gravity = rnd >> 7 & 0x1F;
		sptr->Flags = SF_NONE;
		sptr->MaxYvel = 0;
		sptr->Friction = 34 << power;
	}
}

void TriggerBaseLightning(ITEM_INFO* item) {
	LIGHTNING_STRUCT* lptr;
	PHD_VECTOR s, d;
	int16_t* bptr;
	int16_t g, b, fade;

	g = (GetRandomControl() & 0x1F) + 128;
	b = (GetRandomControl() & 0x1F) + 64;
	fade = item->item_flags[3];

	if (fade > 32)
		fade = 32;
	else {
		g = g * fade >> 5;
		b = b * fade >> 5;
	}

	bptr = (int16_t*)item->data;
	d.x = Base[0].x;
	d.y = Base[0].y;
	d.z = Base[0].z;
	GetJointAbsPosition(&items[bptr[0]], &d, 0);

	for (int i = 0; i < 4; i++) {
		if (item->item_flags[3] & 15) {
			lptr = gt.blptr[i];

			if (lptr) {
				lptr->r = (uint8_t)b;
				lptr->g = (uint8_t)g;
				lptr->b = 0;
				lptr->Life = 50;
				continue;
			}
		}

		s.x = Base[i + 1].x;
		s.y = Base[i + 1].y;
		s.z = Base[i + 1].z;
		GetJointAbsPosition(&items[bptr[0]], &s, 0);
		gt.blptr[i] = TriggerLightning(&s, &d, (GetRandomControl() & 7) + 8, (0x240000 | g) << 8 | b, 13, 48, 3);
	}

	if (GlobalCounter & 1) {
		for (int i = 0; i < 2; i++) {
			if (item->mesh_bits & 2 * Eye[i].mesh_num) {
				s.x = 0;
				s.y = 0;
				s.z = 0;
				GetJointAbsPosition(item, &s, Eye[i].mesh_num);
				TriggerLightningGlow(s.x, s.y, s.z, RGBA(0, g, b, (GetRandomControl() & 3) + fade));
				TriggerGuardianSparks((GAME_VECTOR*)&s, 3, RGBONLY(0, g, b), 0);
			}
		}

		TriggerLightningGlow(d.x, d.y, d.z, RGBA(0, g, b, (GetRandomControl() & 3) + fade + 8));
		TriggerDynamic(d.x, d.y, d.z, (GetRandomControl() & 3) + 16, 0, g, b);
	}

	if (!(GlobalCounter & 3))
		TriggerLightning(&d, (PHD_VECTOR*)&item->pos, (GetRandomControl() & 7) + 8, RGBA(0, g, b, 24), 13, 64, 3);

	TriggerGuardianSparks((GAME_VECTOR*)&d, 3, RGBONLY(0, g, b), 1);
}

void GuardianControl(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	LIGHTNING_STRUCT* lptr;
	PHD_VECTOR pos3;
	GAME_VECTOR	eye, s, d;
	int16_t* bounds;
	int16_t* aptr;
	int32_t DB[6];
	int32_t adiff, dx, dy, dz, dx1, dy1, dz1, lp1, x, y, z, tx, ty, tz, farflag;
	int16_t angles[2], tbounds[6];
	int16_t arm_item;
	uint16_t a1, a2, lp;

	MOD_LEVEL_AUDIO_INFO *audio_info = get_game_mod_level_audio_info(gfCurrentLevel);

	item = &items[item_number];
	aptr = (int16_t*)item->data;

	if (!item->item_flags[0]) {
		if (item->item_flags[2] < 8) {
			if (!(GlobalCounter & 7)) {
				arm_item = aptr[item->item_flags[2] + 1];
				AddActiveItem(arm_item);
				item2 = &items[arm_item];
				item2->status = ITEM_ACTIVE;
				item2->flags |= IFL_CODEBITS;
				item->item_flags[2]++;
			}
		} else if (item->pos.y_pos > item->item_flags[1]) {
			item->fallspeed += 3;

			if (item->fallspeed > 32)
				item->fallspeed = 32;

			item->pos.y_pos -= item->fallspeed;
		} else {
			s.x = 0;
			s.y = 168;
			s.z = 248;
			s.room_number = item->room_number;
			GetJointAbsPosition(item, (PHD_VECTOR*)&s, 0);

			d.x = 0;
			d.y = 0;
			d.z = 0;
			GetJointAbsPosition(lara_item, (PHD_VECTOR*)&d, LM_HEAD);

			if (LOS(&s, &d)) {
				item->item_flags[0]++;
				item->item_flags[1] = (int16_t)item->pos.y_pos;
				item->item_flags[2] = 0xA50;
			}
		}
	} else if (item->item_flags[0] <= 2) {
		item->trigger_flags++;
		item->pos.y_pos = item->item_flags[1] - (HALF_CLICK_SIZE * phd_sin(item->item_flags[2]) >> W2V_SHIFT);
		item->item_flags[2] += DEGREES_TO_ROTATION(3);
		s.x = 0;
		s.y = 168;
		s.z = 248;
		s.room_number = item->room_number;
		GetJointAbsPosition(item, (PHD_VECTOR*)&s, 0);

		if (item->item_flags[0] == 1) {
			d.x = 0;
			d.y = 0;
			d.z = 0;
			GetJointAbsPosition(lara_item, (PHD_VECTOR*)&d, LM_HEAD);
			dx = s.x - d.x;
			dy = s.y - d.y;
			dz = s.z - d.z;
			adiff = phd_sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));

			if (LOS(&s, &d) && adiff <= 0x2000 && lara_item->hit_points > 0 && !lara.burn && (gt.x || gt.y || gt.z)) {
				d.x = 0;
				d.y = 0;
				d.z = 0;
				GetJointAbsPosition(lara_item, (PHD_VECTOR*)&d, LM_HIPS);
				gt.TrackSpeed = 3;
				gt.TrackLara = 1;
			} else {
				farflag = !(GetRandomControl() & 0x7F) && item->trigger_flags > 150;
				lp1 = item->item_flags[3];
				item->item_flags[3]--;

				if (lp1 > 0 && !farflag) {
					d.x = gt.x;
					d.y = gt.y;
					d.z = gt.z;
				} else {
					a1 = uint16_t(4096 - (GetRandomControl() >> 2));

					if (farflag)
						a2 = (GetRandomControl() & 0x3FFF) + item->pos.y_rot + 24576;
					else
						a2 = uint16_t(GetRandomControl() << 1);

					lp = (GetRandomControl() & 0x1FFF) + 0x2000;
					d.x = s.x + ((lp * phd_cos(a1) >> W2V_SHIFT) * phd_sin(a2) >> W2V_SHIFT);
					d.y = s.y + (lp * phd_sin(a1) >> W2V_SHIFT);
					d.z = s.z + ((lp * phd_cos(a1) >> W2V_SHIFT) * phd_cos(a2) >> W2V_SHIFT);

					if (farflag) {
						gt.TrackSpeed = 2;
						item->trigger_flags = 0;
					} else
						gt.TrackSpeed = (GetRandomControl() & 2) + 3;

					item->item_flags[3] = gt.TrackSpeed * ((GetRandomControl() & 3) + 8);
				}

				gt.TrackLara = 0;
			}

			gt.x = d.x;
			gt.y = d.y;
			gt.z = d.z;
		} else {
			gt.TrackSpeed = 3;

			if (JustLoaded) {
				d.x = s.x + ((0x2000 * phd_cos(item->pos.x_rot + 0xD00) >> W2V_SHIFT) * phd_sin(item->pos.y_rot) >> W2V_SHIFT);
				d.y = s.y + (0x2000 * phd_sin(0xD00 - item->pos.x_rot) >> W2V_SHIFT);
				d.z = s.z + ((0x2000 * phd_cos(item->pos.x_rot + 0xD00) >> W2V_SHIFT) * phd_cos(item->pos.y_rot) >> W2V_SHIFT);

				gt.y = d.y;
				gt.x = d.x;
				gt.z = d.z;
			} else {
				d.x = gt.x;
				d.y = gt.y;
				d.z = gt.z;
			}
		}

		phd_GetVectorAngles(gt.x - s.x, gt.y - s.y, gt.z - s.z, angles);
		InterpolateAngle(angles[0], &item->pos.y_rot, &gt.Ydiff, gt.TrackSpeed);
		InterpolateAngle(angles[1] + 0xD00, &item->pos.x_rot, &gt.Xdiff, gt.TrackSpeed);

		if (item->item_flags[0] == 1) {
			if (gt.TrackLara) {
				if (!(GetRandomControl() & 0x1F) && abs(gt.Xdiff) < BLOCK_SIZE && abs(gt.Ydiff) < BLOCK_SIZE && !lara_item->fallspeed || !(GetRandomControl() & 0x1FF)) {
					item->item_flags[0]++;
					item->item_flags[3] = 0;
				}
			} else if (!(GetRandomControl() & 0x3F) && item->trigger_flags > 300) {
				item->item_flags[0]++;
				item->trigger_flags = 0;
				item->item_flags[3] = 0;
			}
		} else {
			if (item->item_flags[3] <= 90) {
				SoundEffect(audio_info->god_head_charge_sfx_id, &item->pos, 0);
				TriggerBaseLightning(item);
				item->item_flags[3]++;
			}

			if (item->item_flags[3] >= 90) {
				a1 = (GetRandomControl() & 0x1F) + HALF_CLICK_SIZE;
				a2 = (GetRandomControl() & 0x1F) + QUARTER_CLICK_SIZE;
				lptr = gt.elptr[0];

				if (!lptr)
					lptr = gt.elptr[1];

				if ((item->item_flags[3] <= 90 || !lptr || lptr->Life) && lara_item->hit_points > 0 && !lara.burn) {
					if (item->item_flags[3] > 90 && lptr && lptr->Life < 16) {
						a1 = a1 * lptr->Life >> 4;
						a2 = a2 * lptr->Life >> 4;
					}

					for (int i = 0; i < 2; i++) {
						if (!(item->mesh_bits & 2 * Eye[i].mesh_num)) {
							if (item->item_flags[3] > 90 && gt.elptr[i]) {
								gt.elptr[i]->Life = 0;
								gt.elptr[i] = 0;
							}
						} else {
							d.x = 0;
							d.y = 0;
							d.z = 0;
							GetJointAbsPosition(item, (PHD_VECTOR*)&d, Eye[i].mesh_num);
							eye.x = d.x + ((0x2000 * phd_cos(angles[1]) >> W2V_SHIFT) * phd_sin(item->pos.y_rot) >> W2V_SHIFT);
							eye.y = d.y + (0x2000 * phd_sin(-angles[1]) >> W2V_SHIFT);
							eye.z = d.z + ((0x2000 * phd_cos(angles[1]) >> W2V_SHIFT) * phd_cos(item->pos.y_rot) >> W2V_SHIFT);

							if (item->item_flags[3] != 90 && gt.elptr[i]) {
								SoundEffect(audio_info->god_head_laser_loop_sfx_id, &item->pos, 0);
								gt.elptr[i]->Point[0].x = d.x;
								gt.elptr[i]->Point[0].y = d.y;
								gt.elptr[i]->Point[0].z = d.z;
							} else {
								d.room_number = item->room_number;
								gt.ricochet[i] = (int8_t)LOS(&d, &eye);
								gt.elptr[i] = TriggerLightning((PHD_VECTOR*)&d, (PHD_VECTOR*)&eye, (GetRandomControl() & 7) + 4, RGBA(0, a1, a2, 0x64), 12, 64, 5);
								StopSoundEffect(audio_info->god_head_charge_sfx_id);
								SoundEffect(audio_info->god_head_blast_sfx_id, &item->pos, 0);
							}

							if (GlobalCounter & 1) {
								TriggerGuardianSparks(&d, 3, RGBONLY(0, a1, a2), 0);
								TriggerLightningGlow(d.x, d.y, d.z, RGBA(0, a1, a2, (GetRandomControl() & 3) + 32));
								TriggerDynamic(d.x, d.y, d.z, (GetRandomControl() & 3) + 16, 0, a1, a2);

								if (!gt.ricochet[i]) {
									TriggerLightningGlow(gt.elptr[i]->Point[3].x, gt.elptr[i]->Point[3].y, gt.elptr[i]->Point[3].z, RGBA(0, a1, a2, (GetRandomControl() & 3) + 16));
									TriggerDynamic(gt.elptr[i]->Point[3].x, gt.elptr[i]->Point[3].y, gt.elptr[i]->Point[3].z, (GetRandomControl() & 3) + 6, 0, a1, a2);
									TriggerGuardianSparks((GAME_VECTOR*)&gt.elptr[i]->Point[3], 3, RGBONLY(0, a1, a2), 0);
								}
							}

							if (!lara.burn) {
								farflag = 0;
								bounds = GetBoundsAccurate(lara_item);

								phd_PushUnitMatrix();
								phd_RotYXZ(lara_item->pos.y_rot, lara_item->pos.x_rot, lara_item->pos.z_rot);
								phd_SetTrans(0, 0, 0);
								mRotBoundingBoxNoPersp(bounds, tbounds);
								phd_PopMatrix();

								DB[0] = lara_item->pos.x_pos + tbounds[0];
								DB[1] = lara_item->pos.x_pos + tbounds[1];
								DB[2] = lara_item->pos.y_pos + tbounds[2];
								DB[3] = lara_item->pos.y_pos + tbounds[3];
								DB[4] = lara_item->pos.z_pos + tbounds[4];
								DB[5] = lara_item->pos.z_pos + tbounds[5];
								dx1 = lara_item->pos.x_pos + ((bounds[0] + bounds[1]) >> 1) - d.x;
								dy1 = lara_item->pos.y_pos + ((bounds[2] + bounds[3]) >> 1) - d.y;
								dz1 = lara_item->pos.z_pos + ((bounds[4] + bounds[5]) >> 1) - d.z;
								adiff = phd_sqrt(SQUARE(dx1) + SQUARE(dy1) + SQUARE(dz1));

								if (adiff < 0x2000) {
									adiff += HALF_BLOCK_SIZE;

									if (adiff < 0x2000) {
										eye.x = d.x + (eye.x - d.x) * adiff / 0x2000;
										eye.y = d.y + (eye.y - d.y) * adiff / 0x2000;
										eye.z = d.z + (eye.z - d.z) * adiff / 0x2000;
									}

									tx = (eye.x - d.x) >> 5;
									ty = (eye.y - d.y) >> 5;
									tz = (eye.z - d.z) >> 5;
									x = d.x;
									y = d.y;
									z = d.z;
									pos3.x = gt.elptr[i]->Point[3].x - d.x;
									pos3.y = gt.elptr[i]->Point[3].y - d.y;
									pos3.z = gt.elptr[i]->Point[3].z - d.z;

									for (lp1 = 0; lp1 < 32; lp1++) {
										if (farflag) {
											farflag--;

											if (!farflag)
												break;
										}

										if (abs(pos3.x) < 280 && abs(pos3.y) < 280 && abs(pos3.z) < 280)
											farflag = 2;

										if (x > DB[0] && x < DB[1] && y > DB[2] && y < DB[3] && z > DB[4] && z < DB[5]) {
											lp1 = 999;
											break;
										}

										x += tx;
										y += ty;
										z += tz;
										pos3.x -= tx;
										pos3.y -= ty;
										pos3.z -= tz;
									}

									if (lp1 == 999) {
										LaraBurn();
#ifdef TR5_BEHAVIOUR
										lara.BurnCount = 48;
										lara.BurnBlue = 2;
#endif
										lara_item->hit_points = 0;
									}
								}
							}
						}
					}
				} else {
					if (lptr) {
						gt.elptr[0] = 0;
						gt.elptr[1] = 0;
					}

					item->item_flags[0] = 1;
					item->trigger_flags = 0;
				}
			}
		}
	} else {
#ifdef TR5_BEHAVIOUR
		if (!(GlobalCounter & 7) && item->current_anim_state < 8)
#else
		if (item->current_anim_state < 8)
#endif
		{
			items[aptr[item->current_anim_state + 1]].goal_anim_state = 2;
			item->current_anim_state++;
		}

		if (item->current_anim_state > 0) {
			for (int i = 0; i < 8; i++) {
				arm_item = aptr[i + 1];
				item2 = &items[arm_item];

				if (item2->anim_number == objects[item2->object_number].anim_index + 1 && item2->frame_number == anims[item2->anim_number].frame_end && item2->mesh_bits & 1) {
					SoundEffect(audio_info->god_head_smash_sfx_id, &item2->pos, 0);
					ExplodeItemNode(item2, 0, 0, 128);
					KillItem(arm_item);
				}
			}
		}

		item->pos.y_pos = item->item_flags[1] - ((192 - item->speed) * phd_sin(item->item_flags[2]) >> W2V_SHIFT);
		item->item_flags[2] += 182 * item->speed;

		if (!(GlobalCounter & 7)) {
			item->item_flags[3] = (GetRandomControl() & 0x3FFF) + item->pos.y_rot - (BLOCK_SIZE * 4);
			item->trigger_flags = (GetRandomControl() & 0x1000) - (BLOCK_SIZE * 2);
		}

		InterpolateAngle(item->item_flags[3], &item->pos.y_rot, 0, 2);
		InterpolateAngle(item->trigger_flags, &item->pos.x_rot, 0, 2);
		item->speed++;

		if (item->speed > 136) {
			ExplodeItemNode(&items[aptr[0]], 0, 0, 0x80);
			KillItem(aptr[0]);
			ExplodeItemNode(item, 0, 0, 0x80);
			item->pos.y_pos -= CLICK_SIZE;
			TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -2, 2, item->room_number);
			TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, 0, 2, item->room_number);
			TriggerShockwave((PHD_VECTOR*)&item->pos, 0xA00020, 64, 0x24008040, 0);
			TriggerShockwave((PHD_VECTOR*)&item->pos, 0xA00020, 64, 0x24008040, 0x3000);
			TriggerShockwave((PHD_VECTOR*)&item->pos, 0xA00020, 64, 0x24008040, 0x6000);
			items[aptr[9]].pos.y_pos = item->pos.y_pos;
			TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, 0);
			SoundEffect(audio_info->god_head_blast_sfx_id, &item->pos, 0x800000 | SFX_SETPITCH);
			SoundEffect(SFX_EXPLOSION2, &item->pos, 0x1400000 | SFX_SETPITCH);
			SoundEffect(SFX_EXPLOSION1, &item->pos, 0);
			SoundEffect(SFX_EXPLOSION1, &item->pos, 0x400000 | SFX_SETPITCH);
			KillItem(item_number);
#ifndef TR5_BEHAVIOUR
			item->after_death = 128;
#endif
		}
	}

	if (item->item_flags[0] < 3) {
#ifdef TR5_BEHAVIOUR
		for (lp = 0; lp < 8; lp++) {
			item2 = &items[aptr[lp + 1]];

			if (item2->anim_number == objects[item2->object_number].anim_index && item2->frame_number != anims[item2->anim_number].frame_end)
				break;
		}
#else
		lp = 8;
#endif

		if (lp == 8 && !(item->mesh_bits & 6)) {
			if (gt.elptr[0])
				gt.elptr[0]->Life = 2;

			if (gt.elptr[1])
				gt.elptr[1]->Life = 2;

			gt.elptr[0] = 0;
			gt.elptr[1] = 0;
			item->item_flags[0] = 3;
			item->item_flags[3] = (GetRandomControl() & 0x1000) + item->pos.y_rot - (BLOCK_SIZE * 2);
			item->speed = 3;
			item->trigger_flags = (GetRandomControl() & 0x1000) + item->pos.x_rot - (BLOCK_SIZE * 2);
		}
	}
}
