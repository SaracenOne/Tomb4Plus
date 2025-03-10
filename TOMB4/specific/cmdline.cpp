#include "../tomb4/pch.h"
#include "cmdline.h"
#include "function_stubs.h"
#include "../game/gameflow.h"
#include "registry.h"
#include "winmain.h"
#include "platform.h"

uint8_t ASCIIToANSITable[7][2] = {
	{0x82, 0xE9},  // '‚' -> 'é'
	{0x8A, 0xE8},  // 'Š' -> 'è'
	{0x88, 0xEA},  // 'ˆ' -> 'ê'
	{0x94, 0xF6},  // '”' -> 'ö'
	{0x85, 0xE0},  // '…' -> 'à'
	{0xA0, 0xE1},  // ' ' -> 'á'
	{0xA2, 0xF3}   // '¢' -> 'ó'
};

#ifdef UNICODE
wchar_t wide_string_temp_buffer[128];
#endif

bool fmvs_disabled = false;

#ifdef _WIN32
static LRESULT nDDDevice = 0;
static LRESULT nD3DDevice = 1;
#endif

static bool Filter = false;
static bool VolumetricFx = true;
static bool BumpMap = false;
static bool TextLow = false;


void CLNoFMV(char* cmd) {
	GlobalLog("CLNoFMV");

	if (cmd)
		fmvs_disabled = 0;
	else
		fmvs_disabled = 1;
}

void CLPath(char* cmd) {
	GlobalLog("CLPath");

	if (strcmp(cmd, "_INIT") == 0) {
		char cwd[1024];
		if (TR_GETCWD(cwd, sizeof(cwd)) != NULL) {
			working_dir_path = cwd;
			working_dir_path += PATH_SEPARATOR;
		} else {
			working_dir_path = ".";
			working_dir_path += PATH_SEPARATOR;
		}
	} else {
		working_dir_path = cmd;
		if (platform_string_ends_with(working_dir_path.c_str(), PATH_SEPARATOR) == 0) {
			working_dir_path += PATH_SEPARATOR;
		}
	}
}

#ifdef _WIN32
void InitDSDevice(HWND dlg, HWND hwnd) {
	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

	for (int i = 0; i < App.DXInfo.nDSInfo; i++) {
#ifdef UNICODE
		wchar_t wide_string[80];
		MultiByteToWideChar(CP_UTF8, 0, App.DXInfo.DSInfo[i].About, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)wide_string);
#else
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)App.DXInfo.DSInfo[i].About);
#endif
	}

	if (!App.DXInfo.nDSInfo) {
#ifdef UNICODE
		wchar_t wide_string[80];
		MultiByteToWideChar(CP_UTF8, 0, GetFixedStringForTextID(TXT_No_Sound_Card_Installed), -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)wide_string);
#else
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)GetFixedStringForTextID(TXT_No_Sound_Card_Installed));
#endif
		EnableWindow(GetDlgItem(dlg, 1018), 0);
		SendMessage(GetDlgItem(dlg, 1018), BM_SETCHECK, 1, 0);
		EnableWindow(hwnd, 0);
	}

	SendMessage(hwnd, CB_SETCURSEL, 0, 0);
}

void InitTFormats(HWND dlg, HWND hwnd) {
	char buffer[40];

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	EnableWindow(GetDlgItem(dlg, 1006), 1);
	sprintf(buffer, "%d %s RGBA %d%d%d%d", 32, GetFixedStringForTextID(TXT_Bit), 8, 8, 8, 8);

#ifdef UNICODE
	wchar_t wide_string[40];
	MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)wide_string);
#else
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)buffer);
#endif
	SendMessage(hwnd, CB_SETCURSEL, 0, 0);
}

void InitResolution(HWND dlg, HWND hwnd, bool resetvms) {
	int32_t bpp, w, h, n;
	char buffer[40];
	bool software;

	n = 0;

	SendMessage(GetDlgItem(dlg, 1010), BM_SETCHECK, 1, 0);
	SendMessage(GetDlgItem(dlg, 1011), BM_SETCHECK, 0, 0);
	EnableWindow(GetDlgItem(dlg, 1011), 0);
	software = SendMessage(GetDlgItem(dlg, 1011), BM_GETCHECK, 0, 0);

	if (resetvms) {
		SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

		int display_mode_count = SDL_GetNumDisplayModes(0);
		if (display_mode_count < 1) {
			platform_fatal_error("SDL_GetNumDisplayModes failed: %s", SDL_GetError());
			return;
		}

		SDL_DisplayMode previous_mode;
		previous_mode.w = -1;
		previous_mode.h = -1;

		for (int i = display_mode_count-1; i > 0; i--) {
			SDL_DisplayMode mode;
			if (SDL_GetDisplayMode(0, i, &mode) != 0) {
				platform_fatal_error("SDL_GetDisplayMode failed: %s", SDL_GetError());
				return;
			}

			if (mode.w == previous_mode.w && mode.h == previous_mode.h) {
				continue;
			}

			w = mode.w;
			h = mode.h;
			bpp = 32;

			{
				sprintf(buffer, "%dx%d %d %s", w, h, bpp, GetFixedStringForTextID(TXT_Bit));
#ifdef UNICODE
				wchar_t wide_string[40];
				MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
				SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)wide_string);
#else
				SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)buffer);
#endif
				SendMessage(hwnd, CB_SETITEMDATA, n, i);

				if (w == 640 && h == 480)
					SendMessage(hwnd, CB_SETCURSEL, n, 0);

				n++;
			}
			previous_mode = mode;
		}
	}

	EnableWindow(GetDlgItem(dlg, 1025), 1);
	SendMessage(GetDlgItem(dlg, 1025), BM_SETCHECK, 1, 0);

	if (software) {
		EnableWindow(GetDlgItem(dlg, 1029), 0);
		VolumetricFx = 0;
	} else {
		EnableWindow(GetDlgItem(dlg, 1029), 1);
	}

	SendMessage(GetDlgItem(dlg, 1029), BM_SETCHECK, VolumetricFx, 0);
	SendMessage(GetDlgItem(dlg, 1012), BM_SETCHECK, Filter, 0);

	if (software) {
		EnableWindow(GetDlgItem(dlg, 1016), 0);
		BumpMap = 0;
	} else
		EnableWindow(GetDlgItem(dlg, 1016), 1);

	SendMessage(GetDlgItem(dlg, 1016), BM_SETCHECK, BumpMap, 0);

	if (software) {
		EnableWindow(GetDlgItem(dlg, 1014), 0);
		TextLow = 0;
	} else
		EnableWindow(GetDlgItem(dlg, 1014), 1);

	SendMessage(GetDlgItem(dlg, 1014), BM_SETCHECK, TextLow, 0);

	if (TextLow) {
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 1, 0);
		EnableWindow(GetDlgItem(dlg, 1015), 0);
	} else {
		EnableWindow(GetDlgItem(dlg, 1015), 1);
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 0, 0);
	}

	if (!BumpMap) {
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 0, 0);
		EnableWindow(GetDlgItem(dlg, 1015), 0);
	}

	if (resetvms)
		InitTFormats(dlg, GetDlgItem(dlg, 1006));

}

void InitD3DDevice(HWND dlg, HWND hwnd) {
	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	// Tomb4Plus - skip first device (software emulation) since we have no support for it.

#ifdef UNICODE
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"OpenGL");
#else
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"OpenGL");
#endif

	SendMessage(hwnd, CB_SETCURSEL, 0, 0);
	//

	nD3DDevice = 1;
	InitResolution(dlg, GetDlgItem(dlg, 1004), 1);
}

void InitDDDevice(HWND dlg, HWND hwnd) {
	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

#ifdef UNICODE
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"BGFX");
#else
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"BGFX");
#endif

	SendMessage(hwnd, CB_SETCURSEL, 0, 0);

	InitD3DDevice(dlg, GetDlgItem(dlg, 1003));
}

LPARAM ConvertToWin32DialogString(char* s) {
#ifdef UNICODE
	size_t l = strlen(s);
	if (l >= 128) {
		platform_fatal_error("ConvertToWin32DialogString: Invalid String Length.");
	}
	MultiByteToWideChar(CP_UTF8, 0, s, -1, wide_string_temp_buffer, sizeof(wide_string_temp_buffer) / sizeof(wchar_t));

	return (LPARAM)wide_string_temp_buffer;
#else
	return (LPARAM)s;
#endif
}
#endif

char* MapASCIIToANSI(char* s, char* d) {
	char* p;
	size_t l;
	int8_t c;
	bool found;

	l = strlen(s);
	p = d;

	for (int i = 0; i < l; i++) {
		c = *s++;

		if (c >= 0x80) {
			found = 0;

			for (int i = 0; i < 7; i++) {
				if (c == ASCIIToANSITable[i][0]) {
					c = ASCIIToANSITable[i][1];
					found = 1;
					break;
				}
			}

			if (!found)
				GlobalLog("Reqd : %x", c);
		}

		*d++ = c;
	}

	*d = 0;
	return p;
}