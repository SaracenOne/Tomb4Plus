#include "../tomb4/pch.h"
#include "flmtorch.h"
#include "effect2.h"
#include "../specific/function_stubs.h"
#include "sound.h"
#include "lara_states.h"
#include "objects.h"
#include "collide.h"
#include "switch.h"
#include "items.h"

static short FireBounds[12] = { 0, 0, 0, 0, 0, 0, -1820, 1820, -5460, 5460, -1820, 1820 };

void TriggerTorchFlame(short item_number, long node)
{
	SPARKS* sptr;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = (GetRandomControl() & 0x1F) + 48;
	sptr->sB = 48;
	sptr->dR = (GetRandomControl() & 0x3F) + 192;
	sptr->dG = (GetRandomControl() & 0x3F) + 128;
	sptr->dB = 32;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 12;
	sptr->TransType = 2;
	sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->sLife = sptr->Life;
	sptr->x = (GetRandomControl() & 0xF) - 8;
	sptr->y = 0;
	sptr->z = (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (GetRandomControl() & 0xFF) - 128;
	sptr->Yvel = -16 - (GetRandomControl() & 0xF);
	sptr->Zvel = (GetRandomControl() & 0xFF) - 128;
	sptr->Friction = 5;
	sptr->Flags = 4762;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -16 - (GetRandomControl() & 0xF);
	else
		sptr->RotAdd = (GetRandomControl() & 0xF) + 16;

	sptr->Gravity = -16 - (GetRandomControl() & 0x1F);
	sptr->NodeNumber = (uchar)node;
	sptr->MaxYvel = -16 - (GetRandomControl() & 7);
	sptr->FxObj = (uchar)item_number;
	sptr->Scalar = 1;
	sptr->Size = (GetRandomControl() & 0x1F) + 80;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size >> 3;
	SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &items[item_number].pos, SFX_DEFAULT);
}

void FireCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;
	short rot;

	item = &items[item_number];

	if (lara.gun_type == WEAPON_TORCH && lara.gun_status == LG_READY && !lara.left_arm.lock && (item->status & 1) != lara.LitTorch &&
		item->timer != -1 && input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && !l->gravity_status)
	{
		rot = item->pos.y_rot;

		if (item->object_number == FLAME_EMITTER)
		{
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -800;
			FireBounds[5] = 800;
		}
		else if (item->object_number == FLAME_EMITTER2)
		{
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -600;
			FireBounds[5] = 600;
		}
		else if (item->object_number == SPRINKLER)
		{
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -384;
			FireBounds[5] = 0;
		}

		item->pos.y_rot = l->pos.y_rot;

		if (TestLaraPosition(FireBounds, item, l))
		{
			if (item->object_number == SPRINKLER)
				l->anim_number = ANIM_LIGHT_TORCH4;
			else
			{
				l->item_flags[3] = 1;
				l->anim_number = short((ABS(l->pos.y_pos - item->pos.y_pos) >> 8) + ANIM_LIGHT_TORCH1);
			}

			l->current_anim_state = AS_CONTROLLED;
			l->frame_number = anims[l->anim_number].frame_base;
			lara.flare_control_left = 0;
			lara.left_arm.lock = 3;
			lara.GeneralPtr = (void*)item_number;
		}

		item->pos.y_rot = rot;
	}

	if (lara.GeneralPtr == (void*)item_number && item->status != ITEM_ACTIVE && l->current_anim_state == AS_CONTROLLED &&
		l->anim_number >= ANIM_LIGHT_TORCH1 && l->anim_number <= ANIM_LIGHT_TORCH5 && l->frame_number - anims[l->anim_number].frame_base == 40)
	{
		if (item->object_number == SPRINKLER)
		{
			l->item_flags[3] = 0;
			lara.LitTorch = 0;
		}

		TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, item->flags & IFL_CODEBITS);
		item->flags |= IFL_CODEBITS;
		item->item_flags[3] = 0;
		item->status = ITEM_ACTIVE;
		AddActiveItem(item_number);
	}
}

void inject_flmtorch(bool replace)
{
	INJECT(0x0041F390, TriggerTorchFlame, replace);
	INJECT(0x0041F510, FireCollision, replace);
}
