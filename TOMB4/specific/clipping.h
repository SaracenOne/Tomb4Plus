#pragma once
#include "../global/types.h"

int32_t ZClipper(int32_t n, GFXTLBUMPVERTEX *in, GFXTLBUMPVERTEX *out);
int32_t visible_zclip(GFXTLVERTEX *v0, GFXTLVERTEX *v1, GFXTLVERTEX *v2);
int32_t XYUVGClipper(int32_t n, GFXTLBUMPVERTEX *in);
