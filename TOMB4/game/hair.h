#pragma once
#include "../global/types.h"

enum LaraHairUpdateType {
	LARA_HAIR_UPDATE_TYPE_BRAID,
	LARA_HAIR_UPDATE_TYPE_PIGTAILS_LEFT,
	LARA_HAIR_UPDATE_TYPE_PIGTAILS_RIGHT,
};

void InitialiseHair();
void HairControl(bool in_cutscene, LaraHairUpdateType lara_hair_update_type, int16_t* cutscenething);
void GetCorrectStashPoints(int32_t pigtail, int32_t hair_node, int32_t skin_node);
void DrawHair();

extern HAIR_STRUCT hairs[2][7];
