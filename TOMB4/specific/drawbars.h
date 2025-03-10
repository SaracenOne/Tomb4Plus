#pragma once
#include "../global/types.h"

void S_DrawHealthBar(int32_t pos);
void S_DrawAirBar(int32_t pos);
void S_DrawDashBar(int32_t pos);
void S_DrawEnemyBar(int32_t pos);
void DoSlider(int32_t x, int32_t y, int32_t width, int32_t height, int32_t pos, int32_t c1, int32_t c2, int32_t c3);
void S_InitLoadBar(int32_t maxpos);
void S_LoadBar();
