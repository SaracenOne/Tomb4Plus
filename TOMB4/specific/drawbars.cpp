#include "../tomb4/pch.h"
#include "LoadSave.h"
#include "function_table.h"
#include "3dmath.h"
#include "function_stubs.h"
#include "gamemain.h"
#include "output.h"
#include "../game/camera.h"
#include "../game/lara.h"
#include "../game/gameflow.h"
#include "winmain.h"
#include "drawroom.h"
#include "polyinsert.h"
#include "../tomb4/tomb4.h"
#include "texture.h"
#include "../tomb4/mod_config.h"

static float loadbar_pos;
static int32_t loadbar_maxpos;

static GouraudBarColourSet healthBarColourSet = {
	{ 64, 96, 128, 96, 64 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 128, 192, 255, 192, 128 },
	{ 0, 0, 0, 0, 0 }
};

static GouraudBarColourSet poisonBarColourSet = {
	{ 64, 96, 128, 96, 64 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 64, 96, 128, 96, 64 },
	{ 0, 0, 0, 0, 0 },
	{ 128, 192, 255, 192, 128 }
};

static GouraudBarColourSet airBarColourSet = {
	{ 0, 0, 0, 0, 0 },
	{ 113, 146, 113, 93, 74 },
	{ 123, 154, 123, 107, 91 },
	{ 0, 0, 0, 0, 0 },
	{ 113, 146, 113, 93, 74 },
	{ 0, 0, 0, 0, 0 }
};

static GouraudBarColourSet dashBarColourSet = {
	{ 144, 192, 240, 192, 144 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 144, 192, 240, 192, 144 },
	{ 144, 192, 240, 192, 144 },
	{ 0, 0, 0, 0, 0 }
};

static GouraudBarColourSet loadBarColourSet = {
	{ 48, 96, 127, 80, 32 },
	{ 0, 0, 0, 0, 0 },
	{ 48, 96, 127, 80, 32 },
	{ 0, 0, 0, 0, 0 },
	{ 48, 96, 127, 80, 32 },
	{ 48, 96, 127, 80, 32 }
};

static GouraudBarColourSet enemyBarColourSet = {
	{ 128, 192, 255, 192, 128 },
	{ 64, 96, 128, 96, 64 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 123, 154, 123, 107, 91 },
	{ 0, 0, 0, 0, 0 }
};

static void DrawColoredRect(float x0, float y0, float x1, float y1, float z, uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3, TEXTURESTRUCT* tex) {
	GFXTLVERTEX* v;

	v = MyVertexBuffer;

	v[0].sx = x0;
	v[0].sy = y0;
	v[0].color = GFX_RGBA_SETALPHA(c0, 0xFF);

	v[1].sx = x1;
	v[1].sy = y0;
	v[1].color = GFX_RGBA_SETALPHA(c1, 0xFF);

	v[2].sx = x1;
	v[2].sy = y1;
	v[2].color = GFX_RGBA_SETALPHA(c2, 0xFF);

	v[3].sx = x0;
	v[3].sy = y1;
	v[3].color = GFX_RGBA_SETALPHA(c3, 0xFF);

	for (int i = 0; i < 4; i++) {
		v[i].sz = z;
		v[i].rhw = f_mpersp / z * f_moneopersp;
		v[i].specular = 0xFF000000;
	}

	AddQuadSorted(v, 0, 1, 2, 3, tex, 0);
}

static void S_DrawGouraudBar(int32_t x, int32_t y, int32_t width, int32_t height, int32_t pos, GouraudBarColourSet* colour, bool scaled) {
	TEXTURESTRUCT tex;
	float bar, max, h, x0, y0, x1, y1;
	int32_t p, r, g, b, c0, c1, c2, c3;

	nPolyType = 6;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	tex.drawtype = 0;
	tex.tpage = 0;

	h = (float)height / 3.0F;
	max = (float)pos / 100.0F;
	bar = (float)width * max;

	x0 = (float)x;
	y0 = (float)y;
	x1 = x + bar;
	y1 = y + h;

	r = colour->abLeftRed[0];
	g = colour->abLeftGreen[0];
	b = colour->abLeftBlue[0];
	r -= r >> 2;
	g -= g >> 2;
	b -= b >> 2;
	c2 = RGBONLY(r, g, b);

	r = (int32_t)((1 - max) * colour->abLeftRed[0] + max * colour->abRightRed[0]);
	g = (int32_t)((1 - max) * colour->abLeftGreen[0] + max * colour->abRightGreen[0]);
	b = (int32_t)((1 - max) * colour->abLeftBlue[0] + max * colour->abRightBlue[0]);
	r -= r >> 2;
	g -= g >> 2;
	b -= b >> 2;
	c3 = RGBONLY(r, g, b);

	DrawColoredRect(x0, y0, x1, y1, f_mznear, 0, 0, c3, c2, &tex);

	for (int i = 0; i < 4; i++) {
		c0 = RGBONLY(colour->abLeftRed[i], colour->abLeftGreen[i], colour->abLeftBlue[i]);
		r = (int32_t)((1 - max) * colour->abLeftRed[i] + max * colour->abRightRed[i]);
		g = (int32_t)((1 - max) * colour->abLeftGreen[i] + max * colour->abRightGreen[i]);
		b = (int32_t)((1 - max) * colour->abLeftBlue[i] + max * colour->abRightBlue[i]);
		c1 = RGBONLY(r, g, b);
		c2 = RGBONLY(colour->abLeftRed[i + 1], colour->abLeftGreen[i + 1], colour->abLeftBlue[i + 1]);
		r = (int32_t)((1 - max) * colour->abLeftRed[i + 1] + max * colour->abRightRed[i + 1]);
		g = (int32_t)((1 - max) * colour->abLeftGreen[i + 1] + max * colour->abRightGreen[i + 1]);
		b = (int32_t)((1 - max) * colour->abLeftBlue[i + 1] + max * colour->abRightBlue[i + 1]);
		c3 = RGBONLY(r, g, b);

		y0 += h;
		y1 += h;
		DrawColoredRect(x0, y0, x1, y1, f_mznear, c0, c1, c3, c2, &tex);
	}

	r = colour->abLeftRed[4];
	g = colour->abLeftGreen[4];
	b = colour->abLeftBlue[4];
	r -= r >> 2;
	g -= g >> 2;
	b -= b >> 2;
	c0 = RGBONLY(r, g, b);

	r = (int32_t)((1 - max) * colour->abLeftRed[4] + max * colour->abRightRed[4]);
	g = (int32_t)((1 - max) * colour->abLeftGreen[4] + max * colour->abRightGreen[4]);
	b = (int32_t)((1 - max) * colour->abLeftBlue[4] + max * colour->abRightBlue[4]);
	r -= r >> 2;
	g -= g >> 2;
	b -= b >> 2;
	c1 = RGBONLY(r, g, b);

	y0 += h;
	y1 += h;
	DrawColoredRect(x0, y0, x1, y1, f_mznear, c0, c1, 0, 0, &tex);

	x0 = (float)x;
	y0 = (float)y;
	x1 = float(x + width);
	y1 = y + (h * 6);

	if (scaled)
		p = GetRenderScale(1);
	else
		p = GetFixedScale(1);

	DrawColoredRect(x0 - p, y0, x1 + p, y1, f_mznear + 1, 0, 0, 0, 0, &tex);
	DrawColoredRect(x0 - (2 * p), y0 - p, x1 + (2 * p), y1 + p, f_mznear + 2, 0xFF508282, 0xFFA0A0A0, 0xFF508282, 0xFFA0A0A0, &tex);
	DrawColoredRect(x0 - (3 * p), y0 + p, x1 + (3 * p), y1 - p, f_mznear + 3, 0xFF284141, 0xFF505050, 0xFF284141, 0xFF505050, &tex);
}

static void S_DoTR5Bar(int32_t x, int32_t y, int32_t width, int32_t height, int32_t pos, int32_t clr1, int32_t clr2, bool scaled) {
	TEXTURESTRUCT tex;
	float r1, g1, b1, r2, g2, b2, r, g, b, mul;
	int32_t bar, y2, p, lr, lg, lb, c0, c1, c2, c3;

	nPolyType = 6;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	tex.drawtype = 0;
	tex.tpage = 0;

	if (scaled)
		p = GetRenderScale(1);
	else
		p = GetFixedScale(1);

	y2 = y + height;
	bar = width * pos / 100;

	r1 = (float)CLRR(clr1);
	g1 = (float)CLRG(clr1);
	b1 = (float)CLRB(clr1);
	r2 = (float)CLRR(clr2);
	g2 = (float)CLRG(clr2);
	b2 = (float)CLRB(clr2);

	mul = (float)bar / (float)width;
	r = r1 + ((r2 - r1) * mul);
	g = g1 + ((g2 - g1) * mul);
	b = b1 + ((b2 - b1) * mul);

	lr = (int32_t)r1;
	lg = (int32_t)g1;
	lb = (int32_t)b1;
	c0 = RGBONLY(lr >> 1, lg >> 1, lb >> 1);
	c2 = RGBONLY(lr, lg, lb);

	lr = (int32_t)r;
	lg = (int32_t)g;
	lb = (int32_t)b;
	c1 = RGBONLY(lr >> 1, lg >> 1, lb >> 1);
	c3 = RGBONLY(lr, lg, lb);

	DrawColoredRect((float)x, (float)y, float(x + bar), (float)y2, f_mznear, c0, c1, c3, c2, &tex);
	DrawColoredRect((float)x, (float)y2, float(x + bar), float(y2 + height), f_mznear, c2, c3, c1, c0, &tex);

	DrawColoredRect((float)x, (float)y, float(x + width), float(y2 + height), f_mznear + 1, 0, 0, 0, 0, &tex);
	DrawColoredRect(float(x - p), float(y - p), float(x + width + p), float(y2 + height + p), f_mznear + 2, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, &tex);
}

static void DoBar(int32_t x, int32_t y, int32_t width, int32_t height, int32_t pos, int32_t c1, int32_t c2, bool scaled) {
	TEXTURESTRUCT tex;
	int32_t p, xw, y2, bar;

	nPolyType = 6;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	tex.drawtype = 0;
	tex.tpage = 0;

	if (scaled)
		p = GetRenderScale(1);
	else
		p = GetFixedScale(1);

	xw = x + width;
	y2 = y + height;
	bar = width * pos / 100;

	DrawColoredRect((float)x, (float)y, float(x + bar), float(y2), f_mznear, c1, c1, c2, c2, &tex);
	DrawColoredRect((float)x, (float)y2, float(x + bar), float(y2 + height), f_mznear, c2, c2, c1, c1, &tex);

	DrawColoredRect((float)x, (float)y, (float)xw, float(y2 + height), f_mznear + 1, 0, 0, 0, 0, &tex);
	DrawColoredRect(float(x - p), float(y - p), float(xw + p), float(y2 + height + p), f_mznear + 2, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, &tex);
}

static void DoBarCustom(
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    int32_t pos,
    MOD_LEVEL_BAR_INFO* bar_info,
    bool scaled) {
	TEXTURESTRUCT tex;
	int32_t p, xw, y2, bar;

	nPolyType = 6;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	tex.drawtype = 0;
	tex.tpage = 0;

	if (scaled)
		p = GetRenderScale(1);
	else
		p = GetFixedScale(1);

	xw = x + width;
	y2 = y + height;
	bar = width * pos / 100;


	DrawColoredRect((float)x, (float)y, float(x + bar), float(y2), f_mznear, bar_info->lower_rect.lower_left_color, bar_info->lower_rect.lower_right_color, bar_info->lower_rect.upper_right_color, bar_info->lower_rect.upper_left_color, &tex);
	DrawColoredRect((float)x, (float)y2, float(x + bar), float(y2 + height), f_mznear, bar_info->upper_rect.lower_left_color, bar_info->upper_rect.lower_right_color, bar_info->upper_rect.upper_right_color, bar_info->upper_rect.upper_left_color, &tex);

	DrawColoredRect((float)x, (float)y, (float)xw, float(y2 + height), f_mznear + 1, bar_info->background_rect.lower_left_color, bar_info->background_rect.lower_right_color, bar_info->background_rect.upper_right_color, bar_info->background_rect.upper_left_color, &tex);
	DrawColoredRect(float(x - p), float(y - p), float(xw + p), float(y2 + height + p), f_mznear + 2, bar_info->border_rect.lower_left_color, bar_info->border_rect.lower_right_color, bar_info->border_rect.upper_right_color, bar_info->border_rect.upper_left_color, &tex);
}

static void S_DrawHealthBar2(int32_t pos) {
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	w = GetFixedScale(150);
	h = GetFixedScale(6);
	x = phd_centerx - GetFixedScale(75);
	y = GetFixedScale(100);

	if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
		MOD_LEVEL_BAR_INFO* bar_info = nullptr;
		if (lara.poisoned) {
			bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->poison_bar;
		} else {
			bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->health_bar;
		}
		DoBarCustom(x, y, w, h, pos, bar_info, 1);
	} else if (tomb4.bar_mode == BAR_MODE_PSX)
		S_DrawGouraudBar(x, y, w, h, pos, lara.poisoned ? &poisonBarColourSet : &healthBarColourSet, 0);
	else if (tomb4.bar_mode == BAR_MODE_IMPROVED)
		S_DoTR5Bar(x, y, w, h, pos, 0xA00000, lara.poisoned ? 0xA0A000 : 0x00A000, 0);
	else
		DoBar(x, y, w, h, pos, 0xFF000000, lara.poisoned ? 0xFFFFFF00 : 0xFFFF0000, 0);
}

static void S_DrawEnemyBar2(int32_t pos) {
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	w = GetFixedScale(150);
	h = GetFixedScale(6);
	x = phd_centerx - GetFixedScale(75);
	y = GetFixedScale(117);

	if (tomb4.bar_mode == BAR_MODE_CUSTOM)
		DoBar(x, y, w, h, pos, 0xFF000000, 0xFFFFA000, 0);
	else if (tomb4.bar_mode == BAR_MODE_PSX)
		S_DrawGouraudBar(x, y, w, h, pos, &enemyBarColourSet, 0);
	else if (tomb4.bar_mode == BAR_MODE_IMPROVED)
		S_DoTR5Bar(x, y, w, h, pos, 0xA00000, 0xA0A000, 0);
	else if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
		MOD_LEVEL_BAR_INFO* bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->enemy_bar;
		DoBarCustom(x, y, w, h, pos, bar_info, 1);
	} else
		DoBar(x, y, w, h, pos, 0xFF000000, 0xFFFFA000, 0);
}

void S_DrawHealthBar(int32_t pos) {
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	if (!gfCurrentLevel)
		return;

	if (BinocularRange) {
		S_DrawHealthBar2(pos);
		return;
	}

	w = GetRenderScale(150);
	h = GetRenderScale(6);

	if (tomb4.bars_pos == BARS_POS_ORIGINAL || tomb4.bars_pos == BARS_POS_IMPROVED) { //original or improved
		x = GetRenderScale(8);
		y = GetRenderScale(8);
	} else if (tomb4.bars_pos == BARS_POS_PSX) { // psx
		x = GetRenderScale(36);
		x = phd_winwidth - w - x;
		y = GetRenderScale(18);
	} else if (tomb4.bars_pos == BARS_POS_CUSTOM) { // custom
		MOD_LEVEL_BARS_INFO* bars_info = get_game_mod_level_bars_info(gfCurrentLevel);

		w = GetRenderScale(bars_info->health_bar.width);
		h = GetRenderScale(bars_info->health_bar.height) / 2;
		x = GetRenderScale(bars_info->health_bar.x);
		y = GetRenderScale(bars_info->health_bar.y);
	}

	if (tomb4.bar_mode == BAR_MODE_IMPROVED)
		S_DoTR5Bar(x, y, w, h, pos, 0xA00000, lara.poisoned ? 0xA0A000 : 0x00A000, 1);
	else if (tomb4.bar_mode == BAR_MODE_PSX)
		S_DrawGouraudBar(x, y, w, h, pos, lara.poisoned ? &poisonBarColourSet : &healthBarColourSet, 1);
	else if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
		MOD_LEVEL_BAR_INFO* bar_info = nullptr;
		if (lara.poisoned) {
			bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->poison_bar;
		} else {
			bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->health_bar;
		}
		DoBarCustom(x, y, w, h, pos, bar_info, 1);
	} else
		DoBar(x, y, w, h, pos, 0xFF000000, lara.poisoned ? 0xFFFFFF00 : 0xFFFF0000, 1);
}

void S_DrawAirBar(int32_t pos) {
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	if (!gfCurrentLevel)
		return;

	w = GetRenderScale(150);
	h = GetRenderScale(6);

	if (tomb4.bars_pos == BARS_POS_ORIGINAL) { //original
		x = phd_winwidth - w - GetRenderScale(8);
		y = GetRenderScale(25);
	} else if (tomb4.bars_pos == BARS_POS_IMPROVED) { //improved
		x = phd_winwidth - w - GetRenderScale(8);
		y = GetRenderScale(8);
	} else if (tomb4.bars_pos == BARS_POS_PSX) { // PSX
		x = GetRenderScale(36);
		x = phd_winwidth - w - x;
		y = GetRenderScale(43);
	} else if (tomb4.bars_pos == BARS_POS_CUSTOM) { //custom
		MOD_LEVEL_BARS_INFO* bars_info = get_game_mod_level_bars_info(gfCurrentLevel);

		w = GetRenderScale(bars_info->health_bar.width);
		h = GetRenderScale(bars_info->health_bar.height) / 2;
		x = phd_winwidth - w - GetRenderScale(8);
		y = GetRenderScale(25);
	}

	if (tomb4.bar_mode == BAR_MODE_IMPROVED)
		S_DoTR5Bar(x, y, w, h, pos, 0x0000A0, 0x0050A0, 1);
	else if (tomb4.bar_mode == BAR_MODE_PSX)
		S_DrawGouraudBar(x, y, w, h, pos, &airBarColourSet, 1);
	else if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
		MOD_LEVEL_BAR_INFO* bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->air_bar;
		DoBarCustom(x, y, w, h, pos, bar_info, 1);
	} else
		DoBar(x, y, w, h, pos, 0xFF000000, 0xFF0000FF, 1);
}

void S_DrawDashBar(int32_t pos) {
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	if (!gfCurrentLevel)
		return;

	w = GetRenderScale(150);
	h = GetRenderScale(6);

	if (tomb4.bars_pos == BARS_POS_ORIGINAL) { //original
		x = phd_winwidth - w - GetRenderScale(8);
		y = GetRenderScale(8);
	} else if (tomb4.bars_pos == BARS_POS_IMPROVED) { //improved
		x = phd_winwidth - w - GetRenderScale(8);
		y = GetRenderScale(25);
	} else if (tomb4.bars_pos == BARS_POS_PSX) { //psx
		x = GetRenderScale(36);
		x = phd_winwidth - w - x;
		y = GetRenderScale(68);
	} else if (tomb4.bars_pos == BARS_POS_CUSTOM) { //custom
		MOD_LEVEL_BARS_INFO* bars_info = get_game_mod_level_bars_info(gfCurrentLevel);

		w = GetRenderScale(bars_info->health_bar.width);
		h = GetRenderScale(bars_info->health_bar.height) / 2;
		x = phd_winwidth - w - GetRenderScale(bars_info->health_bar.x);
		y = GetRenderScale(bars_info->health_bar.y);
	}

	if (tomb4.bar_mode == BAR_MODE_IMPROVED)
		S_DoTR5Bar(x, y, w, h, pos, 0xA0A000, 0x00A000, 1);
	else if (tomb4.bar_mode == BAR_MODE_PSX)
		S_DrawGouraudBar(x, y, w, h, pos, &dashBarColourSet, 1);
	else if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
		MOD_LEVEL_BAR_INFO* bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->sprint_bar;
		DoBarCustom(x, y, w, h, pos, bar_info, 1);
	} else
		DoBar(x, y, w, h, pos, 0xFF000000, 0xFF00FF00, 1);
}

void S_DrawEnemyBar(int32_t pos) {
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	if (BinocularRange) {
		S_DrawEnemyBar2(pos);
		return;
	}

	w = GetRenderScale(150);
	h = GetRenderScale(6);

	if (tomb4.bars_pos == BARS_POS_ORIGINAL || tomb4.bars_pos == BARS_POS_IMPROVED) { //original or improved
		x = GetRenderScale(8);
		y = GetRenderScale(25);
	} else if (tomb4.bars_pos == BARS_POS_PSX) { //psx
		x = GetRenderScale(36);
		x = phd_winwidth - w - x;
		y = GetRenderScale(93);
	} else if (tomb4.bars_pos == BARS_POS_CUSTOM) { //custom
		MOD_LEVEL_BARS_INFO* bars_info = get_game_mod_level_bars_info(gfCurrentLevel);

		w = GetRenderScale(bars_info->enemy_bar.width);
		h = GetRenderScale(bars_info->enemy_bar.height) / 2;
		x = GetRenderScale(bars_info->enemy_bar.x);
		y = GetRenderScale(bars_info->enemy_bar.y);
	}

	if (tomb4.bar_mode == BAR_MODE_PSX)
		S_DrawGouraudBar(x, y, w, h, pos, &enemyBarColourSet, 1);
	else if (tomb4.bar_mode == BAR_MODE_IMPROVED)
		S_DoTR5Bar(x, y, w, h, pos, 0xA00000, 0xA0A000, 1);
	else if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
		MOD_LEVEL_BAR_INFO* bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->enemy_bar;
		DoBarCustom(x, y, w, h, pos, bar_info, 1);
	} else
		DoBar(x, y, w, h, pos, 0xFF000000, 0xFFFFA000, 1);
}

void DoSlider(int32_t x, int32_t y, int32_t width, int32_t height, int32_t pos, int32_t c1, int32_t c2, int32_t c3) {
	TEXTURESTRUCT tex;
	float sx, sy, w, h;
	static float V;

	nPolyType = 4;
	V += 0.01F;

	if (V > 0.99F)
		V = 0;

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;

	sx = (float)x * (float)phd_winxmax / 640.0F;
	sy = (float)y;
	w = (float)GetFixedScale(width);
	h = (float)GetFixedScale(height >> 1);

	tex.tpage = uint16_t(nTextures - 1);
	tex.drawtype = 0;
	tex.flag = 0;
	tex.u1 = 0;
	tex.v1 = V;
	tex.u2 = 1;
	tex.v2 = V;
	tex.u3 = 1;
	tex.v3 = V + 0.01F;
	tex.u4 = 0;
	tex.v4 = V + 0.01F;
	DrawColoredRect(sx, sy, sx + w, sy + h, f_mznear + 2, c1, c1, c2, c2, &tex);
	DrawColoredRect(sx, sy + h, sx + w, sy + (h * 2), f_mznear + 2, c2, c2, c1, c1, &tex);

	tex.tpage = 0;
	DrawColoredRect(sx - 1, sy - 1, sx + w + 1, sy + (h * 2) + 1, f_mznear + 4, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, &tex);

	w = pos * w / 100;
	tex.drawtype = 2;
	DrawColoredRect(sx, sy, sx + w + 1, sy + (h * 2), f_mznear + 1, c3, c3, c3, c3, &tex);
}

void S_InitLoadBar(int32_t maxpos) {
	loadbar_pos = 0;
	loadbar_maxpos = maxpos;
}

void S_LoadBar() {
	int32_t x, y, w, h;

	if (gfCurrentLevel || App.dx.Flags & DXF_HWR) {
		_BeginScene();
		InitBuckets();
		InitialiseSortList();
		loadbar_pos += 100 / loadbar_maxpos;

		if (tomb4.tr5_loadbar) {
			x = GetFixedScale(170);
			w = phd_winwidth - (x << 1);
			h = GetFixedScale(5);
			y = phd_winheight - h - GetFixedScale(20);

			if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
				MOD_LEVEL_BAR_INFO* bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->loading_bar;
				DoBarCustom(x, y, w, h, (int32_t)loadbar_pos, bar_info, 0);
			} else if (tomb4.bar_mode == BAR_MODE_PSX)
				S_DrawGouraudBar(x, y, w, h, (int32_t)loadbar_pos, &loadBarColourSet, 0);
			else if (tomb4.bar_mode == BAR_MODE_IMPROVED)
				S_DoTR5Bar(x, y, w, h, (int32_t)loadbar_pos, 0x0000A0, 0x0000F0, 0);
			else
				DoBar(x, y, w, h, (int32_t)loadbar_pos, 0xFF000000, 0xFF9F1F80, 0);
		} else {
			x = GetFixedScale(20);
			w = phd_winwidth - (x << 1);
			h = GetFixedScale(7);
			y = phd_winheight - h - GetFixedScale(20);

			if (tomb4.bar_mode == BAR_MODE_CUSTOM) {
				MOD_LEVEL_BAR_INFO* bar_info = &get_game_mod_level_bars_info(gfCurrentLevel)->loading_bar;
				DoBarCustom(x, y, w, h, (int32_t)loadbar_pos, bar_info, 0);
			} else if (tomb4.bar_mode == BAR_MODE_PSX)
				S_DrawGouraudBar(x, y, w, h, (int32_t)loadbar_pos, &loadBarColourSet, 0);
			else if (tomb4.bar_mode == BAR_MODE_IMPROVED)
				S_DoTR5Bar(x, y, w, h, (int32_t)loadbar_pos, 0xFF7F007F, 0xFF007F7F, 0);
			else
				DoBar(x, y, w, h, (int32_t)loadbar_pos, 0xFF000000, 0xFF9F1F80, 0);
		}
	}
}
