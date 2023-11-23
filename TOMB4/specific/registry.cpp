#include "../tomb4/pch.h"
#include "registry.h"
#include "LoadSave.h"
#include "cmdline.h"
#include "input.h"
#include "winmain.h"


#ifdef USE_INI
#ifndef USE_SDL
#error "INI config support requires SDL!"
#endif

#include <SDL_filesystem.h>

#include <filesystem>
#include <algorithm>
#include <string>

#include "simpleIni.h"

CSimpleIniA ini;
const char *config_file_path = "";
const char *current_section = "";
bool section_just_created = false;

char *CFG_GetTomb4PlusPrefPath() {
	return SDL_GetPrefPath("", "Tomb4Plus");

}
#else
static HKEY phkResult;
static DWORD dwDisposition;
#endif

static bool REG_Setup;

#ifndef USE_INI
bool REG_OpenKey(const char *lpSubKey)
{
	return RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, (CHAR*)"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &phkResult, &dwDisposition) == ERROR_SUCCESS;
}
#endif

bool OpenRegistry(const char *section_key)
{
#ifdef USE_INI
	std::string data_path = CFG_GetTomb4PlusPrefPath();
	std::string config_file_path = data_path + "config.ini";

	ini.SetUnicode();

	SI_Error rc = ini.LoadFile(config_file_path.c_str());
	if (rc >= 0) {
		section_just_created = false;
		if (!ini.SectionExists(section_key)) {
			section_just_created = true;
		}
	}

	current_section = section_key;

	return true;
#else
	char buffer[256];

	if (!section_key)
		return REG_OpenKey("Software\\Core Design\\Tomb Raider IV");

	sprintf(buffer, "%s\\%s", "Software\\Core Design\\Tomb Raider IV", section_key);
	return REG_OpenKey(buffer);
#endif
}

#ifndef USE_INI
void REG_CloseKey()
{
	RegCloseKey(phkResult);
}
#endif

void CloseRegistry()
{
#ifdef USE_INI
	std::string data_path = CFG_GetTomb4PlusPrefPath();

	ini.SetUnicode();

	ini.SaveFile((data_path + std::string("config.ini")).c_str());
#else
	REG_CloseKey();
#endif
}

void REG_WriteLong(char* SubKeyName, ulong value)
{
#ifdef USE_INI
	ini.SetLongValue(current_section, SubKeyName, value);
#else
	RegSetValueEx(phkResult, SubKeyName, 0, REG_DWORD, (CONST BYTE*) & value, sizeof(ulong));
#endif
}

void REG_WriteBool(char* SubKeyName, bool value)
{
#ifdef USE_INI
	ini.SetBoolValue(current_section, SubKeyName, value);
#else
	ulong Lvalue;

	Lvalue = (ulong)value;
	RegSetValueEx(phkResult, SubKeyName, 0, REG_DWORD, (CONST BYTE*) & Lvalue, sizeof(ulong));
#endif
}

void REG_WriteString(char* SubKeyName, char* string, long length)
{
	long checkLength;

#ifdef USE_INI
	if (string)
	{
		ini.SetValue(current_section, SubKeyName, string);
	}
	else
	{
		ini.Delete(current_section, SubKeyName);
	}
#else
	if (string)
	{
		if (length < 0)
			checkLength = strlen(string);
		else
			checkLength = length;

		RegSetValueEx(phkResult, SubKeyName, 0, REG_SZ, (CONST BYTE*)string, checkLength + 1);
	}
	else
		RegDeleteValue(phkResult, SubKeyName);
#endif
}

void REG_WriteFloat(char* SubKeyName, float value)
{
#ifdef USE_INI
	ini.SetDoubleValue(current_section, SubKeyName, value);
#else
	long length;
	char buf[64];

	length = sprintf(buf, "%.5f", value);
	REG_WriteString(SubKeyName, buf, length);
#endif
}

bool REG_ReadLong(char* SubKeyName, ulong& value, ulong defaultValue)
{
#ifdef USE_INI
	value = ini.GetLongValue(current_section, SubKeyName, defaultValue);
	return true;
#else
	ulong type;
	ulong cbData;

	cbData = 4;

	if (RegQueryValueEx(phkResult, SubKeyName, 0, &type, (LPBYTE)&value, &cbData) == ERROR_SUCCESS && type == REG_DWORD && cbData == 4)
		return 1;

	REG_WriteLong(SubKeyName, defaultValue);
	value = defaultValue;
	return 0;
#endif
}

bool REG_ReadBool(char* SubKeyName, bool& value, bool defaultValue)
{
#ifdef USE_INI
	value = ini.GetBoolValue(current_section, SubKeyName, defaultValue);
	return true;
#else
	ulong type;
	ulong cbData;
	ulong data;

	cbData = 4;

	if (RegQueryValueEx(phkResult, SubKeyName, 0, &type, (LPBYTE)&data, &cbData) == ERROR_SUCCESS && type == REG_DWORD && cbData == 4)
	{
		value = (bool)data;
		return 1;
	}

	REG_WriteBool(SubKeyName, defaultValue);
	value = defaultValue;
	return 0;
#endif
}

bool REG_ReadString(char* SubKeyName, char* value, long length, char* defaultValue)
{
#ifdef USE_INI
	const char* loaded_string = ini.GetValue(current_section, SubKeyName, defaultValue);
	if (loaded_string) {
		long loaded_str_length = strlen(loaded_string);
		if (loaded_str_length <= length) {
			memcpy(value, loaded_string, loaded_str_length);
		}
	}

	return false;
#else
	ulong type;
	ulong cbData;
	long len;

	cbData = length;

	if (RegQueryValueEx(phkResult, SubKeyName, 0, &type, (LPBYTE)value, (LPDWORD)&cbData) == ERROR_SUCCESS && type == REG_SZ)
		return 1;

	if (defaultValue)
	{
		REG_WriteString(SubKeyName, defaultValue, -1);
		len = strlen(defaultValue) + 1;

		if (len > length)
		{
			len = length - 1;
			value[len] = 0;
		}

		memcpy(value, defaultValue, len);
	}
	else
		RegDeleteValue(phkResult, SubKeyName);

	return 0;
#endif
}

bool REG_ReadFloat(char* SubKeyName, float& value, float defaultValue)
{
#ifdef USE_INI
	value = ini.GetDoubleValue(current_section, SubKeyName, defaultValue);
	return true;
#else
	char buf[64];

	if (REG_ReadString(SubKeyName, buf, sizeof(buf), 0))
	{
		value = (float)atof(buf);
		return 1;
	}

	REG_WriteFloat(SubKeyName, defaultValue);
	value = defaultValue;
	return 0;
#endif
}

bool LoadSettings()
{
	ulong key;
	bool val;

	if (!OpenRegistry("System"))
		return 0;

	REG_ReadBool((char*)"Setup", REG_Setup, 0);

	if (REG_Setup)
	{
		App.TextureSize = 256;
		App.BumpMapSize = 256;
		App.StartFlags = DXF_FPUSETUP;

		REG_ReadLong((char*)"DD", (ulong&)App.DXInfo.nDD, 0);
		REG_ReadLong((char*)"D3D", (ulong&)App.DXInfo.nD3D, 0);
		REG_ReadLong((char*)"VMode", (ulong&)App.DXInfo.nDisplayMode, 0);
		REG_ReadLong((char*)"TFormat", (ulong&)App.DXInfo.nTexture, 0);
		REG_ReadLong((char*)"DS", (ulong&)App.DXInfo.nDS, 0);
		REG_ReadBool((char*)"BumpMap", App.BumpMapping, 1);
		REG_ReadBool((char*)"Filter", App.Filtering, 1);
		REG_ReadBool((char*)"DisableSound", App.SoundDisabled, 0);
		REG_ReadBool((char*)"Volumetric", App.Volumetric, 1);
		REG_ReadBool((char*)"NoFMV", fmvs_disabled, 0);

		REG_ReadBool((char*)"TextLow", val, 0);

		if (val)
			App.TextureSize = 128;

		REG_ReadBool((char*)"BumpLow", val, 0);

		if (val)
			App.BumpMapSize = 128;

		REG_ReadBool((char*)"HardWare", val, 0);

		if (val)
			App.StartFlags |= DXF_ZBUFFER | DXF_HWR;

		REG_ReadBool((char*)"Window", val, 0);

		if (val)
			App.StartFlags |= DXF_WINDOWED;
		else
			App.StartFlags |= DXF_FULLSCREEN;
	}

	CloseRegistry();

	OpenRegistry("Game");

	REG_ReadLong((char*)"Key0", key, layout[0][0]);
	layout[1][0] = (short)key;

	REG_ReadLong((char*)"Key1", key, layout[0][1]);
	layout[1][1] = (short)key;

	REG_ReadLong((char*)"Key2", key, layout[0][2]);
	layout[1][2] = (short)key;

	REG_ReadLong((char*)"Key3", key, layout[0][3]);
	layout[1][3] = (short)key;

	REG_ReadLong((char*)"Key4", key, layout[0][4]);
	layout[1][4] = (short)key;

	REG_ReadLong((char*)"Key5", key, layout[0][5]);
	layout[1][5] = (short)key;

	REG_ReadLong((char*)"Key6", key, layout[0][6]);
	layout[1][6] = (short)key;

	REG_ReadLong((char*)"Key7", key, layout[0][7]);
	layout[1][7] = (short)key;

	REG_ReadLong((char*)"Key8", key, layout[0][8]);
	layout[1][8] = (short)key;

	REG_ReadLong((char*)"Key9", key, layout[0][9]);
	layout[1][9] = (short)key;

	REG_ReadLong((char*)"Key10", key, layout[0][10]);
	layout[1][10] = (short)key;

	REG_ReadLong((char*)"Key11", key, layout[0][11]);
	layout[1][11] = (short)key;

	REG_ReadLong((char*)"Key12", key, layout[0][12]);
	layout[1][12] = (short)key;

	REG_ReadLong((char*)"Key13", key, layout[0][13]);
	layout[1][13] = (short)key;

	REG_ReadLong((char*)"Key14", key, layout[0][14]);
	layout[1][14] = (short)key;

	REG_ReadLong((char*)"Key15", key, layout[0][15]);
	layout[1][15] = (short)key;

	REG_ReadLong((char*)"Key16", key, layout[0][16]);
	layout[1][16] = (short)key;

	REG_ReadLong((char*)"Key17", key, layout[0][17]);
	layout[1][17] = (short)key;

	REG_ReadLong((char*)"MusicVolume", (ulong&)MusicVolume, 80);
	REG_ReadLong((char*)"SFXVolume", (ulong&)SFXVolume, 90);
	REG_ReadLong((char*)"ControlMethod", (ulong&)ControlMethod, 0);
	REG_ReadLong((char*)"SoundQuality", (ulong&)SoundQuality, 1);
	REG_ReadLong((char*)"AutoTarget", (ulong&)App.AutoTarget, 1);
	REG_ReadLong((char*)"WindowX", (ulong&)App.dx.rScreen, 0);
	REG_ReadLong((char*)"WindowY", (ulong&)App.dx.rScreen.top, 0);

	CloseRegistry();
	CheckKeyConflicts();
	return REG_Setup;
}

void SaveSettings()
{
	OpenRegistry("Game");
	REG_WriteLong((char*)"Key0", layout[1][0]);
	REG_WriteLong((char*)"Key1", layout[1][1]);
	REG_WriteLong((char*)"Key2", layout[1][2]);
	REG_WriteLong((char*)"Key3", layout[1][3]);
	REG_WriteLong((char*)"Key4", layout[1][4]);
	REG_WriteLong((char*)"Key5", layout[1][5]);
	REG_WriteLong((char*)"Key6", layout[1][6]);
	REG_WriteLong((char*)"Key7", layout[1][7]);
	REG_WriteLong((char*)"Key8", layout[1][8]);
	REG_WriteLong((char*)"Key9", layout[1][9]);
	REG_WriteLong((char*)"Key10", layout[1][10]);
	REG_WriteLong((char*)"Key11", layout[1][11]);
	REG_WriteLong((char*)"Key12", layout[1][12]);
	REG_WriteLong((char*)"Key13", layout[1][13]);
	REG_WriteLong((char*)"Key14", layout[1][14]);
	REG_WriteLong((char*)"Key15", layout[1][15]);
	REG_WriteLong((char*)"Key16", layout[1][16]);
	REG_WriteLong((char*)"Key17", layout[1][17]);
	REG_WriteLong((char*)"ControlMethod", ControlMethod);
	REG_WriteLong((char*)"MusicVolume", MusicVolume);
	REG_WriteLong((char*)"SFXVolume", SFXVolume);
	REG_WriteLong((char*)"SoundQuality", SoundQuality);
	REG_WriteLong((char*)"AutoTarget", App.AutoTarget);
	REG_WriteLong((char*)"WindowX", App.dx.rScreen.left);
	REG_WriteLong((char*)"WindowY", App.dx.rScreen.top);
	CloseRegistry();

	OpenRegistry("System");
	REG_WriteLong((char*)"VMode", App.DXInfo.nDisplayMode);
	REG_WriteBool((char*)"Window", (App.dx.Flags & DXF_WINDOWED) != 0);
	CloseRegistry();
}

bool SaveSetup(HWND hDlg)
{
	OpenRegistry("System");

	REG_WriteLong((char*)"DD", SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0));
	REG_WriteLong((char*)"D3D", SendMessage(GetDlgItem(hDlg, 1003), CB_GETCURSEL, 0, 0));
	REG_WriteLong((char*)"VMode", SendMessage(GetDlgItem(hDlg, 1004), CB_GETITEMDATA, SendMessage(GetDlgItem(hDlg, 1004), CB_GETCURSEL, 0, 0), 0));
	REG_WriteLong((char*)"DS", SendMessage(GetDlgItem(hDlg, 1005), CB_GETCURSEL, 0, 0));
	REG_WriteLong((char*)"TFormat", SendMessage(GetDlgItem(hDlg, 1006), CB_GETCURSEL, 0, 0));

	REG_WriteBool((char*)"Filter", SendMessage(GetDlgItem(hDlg, 1012), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"BumpMap", SendMessage(GetDlgItem(hDlg, 1016), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"HardWare", SendMessage(GetDlgItem(hDlg, 1010), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"DisableSound", SendMessage(GetDlgItem(hDlg, 1018), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"TextLow", SendMessage(GetDlgItem(hDlg, 1014), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"BumpLow", SendMessage(GetDlgItem(hDlg, 1015), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"Window", SendMessage(GetDlgItem(hDlg, 1025), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"Volumetric", SendMessage(GetDlgItem(hDlg, 1029), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"NoFMV", SendMessage(GetDlgItem(hDlg, 1030), BM_GETCHECK, 0, 0));
	REG_WriteBool((char*)"Setup", 1);

	CloseRegistry();
	return 1;
}

bool REG_KeyWasCreated()
{
#ifdef USE_INI
	return section_just_created;
#else
	return dwDisposition == REG_CREATED_NEW_KEY;
#endif
}

