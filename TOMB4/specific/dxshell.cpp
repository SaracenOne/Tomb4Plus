#include "../tomb4/pch.h"
#include "dxshell.h"
#include "function_stubs.h"
#include "winmain.h"
#include "cmdline.h"

#include <SDL.h>

DXPTR* G_dxptr;
DXINFO* G_dxinfo;

int32_t keymap_count = 0;
const Uint8 *keymap;
SDL_GameController *controller = nullptr;
const char* controller_name = nullptr;
SDL_GameControllerType controller_type = SDL_CONTROLLER_TYPE_UNKNOWN;

const Uint8 *SDLReadKeyboard(const Uint8* KeyMap) {
	SDL_PumpEvents();

	const Uint8* sdl_keymap = SDL_GetKeyboardState(&keymap_count);

	KeyMap = sdl_keymap;

	return KeyMap;
}

void* AddStruct(void* p, int32_t num, int32_t size) {
	void* ptr;

	if (!num)
		ptr = SYSTEM_MALLOC(size);
	else
		ptr = SYSTEM_REALLOC(p, size * (num + 1));

	if (((int8_t*)ptr + size * num)) {
		memset((int8_t*)ptr + size * num, 0, size);
	}
	return ptr;
}

#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
#ifdef UNICODE
BOOL __stdcall DXEnumDirectSound(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext) {
#else
BOOL __stdcall DXEnumDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext) {
#endif
	DXINFO* dxinfo;
	DXDIRECTSOUNDINFO* DSInfo;
	int32_t nDSInfo;

	Log(2, "DXEnumDirectSound");
	dxinfo = (DXINFO*)lpContext;
	nDSInfo = dxinfo->nDSInfo;
	dxinfo->DSInfo = (DXDIRECTSOUNDINFO*)AddStruct(dxinfo->DSInfo, nDSInfo, sizeof(DXDIRECTSOUNDINFO));
	DSInfo = &dxinfo->DSInfo[nDSInfo];

	if (lpGuid) {
		DSInfo->lpGuid = &DSInfo->Guid;
		DSInfo->Guid = *lpGuid;
	} else
		DSInfo->lpGuid = 0;

#ifdef UNICODE
	wchar_t wide_string[260];
	MultiByteToWideChar(CP_UTF8, 0, DSInfo->About, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
	lstrcpy(wide_string, lpcstrDescription);
	MultiByteToWideChar(CP_UTF8, 0, DSInfo->Name, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
	lstrcpy(wide_string, lpcstrModule);
#else
	lstrcpy(DSInfo->About, lpcstrDescription);
	lstrcpy(DSInfo->Name, lpcstrModule);
#endif
	Log(5, "Found - %s %s", DSInfo->About, DSInfo->Name);
	dxinfo->nDSInfo++;
	return DDENUMRET_OK;
}
#endif

#ifdef _WIN32
int32_t DXGetInfo(DXINFO* dxinfo, HWND hwnd) {
#else
int32_t DXGetInfo(DXINFO* dxinfo) {
#endif
	Log(2, "DXInitialise");
	Log(5, "Enumerating DirectDraw Devices");
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)	// Dummy information
	dxinfo->nDSInfo = 1;
	dxinfo->DSInfo = (DXDIRECTSOUNDINFO*)SYSTEM_MALLOC(sizeof(DXDIRECTSOUNDINFO));
	if (dxinfo->DSInfo) {
		const char *MiniAudioString = "MiniAudio Device";
		memcpy(dxinfo->DSInfo[0].Name, MiniAudioString, strlen(MiniAudioString) + 1);
		memcpy(dxinfo->DSInfo[0].About, MiniAudioString, strlen(MiniAudioString) + 1);
	}
#else
	DXAttempt(DirectSoundEnumerate(DXEnumDirectSound, dxinfo));
#endif
	G_dxinfo = dxinfo;
	return 1;
}

void DXFreeInfo(DXINFO* dxinfo) {
}

#ifdef _WIN32
int32_t DXCreate(int32_t w, int32_t h, int32_t bpp, int32_t Flags, DXPTR* dxptr, HWND hWnd, int32_t WindowStyle) {
#else
int32_t DXCreate(int32_t w, int32_t h, int32_t bpp, int32_t Flags, DXPTR* dxptr) {
#endif
	int32_t flag;

	flag = 0;
	Log(2, "DXCreate");
	G_dxptr = dxptr;
	G_dxptr->Flags = Flags;
#if _WIN32
	G_dxptr->hWnd = hWnd;
	G_dxptr->WindowStyle = WindowStyle;
#endif

	if (Flags & 64)
		flag = 1;

	G_dxptr->dwRenderWidth = w;
	G_dxptr->dwRenderHeight = h;

	return 1;
}

int32_t DXChangeVideoMode() {
	// TODO: Put BGFX code here...
	return 1;
}

int32_t DXToggleFullScreen() {
	// TODO: Put BGFX code here...
	return 1;
}