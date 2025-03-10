#include "../tomb4/pch.h"
#include "fmv.h"
#include "dxshell.h"
#include "audio.h"
#include "lighting.h"
#include "function_table.h"
#include "winmain.h"
#include "input.h"
#include "3dmath.h"
#include "../game/text.h"
#include "d3dmatrix.h"
#include "dxsound.h"
#include "../game/control.h"
#include "cmdline.h"
#include "gamemain.h"
#include "LoadSave.h"
#include "../tomb4/mod_config.h"

bool LoadBinkStuff() {
	return 0;
}

void FreeBinkStuff() {
}

void ShowBinkFrame() {
}

int32_t PlayFmvNow(int32_t num) {
	if (get_game_mod_global_info()->tr_level_editor) {
		return 0;
	}
	return 0;
}
