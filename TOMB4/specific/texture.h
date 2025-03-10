#pragma once
#include "../global/types.h"

typedef void(TR_CDECL* rgbfunc)(uint8_t*, uint8_t*, uint8_t*);

bgfx::TextureHandle CreateTexturePage(int32_t w, int32_t h, int32_t MipMapCount, int32_t* pSrc, rgbfunc RGBM, int32_t format);
void FreeTextures();

extern TEXTURE* Textures;
extern int32_t nTextures;
