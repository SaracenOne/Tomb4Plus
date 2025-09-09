#include "../../tomb4/pch.h"
#include "../mod_config.h"
#include "../../game/savegame.h"
#include "../../game/gameflow.h"
#include "../../specific/audio.h"

void T4PTriggerSecret(int32_t secret_number) {
	if (!(savegame.Level.Secrets & 1 << secret_number)) {
		MOD_LEVEL_AUDIO_INFO *mod_audio_info = get_game_mod_level_audio_info(gfCurrentLevel);

		if (mod_audio_info->secret_track >= 0) {
			S_CDPlay(mod_audio_info->secret_track, 0);
		}
		savegame.Level.Secrets |= 1 << secret_number;
		savegame.Game.Secrets++;
	}
}