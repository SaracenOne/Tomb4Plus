#include "../tomb4/pch.h"
#include "texture.h"
#include "dxshell.h"
#include "function_stubs.h"
#include "winmain.h"

TEXTURE* Textures;
int32_t nTextures;

bgfx::TextureHandle CreateTexturePage(int32_t w, int32_t h, int32_t MipMapCount, int32_t* pSrc, rgbfunc RGBM, int32_t format) {
	bgfx::TextureHandle tSurf;

	int32_t* lS;
	int32_t* lD;
	int16_t* sS;
	int16_t* sD;
	char* cD;
	uint32_t c, o, ro, go, bo, ao;
	uint8_t r, g, b, a;

	uint16_t buffer_width = uint16_t(w);
	uint16_t buffer_height = uint16_t(h);

	if (w < 32 || h < 32) {
		MipMapCount = 0;
	}

	const bgfx::Memory* texture_buffer = bgfx::alloc(((buffer_width * buffer_height) * sizeof(int32_t)));

	if (!format) {
		lS = pSrc;
		cD = (char*)texture_buffer->data;

		for (uint32_t y = 0; y < buffer_height; y++) {
			for (uint32_t x = 0; x < buffer_width; x++) {
				c = *(lS + x * 256 / w + y * 0x10000 / h);
				r = CLRR(c);
				g = CLRG(c);
				b = CLRB(c);
				a = CLRA(c);

				if (RGBM)
					RGBM(&r, &g, &b);

				ro = r;
				go = g << 8;
				bo = b << 16;
				ao = a << 24;
				o = ro | go | bo | ao;

				for (int32_t i = 32; i > 0; i -= 8) {
					*cD++ = (int8_t)o;
					o >>= 8;
				}
			}
		}
	} else if (format == 2) {
		sS = (int16_t*)pSrc;
		sD = (int16_t*)texture_buffer->data;

		for (uint32_t y = 0; y < buffer_height; y++) {
			for (uint32_t x = 0; x < buffer_width; x++)
				*sD++ = *(sS + x * 256 / w + y * 0x10000 / h);
		}
	} else if (format == 1) {
		lS = pSrc;

		lD = (int32_t*)texture_buffer->data;

		for (uint32_t y = 0; y < buffer_height; y++) {
			for (uint32_t x = 0; x < buffer_width; x++)
				*lD++ = *(lS + x * 256 / w + y * 0x10000 / h);
		}
	}

	uint64_t flags = BGFX_TEXTURE_NONE;
	if (App.Filtering) {
		flags |= BGFX_SAMPLER_NONE;
	} else {
		flags |= BGFX_SAMPLER_POINT;
	}
	tSurf = bgfx::createTexture2D(buffer_width, buffer_height, false, 1, bgfx::TextureFormat::BGRA8, flags, texture_buffer);

	return tSurf;
}

void FreeTextures() {
	TEXTURE* tex;

	for (int32_t i = 0; i < nTextures; i++) {
		tex = &Textures[i];

		if (bgfx::isValid(tex->tex)) {
			bgfx::destroy(tex->tex);
			Log(4, "Released %s @ %i", "Texture", i);
		} else {
			Log(1, "%s Attempt To Release NULL Ptr", "Texture");
		}
	}

	if (Textures) {
		SYSTEM_FREE(Textures);
		Textures = nullptr;
	}
}
