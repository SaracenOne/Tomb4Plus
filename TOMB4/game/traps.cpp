#include "../tomb4/pch.h"

#include "../specific/function_stubs.h"
#include "../specific/3dmath.h"
#include "../specific/output.h"
#include "../specific/input.h"

#include "gameflow.h"
#include "traps.h"
#include "control.h"
#include "effect2.h"
#include "sound.h"
#include "tomb4fx.h"
#include "effects.h"
#include "items.h"
#include "draw.h"
#include "objects.h"
#include "sphere.h"
#include "lara_states.h"
#include "collide.h"
#include "delstuff.h"
#include "switch.h"
#include "deltapak.h"
#include "camera.h"
#include "lara.h"
#include "debris.h"
#include "objects/vehicles/jeep.h"

#include "../tomb4/mod_config.h"

#include "../tomb4/tomb4plus/t4plus_items.h"
#include "../specific/file.h"

int16_t SPxzoffs[8] = { 0, 0, 0x200, 0, 0, 0, -0x200, 0 };
int16_t SPyoffs[8] = { -0x400, 0, -0x200, 0, 0, 0, -0x200, 0 };
int16_t SPDETyoffs[8] = { 0x400, 0x200, 0x200, 0x200, 0, 0x200, 0x200, 0x200 };

static uint8_t Flame3xzoffs[16][2] = {
	{ 9, 9 },
	{ 24, 9 },
	{ 40, 9 },
	{ 55, 9 },
	{ 9, 24 },
	{ 24, 24},
	{ 40, 24 },
	{ 55, 24 },
	{ 9, 40 },
	{ 24, 40 },
	{ 40, 40 },
	{ 55, 40 },
	{ 9, 55 },
	{ 24, 55 },
	{ 40, 55 },
	{ 55, 55 }
};

int16_t floor_fires[16 * 3] = {	//16 points on the burning floor that spawn fires!
	//xoff, zoff, size
	-96, 1216, 2,
	560, 736, 2,
	-432, -976, 2,
	-64, -128, 2,
	824, 64, 2,
	456, -352, 1,
	392, 352, 1,
	1096, 608, 1,
	-424, -416, 1,
	520, 1152, 1,
	-248, 516, 1,
	-808, 80, 1,
	-1192, -384, 0,
	-904, -864, 0,
	-136, -912, 0,
	184, 608, 0
};

int16_t deadly_floor_fires[4 * 2] = {	//4 points on the burning floor that kill Lara if she is too close at explode time
	//xoff, zoff
	-HALF_BLOCK_SIZE, -HALF_BLOCK_SIZE,
	0, 0,
	HALF_BLOCK_SIZE, HALF_BLOCK_SIZE,
	0, HALF_BLOCK_SIZE + CLICK_SIZE
};

static PHD_VECTOR FloorTrapDoorPos = { 0, 0, -655 };
static PHD_VECTOR CeilingTrapDoorPos = { 0, 1056, -480 };
static int16_t FloorTrapDoorBounds[12] = { -256, 256, 0, 0, -1024, -256, -1820, 1820, -5460, 5460, -1820, 1820 };
static int16_t CeilingTrapDoorBounds[12] = { -256, 256, 0, 900, -768, -256, -1820, 1820, -5460, 5460, -1820, 1820 };

int8_t LibraryTab[MAX_LIBRARY_TABS];

void FlameEmitterControl(int16_t item_number) {
	ITEM_INFO* item;
	uint32_t distance;
	int32_t x, z;

	item = &items[item_number];

	if (!TriggerActive(item)) {
		if (item->trigger_flags >= 0 && item->trigger_flags < MAX_LIBRARY_TABS)
			LibraryTab[item->trigger_flags] = 0;

		return;
	}

	if (item->trigger_flags < 0) {
		if ((-item->trigger_flags & 7) == 2 || (-item->trigger_flags & 7) == 7) {
			SoundEffect(SFX_FLAME_EMITTER, &item->pos, 0);
			TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
			TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos,
			               (GetRandomControl() & 3) + 20, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
		} else {
			if (item->item_flags[0]) {
				if (item->item_flags[1])
					item->item_flags[1] -= item->item_flags[1] >> 2;

				if (item->item_flags[2])
					item->item_flags[2] += 8;

				item->item_flags[0]--;

				if (!item->item_flags[0])
					item->item_flags[3] = (GetRandomControl() & 0x3F) + 150;
			} else {
				item->item_flags[3]--;

				if (!item->item_flags[3]) {
					if (-item->trigger_flags >> 3)
						item->item_flags[0] = (GetRandomControl() & 0x1F) + 30 * (-item->trigger_flags >> 3);
					else
						item->item_flags[0] = (GetRandomControl() & 0x3F) + 60;
				}

				if (item->item_flags[2])
					item->item_flags[2] -= 8;

				if (item->item_flags[1] > -8192)
					item->item_flags[1] -= 512;
			}

			if (item->item_flags[2])
				AddFire(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 0, item->room_number, item->item_flags[2] & 0xFF);

			if (item->item_flags[1]) {
				SoundEffect(SFX_FLAME_EMITTER, &item->pos, 0);

				if (item->item_flags[1] > -8192)
					TriggerSuperJetFlame(item, item->item_flags[1], GlobalCounter & 1);
				else
					TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);

				TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (-item->item_flags[1] >> 10) - (GetRandomControl() & 1) + 16, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
			} else
				TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 10 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
		}

		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
	} else {
		if (item->trigger_flags >= 0 && item->trigger_flags < MAX_LIBRARY_TABS)
			LibraryTab[item->trigger_flags] = 1;
		AddFire(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, item->room_number, 0);
		TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16 - (GetRandomControl() & 1),
		               (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);

		if (!lara.burn && ItemNearLara(&item->pos, 600)) {
			x = lara_item->pos.x_pos - item->pos.x_pos;
			z = lara_item->pos.z_pos - item->pos.z_pos;
			distance = SQUARE(x) + SQUARE(z);

			if (distance < 0x40000)
				LaraBurn();
		}
	}
}

static int32_t OnTwoBlockPlatform(ITEM_INFO* item, int32_t x, int32_t z) {
	int32_t tx, tz;

	if (!item->mesh_bits)
		return 0;

	x >>= 10;
	z >>= 10;
	tx = item->pos.x_pos >> 10;
	tz = item->pos.z_pos >> 10;

	if (!item->pos.y_rot && (x == tx || x == tx - 1) && (z == tz || z == tz + 1))
		return 1;

	if (item->pos.y_rot == -0x8000 && (x == tx || x == tx + 1) && (z == tz || z == tz - 1))
		return 1;

	if (item->pos.y_rot == 0x4000 && (z == tz || z == tz - 1) && (x == tx || x == tx + 1))
		return 1;

	if (item->pos.y_rot == -0x4000 && (z == tz || z == tz - 1) && (x == tx || x == tx - 1))
		return 1;

	return 0;
}

void TwoBlockPlatformFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height) {
	if (OnTwoBlockPlatform(item, x, z)) {
		if (y <= item->pos.y_pos + 32 && item->pos.y_pos < *height) {
			*height = item->pos.y_pos;
			OnObject = 1;
			height_type = 0;
		}
	}
}

void TwoBlockPlatformCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height) {
	if (OnTwoBlockPlatform(item, x, z)) {
		if (y > item->pos.y_pos + (QUARTER_CLICK_SIZE / 2) && item->pos.y_pos > *height)
			*height = item->pos.y_pos + CLICK_SIZE;
	}
}

void ControlTwoBlockPlatform(int16_t item_number) {
	ITEM_INFO* item;
	int32_t height;
	int16_t room_number;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->trigger_flags) {
		if (item->pos.y_pos > item->item_flags[0] - (int32_t(item->trigger_flags & 0xFFFFFFF0) << 4))
			item->pos.y_pos -= item->trigger_flags & 0xF;

		room_number = item->room_number;
		item->floor = GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number),
		                        item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (room_number != item->room_number)
			ItemNewRoom(item_number, room_number);
	} else {
		OnObject = 0;
		height = lara_item->pos.y_pos + 1;
		TwoBlockPlatformFloor(item, lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &height);

		if (!OnObject || lara_item->anim_number == 89)
			item->item_flags[1] = -1;
		else
			item->item_flags[1] = 1;

		if (item->item_flags[1] > 0) {
			if (item->pos.y_pos >= item->item_flags[0] + 128)
				item->item_flags[1] = -1;
			else {
				SoundEffect(SFX_RUMBLE_NEXTDOOR, &item->pos, SFX_DEFAULT);
				item->pos.y_pos += 4;
			}
		} else if (item->item_flags[1] < 0) {
			if (item->pos.y_pos <= item->item_flags[0])
				item->item_flags[1] = 1;
			else {
				SoundEffect(SFX_RUMBLE_NEXTDOOR, &item->pos, SFX_DEFAULT);
				item->pos.y_pos -= 4;
			}
		}
	}
}

void ControlJobySpike(int16_t item_number) {
	ITEM_INFO* item;
	int16_t* frm[2];
	int32_t rate, y, h;

	item = &items[item_number];

	// T4Plus: Animation safety check
	if (item->anim_number < 0 || item->anim_number >= num_anims) {
		return;
	}

	if (TriggerActive(item)) {
		SoundEffect(SFX_METAL_SCRAPE_LOOP, &item->pos, SFX_DEFAULT);
		GetFrames(lara_item, frm, &rate);
		y = lara_item->pos.y_pos + frm[0][2];
		h = item->pos.y_pos + (3328 * item->item_flags[1] >> 12);

		if (lara_item->hit_points > 0 && h > y && abs(item->pos.x_pos - lara_item->pos.x_pos) < HALF_BLOCK_SIZE && abs(item->pos.z_pos - lara_item->pos.z_pos) < HALF_BLOCK_SIZE) {
			DoBloodSplat(lara_item->pos.x_pos + (GetRandomControl() & 0x7F) - 64, GetRandomControl() % (h - y) + y, lara_item->pos.z_pos + (GetRandomControl() & 0x7F) - 64, (GetRandomControl() & 3) + 2, (int16_t)(2 * GetRandomControl()), item->room_number);
			lara_item->hit_points -= 8;
		}

		if (!item->item_flags[2]) {
			if (item->item_flags[0] < 4096)
				item->item_flags[0] += (item->item_flags[0] >> 6) + 2;
		} else if (item->item_flags[0] > -4096)
			item->item_flags[0] += (item->item_flags[0] >> 6) - 2;

		if (item->item_flags[1] < item->item_flags[3])
			item->item_flags[1] += 3;

		item->pos.y_rot += item->item_flags[0];
	}
}

void DrawScaledSpike(ITEM_INFO* item) {
	PHD_VECTOR scale;
	ROOM_INFO* r;
	int16_t** meshpp;
	int16_t* frm[2];
	int32_t rate, clip, lp;

	// T4Plus: Animation safety check
	if (item->anim_number < 0 || item->anim_number >= num_anims) {
		return;
	}

	if (item->object_number != TEETH_SPIKES || item->item_flags[1]) {
		if ((item->object_number == RAISING_BLOCK1 || item->object_number == RAISING_BLOCK2) && item->trigger_flags && !item->item_flags[0]) {
			for (lp = 1; lp < MAX_LIBRARY_TABS; lp++) {
				if (!LibraryTab[lp])
					break;
			}

			if (lp == 8) {
				item->item_flags[0] = 1;
				item->touch_bits = 0;
				AddActiveItem(int16_t(item - items));
				item->flags |= IFL_CODEBITS;
				item->status = ITEM_ACTIVE;
			}
		}

		r = &room[item->room_number];
		phd_left = r->left;
		phd_right = r->right;
		phd_top = r->top;
		phd_bottom = r->bottom;
		GetFrames(item, frm, &rate);
		phd_PushMatrix();
		phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
		phd_RotX(item->pos.x_rot);
		phd_RotZ(item->pos.z_rot);
		phd_RotY(item->pos.y_rot);
		clip = S_GetObjectBounds(frm[0]);

		if (clip) {
			meshpp = &meshes[objects[item->object_number].mesh_index];

			if (item->object_number == EXPANDING_PLATFORM) {
				scale.x = 0x4000;
				scale.y = 0x4000;
				scale.z = item->item_flags[1] << 2;
			} else {
				scale.y = item->item_flags[1] << 2;

				if (item->object_number != JOBY_SPIKES) {
					scale.x = 0x4000;
					scale.z = 0x4000;
				} else {
					scale.x = 12288;
					scale.z = 12288;
				}
			}

			ScaleCurrentMatrix(&scale);
			CalculateObjectLighting(item, frm[0]);
			phd_PutPolygons(*meshpp, clip);
		}

		phd_left = 0;
		phd_right = phd_winwidth;
		phd_top = 0;
		phd_bottom = phd_winheight;
		phd_PopMatrix();
	}
}

void ControlSlicerDicer(int16_t item_number) {
	ITEM_INFO* item;
	int32_t distance;
	int16_t room_number;

	item = &items[item_number];
	SoundEffect(SFX_METAL_SCRAPE_LOOP, &item->pos, SFX_DEFAULT);
	SoundEffect(SFX_METAL_SCRAPE_LOOP1, &item->pos, SFX_DEFAULT);
	distance = ((BLOCK_SIZE * 4) + HALF_BLOCK_SIZE) * phd_cos(item->trigger_flags) >> W2V_SHIFT;
	item->pos.x_pos = CLICK_SIZE * item->item_flags[0] + (phd_sin(item->pos.y_rot) * distance >> W2V_SHIFT);
	item->pos.y_pos = CLICK_SIZE * item->item_flags[1] - (((BLOCK_SIZE * 4) + HALF_BLOCK_SIZE) * phd_sin(item->trigger_flags) >> W2V_SHIFT);
	item->pos.z_pos = CLICK_SIZE * item->item_flags[2] + (phd_cos(item->pos.y_rot) * distance >> W2V_SHIFT);
	item->trigger_flags += 0xaa;
	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	AnimateItem(item);
}

void ControlSprinkler(int16_t item_number) {
	ITEM_INFO* item;
	DRIP_STRUCT* drip;
	SPARKS* sptr;
	SMOKE_SPARKS* smokeptr;
	int32_t vel, size;

	item = &items[item_number];

	if (item->item_flags[0] < 1200) {
		item->item_flags[0]++;

		if (!(wibble & 0xF) && (item->item_flags[0] <= 600 || GetRandomControl() % (item->item_flags[0] - 600) < 100)) {
			drip = &Drips[GetFreeDrip()];
			drip->x = (GetRandomControl() & 0x1F) + item->pos.x_pos - 16;
			drip->y = (GetRandomControl() & 0x1F) + item->pos.y_pos - 944;
			drip->z = (GetRandomControl() & 0x1F) + item->pos.z_pos - 16;
			drip->On = 1;
			drip->R = 112;
			drip->G = (GetRandomControl() & 0x1F) + 128;
			drip->B = (GetRandomControl() & 0x1F) + 128;
			drip->Yvel = (GetRandomControl() & 0xF) + 16;
			drip->Gravity = (GetRandomControl() & 0x1F) + 32;
			drip->Life = (GetRandomControl() & 0x1F) + 16;
			drip->RoomNumber = item->room_number;
		}
	}

	if (item->item_flags[0] <= 600) {
		SoundEffect(SFX_SANDHAM_IN_THE_HOUSE, &item->pos, SFX_DEFAULT);

		for (int i = 0; i < 3; i++) {
			sptr = &spark[GetFreeSpark()];
			sptr->On = 1;
			sptr->sR = 112;
			sptr->sG = (GetRandomControl() & 0x1F) + 128;
			sptr->sB = (GetRandomControl() & 0x1F) + 128;
			sptr->dR = sptr->sR >> 1;
			sptr->dG = sptr->sG >> 1;
			sptr->dB = sptr->sB >> 1;
			sptr->ColFadeSpeed = 4;
			sptr->FadeToBlack = 8;
			sptr->Life = 20;
			sptr->sLife = 20;
			sptr->TransType = 2;
			vel = ((GlobalCounter & 3) << 10) + (GetRandomControl() & 0x3FF);
			sptr->Xvel = -rcossin_tbl[vel << 1] >> 2;
			sptr->Xvel = (sptr->Xvel * (GetRandomControl() & 0xFF)) >> 8;
			sptr->Yvel = -32 - (GetRandomControl() & 0x1F);
			sptr->Zvel = rcossin_tbl[(vel << 1) + 1] >> 2;
			sptr->Zvel = (sptr->Zvel * (GetRandomControl() & 0xFF)) >> 8;
			sptr->x = item->pos.x_pos + (sptr->Xvel >> 3);
			sptr->y = (sptr->Yvel >> 5) + item->pos.y_pos - 928;
			sptr->z = item->pos.z_pos + (sptr->Zvel >> 3);
			sptr->Friction = 4;
			sptr->Flags = GetRandomControl() & SF_NOKILL;
			sptr->Gravity = (GetRandomControl() & 0x3F) + 64;
			sptr->MaxYvel = 0;
		}

		for (int i = 0; i < 1; i++) {
			smokeptr = &smoke_spark[GetFreeSmokeSpark()];
			smokeptr->On = 1;
			smokeptr->sShade = 0;
			smokeptr->dShade = (GetRandomControl() & 0x1F) + 32;
			smokeptr->ColFadeSpeed = 4;
			smokeptr->FadeToBlack = 8 - (GetRandomControl() & 3);
			smokeptr->TransType = 2;
			smokeptr->Life = (GetRandomControl() & 3) + 24;
			smokeptr->sLife = smokeptr->Life;
			smokeptr->x = (GetRandomControl() & 0x1F) + item->pos.x_pos - (QUARTER_CLICK_SIZE / 4);
			smokeptr->y = (GetRandomControl() & 0x1F) + item->pos.y_pos - 944;
			smokeptr->z = (GetRandomControl() & 0x1F) + item->pos.z_pos - (QUARTER_CLICK_SIZE / 4);
			smokeptr->Xvel = 2 * (GetRandomControl() & 0x1FF) - HALF_BLOCK_SIZE;

			if (i)
				smokeptr->Yvel = (GetRandomControl() & 0x1F) - 16;
			else
				smokeptr->Yvel = 2 * (GetRandomControl() & 0x1FF) + HALF_BLOCK_SIZE;

			smokeptr->Zvel = 2 * (GetRandomControl() & 0x1FF) - HALF_BLOCK_SIZE;
			smokeptr->Friction = 3;
			smokeptr->Flags = 16;
			smokeptr->RotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				smokeptr->RotAdd = -64 - (GetRandomControl() & 0x3F);
			else
				smokeptr->RotAdd = (GetRandomControl() & 0x3F) + 64;

			smokeptr->MaxYvel = 0;
			smokeptr->Gravity = -4 - (GetRandomControl() & 3);
			size = (GetRandomControl() & 0xF) + 16;

			if (!i)
				size <<= 2;

			smokeptr->dSize = (uint8_t)size;
			smokeptr->sSize = smokeptr->dSize >> 3;
			smokeptr->Size = smokeptr->dSize >> 3;
		}
	}
}

void ControlMineHelicopter(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* sentry;
	SPHERE* sphere;
	int32_t nSpheres;
	int16_t sentries, fade;

	item = &items[item_number];
	nSpheres = GetSpheres(item, Slist, 1);

	if (item->item_flags[0] < 150) {
		item->item_flags[0]++;
		fade = item->item_flags[0] * 4;

		if (fade > 255)
			fade = 0;

		for (int i = 0; i < nSpheres; i++) {
			sphere = &Slist[i];

			if (!i || i > 5)
				AddFire(sphere->x, sphere->y, sphere->z, 2, item->room_number, fade);
		}

		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);
	} else {
		SoundEffect(SFX_EXPLOSION1, &item->pos, SFX_DEFAULT);
		SoundEffect(SFX_EXPLOSION2, &item->pos, SFX_DEFAULT);
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0x1800000 | SFX_SETPITCH);

		for (int i = 0; i < nSpheres; i++) {
			sphere = &Slist[i];

			if (i >= 7 && i != 9) {
				TriggerExplosionSparks(sphere->x, sphere->y, sphere->y, 3, -2, 0, -item->room_number);
				TriggerExplosionSparks(sphere->x, sphere->y, sphere->y, 3, -1, 0, -item->room_number);
				TriggerShockwave((PHD_VECTOR*)&sphere->x, 0x1300030, (GetRandomControl() & 0x1F) + 112, 0x20806000, 0x800);
			}
		}

		for (int i = 0; i < nSpheres; i++)
			ExplodeItemNode(item, i, 0, -128);

		FlashFadeR = 255;
		FlashFadeG = 192;
		FlashFadeB = 64;
		FlashFader = 32;

		for (sentries = room[item->room_number].item_number; sentries != NO_ITEM; sentries = sentry->next_item) {
			sentry = &items[sentries];

			if (sentry->object_number == SENTRY_GUN)
				sentry->mesh_bits &= ~0x40;
		}

		KillItem(item_number);
	}
}

void MineCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	// Tomb4Plus
	if (lara.water_status == LW_FLYCHEAT) {
		return;
	}

	ITEM_INFO* item;
	ITEM_INFO* mines;

	item = &items[item_number];

	if (item->trigger_flags || item->item_flags[3])
		return;

	if (l->anim_number == ANIM_MINEDETECT && l->frame_number >= anims[ANIM_MINEDETECT].frame_base + 57) {
		for (int i = 0; i < level_items; i++) {
			mines = &items[i];

			if (mines->object_number != MINE || mines->status == ITEM_INVISIBLE || mines->trigger_flags)
				continue;

			TriggerExplosionSparks(mines->pos.x_pos, mines->pos.y_pos, mines->pos.z_pos, 3, -2, 0, mines->room_number);

			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(mines->pos.x_pos, mines->pos.y_pos, mines->pos.z_pos, 3, -1, 0, mines->room_number);

			mines->mesh_bits = 1;
			ExplodeItemNode(mines, 0, 0, -32);
			KillItem(i);

			if (!(GetRandomControl() & 3))
				SoundEffect(SFX_MINE_EXP_OVERLAY, &mines->pos, SFX_DEFAULT);

			mines->status = ITEM_INVISIBLE;
		}
	} else if (TestBoundsCollide(item, l, 512)) {
		TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -2, 0, item->room_number);

		for (int i = 0; i < 2; i++)
			TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -1, 0, item->room_number);

		item->mesh_bits = 1;
		ExplodeItemNode(item, 0, 0, 128);
		KillItem(item_number);
		l->anim_number = ANIM_MINEDEATH;
		l->frame_number = anims[ANIM_MINEDEATH].frame_base;
		l->current_anim_state = AS_DEATH;
		l->speed = 0;
		SoundEffect(SFX_MINE_EXP_OVERLAY, &item->pos, SFX_DEFAULT);
	}
}

void FallingSquishyBlockCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	// Tomb4Plus
	if (lara.water_status == LW_FLYCHEAT) {
		return;
	}

	ITEM_INFO * item;

	item = &items[item_number];

	if (TestBoundsCollide(item, l, coll->radius) && TestCollision(item, l)) {
		if (item->frame_number - anims[item->anim_number].frame_base <= 8) {
			item->frame_number += 2;
			l->hit_points = 0;
			l->current_anim_state = AS_DEATH;
			l->goal_anim_state = AS_DEATH;
			l->anim_number = ANIM_FBLOCK_DEATH;
			l->frame_number = anims[ANIM_FBLOCK_DEATH].frame_base + 50;
			l->fallspeed = 0;
			l->speed = 0;

			for (int i = 0; i < 12; i++)
				TriggerBlood(l->pos.x_pos, l->pos.y_pos - 128, l->pos.z_pos, GetRandomControl() << 1, 3);
		} else if (l->hit_points > 0)
			ItemPushLara(item, l, coll, 0, 1);
	}
}

void ControlFallingSquishyBlock(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (TriggerActive(item)) {
		if (item->item_flags[0] < 60) {
			SoundEffect(SFX_EARTHQUAKE_LOOP, &item->pos, SFX_DEFAULT);
			camera.bounce = (item->item_flags[0] - 92) >> 1;
			item->item_flags[0]++;
		} else {
			if (item->frame_number - anims[item->anim_number].frame_base == 8)
				camera.bounce = -96;

			AnimateItem(item);
		}
	}
}

void ControlLRSquishyBlock(int16_t item_number) {
	ITEM_INFO* item;
	uint16_t ang;
	int16_t frame;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	frame = item->frame_number - anims[item->anim_number].frame_base;

	if (item->touch_bits) {
		ang = (uint16_t)phd_atan(item->pos.z_pos - lara_item->pos.z_pos, item->pos.x_pos - lara_item->pos.x_pos) - item->pos.y_rot;

		if (!frame && ang > 0xA000 && ang < 0xE000) {
			item->item_flags[0] = 9;
			lara_item->hit_points = 0;
			lara_item->pos.y_rot = item->pos.y_rot - 0x4000;
		} else if (frame == 33 && ang > 0x2000 && ang < 0x6000) {
			item->item_flags[0] = 42;
			lara_item->hit_points = 0;
			lara_item->pos.y_rot = item->pos.y_rot + 0x4000;
		}
	}

	if (!item->item_flags[0] || frame != item->item_flags[0])
		AnimateItem(item);
}

void ControlSethBlade(int16_t item_number) {
	ITEM_INFO* item;
	int16_t frame;

	item = &items[item_number];
	*(int32_t*)&item->item_flags[0] = 0;

	if (!TriggerActive(item))
		return;

	if (item->current_anim_state == 2) {
		if (item->item_flags[2] > 1)
			item->item_flags[2]--;
		else if (item->item_flags[2] == 1) {
			item->goal_anim_state = 1;
			item->item_flags[2] = 0;
		} else if (!item->item_flags[2] && item->trigger_flags > 0)
			item->item_flags[2] = item->trigger_flags;
	} else {
		frame = item->frame_number - anims[item->anim_number].frame_base;

		if (frame && frame <= 6)
			*(int32_t*)&item->item_flags[0] = -1;
		else if (frame >= 7 && frame <= 15)
			*(int32_t*)&item->item_flags[0] = 448;
		else
			*(int32_t*)&item->item_flags[0] = 0;

		item->item_flags[3] = 1000;
	}

	AnimateItem(item);
}

void ControlPlinthBlade(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (TriggerActive(item)) {
		if (item->frame_number == anims[item->anim_number].frame_end)
			item->item_flags[3] = 0;
		else
			item->item_flags[3] = 200;

		AnimateItem(item);
	} else
		item->frame_number = anims[item->anim_number].frame_base;
}

void ControlMovingBlade(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (TriggerActive(item)) {
		item->item_flags[3] = 50;
		AnimateItem(item);
	} else
		item->frame_number = anims[item->anim_number].frame_base;
}

void ControlCatwalkBlade(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (TriggerActive(item)) {
		if (item->frame_number == anims[item->anim_number].frame_end || item->frame_number - anims[item->anim_number].frame_base < 38)
			item->item_flags[3] = 0;
		else
			item->item_flags[3] = 100;

		AnimateItem(item);
	} else
		item->frame_number = anims[item->anim_number].frame_base;
}

void ControlBirdBlade(int16_t item_number) {
	ITEM_INFO* item;
	int16_t frame;

	item = &items[item_number];
	item->item_flags[3] = 100;

	if (TriggerActive(item)) {
		frame = item->frame_number - anims[item->anim_number].frame_base;

		if (frame <= 14 || frame >= 31)
			*(int32_t*)&item->item_flags[0] = 0;
		else
			*(int32_t*)&item->item_flags[0] = 6;

		AnimateItem(item);
	} else {
		item->frame_number = anims[item->anim_number].frame_base;
		*(int32_t*)&item->item_flags[0] = 0;
	}
}

void Control4xFloorRoofBlade(int16_t item_number) {
	ITEM_INFO* item;
	int16_t frame;

	item = &items[item_number];

	if (TriggerActive(item)) {
		frame = item->frame_number - anims[item->anim_number].frame_base;

		if (frame <= 5 || frame >= 58 || frame >= 8 && frame <= 54)
			*(int32_t*)&item->item_flags[0] = 0;
		else {
			if (frame > 7)
				item->item_flags[3] = 200;
			else
				item->item_flags[3] = 20;

			*(int32_t*)&item->item_flags[0] = 30;
		}

		AnimateItem(item);
	} else {
		item->frame_number = anims[item->anim_number].frame_base;
		*(int32_t*)&item->item_flags[0] = 0;
	}
}

void ControlSpikeball(int16_t item_number) {
	ITEM_INFO* item;
	int16_t frame;

	item = &items[item_number];
	frame = item->frame_number - anims[item->anim_number].frame_base;

	if (TriggerActive(item)) {
		if ((frame <= 14 || frame >= 24) && (frame < 138 || frame > 140)) {
			if (frame < 141)
				*(int32_t*)&item->item_flags[0] = 0;
			else {
				item->item_flags[3] = 50;
				*(int32_t*)&item->item_flags[0] = 0x7FF800;
			}
		} else {
			item->item_flags[3] = 150;
			*(int32_t*)&item->item_flags[0] = 0x7FF800;
		}

		AnimateItem(item);
	} else {
		item->frame_number = anims[item->anim_number].frame_base;
		*(int32_t*)&item->item_flags[0] = 0;
	}
}

void ControlHammer(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* item2;
	int32_t hammered;		//hammer touched a pushable (Senet lose path)
	int16_t frame, target_item;

	item = &items[item_number];
	frame = item->frame_number - anims[item->anim_number].frame_base;
	item->item_flags[3] = 150;

	if (!TriggerActive(item)) {
		*(int32_t*)&item->item_flags[0] = 0;
		return;
	}

	hammered = 0;

	if (!item->trigger_flags) {
		if (frame < 52)
			*(int32_t*)&item->item_flags[0] = 0xE0;
		else
			*(int32_t*)&item->item_flags[0] = 0;
	} else if (item->current_anim_state == 1 && item->goal_anim_state == 1) {
		if (item->item_flags[2]) {
			if (item->trigger_flags == 3) {
				item->flags &= ~IFL_CODEBITS;
				item->item_flags[2] = 0;
			} else if (item->trigger_flags == 4)
				item->item_flags[2]--;
			else
				item->item_flags[2] = 0;
		} else {
			item->anim_number = objects[HAMMER].anim_index + 1;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = 2;
			item->goal_anim_state = 2;
			item->item_flags[2] = 60;
		}
	} else {
		item->goal_anim_state = 1;

		if (frame < 52)
			*(int32_t*)&item->item_flags[0] = 0x7E0;
		else
			*(int32_t*)&item->item_flags[0] = 0;

		if (frame == 8) {
			if (item->trigger_flags == 2) {
				for (target_item = room[item->room_number].item_number; target_item != NO_ITEM; target_item = item2->next_item) {
					item2 = &items[target_item];

					if (item2->object_number == OBELISK &&
					        item2->pos.y_rot == -0x4000 &&
					        items[item2->item_flags[0]].pos.y_rot == 0x4000 &&
					        !items[item2->item_flags[1]].pos.y_rot) {
						item2->flags |= IFL_CODEBITS;
						items[item2->item_flags[0]].flags |= IFL_CODEBITS;
						items[item2->item_flags[1]].flags |= IFL_CODEBITS;
						break;
					}
				}

				SoundEffect(SFX_DOOR_GEN_THUD, &item->pos, SFX_DEFAULT);
				SoundEffect(SFX_EXPLOSION2, &item->pos, SFX_DEFAULT);
			} else {
				for (target_item = room[item->room_number].item_number; target_item != NO_ITEM; target_item = item2->next_item) {
					item2 = &items[target_item];

					if (item2->object_number >= PUSHABLE_OBJECT1 && item2->object_number <= PUSHABLE_OBJECT4 &&
					        item2->pos.x_pos == item->pos.x_pos && item2->pos.z_pos == item->pos.z_pos) {
						ExplodeItemNode(item2, 0, 0, 128);
						KillItem(target_item);
						hammered = 1;
					}
				}

				if (hammered) {
					for (target_item = room[item->room_number].item_number; target_item != NO_ITEM; target_item = item2->next_item) {
						item2 = &items[target_item];

						if (item2->object_number == PUZZLE_ITEM4_COMBO1 ||
						        item2->object_number == PUZZLE_ITEM4_COMBO2 ||
						        item2->object_number == PUZZLE_ITEM5) {
							if (item2->pos.x_pos == item->pos.x_pos && item2->pos.z_pos == item->pos.z_pos)
								item2->status = ITEM_INACTIVE;
						}
					}
				}
			}
		} else if (frame > 52 && item->trigger_flags == 2)
			item->flags &= ~IFL_CODEBITS;
	}

	AnimateItem(item);
}

void ControlStargate(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	item->item_flags[3] = 50;

	if (TriggerActive(item)) {
		SoundEffect(SFX_STARGATE_SWIRL, &item->pos, SFX_DEFAULT);
		*(int32_t*)&item->item_flags[0] = 0x36DB600;
		AnimateItem(item);
	} else
		*(int32_t*)&item->item_flags[0] = 0;
}

void ControlPlough(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];
	item->item_flags[3] = 50;

	if (TriggerActive(item)) {
		*(int32_t*)&item->item_flags[0] = 0x3F000;
		AnimateItem(item);
	} else
		*(int32_t*)&item->item_flags[0] = 0;
}

void ControlChain(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->trigger_flags) {
		item->item_flags[2] = 1;
		item->item_flags[3] = 75;

		if (TriggerActive(item)) {
			*(int32_t*)&item->item_flags[0] = 0x3F000;
			AnimateItem(item);
			return;
		}
	} else {
		item->item_flags[3] = 25;

		if (TriggerActive(item)) {
			*(int32_t*)&item->item_flags[0] = 0x780;
			AnimateItem(item);
			return;
		}
	}

	*(int32_t*)&item->item_flags[0] = 0;
}

void ControlBurningFloor(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* torch;
	SPHERE* sphere;
	int32_t nSpheres, dx, dy, dz;
	int16_t torch_num, xoff, zoff, size;

	item = &items[item_number];

	if (!item->item_flags[3]) {
		nSpheres = 0;
		torch_num = room[item->room_number].item_number;

		while (1) {
			torch = &items[torch_num];

			if (torch->object_number == BURNING_TORCH_ITEM && !torch->speed && !torch->fallspeed && torch->item_flags[3]) {
				if (!nSpheres) {
					nSpheres = GetSpheres(item, Slist, 1);

					for (int i = 0; i < nSpheres; i++) {
						sphere = &Slist[i];
						dx = sphere->x - torch->pos.x_pos;
						dy = sphere->y - torch->pos.y_pos;
						dz = sphere->z - torch->pos.z_pos;

						if (SQUARE(dx) + SQUARE(dy) + SQUARE(dz) > SQUARE(sphere->r + 32)) {
							item->item_flags[3] = 1;
							KillItem(torch_num);
							return;
						}
					}
				}
			}

			torch_num = torch->next_item;

			if (torch_num == NO_ITEM)
				return;
		}
	}

	for (int i = 0; i < 15; i++) {
		xoff = floor_fires[(i * 3) + 0];
		zoff = floor_fires[(i * 3) + 1];
		size = floor_fires[(i * 3) + 2];

		if (item->item_flags[size])
			AddFire(item->pos.x_pos + xoff, item->pos.y_pos - (size << 6) - 64, item->pos.z_pos + zoff,
			        size, item->room_number, item->item_flags[size]);
	}

	if (!lara.burn) {
		for (int i = 0; i < 3; i++) {
			xoff = deadly_floor_fires[(i * 2) + 0];
			zoff = deadly_floor_fires[(i * 2) + 1];
			dx = abs(item->pos.x_pos + xoff - lara_item->pos.x_pos);
			dy = abs(item->pos.y_pos - lara_item->pos.y_pos);
			dz = abs(item->pos.z_pos + xoff - lara_item->pos.z_pos);	//ORIGINAL BUG uses xoff instead of zoff, only affects last test

			if (dx < 200 && dy < 200 && dz < 200) {
				LaraBurn();
				lara_item->hit_points = 100;
				item->item_flags[3] = 450;
				item->item_flags[0] = 2;
				item->item_flags[1] = 2;
				item->item_flags[2] = 2;
				FlashFadeR = 255;
				FlashFadeG = 64;
				FlashFadeB = 0;
				FlashFader = 32;
			}
		}
	}

	if (item->item_flags[3] < 450) {
		item->item_flags[0] += 4;

		if (item->item_flags[3] > 30)
			item->item_flags[1] += 4;

		if (item->item_flags[3] > 60)
			item->item_flags[2] += 8;

		if (item->item_flags[0] > 255)
			item->item_flags[0] = 255;

		if (item->item_flags[1] > 255)
			item->item_flags[1] = 255;

		if (item->item_flags[2] > 255)
			item->item_flags[2] = 255;

		item->item_flags[3]++;
		item->required_anim_state = 127 - item->item_flags[3] / 6;
	} else {
		item->item_flags[0] -= 4;
		item->item_flags[1] -= 3;
		item->item_flags[2] -= 2;

		if (item->item_flags[0] < 2)
			item->item_flags[0] = 2;

		if (item->item_flags[1] < 2)
			item->item_flags[1] = 2;

		if (item->item_flags[2] < 2)
			item->item_flags[2] = 2;

		if (item->item_flags[0] == 2 && item->item_flags[1] == 2 && item->item_flags[2] == 2) {
			FlipMap(0);
			ExplodeItemNode(item, 0, 1, -24);
			ExplodeItemNode(item, 1, 1, -24);
			ExplodeItemNode(item, 2, 1, -24);
			ExplodeItemNode(item, 3, 1, -24);
			ExplodeItemNode(item, 4, 1, -32);
			KillItem(item_number);
		}
	}
}

ITEM_INFO *GetPushableForRaisingBlock(ITEM_INFO* item, int check_range) {
	ITEM_INFO** itemlist = nullptr;
	itemlist = (ITEM_INFO**)&tsv_buffer[0];
	ITEM_INFO* collided;
	GetCollidedObjects(item, check_range, 1, itemlist, 0, 0);

	if (itemlist[0]) {
		for (int i = 0; itemlist[0] != 0; i++, itemlist++) {
			collided = itemlist[0];

			if (collided->object_number >= PUSHABLE_OBJECT1 && collided->object_number <= PUSHABLE_OBJECT5) {
				if (
				    collided->pos.y_pos <= item->pos.y_pos &&
				    collided->pos.x_pos >= item->pos.x_pos &&
				    collided->pos.z_pos == item->pos.z_pos) {
					return collided;
				}
			}
		}
	}

	return nullptr;
}

void ControlRaisingBlock(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (TriggerActive(item)) {
		if (!item->item_flags[2]) {
			if (item->object_number == RAISING_BLOCK2)
				AlterFloorHeight(item, -(BLOCK_SIZE * 2));
			else
				AlterFloorHeight(item, -BLOCK_SIZE);

			item->item_flags[2] = 1;
		}

		if (item->item_flags[1] < 4096) {
			SoundEffect(SFX_RUMBLE_NEXTDOOR, &item->pos, SFX_DEFAULT);
			item->item_flags[1] += 64;

			// TRNG
			if (get_game_mod_global_info()->trng_advanced_block_raising_behaviour) {
				// TRNG
				ITEM_INFO* pushable_item = GetPushableForRaisingBlock(item, item->object_number == RAISING_BLOCK2 ? (BLOCK_SIZE * 2) : BLOCK_SIZE);
				if (pushable_item) {
					int32_t height_different = item->object_number == RAISING_BLOCK2 ? -32 : -16;
					pushable_item->pos.y_pos += height_different;

					int16_t pushable_room_number = pushable_item->room_number;
					FLOOR_INFO* floor_info = GetFloor(pushable_item->pos.x_pos, pushable_item->pos.y_pos + height_different, pushable_item->pos.z_pos, &pushable_room_number);
					if (pushable_item->room_number != pushable_room_number) {
						ItemNewRoom(T4PlusGetIDForItemInfo(pushable_item), pushable_room_number);
					}
				}
			}

			if (item->trigger_flags && abs(item->pos.x_pos - lara_item->pos.x_pos) < (BLOCK_SIZE * 10) &&
			        abs(item->pos.y_pos - lara_item->pos.y_pos) < (BLOCK_SIZE * 10) && abs(item->pos.z_pos - lara_item->pos.z_pos) < (BLOCK_SIZE * 10)) {
				if (item->item_flags[1] == 64 || item->item_flags[1] == 4096)
					camera.bounce = -32;
				else
					camera.bounce = -16;
			}
		}
	} else if (item->item_flags[1] > 0) {
		SoundEffect(SFX_RUMBLE_NEXTDOOR, &item->pos, SFX_DEFAULT);

		if (item->trigger_flags && abs(item->pos.x_pos - lara_item->pos.x_pos) < (BLOCK_SIZE * 10) &&
		        abs(item->pos.y_pos - lara_item->pos.y_pos) < (BLOCK_SIZE * 10) && abs(item->pos.z_pos - lara_item->pos.z_pos) < (BLOCK_SIZE * 10)) {
			if (item->item_flags[1] == 64 || item->item_flags[1] == 4096)
				camera.bounce = -32;
			else
				camera.bounce = -16;
		}

		item->item_flags[1] -= 64;

		// TRNG
		if (get_game_mod_global_info()->trng_advanced_block_raising_behaviour) {
			ITEM_INFO* pushable_item = GetPushableForRaisingBlock(item, item->object_number == RAISING_BLOCK2 ? 2048 : 1024);
			if (pushable_item) {
				int32_t height_different = item->object_number == RAISING_BLOCK2 ? 32 : 16;
				pushable_item->pos.y_pos += height_different;
				int16_t pushable_room_number = pushable_item->room_number;
				FLOOR_INFO* floor_info = GetFloor(pushable_item->pos.x_pos, pushable_item->pos.y_pos + height_different, pushable_item->pos.z_pos, &pushable_room_number);
				if (pushable_item->room_number != pushable_room_number) {
					ItemNewRoom(T4PlusGetIDForItemInfo(pushable_item), pushable_room_number);
				}
			}
		}
	} else if (item->item_flags[2]) {
		if (item->object_number == RAISING_BLOCK2)
			AlterFloorHeight(item, (BLOCK_SIZE * 2));
		else
			AlterFloorHeight(item, BLOCK_SIZE);

		item->item_flags[2] = 0;
	}
}

void ControlScaledSpike(int16_t item_number) {
	ITEM_INFO* item;

	int16_t* bounds;
	int16_t* larabounds;
	int32_t dx, dy, dz, num;
	int16_t room_number, yt, yb, iyb1, iyb2, hit;

	item = &items[item_number];

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, TEETH_SPIKES);
	MOD_LEVEL_MISC_INFO *misc_info = get_game_mod_level_misc_info(gfCurrentLevel);

	if (!TriggerActive(item) || item->item_flags[2]) {
		if (TriggerActive(item)) {
			item->item_flags[0] += (item->item_flags[0] >> 3) + 32;
			item->item_flags[1] -= item->item_flags[0];

			if (item->item_flags[1] < 0) {
				item->item_flags[0] = 1024;
				item->item_flags[1] = 0;
				item->status = ITEM_INVISIBLE;
			}

			if (item->trigger_flags & 32)
				item->item_flags[2] = 1;
			else if (item->item_flags[2])
				item->item_flags[2]--;
		} else if (!item->timer) {
			item->item_flags[0] += (item->item_flags[0] >> 3) + 32;

			if (item->item_flags[1] > 0) {
				item->item_flags[1] -= item->item_flags[0];

				if (item->item_flags[1] < 0)
					item->item_flags[1] = 0;
			}
		}
	} else {
		if (misc_info->enable_teeth_spikes_kill_enemies) {
			JeepBaddieCollision(item);
		}

		if (item->item_flags[0] == 1024)
			SoundEffect(SFX_TEETH_SPIKES, &item->pos, SFX_DEFAULT);

		item->status = ITEM_ACTIVE;
		hit = (int16_t)TestBoundsCollideTeethSpikes(item);

		if (lara_item->hit_points > 0 && hit) {
			bounds = GetBestFrame(item);
			larabounds = GetBestFrame(lara_item);
			num = 0;

			if ((item->item_flags[0] > 1024 || lara_item->gravity_status) && (item->trigger_flags & 7) > 2 && (item->trigger_flags & 7) < 6) {
				if (lara_item->fallspeed > 6 || item->item_flags[0] > 1024) {
					lara_item->hit_points = -1;
					num = 20;
				}
			} else {
				lara_item->hit_points -= mod_object_customization->damage_1;
				num = (GetRandomControl() & 3) + 2;
			}

			yt = int16_t(item->pos.y_pos + larabounds[2]);
			yb = int16_t(item->pos.y_pos + larabounds[3]);

			if ((item->trigger_flags & 0xF) == 8 || !(item->trigger_flags & 0xF)) {
				iyb1 = -bounds[3];
				iyb2 = -bounds[2];
			} else {
				iyb1 = bounds[2];
				iyb2 = bounds[3];
			}

			if (yt < item->pos.y_pos + iyb1)
				yt = int16_t(iyb1 + item->pos.y_pos);

			if (yb > item->pos.y_pos + iyb2)
				yb = int16_t(iyb2 + item->pos.y_pos);

			dy = abs(yt - yb) + 1;

			if ((item->trigger_flags & 7) == 2 || (item->trigger_flags & 7) == 6)
				num >>= 1;

			while (num > 0) {
				dx = (GetRandomControl() & 0x7F) + lara_item->pos.x_pos - 64;
				dz = (GetRandomControl() & 0x7F) + lara_item->pos.z_pos - 64;
				TriggerBlood(dx, yb - GetRandomControl() % dy, dz, GetRandomControl() << 1, 1);
				num--;
			}

			if (lara_item->hit_points <= 0) {
				room_number = lara_item->room_number;
				dy = GetHeight(GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &room_number),
				               lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

				if (item->pos.y_pos >= lara_item->pos.y_pos && (dy - lara_item->pos.y_pos < 50 || misc_info->lara_impales_on_spikes)) {
					lara_item->anim_number = ANIM_SPIKED;
					lara_item->frame_number = anims[ANIM_SPIKED].frame_base;
					lara_item->current_anim_state = AS_DEATH;
					lara_item->goal_anim_state = AS_DEATH;
					lara_item->gravity_status = 0;
				}
			}
		}

		item->item_flags[0] += 128;
		item->item_flags[1] += item->item_flags[0];

		if (item->item_flags[1] >= 5120) {
			item->item_flags[1] = 5120;

			if (item->item_flags[0] <= 1024) {
				item->item_flags[0] = 0;

				if (!(item->trigger_flags & 16) && lara_item->hit_points > 0)
					item->item_flags[2] = 64;
			} else
				item->item_flags[0] = -item->item_flags[0] >> 1;
		}
	}
}

void FlameEmitter3Control(int16_t item_number) {
	ITEM_INFO* item, * item2;
	PHD_3DPOS pos;
	PHD_VECTOR s, d;
	int32_t x, z, distance, r, g, b;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->trigger_flags) {
		SoundEffect(SFX_ELEC_ARCING_LOOP, &item->pos, SFX_DEFAULT);
		g = (GetRandomControl() & 0x3F) + 192;
		b = (GetRandomControl() & 0x3F) + 192;
		s.x = item->pos.x_pos;
		s.y = item->pos.y_pos;
		s.z = item->pos.z_pos;

		if (!(GlobalCounter & 3) && (item->trigger_flags == 2 || item->trigger_flags == 4)) {
			d.x = item->pos.x_pos + (2048 * phd_sin(item->pos.y_rot - 0x8000) >> W2V_SHIFT);
			d.y = item->pos.y_pos;
			d.z = item->pos.z_pos + (2048 * phd_cos(item->pos.y_rot - 0x8000) >> W2V_SHIFT);

			if (GetRandomControl() & 3)
				TriggerLightning(&s, &d, (GetRandomControl() & 0x1F) + 64, RGBA(0, g, b, 24), 0, 32, 3);
			else
				TriggerLightning(&s, &d, (GetRandomControl() & 0x1F) + 96, RGBA(0, g, b, 32), 1, 32, 3);
		}

		if (item->trigger_flags >= 3 && !(GlobalCounter & 1)) {
			d.x = 0;
			d.y = -64;
			d.z = 20;
			item2 = &items[item->item_flags[2 + (GlobalCounter >> 2 & 1)]];
			GetJointAbsPosition(item2, &d, 0);

			if (!(GlobalCounter & 3)) {
				if (GetRandomControl() & 3)
					TriggerLightning(&s, &d, (GetRandomControl() & 0x1F) + 64, RGBA(0, g, b, 24), 0, 32, 5);
				else
					TriggerLightning(&s, &d, (GetRandomControl() & 0x1F) + 96, RGBA(0, g, b, 32), 1, 32, 5);
			}

			if (item->trigger_flags != 3 || item2->trigger_flags)
				TriggerLightningGlow(d.x, d.y, d.z, RGBA(0, g, b, 64));
		}

		if ((GlobalCounter & 3) == 2) {
			s.x = item->pos.x_pos;
			s.y = item->pos.y_pos;
			s.z = item->pos.z_pos;
			d.x = s.x + (GetRandomControl() & 0x1FF) - 256;
			d.y = s.y + (GetRandomControl() & 0x1FF) - 256;
			d.z = s.z + (GetRandomControl() & 0x1FF) - 256;
			TriggerLightning(&s, &d, (GetRandomControl() & 0xF) + 16, RGBA(0, g, b, 24), 3, 32, 3);
			TriggerLightningGlow(s.x, s.y, s.z, RGBA(0, g, b, 32));
		}
	} else {
		if (!item->item_flags[0]) {
			item->item_flags[0] = (GetRandomControl() & 3) + 8;
			distance = GetRandomControl() & 0x3F;
			item->item_flags[1] = int16_t(distance == item->item_flags[1] ? (distance + 13) & 0x3F : distance);
		} else
			item->item_flags[0]--;

		if (!(wibble & 4)) {
			x = 16 * (Flame3xzoffs[item->item_flags[1] & 7][0] - 32);
			z = 16 * (Flame3xzoffs[item->item_flags[1] & 7][1] - 32);
			TriggerFireFlame(item->pos.x_pos + x, item->pos.y_pos, item->pos.z_pos + z, -1, 2);
		}

		if (wibble & 4) {
			x = 16 * (Flame3xzoffs[(item->item_flags[1] >> 3) + 8][0] - 32);
			z = 16 * (Flame3xzoffs[(item->item_flags[1] >> 3) + 8][1] - 32);
			TriggerFireFlame(item->pos.x_pos + x, item->pos.y_pos, item->pos.z_pos + z, -1, 2);
		}

		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);
		distance = GetRandomControl();
		r = (distance & 0x3F) + 192;
		g = (distance >> 4 & 0x1F) + 96;
		TriggerDynamic(x, item->pos.y_pos, z, 12, r, g, 0);
		pos.x_pos = item->pos.x_pos;
		pos.y_pos = item->pos.y_pos;
		pos.z_pos = item->pos.z_pos;

		if (ItemNearLara(&pos, 600) && !lara.burn) {
			lara_item->hit_points -= 5;
			lara_item->hit_status = 1;
			x = lara_item->pos.x_pos - pos.x_pos;
			z = lara_item->pos.z_pos - pos.z_pos;
			distance = SQUARE(x) + SQUARE(z);

			if (distance < 202500)
				LaraBurn();
		}
	}
}

void FlameControl(int16_t fx_number) {
	FX_INFO* fx;
	int32_t r, g, b, wh;

	if (lara.water_status == LW_FLYCHEAT) {
		KillEffect(fx_number);
		lara.burn = 0;
		return;
	}

	fx = &effects[fx_number];

	for (int i = 14; i > 0; i--) {
		if (!(wibble & 0xC)) {
			fx->pos.x_pos = 0;
			fx->pos.y_pos = 0;
			fx->pos.z_pos = 0;
			GetLaraJointPos((PHD_VECTOR*)&fx->pos, i);
			TriggerFireFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 255 - lara.BurnGreen);
		}
	}

	if (lara.BurnGreen) {
		r = GetRandomControl() & 0x3F;
		g = (GetRandomControl() & 0x3F) + 192;
		b = (GetRandomControl() & 0x1F) + 96;
		TriggerDynamic(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, 13, r, g, b);
	} else {
		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 96;
		TriggerDynamic(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, 13, r, g, 0);
	}

	if (lara_item->room_number != fx->room_number)
		EffectNewRoom(fx_number, lara_item->room_number);

	wh = GetWaterHeight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number);

	if (wh != NO_HEIGHT && fx->pos.y_pos > wh) {
		KillEffect(fx_number);
		lara.burn = 0;
		return;
	}

	SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &fx->pos, SFX_DEFAULT);
	lara_item->hit_points -= 7;
	lara_item->hit_status = 1;
}

void FlameEmitter2Control(int16_t item_number) {
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	int32_t r, g;
	int16_t room_number;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->trigger_flags < 0) {
		if (!item->item_flags[0]) {
			FlipMap(-item->trigger_flags);
			flipmap[-item->trigger_flags] ^= IFL_CODEBITS;
			item->item_flags[0] = 1;
		}

		return;
	}

	if (item->trigger_flags != 2) {
		if (item->trigger_flags == 123)
			AddFire(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 1, item->room_number, item->item_flags[3]);
		else
			AddFire(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 1 - item->trigger_flags, item->room_number, item->item_flags[3]);
	}

	if (!item->trigger_flags || item->trigger_flags == 2) {
		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 96;

		if (item->item_flags[3]) {
			r = (r * item->item_flags[3]) >> 8;
			g = (g * item->item_flags[3]) >> 8;
		}

		TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 10, r, g, 0);
	}

	if (item->trigger_flags == 2) {
		item->pos.x_pos += phd_sin(item->pos.y_rot + 0x8000) >> 11;
		item->pos.z_pos += phd_cos(item->pos.y_rot + 0x8000) >> 11;
		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (room[room_number].flags & ROOM_UNDERWATER) {
			FlashFadeR = 255;
			FlashFadeG = 128;
			FlashFadeB = 0;
			FlashFader = 32;
			KillItem(item_number);
			return;
		}

		if (item->room_number != room_number)
			ItemNewRoom(item_number, room_number);

		item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (wibble & 7)
			TriggerFireFlame(item->pos.x_pos, item->pos.y_pos - 32, item->pos.z_pos, -1, 1);
	}

	SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);
}

void LaraBurn() {
	int16_t fx_number;

	if (!lara.burn && !lara.BurnGreen) {
		fx_number = CreateEffect(lara_item->room_number);

		if (fx_number != NO_ITEM) {
			effects[fx_number].object_number = FLAME;
			lara.burn = 1;
		}
	}
}

void LavaBurn(ITEM_INFO* item) {
	FLOOR_INFO* floor;
	int16_t room_number;

	if (item->hit_points >= 0 && lara.water_status != LW_FLYCHEAT) {
		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, 32000, item->pos.z_pos, &room_number);

		if (item->floor == GetHeight(floor, item->pos.x_pos, 32000, item->pos.z_pos)) {
			item->hit_status = 1;
			item->hit_points = -1;
			LaraBurn();
		}
	}
}

int32_t TestBoundsCollideTeethSpikes(ITEM_INFO* item) {
	int16_t* bounds;
	int32_t x, y, z, rad, xMin, xMax, zMin, zMax;

	if (item->trigger_flags & 8) {
		x = item->pos.x_pos & ~0x3FF | 0x200;
		z = (item->pos.z_pos + SPxzoffs[item->trigger_flags & 7]) & ~0x3FF | 0x200;
	} else {
		x = (item->pos.x_pos - SPxzoffs[item->trigger_flags & 7]) & ~0x3FF | 0x200;
		z = item->pos.z_pos & ~0x3FF | 0x200;
	}

	if (item->trigger_flags & 1)
		rad = 300;
	else
		rad = 480;

	y = item->pos.y_pos + SPDETyoffs[item->trigger_flags & 7];
	bounds = GetBestFrame(lara_item);

	if (lara_item->pos.y_pos + bounds[2] > y || lara_item->pos.y_pos + bounds[3] < y - 900)
		return 0;

	xMin = lara_item->pos.x_pos + bounds[0];
	xMax = lara_item->pos.x_pos + bounds[1];
	zMin = lara_item->pos.z_pos + bounds[4];
	zMax = lara_item->pos.z_pos + bounds[5];
	return xMin <= x + rad && xMax >= x - rad && zMin <= z + rad && zMax >= z - rad;
}

const int MAX_ROLLING_BALL_VALID_ROOMS = 22;

int GetRollingBallRooms(ITEM_INFO* item, int16_t* valid_rooms) {
	int16_t* doors;
	int32_t j;
	int16_t room_count;

	room_count = 1;
	valid_rooms[0] = item->room_number;
	doors = room[item->room_number].door;

	if (doors) {
		for (int i = *doors++; i > 0; i--, doors += 16) {
			for (j = 0; j < room_count; j++) {
				if (valid_rooms[j] == *doors)
					break;
			}

			if (j == room_count) {
				valid_rooms[room_count] = *doors;
				room_count++;
			}
		}
	}

	return room_count;
}

void RollingBallBaddieCollision(ITEM_INFO* rolling_ball, int16_t* valid_rooms, int valid_room_count) {
	ITEM_INFO* item;
	OBJECT_INFO* obj;
	int32_t dx, dy, dz;
	int16_t item_number;

	for (int i = 0; i < valid_room_count; i++) {
		for (item_number = room[valid_rooms[i]].item_number; item_number != NO_ITEM; item_number = item->next_item) {
			item = &items[item_number];

			if (item->collidable && item->status != ITEM_INVISIBLE && item != lara_item && item != rolling_ball) {
				obj = &objects[item->object_number];

				if (obj->collision && obj->intelligent) {
					dx = rolling_ball->pos.x_pos - item->pos.x_pos;
					dy = rolling_ball->pos.y_pos - item->pos.y_pos;
					dz = rolling_ball->pos.z_pos - item->pos.z_pos;

					if (dx > -2048 && dx < 2048 && dz > -2048 && dz < 2048 && dy > -2048 && dy < 2048) {
						if (TestBoundsCollide(item, rolling_ball, 500)) {
							DoLotsOfBlood(item->pos.x_pos, rolling_ball->pos.y_pos - 256, item->pos.z_pos, (GetRandomControl() & 3) + 8,
							              rolling_ball->pos.y_rot, item->room_number, 3);
							item->hit_points = 0;
						}
					}
				}
			}
		}
	}

	return;
}

void RollingBallCollideStaticObjects(int32_t x, int32_t y, int32_t z, int32_t height, int16_t *valid_rooms, int valid_room_count) {
	MESH_INFO* mesh;
	STATIC_INFO* sinfo;
	ROOM_INFO* r;
	PHD_VECTOR pos;
	int32_t j;
	static int32_t RollingBallBounds[6] = { 0, 0, 0, 0, 0, 0 };
	static int32_t CollidedStaticBounds[6] = { 0, 0, 0, 0, 0, 0 };

	pos.x = x;
	pos.y = y;
	pos.z = z;
	RollingBallBounds[0] = x + CLICK_SIZE;
	RollingBallBounds[1] = x - CLICK_SIZE;
	RollingBallBounds[2] = y;
	RollingBallBounds[3] = y - height;
	RollingBallBounds[4] = z + CLICK_SIZE;
	RollingBallBounds[5] = z - CLICK_SIZE;

	int i = 0;
	for (int i = 0; i < valid_room_count; i++) {
		r = &room[valid_rooms[i]];
		mesh = r->mesh;

		for (j = r->num_meshes; j > 0; j--, mesh++) {
			sinfo = &static_objects[mesh->static_number];

			if (mesh->Flags & 1) {
				MOD_LEVEL_STATIC_INFO *static_info = &get_game_mod_level_statics_info(gfCurrentLevel)->static_info[mesh->static_number];
				if (static_info->large_objects_can_shatter) {
					CollidedStaticBounds[2] = mesh->y + sinfo->y_maxc;
					CollidedStaticBounds[3] = mesh->y + sinfo->y_minc;

					if (mesh->y_rot == -0x8000) {
						CollidedStaticBounds[0] = mesh->x - sinfo->x_minc;
						CollidedStaticBounds[1] = mesh->x - sinfo->x_maxc;
						CollidedStaticBounds[4] = mesh->z - sinfo->z_minc;
						CollidedStaticBounds[5] = mesh->z - sinfo->z_maxc;
					} else if (mesh->y_rot == -0x4000) {
						CollidedStaticBounds[0] = mesh->x - sinfo->z_minc;
						CollidedStaticBounds[1] = mesh->x - sinfo->z_maxc;
						CollidedStaticBounds[4] = mesh->z + sinfo->x_maxc;
						CollidedStaticBounds[5] = mesh->z + sinfo->x_minc;
					} else if (mesh->y_rot == 0x4000) {
						CollidedStaticBounds[0] = mesh->x + sinfo->z_maxc;
						CollidedStaticBounds[1] = mesh->x + sinfo->z_minc;
						CollidedStaticBounds[4] = mesh->z - sinfo->x_minc;
						CollidedStaticBounds[5] = mesh->z - sinfo->x_maxc;
					} else {
						CollidedStaticBounds[0] = mesh->x + sinfo->x_maxc;
						CollidedStaticBounds[1] = mesh->x + sinfo->x_minc;
						CollidedStaticBounds[4] = mesh->z + sinfo->z_maxc;
						CollidedStaticBounds[5] = mesh->z + sinfo->z_minc;
					}

					if (RollingBallBounds[0] > CollidedStaticBounds[1] &&
					        RollingBallBounds[1] < CollidedStaticBounds[0] &&
					        RollingBallBounds[2] > CollidedStaticBounds[3] &&
					        RollingBallBounds[3] < CollidedStaticBounds[2] &&
					        RollingBallBounds[4] > CollidedStaticBounds[5] &&
					        RollingBallBounds[5] < CollidedStaticBounds[4]) {
						ShatterObject(0, mesh, -128, valid_rooms[i], 0);
						if (static_info->shatter_sound_id >= 0) {
							SoundEffect(static_info->shatter_sound_id, (PHD_3DPOS*)&pos, SFX_DEFAULT);
						}
						SmashedMeshRoom[SmashedMeshCount] = valid_rooms[i];
						SmashedMesh[SmashedMeshCount] = mesh;
						SmashedMeshCount++;
						mesh->Flags &= ~1;
					}
				}
			}
		}
		i++;
	}
}

void ControlRollingBall(int16_t item_number) {
	ITEM_INFO* item;
	uint16_t tyrot, destyrot;
	int16_t room_number, velnotadjusted;
	int32_t h, fx, fz, fh, fhf, bz, bh, bhf, rx, rh, rhf, lx, lh, lhf;

	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	item->fallspeed += 6;
	item->pos.x_pos += item->item_flags[0] >> 5;
	item->pos.y_pos += item->fallspeed;
	item->pos.z_pos += item->item_flags[1] >> 5;
	room_number = item->room_number;
	h = GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number), item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) - 512;

	if (item->pos.y_pos > h) {
		if (abs(item->fallspeed) > 16) {
			fz = phd_sqrt(SQUARE(camera.pos.x - item->pos.x_pos) + SQUARE(camera.pos.y - item->pos.y_pos) + SQUARE(camera.pos.z - item->pos.z_pos));

			// NGLE: This flag silences the RollingBall.
			if (!(item->trigger_flags & 0x01) || !global_info->trng_rollingball_extended_ocb) {
				if (fz < 0x4000)
					camera.bounce = -(((0x4000 - fz) * abs(item->fallspeed)) >> W2V_SHIFT);

				SoundEffect(SFX_BOULDER_FALL, &item->pos, SFX_DEFAULT);
			}
		}

		if (item->pos.y_pos - h < 512)
			item->pos.y_pos = h;

		if (item->fallspeed > 64)
			item->fallspeed = -(item->fallspeed >> 2);
		else {
			if (abs(item->speed) <= 512 || GetRandomControl() & 0x1F)
				item->fallspeed = 0;
			else
				item->fallspeed = -(GetRandomControl() % (item->speed >> 3));
		}
	}

	fx = item->pos.x_pos;
	fz = item->pos.z_pos + 128;
	bz = item->pos.z_pos - 128;
	rx = item->pos.x_pos + 128;
	lx = item->pos.x_pos - 128;
	fh = GetHeight(GetFloor(fx, item->pos.y_pos, fz, &room_number), fx, item->pos.y_pos, fz) - 512;
	bh = GetHeight(GetFloor(fx, item->pos.y_pos, bz, &room_number), fx, item->pos.y_pos, bz) - 512;
	rh = GetHeight(GetFloor(rx, item->pos.y_pos, bz + 128, &room_number), rx, item->pos.y_pos, bz + 128) - 512;
	lh = GetHeight(GetFloor(lx, item->pos.y_pos, bz + 128, &room_number), lx, item->pos.y_pos, bz + 128) - 512;
	fx = item->pos.x_pos;
	fz = item->pos.z_pos + 512;
	bz = item->pos.z_pos - 512;
	rx = item->pos.x_pos + 512;
	lx = item->pos.x_pos - 512;
	fhf = GetHeight(GetFloor(fx, item->pos.y_pos, fz, &room_number), fx, item->pos.y_pos, fz) - HALF_BLOCK_SIZE;
	bhf = GetHeight(GetFloor(fx, item->pos.y_pos, bz, &room_number), fx, item->pos.y_pos, bz) - HALF_BLOCK_SIZE;
	rhf = GetHeight(GetFloor(rx, item->pos.y_pos, bz + HALF_BLOCK_SIZE, &room_number), rx, item->pos.y_pos, bz + HALF_BLOCK_SIZE) - HALF_BLOCK_SIZE;
	lhf = GetHeight(GetFloor(lx, item->pos.y_pos, bz + HALF_BLOCK_SIZE, &room_number), lx, item->pos.y_pos, bz + HALF_BLOCK_SIZE) - HALF_BLOCK_SIZE;

	if (item->pos.y_pos - h > -CLICK_SIZE || item->pos.y_pos - fhf >= HALF_BLOCK_SIZE || item->pos.y_pos - rhf >= HALF_BLOCK_SIZE ||
	        item->pos.y_pos - bhf >= HALF_BLOCK_SIZE || item->pos.y_pos - lhf >= HALF_BLOCK_SIZE) {
		velnotadjusted = 0;

		if (fh - h <= CLICK_SIZE) {
			if (fhf - h < -BLOCK_SIZE || fh - h < -CLICK_SIZE) {
				if (item->item_flags[1] <= 0) {
					if (!item->item_flags[1] && item->item_flags[0])
						item->pos.z_pos = (item->pos.z_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				} else {
					item->item_flags[1] = -item->item_flags[1] >> 1;
					item->pos.z_pos = (item->pos.z_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				}
			} else if (fh == h)
				velnotadjusted++;
			else
				item->item_flags[1] += int16_t((fh - h) >> 1);
		}

		if (bh - h > 256)
			velnotadjusted++;
		else if (bhf - h < -BLOCK_SIZE || bh - h < -CLICK_SIZE) {
			if (item->item_flags[1] >= 0) {
				if (!item->item_flags[1] && item->item_flags[0])
					item->pos.z_pos = (item->pos.z_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
			} else {
				item->item_flags[1] = -item->item_flags[1] >> 1;
				item->pos.z_pos = (item->pos.z_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
			}
		} else if (bh == h)
			velnotadjusted++;
		else
			item->item_flags[1] -= (int16_t)((bh - h) >> 1);

		if (velnotadjusted == 2) {
			if (abs(item->item_flags[1]) <= QUARTER_CLICK_SIZE)
				item->item_flags[1] = 0;
			else
				item->item_flags[1] -= item->item_flags[1] >> 6;
		}

		velnotadjusted = 0;

		if (lh - h <= CLICK_SIZE) {
			if (lhf - h < -BLOCK_SIZE || lh - h < -CLICK_SIZE) {
				if (item->item_flags[0] >= 0) {
					if (!item->item_flags[0] && item->item_flags[1])
						item->pos.x_pos = (item->pos.x_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				} else {
					item->item_flags[0] = -item->item_flags[0] >> 1;
					item->pos.x_pos = (item->pos.x_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				}
			} else if (lh == h)
				velnotadjusted++;
			else
				item->item_flags[0] -= (int16_t)((lh - h) >> 1);
		}

		if (rh - h <= CLICK_SIZE) {
			if (rhf - h < -BLOCK_SIZE || rh - h < -CLICK_SIZE) {
				if (item->item_flags[0] <= 0) {
					if (!item->item_flags[0] && item->item_flags[1])
						item->pos.x_pos = (item->pos.x_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				} else {
					item->item_flags[0] = -item->item_flags[0] >> 1;
					item->pos.x_pos = (item->pos.x_pos & -HALF_BLOCK_SIZE) | HALF_BLOCK_SIZE;
				}
			} else if (rh == h)
				velnotadjusted++;
			else
				item->item_flags[0] += (int16_t)((rh - h) >> 1);
		}

		if (velnotadjusted == 2) {
			if (abs(item->item_flags[0]) <= QUARTER_CLICK_SIZE)
				item->item_flags[0] = 0;
			else
				item->item_flags[0] -= item->item_flags[0] >> 6;
		}
	}

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (item->item_flags[0] > 3072)
		item->item_flags[0] = 3072;
	else if (item->item_flags[0] < -3072)
		item->item_flags[0] = -3072;

	if (item->item_flags[1] > 3072)
		item->item_flags[1] = 3072;
	else if (item->item_flags[1] < -3072)
		item->item_flags[1] = -3072;

	tyrot = item->pos.y_rot;

	if (item->item_flags[1] || item->item_flags[0])
		destyrot = uint16_t(phd_atan(item->item_flags[1], item->item_flags[0]));
	else
		destyrot = item->pos.y_rot;

	if (tyrot != destyrot) {
		if (((destyrot - tyrot) & 0x7FFF) >= HALF_BLOCK_SIZE) {
			if (destyrot <= tyrot || destyrot - tyrot >= 0x8000)
				item->pos.y_rot = tyrot - HALF_BLOCK_SIZE;
			else
				item->pos.y_rot = tyrot + HALF_BLOCK_SIZE;
		} else
			item->pos.y_rot = destyrot;
	}

	item->pos.x_rot -= (abs(item->item_flags[0]) + abs(item->item_flags[1])) >> 1;
	GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number), item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	TestTriggers(trigger_index, true, 0);

	// T4Plus: test for either NGLE extended OCB or TREP smash and kill setting.
	bool always_smash_and_kill = get_game_mod_level_misc_info(gfCurrentLevel)->enable_smashing_and_killing_rolling_balls;
	if (global_info->trng_rollingball_extended_ocb || always_smash_and_kill) {
		if (item->trigger_flags & 0x02 || item->trigger_flags & 0x10 || always_smash_and_kill) {
			int16_t valid_rooms[MAX_ROLLING_BALL_VALID_ROOMS];
			int valid_room_count = GetRollingBallRooms(item, valid_rooms);

			// Enemy collision
			if (item->trigger_flags & 0x02 || always_smash_and_kill) {
				RollingBallBaddieCollision(item, valid_rooms, valid_room_count);
			}

			// Shatter object collision
			if (item->trigger_flags & 0x10 || always_smash_and_kill) {
				RollingBallCollideStaticObjects(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 512, valid_rooms, valid_room_count);
			}
		}

		if (global_info->trng_rollingball_extended_ocb) {
			// Water Splash (TODO)
			if (item->trigger_flags & 0x20) {

			}

			// NGLE: can activate regular triggers with this OCB code.
			if (item->trigger_flags & 0x40) {
				TestTriggers(trigger_index, false, 0);
			}
		}
	}
}

// Added support for TRNG pushable rolling balls, but the implementation is still buggy.
// The bounds are likely not calculated correctly, it doesn't reset its position to the center
// of a tile, and lacks special invulnerability frames which makes it easy to die when pushing them.
void RollingBallPush(int16_t item_number, ITEM_INFO* l) {
	ITEM_INFO* item;
	item = &items[item_number];

	MOD_GLOBAL_INFO *global_info = get_game_mod_global_info();
	if (global_info->trng_rollingball_extended_ocb) {
		if (lara.GeneralPtr == item_number) {
			if (l->anim_number == ANIM_BLOCKSWITCH) {
				int16_t quadrant = uint16_t(l->pos.y_rot + 0x2000) / 0x4000;

				// FrameOfMoving
				if (l->frame_number >= anims[l->anim_number].frame_base + 20 && l->frame_number < anims[l->anim_number].frame_base + 50) {
					switch (quadrant) {
						case NORTH:
							item->pos.z_pos += 6;
							break;
						case EAST:
							item->pos.x_pos += 6;
							break;
						case SOUTH:
							item->pos.z_pos -= 6;
							break;
						case WEST:
							item->pos.x_pos -= 6;
							break;
					}
				} else if (l->frame_number >= anims[l->anim_number].frame_base + 50) {
					switch (quadrant) {
						case NORTH:
							if (item->item_flags[1] <= 64)
								item->item_flags[1] = 64;
							item->item_flags[1] += 6 << 1;
							break;
						case EAST:
							if (item->item_flags[0] <= 64)
								item->item_flags[0] = 64;
							item->item_flags[0] += 6 << 1;
							break;
						case SOUTH:
							if (item->item_flags[1] >= -64)
								item->item_flags[1] = -64;
							item->item_flags[1] -= 6 << 1;
							break;
						case WEST:
							if (item->item_flags[0] >= -64)
								item->item_flags[0] = -64;
							item->item_flags[0] -= 6 << 1;
							break;
					}
				}
			} else {
				lara.GeneralPtr = NULL;
			}
		}

		if ((item->trigger_flags & 0x04 && item->status == ITEM_INACTIVE) || item->trigger_flags & 0x08) {
			if (lara.GeneralPtr != item_number && (input & IN_ACTION && lara.gun_status == LG_NO_ARMS &&
			                                       l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH)) {
				static int16_t RollingBallBounds[12] = { -700, 700, -512, 512, -700, 700, -0, 0, -5460, 5460, -0, 0 };
				{
					// Save rotation
					int16_t rotx = item->pos.x_rot;
					int16_t roty = item->pos.y_rot;
					// Change y rotation to Lara's y rotation quadrant
					item->pos.x_rot = 0;
					item->pos.y_rot = (l->pos.y_rot + 0x2000) & 0xC000;
					if (TestLaraPosition(RollingBallBounds, item, l)) {
						int16_t quadrant = uint16_t(l->pos.y_rot + 0x2000) / 0x4000;
						if (1) {

							l->current_anim_state = AS_SWITCHON;
							l->anim_number = ANIM_BLOCKSWITCH;
							item->goal_anim_state = 0;

							l->goal_anim_state = AS_STOP;
							l->frame_number = anims[l->anim_number].frame_base;
							lara.IsMoving = 0;
							lara.head_y_rot = 0;
							lara.head_x_rot = 0;
							lara.torso_y_rot = 0;
							lara.torso_x_rot = 0;
							lara.gun_status = LG_HANDS_BUSY;
							lara.GeneralPtr = item_number;

							T4PlusActivateItem(item_number, false);
						} else {
							l->anim_number = ANIM_PPREADY;
							l->frame_number = anims[ANIM_PPREADY].frame_base;
							l->current_anim_state = AS_PPREADY;

							l->goal_anim_state = AS_STOP;
							l->frame_number = anims[l->anim_number].frame_base;
							lara.IsMoving = 0;
							lara.head_y_rot = 0;
							lara.head_x_rot = 0;
							lara.torso_y_rot = 0;
							lara.torso_x_rot = 0;
							lara.gun_status = LG_HANDS_BUSY;
						}
					}
					// Restore rotation
					item->pos.x_rot = rotx;
					item->pos.y_rot = roty;
				}
			}
		}
	}
}

void RollingBallCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	// Tomb4Plus
	if (lara.water_status == LW_FLYCHEAT) {
		return;
	}

	ITEM_INFO* item;

	item = &items[item_number];

	// NGLE
	RollingBallPush(item_number, lara_item);

	if (!TestBoundsCollide(item, l, coll->radius) || !TestCollision(item, l))
		return;

	if (TriggerActive(item) && (item->item_flags[0] || item->fallspeed)) {
		lara_item->anim_number = ANIM_RBALL_DEATH;
		lara_item->frame_number = anims[ANIM_RBALL_DEATH].frame_base;
		lara_item->current_anim_state = AS_DEATH;
		lara_item->goal_anim_state = AS_DEATH;
		lara_item->gravity_status = 0;
	} else
		ObjectCollision(item_number, l, coll);
}

void DartsControl(int16_t item_number) {
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	int32_t x, z, speed;
	int16_t room_num;

	item = &items[item_number];

	MOD_LEVEL_OBJECT_CUSTOMIZATION *mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, DARTS);

	if (item->touch_bits) {
		lara_item->hit_points -= mod_object_customization->damage_1;
		lara_item->hit_status = 1;
		if (get_game_mod_level_misc_info(gfCurrentLevel)->darts_poison_fix) {
			lara.dpoisoned += 160;
		} else {
			lara.poisoned += 160;
		}
		DoBloodSplat(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (GetRandomControl() & 3) + 4, lara_item->pos.y_rot, lara_item->room_number);
		KillItem(item_number);
	} else {
		x = item->pos.x_pos;
		z = item->pos.z_pos;
		speed = (item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
		item->pos.x_pos += (speed * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
		item->pos.y_pos -= (item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT;
		item->pos.z_pos += (speed * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
		room_num = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);

		if (item->room_number != room_num)
			ItemNewRoom(item_number, room_num);

		item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (item->pos.y_pos >= item->floor) {
			for (int i = 0; i < 4; i++)
				TriggerDartSmoke(x, item->pos.y_pos, z, 0, 0, 1);

			KillItem(item_number);
		}
	}
}

void DartEmitterControl(int16_t item_number) {
	MOD_LEVEL_OBJECTS_INFO* mod_level_object_info = get_game_mod_level_objects_info(gfCurrentLevel);

	ITEM_INFO* item;
	ITEM_INFO* dart;
	int32_t x, z, xLimit, zLimit, xv, zv, rand;
	int16_t num;

	item = &items[item_number];

	if (item->active) {
		if (item->timer > 0) {
			item->timer--;
			return;
		}

		item->timer = mod_level_object_info->darts_interval;
	}

	num = CreateItem();

	if (num != NO_ITEM) {
		x = 0;
		z = 0;
		dart = &items[num];
		dart->object_number = DARTS;
		dart->room_number = item->room_number;

		if (!item->pos.y_rot)
			z = HALF_BLOCK_SIZE;
		else if (item->pos.y_rot == 0x4000)
			x = HALF_BLOCK_SIZE;
		else if (item->pos.y_rot == -0x4000)
			x = -HALF_BLOCK_SIZE;
		else if (item->pos.y_rot == -0x8000)
			z = -HALF_BLOCK_SIZE;

		dart->pos.x_pos = x + item->pos.x_pos;
		dart->pos.y_pos = item->pos.y_pos - HALF_BLOCK_SIZE;
		dart->pos.z_pos = z + item->pos.z_pos;
		InitialiseItem(num);
		dart->pos.x_rot = 0;
		dart->pos.y_rot = item->pos.y_rot + 0x8000;
		dart->speed = mod_level_object_info->darts_speed;
		xLimit = 0;
		zLimit = 0;

		if (x)
			xLimit = abs(x << 1) - 1;
		else
			zLimit = abs(z << 1) - 1;

		for (int i = 0; i < 5; i++) {
			rand = -GetRandomControl();

			if (z >= 0)
				zv = zLimit & rand;
			else
				zv = -(zLimit & rand);

			if (x >= 0)
				xv = xLimit & rand;
			else
				xv = -(xLimit & rand);

			TriggerDartSmoke(dart->pos.x_pos, dart->pos.y_pos, dart->pos.z_pos, xv, zv, 0);
		}

		AddActiveItem(num);
		dart->status = ITEM_ACTIVE;
		SoundEffect(SFX_DART_SPITT, &dart->pos, SFX_DEFAULT);
	}
}

void FallingCeiling(int16_t item_number) {
	ITEM_INFO* item;
	int16_t room_number;

	item = &items[item_number];

	MOD_LEVEL_OBJECT_CUSTOMIZATION* mod_object_customization = get_game_mod_level_object_customization_for_slot(gfCurrentLevel, FALLING_CEILING);

	if (!item->current_anim_state) {
		item->gravity_status = 1;
		item->goal_anim_state = 1;
	} else if (item->current_anim_state == 1 && item->touch_bits) {
		lara_item->hit_points -= mod_object_customization->damage_1;
		lara_item->hit_status = 1;
	}

	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED)
		RemoveActiveItem(item_number);
	else {
		room_number = item->room_number;
		item->floor = GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number),
		                        item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (room_number != item->room_number)
			ItemNewRoom(item_number, room_number);

		if (item->current_anim_state == 1 && item->pos.y_pos >= item->floor) {
			item->pos.y_pos = item->floor;
			item->goal_anim_state = 2;
			item->gravity_status = 0;
			item->fallspeed = 0;
		}
	}
}

void ControlSmashableBikeWall(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (!TriggerActive(item) || lara.vehicle == NO_ITEM)
		return;

	if (TestBoundsCollide(item, &items[lara.vehicle], BLOCK_SIZE)) {
		SoundEffect(SFX_BIKE_HIT_OBJECTS, &item->pos, SFX_DEFAULT);
		item->mesh_bits = -2;
		ExplodingDeath2(item_number, -1, 0x901);
		item->mesh_bits = 0;
		TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, 0);
		KillItem(item_number);
	}
}

void ControlFallingBlock2(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->pos.y_pos == lara_item->pos.y_pos && OnTwoBlockPlatform(item, lara_item->pos.x_pos, lara_item->pos.z_pos) && lara.vehicle != NO_ITEM) {
		SoundEffect(SFX_BIKE_HIT_OBJECTS, &item->pos, SFX_DEFAULT);
		item->mesh_bits = -2;
		ExplodingDeath2(item_number, -1, 417);
		KillItem(item_number);
	}
}

void FallingBlockCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height) {
	int32_t tx, tz;

	tx = x ^ item->pos.x_pos;
	tz = z ^ item->pos.z_pos;

	if (tx & ~(BLOCK_SIZE - 1) || tz & ~(BLOCK_SIZE - 1))
		return;

	if (y > item->pos.y_pos)
		*height = item->pos.y_pos + CLICK_SIZE;
}

void FallingBlockFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height) {
	int32_t tx, tz;

	tx = x ^ item->pos.x_pos;
	tz = z ^ item->pos.z_pos;

	if (tx & ~(BLOCK_SIZE - 1) || tz & ~(BLOCK_SIZE - 1))
		return;

	if (y <= item->pos.y_pos) {
		*height = item->pos.y_pos;
		height_type = WALL;
		OnObject = 1;
	}
}

void FallingBlock(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->item_flags[0] <= 1) {
		item->mesh_bits = -2;
		ExplodingDeath2(item_number, -1, 2465);
		KillItem(item_number);
	} else {
		item->pos.x_rot = (GetRandomControl() & 0x3FF) - 512;
		item->pos.z_rot = (GetRandomControl() & 0x3FF) - 512;
		item->item_flags[0]--;
	}
}

void FallingBlockCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	MOD_LEVEL_OBJECTS_INFO* mod_level_object_info = get_game_mod_level_objects_info(gfCurrentLevel);

	ITEM_INFO* item;
	int32_t x, z, tx, tz;

	item = &items[item_number];
	x = l->pos.x_pos;
	z = l->pos.z_pos;
	tx = item->pos.x_pos;
	tz = item->pos.z_pos;

	if (!item->item_flags[0]
	        && !item->trigger_flags
	        && item->pos.y_pos == l->pos.y_pos
	        && !((tx ^ x) & ~mod_level_object_info->falling_block_tremble)
	        && !((z ^ tz) & ~mod_level_object_info->falling_block_tremble)) {
		SoundEffect(SFX_ROCK_FALL_CRUMBLE, &item->pos, SFX_DEFAULT);
		AddActiveItem(item_number);
		item->item_flags[0] = mod_level_object_info->falling_block_timer;
		item->status = ITEM_ACTIVE;
		item->flags |= IFL_CODEBITS;
	}
}

void CeilingTrapDoorCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (input & IN_ACTION && item->status != ITEM_ACTIVE && l->current_anim_state == AS_UPJUMP &&
	        l->gravity_status && lara.gun_status == LG_NO_ARMS && TestLaraPosition(CeilingTrapDoorBounds, item, l)) {
		AlignLaraPosition(&CeilingTrapDoorPos, item, l);
		lara.head_x_rot = 0;
		lara.head_y_rot = 0;
		lara.torso_x_rot = 0;
		lara.torso_y_rot = 0;
		lara.gun_status = LG_HANDS_BUSY;
		l->gravity_status = 0;
		l->fallspeed = 0;
		l->anim_number = ANIM_PULLTRAP;
		l->frame_number = anims[ANIM_PULLTRAP].frame_base;
		l->current_anim_state = AS_PULLTRAP;
		AddActiveItem(item_number);
		item->status = ITEM_ACTIVE;
		item->goal_anim_state = 1;
		UseForcedFixedCamera = 1;
		ForcedFixedCamera.x = item->pos.x_pos - ((BLOCK_SIZE * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
		ForcedFixedCamera.y = item->pos.y_pos + BLOCK_SIZE;
		ForcedFixedCamera.z = item->pos.z_pos - ((BLOCK_SIZE * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
		ForcedFixedCamera.room_number = item->room_number;
	} else if (item->current_anim_state == 1)
		UseForcedFixedCamera = 0;
}

void FloorTrapDoorCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll) {
	ITEM_INFO* item;
	int32_t y;

	item = &items[item_number];

	if (input & IN_ACTION && item->status != ITEM_ACTIVE && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH
	        && lara.gun_status == LG_NO_ARMS || lara.IsMoving && lara.GeneralPtr == item_number) {
		if (TestLaraPosition(FloorTrapDoorBounds, item, l)) {
			if (MoveLaraPosition(&FloorTrapDoorPos, item, l)) {
				l->anim_number = ANIM_LIFTTRAP;
				l->frame_number = anims[ANIM_LIFTTRAP].frame_base;
				l->current_anim_state = AS_LIFTTRAP;
				lara.IsMoving = 0;
				lara.head_x_rot = 0;
				lara.head_y_rot = 0;
				lara.torso_x_rot = 0;
				lara.torso_y_rot = 0;
				lara.gun_status = LG_HANDS_BUSY;
				AddActiveItem(item_number);
				item->goal_anim_state = 1;
				item->status = ITEM_ACTIVE;
				UseForcedFixedCamera = 1;
				ForcedFixedCamera.x = item->pos.x_pos - ((2048 * phd_sin(item->pos.y_rot) >> W2V_SHIFT));
				ForcedFixedCamera.z = item->pos.z_pos - ((2048 * phd_cos(item->pos.y_rot) >> W2V_SHIFT));
				y = item->pos.y_pos - 2048;

				if (y < room[item->room_number].maxceiling)
					y = room[item->room_number].maxceiling;

				ForcedFixedCamera.y = y;
				ForcedFixedCamera.room_number = item->room_number;
			} else
				lara.GeneralPtr = item_number;
		}
	} else if (item->current_anim_state == 1)
		UseForcedFixedCamera = 0;
}

void OpenTrapDoor(ITEM_INFO* item) {
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	uint16_t pitsky;

	pitsky = item->item_flags[3];
	r = &room[item->room_number];
	floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];

	if (item->pos.y_pos == r->minfloor) {
		floor->pit_room = pitsky & 0xFF;
		r = &room[floor->pit_room];
		floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];
		floor->sky_room = pitsky >> 8;
	} else {
		floor->sky_room = pitsky >> 8;
		r = &room[floor->sky_room];
		floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];
		floor->pit_room = pitsky & 0xFF;
	}

	item->item_flags[2] = 0;
}

void CloseTrapDoor(ITEM_INFO* item) {
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	uint16_t pitsky;

	r = &room[item->room_number];
	floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];

	if (item->pos.y_pos == r->minfloor) {
		pitsky = floor->pit_room;
		floor->pit_room = 255;
		r = &room[pitsky];
		floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];
		pitsky |= floor->sky_room << 8;
		floor->sky_room = 255;
		item->item_flags[2] = 1;
		item->item_flags[3] = pitsky;
	} else if (item->pos.y_pos == r->maxceiling) {
		pitsky = floor->sky_room;
		floor->sky_room = 255;
		r = &room[pitsky];
		floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];
		pitsky <<= 8;
		pitsky |= floor->pit_room;
		floor->pit_room = 255;
		item->item_flags[2] = 1;
		item->item_flags[3] = pitsky;
	} else {
		item->item_flags[2] = 1;
		item->item_flags[3] = 0;
	}
}

void TrapDoorControl(int16_t item_number) {
	ITEM_INFO* item;

	item = &items[item_number];

	if (TriggerActive(item)) {
		if (!item->current_anim_state && item->trigger_flags >= 0)
			item->goal_anim_state = 1;
	} else if (item->current_anim_state == 1)
		item->goal_anim_state = 0;

	AnimateItem(item);

	if (item->current_anim_state == 1 && item->item_flags[2])
		OpenTrapDoor(item);
	else if (!item->current_anim_state && !item->item_flags[2])
		CloseTrapDoor(item);
}

void ControlObelisk(int16_t item_number) {
	ITEM_INFO* item;
	ITEM_INFO* pulley;
	ITEM_INFO* pyramid;
	ITEM_INFO* disc;
	PHD_VECTOR s;
	PHD_VECTOR d;
	int32_t stop, rad;
	int16_t r, g, b;

	item = &items[item_number];

	if (TriggerActive(item)) {
		if (item->item_flags[3] > 346)
			return;

		item->item_flags[3]++;
		r = (GetRandomControl() & 0x1F) + 224;
		g = r - (GetRandomControl() & 0x1F) - 32;
		b = g - (GetRandomControl() & 0x1F) - 128;

		if (!(GlobalCounter & 1)) {
			rad = 0x2000;

			if (item->item_flags[3] >= 256 || GetRandomControl() & 1) {
				if (item->item_flags[3] < 256 && !(GlobalCounter & 3)) {
					SoundEffect(SFX_ELEC_ONE_SHOT, &item->pos, SFX_DEFAULT);
					rad = (GetRandomControl() & 0xFFF) + 3456;
				}

				s.x = item->pos.x_pos + ((3456 * phd_sin(item->pos.y_rot + 0x4000)) >> W2V_SHIFT);
				s.y = item->pos.y_pos - 256;
				s.z = item->pos.z_pos + ((3456 * phd_cos(item->pos.y_rot + 0x4000)) >> W2V_SHIFT);

				d.x = item->pos.x_pos + ((rad * phd_sin(item->pos.y_rot + 0x4000)) >> W2V_SHIFT);
				d.y = item->pos.y_pos;
				d.z = item->pos.z_pos + ((rad * phd_cos(item->pos.y_rot + 0x4000)) >> W2V_SHIFT);

				if (abs(s.x - lara_item->pos.x_pos) < 20480 && abs(s.y - lara_item->pos.y_pos) < 20480 && abs(s.z - lara_item->pos.z_pos) < 20480 &&
				        abs(d.x - lara_item->pos.x_pos) < 20480 && abs(d.y - lara_item->pos.y_pos) < 20480 && abs(d.z - lara_item->pos.z_pos) < 20480) {
					if (!(GlobalCounter & 3))
						TriggerLightning(&s, &d, (GetRandomControl() & 0x1F) + 32, RGBA(r, g, b, 24), 1, 32, 5);

					TriggerLightningGlow(s.x, s.y, s.z, RGBA(r, g, b, 48));
				}
			}
		}

		if (item->item_flags[3] >= 256 && item->trigger_flags == 2) {
			s.x = item->pos.x_pos + ((0x2000 * phd_sin(item->pos.y_rot + 0x4000)) >> W2V_SHIFT);
			s.y = item->pos.y_pos;
			s.z = item->pos.z_pos + ((0x2000 * phd_cos(item->pos.y_rot + 0x4000)) >> W2V_SHIFT);
			SoundEffect(SFX_ELEC_ARCING_LOOP, (PHD_3DPOS*)&s, SFX_DEFAULT);

			if (GlobalCounter & 1) {
				d.x = (GetRandomControl() & 0x3FF) + s.x - HALF_BLOCK_SIZE;
				d.y = (GetRandomControl() & 0x3FF) + s.y - HALF_BLOCK_SIZE;
				d.z = (GetRandomControl() & 0x3FF) + s.z - HALF_BLOCK_SIZE;

				if (abs(s.x - lara_item->pos.x_pos) < (BLOCK_SIZE * 20) && abs(s.y - lara_item->pos.y_pos) < (BLOCK_SIZE * 20) && abs(s.z - lara_item->pos.z_pos) < (BLOCK_SIZE * 20) &&
				        abs(d.x - lara_item->pos.x_pos) < (BLOCK_SIZE * 20) && abs(d.y - lara_item->pos.y_pos) < (BLOCK_SIZE * 20) && abs(d.z - lara_item->pos.z_pos) < (BLOCK_SIZE * 20)) {
					if (item->item_flags[2] != NO_ITEM) {
						pyramid = &items[item->item_flags[2]];
						ExplodeItemNode(pyramid, 0, 0, 128);
						KillItem(item->item_flags[2]);
						TriggerExplosionSparks(s.x, s.y, s.z, 3, -2, 0, pyramid->room_number);
						TriggerExplosionSparks(s.x, s.y, s.z, 3, -1, 0, pyramid->room_number);
						item->item_flags[2] = NO_ITEM;

						disc = find_an_item_with_object_type(PUZZLE_ITEM1_COMBO1);
						disc->status = ITEM_INACTIVE;
						SoundEffect(SFX_EXPLOSION1, &disc->pos, SFX_DEFAULT);
						SoundEffect(SFX_EXPLOSION2, &disc->pos, SFX_DEFAULT);
					}

					TriggerLightning(&s, &d, (GetRandomControl() & 0xF) + 16, RGBA(r, g, b, 24), 3, 24, 3);
					TriggerLightningGlow(s.x, s.y, s.z, RGBA(r, g, b, 64));
				}
			}
		}
	} else {
		AnimateItem(item);
		stop = 0;

		if (item->anim_number == objects[item->object_number].anim_index + 2) {	//done going counter-clockwise
			item->pos.y_rot -= 0x4000;

			if (input & IN_ACTION) {
				item->anim_number = objects[item->object_number].anim_index + 1;
				item->frame_number = anims[item->anim_number].frame_base;
			} else
				stop = 1;
		}

		if (item->anim_number == objects[item->object_number].anim_index + 6) {	//done going clockwise
			item->pos.y_rot += 0x4000;

			if (input & IN_ACTION) {
				item->anim_number = objects[item->object_number].anim_index + 5;
				item->frame_number = anims[item->anim_number].frame_base;
			} else
				stop = 1;
		}

		if (stop) {
			item->anim_number = objects[item->object_number].anim_index + 3;
			item->frame_number = anims[item->anim_number].frame_base;
		}

		if (item->trigger_flags == 2) {
			for (int i = 0; i < level_items; i++) {
				pulley = &items[i];

				if (pulley->object_number == PULLEY) {
					if (item->pos.y_rot == -0x4000 && items[item->item_flags[0]].pos.y_rot == 0x4000 && !items[item->item_flags[1]].pos.y_rot)
						pulley->item_flags[1] = 0;
					else
						pulley->item_flags[1] = 1;

					break;
				}
			}
		}
	}
}
