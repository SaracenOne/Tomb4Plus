#pragma once
#include "../global/types.h"

void DrawFlareInAir(ITEM_INFO* item);
void draw_flare_meshes();
void undraw_flare_meshes();
int32_t DoFlareLight(PHD_VECTOR* pos, int32_t flare_age);
void DoFlareInHand(int32_t flare_age);
void CreateFlare(int16_t object, int32_t thrown);
void set_flare_arm(int32_t frame);
void ready_flare();
void draw_flare();
void undraw_flare();
void FlareControl(int16_t item_number);
