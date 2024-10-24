#include "../tomb4/pch.h"
#include "cmdline.h"
#include "function_stubs.h"
#include "../game/gameflow.h"
#include "registry.h"
#include "winmain.h"
#include "platform.h"

char ASCIIToANSITable[7][2] =
{
	{'�', '�'},
	{'�', '�'},
	{'�', '�'},
	{'�', '�'},
	{'�', '�'},
	{'�', '�'},
	{'�', '�'}
};

#ifdef UNICODE
wchar_t wide_string_temp_buffer[128];
#endif

bool start_setup = false;
bool fmvs_disabled = false;

static LRESULT nDDDevice = 0;
static LRESULT nD3DDevice = 1;
static bool Filter = false;
static bool VolumetricFx = true;
static bool BumpMap = false;
static bool TextLow = false;

void CLSetup(char* cmd)
{
	GlobalLog("CLSetup");

	if (cmd)
		start_setup = 0;
	else
		start_setup = 1;
}

void CLNoFMV(char* cmd)
{
	GlobalLog("CLNoFMV");

	if (cmd)
		fmvs_disabled = 0;
	else
		fmvs_disabled = 1;
}

void CLPath(char* cmd)
{
	GlobalLog("CLPath");

	if (strcmp(cmd, "_INIT") == 0)
	{
		char cwd[1024];
		if (_getcwd(cwd, sizeof(cwd)) != NULL) {
			working_dir_path = cwd;
			working_dir_path += PATH_SEPARATOR;
		}
		else {
			working_dir_path = ".";
			working_dir_path += PATH_SEPARATOR;
		}
	}
	else
	{
		working_dir_path = cmd;
		if (platform_string_ends_with(working_dir_path.c_str(), PATH_SEPARATOR) == 0)
		{
			working_dir_path += PATH_SEPARATOR;
		}
	}
}

void InitDSDevice(HWND dlg, HWND hwnd)
{
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

	if (!App.DXInfo.nDSInfo)
	{
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

void InitTFormats(HWND dlg, HWND hwnd)
{
	char buffer[40];

#ifdef USE_BGFX
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
#else
	DXD3DDEVICE* device;
	DXTEXTUREINFO* tex;
	long bpp, r, g, b, a;
	bool software;

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	EnableWindow(GetDlgItem(dlg, 1006), 1);
	software = SendMessage(GetDlgItem(dlg, 1011), BM_GETCHECK, 0, 0);
	device = &App.DXInfo.DDInfo[nDDDevice].D3DDevices[nD3DDevice];

	for (int i = 0; i < device->nTextureInfos; i++)
	{
		tex = &device->TextureInfos[i];
		bpp = tex->bpp;
		r = tex->rbpp;
		g = tex->gbpp;
		b = tex->bbpp;
		a = tex->abpp;

		sprintf(buffer, "%d %s RGBA %d%d%d%d", bpp, GetFixedStringForTextID(TXT_Bit), r, g, b, a);

#ifdef UNICODE
		wchar_t wide_string[40];
		MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)wide_string);
#else
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)buffer);
#endif

		if (software)
		{
			if (bpp == 32 && r == 8 && b == 8 && g == 8 && a == 8)
			{
				SendMessage(hwnd, CB_SETCURSEL, i, 0);
				EnableWindow(GetDlgItem(dlg, 1006), 0);
			}
		}
		else if (bpp == 16 && r == 5 && b == 5 && g == 5 && a == 1)
			SendMessage(hwnd, CB_SETCURSEL, i, 0);
		else
			SendMessage(hwnd, CB_SETCURSEL, 0, 0);
	}
#endif
}

void InitResolution(HWND dlg, HWND hwnd, bool resetvms)
{
#ifdef USE_BGFX
	long bpp, w, h, n;
	char buffer[40];
	bool software;

	n = 0;

	SendMessage(GetDlgItem(dlg, 1010), BM_SETCHECK, 1, 0);
	SendMessage(GetDlgItem(dlg, 1011), BM_SETCHECK, 0, 0);
	EnableWindow(GetDlgItem(dlg, 1011), 0);
	software = SendMessage(GetDlgItem(dlg, 1011), BM_GETCHECK, 0, 0);

	if (resetvms)
	{
		SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

		int display_mode_count = SDL_GetNumDisplayModes(0);
		if (display_mode_count < 1) {
			platform_fatal_error("SDL_GetNumDisplayModes failed: %s", SDL_GetError());
			return;
		}

		SDL_DisplayMode previous_mode;
		previous_mode.w = -1;
		previous_mode.h = -1;

		for (int i = display_mode_count-1; i > 0; i--)
		{
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

#ifndef USE_SDL
	if (App.DXInfo.DDInfo[nDDDevice].DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
		EnableWindow(GetDlgItem(dlg, 1025), 1);
	else
	{
		EnableWindow(GetDlgItem(dlg, 1025), 0);
		SendMessage(GetDlgItem(dlg, 1025), BM_SETCHECK, 0, 0);
	}
#else
	EnableWindow(GetDlgItem(dlg, 1025), 1);
	SendMessage(GetDlgItem(dlg, 1025), BM_SETCHECK, 1, 0);
#endif

	if (software)
	{
		EnableWindow(GetDlgItem(dlg, 1029), 0);
		VolumetricFx = 0;
	}
	else
	{
		EnableWindow(GetDlgItem(dlg, 1029), 1);
	}

	SendMessage(GetDlgItem(dlg, 1029), BM_SETCHECK, VolumetricFx, 0);
	SendMessage(GetDlgItem(dlg, 1012), BM_SETCHECK, Filter, 0);

	if (software)
	{
		EnableWindow(GetDlgItem(dlg, 1016), 0);
		BumpMap = 0;
	}
	else
		EnableWindow(GetDlgItem(dlg, 1016), 1);

	SendMessage(GetDlgItem(dlg, 1016), BM_SETCHECK, BumpMap, 0);

	if (software)
	{
		EnableWindow(GetDlgItem(dlg, 1014), 0);
		TextLow = 0;
	}
	else
		EnableWindow(GetDlgItem(dlg, 1014), 1);

	SendMessage(GetDlgItem(dlg, 1014), BM_SETCHECK, TextLow, 0);

	if (TextLow)
	{
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 1, 0);
		EnableWindow(GetDlgItem(dlg, 1015), 0);
	}
	else
	{
		EnableWindow(GetDlgItem(dlg, 1015), 1);
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 0, 0);
	}

	if (!BumpMap)
	{
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 0, 0);
		EnableWindow(GetDlgItem(dlg, 1015), 0);
	}

	if (resetvms)
		InitTFormats(dlg, GetDlgItem(dlg, 1006));
#else
	DXD3DDEVICE* device;
	DXDISPLAYMODE* dm;
	long bpp, w, h, n;
	char buffer[40];
	bool software;

	n = 0;

	SendMessage(GetDlgItem(dlg, 1010), BM_SETCHECK, 1, 0);
	SendMessage(GetDlgItem(dlg, 1011), BM_SETCHECK, 0, 0);
	EnableWindow(GetDlgItem(dlg, 1011), 0);
	software = SendMessage(GetDlgItem(dlg, 1011), BM_GETCHECK, 0, 0);

	if (resetvms)
	{
		SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
		device = &App.DXInfo.DDInfo[nDDDevice].D3DDevices[nD3DDevice];

		for (int i = 0; i < device->nDisplayModes; i++)
		{
			dm = &device->DisplayModes[i];
			w = dm->w;
			h = dm->h;
			bpp = dm->bpp;

			if (bpp > 8)
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
		}
	}

#ifndef USE_SDL
	if (App.DXInfo.DDInfo[nDDDevice].DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
		EnableWindow(GetDlgItem(dlg, 1025), 1);
	else
	{
		EnableWindow(GetDlgItem(dlg, 1025), 0);
		SendMessage(GetDlgItem(dlg, 1025), BM_SETCHECK, 0, 0);
	}
#else
	EnableWindow(GetDlgItem(dlg, 1025), 0);
	SendMessage(GetDlgItem(dlg, 1025), BM_SETCHECK, 1, 0);
#endif

	if (software)
	{
		EnableWindow(GetDlgItem(dlg, 1029), 0);
		VolumetricFx = 0;
	}
	else
		EnableWindow(GetDlgItem(dlg, 1029), 1);

	SendMessage(GetDlgItem(dlg, 1029), BM_SETCHECK, VolumetricFx, 0);

	if (software)
	{
		EnableWindow(GetDlgItem(dlg, 1016), 0);
		BumpMap = 0;
	}
	else
		EnableWindow(GetDlgItem(dlg, 1016), 1);

	SendMessage(GetDlgItem(dlg, 1016), BM_SETCHECK, BumpMap, 0);

	if (software)
	{
		EnableWindow(GetDlgItem(dlg, 1014), 0);
		TextLow = 0;
	}
	else
		EnableWindow(GetDlgItem(dlg, 1014), 1);

	SendMessage(GetDlgItem(dlg, 1014), BM_SETCHECK, TextLow, 0);

	if (TextLow)
	{
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 1, 0);
		EnableWindow(GetDlgItem(dlg, 1015), 0);
	}
	else
	{
		EnableWindow(GetDlgItem(dlg, 1015), 1);
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 0, 0);
	}

	if (!BumpMap)
	{
		SendMessage(GetDlgItem(dlg, 1015), BM_SETCHECK, 0, 0);
		EnableWindow(GetDlgItem(dlg, 1015), 0);
	}

	if (resetvms)
		InitTFormats(dlg, GetDlgItem(dlg, 1006));
#endif
}

void InitD3DDevice(HWND dlg, HWND hwnd)
{
#ifdef USE_BGFX
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
#else
	if (App.DXInfo.nDDInfo <= 0) {
		platform_fatal_error("Not DirectDraw infos found.");
		return;
	}

	if (nDDDevice >= App.DXInfo.nDDInfo) {
		nDDDevice = App.DXInfo.nDDInfo - 1;
	} else if (nDDDevice < 0) {
		nDDDevice = 0;
	}

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	DXDIRECTDRAWINFO *ddraw = &App.DXInfo.DDInfo[nDDDevice];

	// Tomb4Plus - skip first device (software emulation) since we have no support for it.
	for (int i = 1; i < ddraw->nD3DDevices; i++) {
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)ddraw->D3DDevices[i].About);
	}
	SendMessage(hwnd, CB_SETCURSEL, 0, 0);
	//

	nD3DDevice = 1;
	InitResolution(dlg, GetDlgItem(dlg, 1004), 1);
#endif
}

void InitDDDevice(HWND dlg, HWND hwnd)
{
#ifdef USE_BGFX
	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

#ifdef UNICODE
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"BGFX");
#else
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"BGFX");
#endif

	SendMessage(hwnd, CB_SETCURSEL, 0, 0);

	InitD3DDevice(dlg, GetDlgItem(dlg, 1003));
#else
	DDDEVICEIDENTIFIER* id;
	char buffer[256];

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

	for (int i = 0; i < App.DXInfo.nDDInfo; i++)
	{
		id = &App.DXInfo.DDInfo[i].DDIdentifier;
		sprintf(buffer, "%s - %s (%d.%d.%02d.%04d)", id->szDescription, id->szDriver,
			(id->liDriverVersion.HighPart >> 16) & 0xFFFF, id->liDriverVersion.HighPart & 0xFFFF,
			(id->liDriverVersion.LowPart >> 16) & 0xFFFF, id->liDriverVersion.LowPart & 0xFFFF);
		
#ifdef UNICODE
		wchar_t wide_string[40];
		MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wide_string, sizeof(wide_string) / sizeof(wchar_t));
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)wide_string);
#else
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)buffer);
#endif
	}

	nDDDevice = App.DXInfo.nDDInfo - 1;
	SendMessage(hwnd, CB_SETCURSEL, nDDDevice, 0);
	InitD3DDevice(dlg, GetDlgItem(dlg, 1003));
#endif
}

LPARAM ConvertToWin32DialogString(char* s)
{
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

char* MapASCIIToANSI(char* s, char* d)
{
	char* p;
	size_t l;
	char c;
	bool found;

	l = strlen(s);
	p = d;

	for (int i = 0; i < l; i++)
	{
		c = *s++;

		if (c >= 0x80)
		{
			found = 0;

			for (int i = 0; i < 7; i++)
			{
				if (c == ASCIIToANSITable[i][0])
				{
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

BOOL CALLBACK DXSetupDlgProc(HWND dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HFONT hfont = 0;
	char d[256];

	if (message == WM_INITDIALOG)
	{
		GlobalLog("WM_INITDIALOG");

		if (Gameflow->Language == JAPAN)
		{
			hfont = (HFONT)GetStockObject(SYSTEM_FONT);
			SendMessage(GetDlgItem(dlg, 1000), WM_SETFONT, 0, (LPARAM)hfont);
			SendMessage(GetDlgItem(dlg, 1003), WM_SETFONT, 0, (LPARAM)hfont);
			SendMessage(GetDlgItem(dlg, 1004), WM_SETFONT, 0, (LPARAM)hfont);
			SendMessage(GetDlgItem(dlg, 1006), WM_SETFONT, 0, (LPARAM)hfont);
			SendMessage(GetDlgItem(dlg, 1005), WM_SETFONT, 0, (LPARAM)hfont);
		}

		SendMessage(GetDlgItem(dlg, 1001), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Graphics_Adapter), d)));
		SendMessage(GetDlgItem(dlg, 1002), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Output_Settings), d)));
		SendMessage(GetDlgItem(dlg, 1), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_OK), d)));
		SendMessage(GetDlgItem(dlg, 2), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Cancel), d)));
		SendMessage(GetDlgItem(dlg, 1009), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Output_Resolution), d)));
		SendMessage(GetDlgItem(dlg, 1012), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Bilinear_Filtering), d)));
		SendMessage(GetDlgItem(dlg, 1016), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Bump_Mapping), d)));
		SendMessage(GetDlgItem(dlg, 1010), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Hardware_Acceleration), d)));
		SendMessage(GetDlgItem(dlg, 1011), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Software_Mode), d)));
		SendMessage(GetDlgItem(dlg, 1017), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Sound_Device), d)));
		SendMessage(GetDlgItem(dlg, 1018), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Disable), d)));
		SendMessage(GetDlgItem(dlg, 1014), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Low_Resolution_Textures), d)));
		SendMessage(GetDlgItem(dlg, 1015), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Low_Resolution_Bump_Maps), d)));
		SendMessage(GetDlgItem(dlg, 1013), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Texture_Bit_Depth), d)));
		SendMessage(GetDlgItem(dlg, 1025), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Windowed), d)));
		SendMessage(GetDlgItem(dlg, 1023), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Render_Options), d)));
		SendMessage(GetDlgItem(dlg, 1029), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_Volumetric_FX), d)));
		SendMessage(GetDlgItem(dlg, 1030), WM_SETTEXT, 0, ConvertToWin32DialogString(MapASCIIToANSI(GetFixedStringForTextID(TXT_No_FMV), d)));
		InitDDDevice(dlg, GetDlgItem(dlg, 1000));
		InitDSDevice(dlg, GetDlgItem(dlg, 1005));
		return 1;
	}

	if (message == WM_COMMAND)
	{
		switch ((ushort)wParam)
		{
		case IDOK:

			if (hfont)
				DeleteObject(hfont);

			SaveSetup(dlg);
			EndDialog(dlg, 1);
			return 1;

		case IDCANCEL:

			if (hfont)
				DeleteObject(hfont);

			EndDialog(dlg, 0);
			return 1;

		case 1000:

			if (((wParam >> 16) & 0xFFFF) == CBN_SELCHANGE)
			{
				nDDDevice = SendMessage(GetDlgItem(dlg, 1000), CB_GETCURSEL, 0, 0);
				InitD3DDevice(dlg, GetDlgItem(dlg, 1003));
			}

			break;

		case 1003:

			if (((wParam >> 16) & 0xFFFF) == CBN_SELCHANGE)
			{
				nD3DDevice = SendMessage(GetDlgItem(dlg, 1003), CB_GETCURSEL, 0, 0);
				InitResolution(dlg, GetDlgItem(dlg, 1004), 1);
			}

			break;

		case 1010:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				nD3DDevice = 1;
				SendMessage(GetDlgItem(dlg, 1003), CB_SETCURSEL, 1, 0);
				InitResolution(dlg, GetDlgItem(dlg, 1004), 1);
			}

			break;

		case 1011:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				nD3DDevice = 1;
				SendMessage(GetDlgItem(dlg, 1003), CB_SETCURSEL, 0, 0);
				InitResolution(dlg, GetDlgItem(dlg, 1004), 1);
			}

			break;

		case 1012:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				if (SendMessage(GetDlgItem(dlg, 1012), BM_GETCHECK, 0, 0))
					Filter = 1;
				else
					Filter = 0;

				InitResolution(dlg, GetDlgItem(dlg, 1004), 0);
			}

			break;

		case 1014:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				if (SendMessage(GetDlgItem(dlg, 1014), BM_GETCHECK, 0, 0))
					TextLow = 1;
				else
					TextLow = 0;

				InitResolution(dlg, GetDlgItem(dlg, 1004), 0);
			}

			break;

		case 1016:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				if (SendMessage(GetDlgItem(dlg, 1016), BM_GETCHECK, 0, 0))
					BumpMap = 1;
				else
					BumpMap = 0;

				InitResolution(dlg, GetDlgItem(dlg, 1004), 0);
			}

			break;

		case 1018:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				if (SendMessage(GetDlgItem(dlg, (ushort)wParam), BM_GETCHECK, 0, 0))
					EnableWindow(GetDlgItem(dlg, 1005), 0);
				else
					EnableWindow(GetDlgItem(dlg, 1005), 1);
			}

			break;

		case 1029:

			if (((wParam >> 16) & 0xFFFF) == BN_CLICKED)
			{
				if (SendMessage(GetDlgItem(dlg, 1029), BM_GETCHECK, 0, 0))
					VolumetricFx = 1;
				else
					VolumetricFx = 0;

				InitResolution(dlg, GetDlgItem(dlg, 1004), 0);
			}

			break;
		}
	}

	return 0;
}

bool DXSetupDialog()
{
	INT_PTR ret;

	ShowCursor(1);
	ret = DialogBox(App.hInstance, MAKEINTRESOURCE(109), 0, (DLGPROC)DXSetupDlgProc);
	ShowCursor(0);

	if (ret == -1)
	{
		platform_message_box("Unable To Initialise Dialog");
		return 0;
	}

	if (!ret)
		return 0;

	return 1;
}
