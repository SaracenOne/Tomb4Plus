#include "../../tomb4/pch.h"

#include "../../global/types.h"
#include "../../game/control.h"
#include "../../game/lara.h"
#include "../../game/camera.h"
#include "../../game/items.h"
#include "../../game/objlight.h"
#include "../mod_config.h"
#include "../../game/gameflow.h"

void ControlTeleporterLight(int16_t item_number) {
	ITEM_INFO *item = &items[item_number];

	// Special teleport trigger
	if (item->trigger_flags == get_game_mod_level_objects_info(gfCurrentLevel)->whitelight_teleport_ocb || game_mod_config.global_info.tomo_swap_whitelight_for_teleporter) {
		if (TriggerActive(item)) {
			item->flags &= ~(IFL_CODEBITS);

			lara_item->pos.x_pos = item->pos.x_pos;
			lara_item->pos.y_pos = item->pos.y_pos;
			lara_item->pos.z_pos = item->pos.z_pos;
			lara_item->pos.y_rot = item->pos.y_rot + 0x8000;

			if (lara_item->room_number != item->room_number)
				ItemNewRoom(lara.item_number, item->room_number);

			camera.fixed_camera = 1;
		}

		return;
	}

	ControlElectricalLight(item_number);
}
