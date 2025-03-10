#pragma once
#include "../global/types.h"

void DXBitMask2ShiftCnt(uint32_t mask, uint8_t* shift, uint8_t* count);
const Uint8 *SDLReadKeyboard(const Uint8* KeyMap);
void* AddStruct(void* p, int32_t num, int32_t size);

#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
#ifdef UNICODE
BOOL __stdcall DXEnumDirectSound(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext);
#else
BOOL __stdcall DXEnumDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
#endif
#endif

#ifdef _WIN32
int32_t DXGetInfo(DXINFO* dxinfo, HWND hwnd);
#else
int32_t DXGetInfo(DXINFO* dxinfo);
#endif

void DXFreeInfo(DXINFO* dxinfo);
int32_t BPPToDDBD(int32_t BPP);

#ifdef _WIN32
int32_t DXCreate(int32_t w, int32_t h, int32_t bpp, int32_t Flags, DXPTR* dxptr, HWND hWnd, int32_t WindowStyle);
#else
int32_t DXCreate(int32_t w, int32_t h, int32_t bpp, int32_t Flags, DXPTR* dxptr);
#endif

extern DXPTR* G_dxptr;
extern DXINFO* G_dxinfo;

extern int keymap_count;
extern const Uint8 *keymap;
extern SDL_GameController *controller;
extern const char* controller_name;
extern SDL_GameControllerType controller_type;