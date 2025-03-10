#include "../tomb4/pch.h"
#include "text.h"
#include "../specific/polyinsert.h"
#include "../specific/specificfx.h"
#include "../specific/texture.h"
#include "../specific/3dmath.h"
#include "../specific/function_stubs.h"
#include "../specific/gamemain.h"
#include "../specific/function_table.h"
#include "../tomb4/mod_config.h"
#include "gameflow.h"

int32_t stash_font_height;
int32_t smol_font_height;
int32_t savegame_font_height;
int32_t small_font;
int32_t font_height;
int32_t GnFrameCounter;

static CVECTOR FontShades[10][32];
static uint8_t ScaleFlag;

int8_t AccentTable[46][2] = {
	{'{', ' '},
	{'u', '^'},
	{'e', '\\'},
	{'a', ']'},
	{'a', '^'},
	{'a', '['},
	{'a', '\\'},
	{'{', ' '},
	{'e', ']'},
	{'e', '^'},
	{'e', '['},
	{'|', '^'},
	{'|', ']'},
	{'|', '['},
	{'A', '^'},
	{'A', ']'},
	{'E', '\\'},
	{' ', ' '},
	{' ', ' '},
	{'o', ']'},
	{'o', '^'},
	{'o', '['},
	{'u', ']'},
	{'u', '['},
	{'y', '^'},
	{'O', '^'},
	{'U', '^'},
	{' ', ' '},
	{'O', '\\'},
	{' ', ' '},
	{' ', ' '},
	{' ', ' '},
	{'a', '\\'},
	{'|', '\\'},
	{'o', '\\'},
	{'u', '\\'},
	{'n', '_'},
	{'N', '_'},
	{' ', ' '},
	{' ', ' '},
	{'}', ' '},
	{' ', ' '},
	{' ', ' '},
	{' ', ' '},
	{' ', ' '},
	{'~', ' '}
};

int custom_glyph_scale_width = 512;
int custom_glyph_scale_height = 240;

#pragma warning(push)
#pragma warning(disable : 4838)
#pragma warning(disable : 4309)
static CVECTOR ShadeFromTo[10][2] = {
	{ {(int8_t)128, (int8_t)128, (int8_t)128, 0}, {(int8_t)128, (int8_t)128, (int8_t)128, 0} },
	{ {(int8_t)128, (int8_t)128, (int8_t)128, 0}, {(int8_t)128, (int8_t)128, (int8_t)128, 0} },
	{ {(int8_t)128, (int8_t)128, (int8_t)128, 0}, {(int8_t)128, (int8_t)128, (int8_t)128, 0} },
	{ {(int8_t)128, 0, 0, 0}, {64, 0, 0, 0} },
	{ {0, 0, (int8_t)160, 0}, {0, 0, 80, 0} },
	{ {(int8_t)128, (int8_t)128, (int8_t)128, 0}, {16, 16, 16, 0} },
	{ {(int8_t)192, (int8_t)128, 64, 0}, {64, 16, 0, 0} },
	{ {16, 16, 16, 0}, {(int8_t)128, (int8_t)128, (int8_t)128, 0} },
	{ {(int8_t)224, (int8_t)192, 0, 0}, {64, 32, 0, 0} },
	{ {(int8_t)128, 0, 0, 0}, {64, 0, 0, 0} },
};
#pragma warning(pop)

static CHARDEF CharDef[CHAR_TABLE_COUNT];

void InitFont() {
	MOD_LEVEL_FONT_INFO *font_info = get_game_mod_level_font_info(gfCurrentLevel);

	custom_glyph_scale_width = font_info->custom_glyph_scale_width;
	custom_glyph_scale_height = font_info->custom_glyph_scale_height;

	// Main font colour
	ShadeFromTo[2][0].b = (font_info->main_font_main_color & 0x00ff0000) >> 16;
	ShadeFromTo[2][0].g = (font_info->main_font_main_color & 0x0000ff00) >> 8;
	ShadeFromTo[2][0].r = (font_info->main_font_main_color & 0x000000ff);
	ShadeFromTo[2][1].b = (font_info->main_font_fade_color & 0x00ff0000) >> 16;
	ShadeFromTo[2][1].g = (font_info->main_font_fade_color & 0x0000ff00) >> 8;
	ShadeFromTo[2][1].r = (font_info->main_font_fade_color & 0x000000ff);

	// Option item colour
	ShadeFromTo[5][0].b = (font_info->inventory_item_font_main_color & 0x00ff0000) >> 16;
	ShadeFromTo[5][0].g = (font_info->inventory_item_font_main_color & 0x0000ff00) >> 8;
	ShadeFromTo[5][0].r = (font_info->inventory_item_font_main_color & 0x000000ff);
	ShadeFromTo[5][1].b = (font_info->inventory_item_font_fade_color & 0x00ff0000) >> 16;
	ShadeFromTo[5][1].g = (font_info->inventory_item_font_fade_color & 0x0000ff00) >> 8;
	ShadeFromTo[5][1].r = (font_info->inventory_item_font_fade_color & 0x000000ff);

	// Option title colour
	ShadeFromTo[6][0].b = (font_info->options_title_font_main_color & 0x00ff0000) >> 16;
	ShadeFromTo[6][0].g = (font_info->options_title_font_main_color & 0x0000ff00) >> 8;
	ShadeFromTo[6][0].r = (font_info->options_title_font_main_color & 0x000000ff);
	ShadeFromTo[6][1].b = (font_info->options_title_font_fade_color & 0x00ff0000) >> 16;
	ShadeFromTo[6][1].g = (font_info->options_title_font_fade_color & 0x0000ff00) >> 8;
	ShadeFromTo[6][1].r = (font_info->options_title_font_fade_color & 0x000000ff);

	// Option title colour
	ShadeFromTo[8][0].b = (font_info->inventory_title_font_main_color & 0x00ff0000) >> 16;
	ShadeFromTo[8][0].g = (font_info->inventory_title_font_main_color & 0x0000ff00) >> 8;
	ShadeFromTo[8][0].r = (font_info->inventory_title_font_main_color & 0x000000ff);
	ShadeFromTo[8][1].b = (font_info->inventory_title_font_fade_color & 0x00ff0000) >> 16;
	ShadeFromTo[8][1].g = (font_info->inventory_title_font_fade_color & 0x0000ff00) >> 8;
	ShadeFromTo[8][1].r = (font_info->inventory_title_font_fade_color & 0x000000ff);

	memcpy(CharDef, font_info->custom_font_table, sizeof(CHARDEF) * CHAR_TABLE_COUNT);

	GFXTLVERTEX v;
	static CHARDEF copy[106];
	static int32_t init = 1;
	uint16_t r, g, b;
	int16_t h, w, yoff;
	uint8_t fr, fg, fb, tr, tg, tb;

	for (int i = 0; i < 10; i++) {
		fr = ShadeFromTo[i][0].r;
		fg = ShadeFromTo[i][0].g;
		fb = ShadeFromTo[i][0].b;
		tr = ShadeFromTo[i][1].r;
		tg = ShadeFromTo[i][1].g;
		tb = ShadeFromTo[i][1].b;

		for (int j = 0; j < 16; j++) {
			r = ((tr * j) >> 4) + ((fr * (16 - j)) >> 4);
			g = ((tg * j) >> 4) + ((fg * (16 - j)) >> 4);
			b = ((tb * j) >> 4) + ((fb * (16 - j)) >> 4);

			if (r > 255)
				r = 255;

			if (g > 255)
				g = 255;

			if (b > 255)
				b = 255;

			CalcColorSplit(RGBONLY(b, g, r), &v.color);

			r = CLRR(v.color);
			g = CLRG(v.color);
			b = CLRB(v.color);
			FontShades[i][j << 1].r = (uint8_t)r;
			FontShades[i][j << 1].g = (uint8_t)g;
			FontShades[i][j << 1].b = (uint8_t)b;
			FontShades[i][j << 1].a = (uint8_t)0xFF;

			r = CLRR(v.specular);
			g = CLRG(v.specular);
			b = CLRB(v.specular);
			FontShades[i][(j << 1) + 1].r = (uint8_t)r;
			FontShades[i][(j << 1) + 1].g = (uint8_t)g;
			FontShades[i][(j << 1) + 1].b = (uint8_t)b;
			FontShades[i][(j << 1) + 1].a = (uint8_t)0xFF;
		}
	}

	if (init) {
		for (int i = 0; i < 106; i++) {
			copy[i].h = CharDef[i].h;
			copy[i].w = CharDef[i].w;
			copy[i].y_offset = CharDef[i].y_offset;
		}

		init = 0;
	}

	for (int i = 0; i < 106; i++) {
		h = int16_t((float)copy[i].h * float(phd_winymax / 240.0F));
		w = int16_t((float)copy[i].w * float(phd_winxmax / 512.0F));
		yoff = int16_t((float)copy[i].y_offset * float(phd_winymax / 240.0F));
		CharDef[i].h = h;
		CharDef[i].w = w;
		CharDef[i].y_offset = yoff;
	}

	font_height = int32_t(float(phd_winymax * font_info->custom_vertical_spacing));
	stash_font_height = font_height;
	savegame_font_height = int32_t(float(3.0F * phd_winymax / 40.0F));
	smol_font_height = int32_t(float(7.0F * phd_winymax / 120.0F));
}

void UpdatePulseColour() {
	GFXTLVERTEX v;
	static uint8_t PulseCnt = 0;
	uint8_t c, r, g, b;

	PulseCnt = (PulseCnt + 1) & 0x1F;

	if (PulseCnt > 16)
		c = -PulseCnt;
	else
		c = PulseCnt;

	c <<= 3;
	CalcColorSplit(RGBONLY(c, c, c), &v.color);

	for (int i = 0; i < 16; i++) {
		r = CLRR(v.color);
		g = CLRG(v.color);
		b = CLRB(v.color);
		FontShades[1][i << 1].r = r;
		FontShades[1][i << 1].g = g;
		FontShades[1][i << 1].b = b;

		r = CLRR(v.specular);
		g = CLRG(v.specular);
		b = CLRB(v.specular);
		FontShades[1][(i << 1) + 1].r = r;
		FontShades[1][(i << 1) + 1].g = g;
		FontShades[1][(i << 1) + 1].b = b;
	}
}

int32_t GetStringLengthScaled(const char* string, int32_t* top, int32_t* bottom, float glyph_scale_width, float glyph_scale_height) {
	CHARDEF* def;
	int32_t s, accent, length, lowest, highest;

	s = *string++;
	length = 0;
	accent = 0;
	lowest = -BLOCK_SIZE;
	highest = BLOCK_SIZE;

	while (s) {
		if (s == '\n')
			break;

		if (s == ' ')
			length += int32_t((float(phd_winxmax + 1) / 640.0F) * 8.0F);
		else if (s == '\t') {
			length += 40;

			if (top) {
				if (highest > -12)
					highest = -12;
			}

			if (bottom) {
				if (lowest < 2)
					lowest = 2;
			}
		} else if (s >= 20) {
			if (s < ' ')
				def = &CharDef[s + 74];
			else {
				if (s >= 128 && s <= 173) {
					accent = 1;
					s = AccentTable[s - 128][0];
				}

				def = &CharDef[s - '!'];
			}

			float scaled_glypth_width = def->w * glyph_scale_width;
			float scaled_glypth_height = def->h * glyph_scale_height;

			if (ScaleFlag)
				length += int32_t(scaled_glypth_width - scaled_glypth_width / 4);
			else
				length += int32_t(scaled_glypth_width);

			int32_t scaled_y_offset = int32_t(def->y_offset * glyph_scale_height);

			if (top) {
				if (scaled_y_offset < highest)
					highest = int32_t(scaled_y_offset);
			}

			if (bottom) {
				if (int32_t(scaled_glypth_height) + scaled_y_offset > lowest)
					lowest = int32_t(scaled_glypth_height) + scaled_y_offset;
			}
		}

		s = *string++;
	}

	if (top) {
		if (accent)
			highest -= 4;

		*top = highest;
	}

	if (bottom)
		*bottom = lowest;

	return length;
}

int32_t GetStringLength(const char* string, int32_t* top, int32_t* bottom) {
	float glyph_scale_width = (float)DEFAULT_GLYPH_SCALE_WIDTH / (float)custom_glyph_scale_width;
	float glyph_scale_height = (float)DEFAULT_GLYPH_SCALE_HEIGHT / (float)custom_glyph_scale_height;

	return GetStringLengthScaled(string, top, bottom, glyph_scale_width, glyph_scale_height);
}

void DrawCharScaled(int32_t x, int32_t y, uint16_t col, CHARDEF* def, float glyph_scale_width, float glyph_scale_height) {
	GFXTLVERTEX* v;
	TEXTURESTRUCT tex;
	float u1, v1, u2, v2;
	int32_t x1, y1, x2, y2, top, bottom;

	v = MyVertexBuffer;

	y1 = y + phd_winymin + int32_t(def->y_offset * glyph_scale_height);
	y2 = y + phd_winymin + int32_t((def->h * glyph_scale_height) + (def->y_offset * glyph_scale_height));

	if (small_font) {
		y1 = int32_t((float)y1 * 0.75F);
		y2 = int32_t((float)y2 * 0.75F);
	}

	x1 = x + phd_winxmin;
	x2 = x1 + int32_t(def->w * glyph_scale_width);
	setXY4(v, x1, y1, x2, y1, x2, y2, x1, y2, (int32_t)f_mznear, clipflags);

	top = *(int32_t*)&FontShades[col][2 * def->top_shade];
	bottom = *(int32_t*)&FontShades[col][2 * def->bottom_shade];
	v[0].color = top;
	v[1].color = top;
	v[2].color = bottom;
	v[3].color = bottom;

	top = *(int32_t*)&FontShades[col][(2 * def->top_shade) + 1];
	bottom = *(int32_t*)&FontShades[col][(2 * def->bottom_shade) + 1];
	v[0].specular = top;
	v[1].specular = top;
	v[2].specular = bottom;
	v[3].specular = bottom;

	u1 = def->u + (1.0F / 512.0F);
	v1 = def->v + (1.0F / 512.0F);
	u2 = 512.0F / float(phd_winxmax + 1) * (float)def->w * (1.0F / 256.0F) + def->u - (1.0F / 512.0F);
	v2 = 240.0F / float(phd_winymax + 1) * (float)def->h * (1.0F / 256.0F) + def->v - (1.0F / 512.0F);
	tex.u1 = u1;
	tex.v1 = v1;
	tex.u2 = u2;
	tex.v2 = v1;
	tex.u3 = u2;
	tex.v3 = v2;
	tex.u4 = u1;
	tex.v4 = v2;

	tex.drawtype = 1;
	tex.tpage = uint16_t(nTextures - 2);
	tex.flag = 0;
	nPolyType = 4;
	AddQuadClippedSorted(v, 0, 1, 2, 3, &tex, 0);
}

void DrawChar(int32_t x, int32_t y, uint16_t col, CHARDEF* def) {
	float glyph_scale_width = (float)DEFAULT_GLYPH_SCALE_WIDTH / (float)custom_glyph_scale_width;
	float glyph_scale_height = (float)DEFAULT_GLYPH_SCALE_HEIGHT / (float)custom_glyph_scale_height;

	DrawCharScaled(x, y, col, def, glyph_scale_width, glyph_scale_height);
}

void PrintStringScaled(int32_t x, int32_t y, uint8_t col, const char* string, uint16_t flags, float glyph_scale_width, float glyph_scale_height) {
	CHARDEF* def;
	CHARDEF* accent;
	int32_t x2, bottom, l, top, bottom2;
	uint8_t s;

	if (flags & FF_BLINK && GnFrameCounter & 0x10)
		return;

	ScaleFlag = (flags & FF_SMALL) != 0;
	x2 = GetStringLengthScaled(string, 0, &bottom, glyph_scale_width, glyph_scale_height);

	if (flags & FF_CENTER)
		x2 = x - (x2 >> 1);
	else if (flags & FF_RJUSTIFY)
		x2 = x - x2;
	else
		x2 = x;

	s = *string++;

	while (s) {
		if (s == '\n') {
			if (*string == '\n') {
				bottom = 0;
				y += 16;
			} else {
				l = GetStringLengthScaled(string, &top, &bottom2, glyph_scale_width, glyph_scale_height);

				if (flags & FF_CENTER)
					x2 = x - (l >> 1);
				else if (flags & FF_RJUSTIFY)
					x2 = x - l;
				else
					x2 = x;

				y += bottom - top + 2;
				bottom = bottom2;
			}

			s = *string++;
			continue;
		}

		if (s == ' ') {
			if (ScaleFlag)
				x2 += 6;
			else
				x2 += int32_t(float(phd_winxmax + 1) / 640.0F * 8.0F);

			s = *string++;
			continue;
		}

		if (s == '\t') {
			x2 += 40;
			s = *string++;
			continue;
		}

		if (s < 20) {
			col = s - 1;
			s = *string++;
			continue;
		}

		if (s >= 128 && s <= 173) {
			def = &CharDef[AccentTable[s - 128][0] - '!'];
			accent = &CharDef[AccentTable[s - 128][1] - '!'];
			DrawCharScaled(x2, y, col, def, glyph_scale_width, glyph_scale_height);

			if (AccentTable[s - 128][1] != ' ')
				DrawCharScaled(def->w / 2 + x2 - 3, y + def->y_offset, col, accent, glyph_scale_width, glyph_scale_height);
		} else {
			if (s < ' ')
				def = &CharDef[s + 74];
			else
				def = &CharDef[s - '!'];

			DrawCharScaled(x2, y, col, def, glyph_scale_width, glyph_scale_height);
		}

		float scaled_glypth_width = def->w * glyph_scale_width;

		if (ScaleFlag)
			x2 += int32_t(scaled_glypth_width - scaled_glypth_width / 4);
		else
			x2 += int32_t(scaled_glypth_width);

		s = *string++;
	}

	ScaleFlag = 0;
}

void PrintString(int32_t x, int32_t y, uint8_t col, const char* string, uint16_t flags) {
	const float glyph_scale_width = (float)DEFAULT_GLYPH_SCALE_WIDTH / (float)custom_glyph_scale_width;
	const float glyph_scale_height = (float)DEFAULT_GLYPH_SCALE_HEIGHT / (float)custom_glyph_scale_height;

	PrintStringScaled(x, y, col, string, flags, glyph_scale_width, glyph_scale_height);
}
