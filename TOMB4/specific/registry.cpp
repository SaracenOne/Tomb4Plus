#include "../tomb4/pch.h"
#include "registry.h"
#include "LoadSave.h"
#include "cmdline.h"
#include "input.h"
#include "winmain.h"
#include <string>

#include <SDL_filesystem.h>

#include <filesystem>
#include <algorithm>

#include "SimpleIni.h"
#include "platform.h"

CSimpleIniA ini;

std::string config_file_path = "";

const char *current_section = "";
bool section_just_created = false;

static bool REG_Setup;

bool OpenRegistry(const char *section_key) {
	config_file_path = platform_get_userdata_path() + "config.ini";

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
}

void CloseRegistry() {
	std::string config_path = platform_get_userdata_path();

	ini.SetUnicode();

	ini.SaveFile((config_path + std::string("config.ini")).c_str());
}

void REG_WriteLong(char* SubKeyName, uint32_t value) {
	ini.SetLongValue(current_section, SubKeyName, value);
}

void REG_WriteBool(char* SubKeyName, bool value) {
	ini.SetBoolValue(current_section, SubKeyName, value);
}

void REG_WriteString(char* SubKeyName, char* string, int32_t length) {
	if (string) {
		ini.SetValue(current_section, SubKeyName, string);
	} else {
		ini.Delete(current_section, SubKeyName);
	}
}

void REG_WriteFloat(char* SubKeyName, float value) {
	ini.SetDoubleValue(current_section, SubKeyName, value);
}

bool REG_ReadLong(char* SubKeyName, uint32_t& value, uint32_t defaultValue) {
	value = ini.GetLongValue(current_section, SubKeyName, defaultValue);
	return true;
}

bool REG_ReadBool(char* SubKeyName, bool& value, bool defaultValue) {
	value = ini.GetBoolValue(current_section, SubKeyName, defaultValue);
	return true;
}

bool REG_ReadString(char* SubKeyName, char* value, int32_t length, char* defaultValue) {
	const char* loaded_string = ini.GetValue(current_section, SubKeyName, defaultValue);
	if (loaded_string) {
		size_t loaded_str_length = strlen(loaded_string);
		if (loaded_str_length <= size_t(length)) {
			memcpy(value, loaded_string, loaded_str_length);
		}
	}

	return false;
}

bool REG_ReadFloat(char* SubKeyName, float& value, float defaultValue) {
	value = (float)ini.GetDoubleValue(current_section, SubKeyName, defaultValue);
	return true;
}

bool LoadSettings() {
	uint32_t key;
	bool val;

	if (!OpenRegistry("System"))
		return false;

	App.TextureSize = 256;
	App.BumpMapSize = 256;
	App.StartFlags = DXF_FPUSETUP;

	REG_ReadLong((char*)"VideoWidth", (uint32_t&)App.VideoWidth, WINDOW_DEFAULT_WIDTH);
	REG_ReadLong((char*)"VideoHeight", (uint32_t&)App.VideoHeight, WINDOW_DEFAULT_HEIGHT);
	REG_ReadLong((char*)"DS", (uint32_t&)App.DXInfo.nDS, 0);
	REG_ReadBool((char*)"BumpMap", App.BumpMapping, false);
	REG_ReadBool((char*)"Filter", App.Filtering, false);
	REG_ReadBool((char*)"DisableSound", App.SoundDisabled, false);
	REG_ReadBool((char*)"Volumetric", App.Volumetric, true);
	REG_ReadBool((char*)"NoFMV", fmvs_disabled, false);

	REG_ReadBool((char*)"TextLow", val, false);

	if (val) {
		App.TextureSize = 128;
	}

	REG_ReadBool((char*)"BumpLow", val, false);

	if (val)
		App.BumpMapSize = 128;

	REG_ReadBool((char*)"HardWare", val, false);

	if (val)
		App.StartFlags |= DXF_ZBUFFER | DXF_HWR;

	REG_ReadBool((char*)"Window", val, true);

	if (val) {
		App.StartFlags |= DXF_WINDOWED;
	} else {
		App.StartFlags |= DXF_FULLSCREEN;
	}

	CloseRegistry();

	OpenRegistry("Game");

	REG_ReadLong((char*)"Key0", key, keyboard_layout[0][0]);
	keyboard_layout[1][0] = (int16_t)key;

	REG_ReadLong((char*)"Key1", key, keyboard_layout[0][1]);
	keyboard_layout[1][1] = (int16_t)key;

	REG_ReadLong((char*)"Key2", key, keyboard_layout[0][2]);
	keyboard_layout[1][2] = (int16_t)key;

	REG_ReadLong((char*)"Key3", key, keyboard_layout[0][3]);
	keyboard_layout[1][3] = (int16_t)key;

	REG_ReadLong((char*)"Key4", key, keyboard_layout[0][4]);
	keyboard_layout[1][4] = (int16_t)key;

	REG_ReadLong((char*)"Key5", key, keyboard_layout[0][5]);
	keyboard_layout[1][5] = (int16_t)key;

	REG_ReadLong((char*)"Key6", key, keyboard_layout[0][6]);
	keyboard_layout[1][6] = (int16_t)key;

	REG_ReadLong((char*)"Key7", key, keyboard_layout[0][7]);
	keyboard_layout[1][7] = (int16_t)key;

	REG_ReadLong((char*)"Key8", key, keyboard_layout[0][8]);
	keyboard_layout[1][8] = (int16_t)key;

	REG_ReadLong((char*)"Key9", key, keyboard_layout[0][9]);
	keyboard_layout[1][9] = (int16_t)key;

	REG_ReadLong((char*)"Key10", key, keyboard_layout[0][10]);
	keyboard_layout[1][10] = (int16_t)key;

	REG_ReadLong((char*)"Key11", key, keyboard_layout[0][11]);
	keyboard_layout[1][11] = (int16_t)key;

	REG_ReadLong((char*)"Key12", key, keyboard_layout[0][12]);
	keyboard_layout[1][12] = (int16_t)key;

	REG_ReadLong((char*)"Key13", key, keyboard_layout[0][13]);
	keyboard_layout[1][13] = (int16_t)key;

	REG_ReadLong((char*)"Key14", key, keyboard_layout[0][14]);
	keyboard_layout[1][14] = (int16_t)key;

	REG_ReadLong((char*)"Key15", key, keyboard_layout[0][15]);
	keyboard_layout[1][15] = (int16_t)key;

	REG_ReadLong((char*)"Key16", key, keyboard_layout[0][16]);
	keyboard_layout[1][16] = (int16_t)key;

	REG_ReadLong((char*)"Key17", key, keyboard_layout[0][17]);
	keyboard_layout[1][17] = (int16_t)key;

	REG_ReadBool((char*)"UseGamepad", use_gamepad, true);

	REG_ReadLong((char*)"MusicVolume", (uint32_t&)MusicVolume, 80);
	REG_ReadLong((char*)"SFXVolume", (uint32_t&)SFXVolume, 90);
	REG_ReadLong((char*)"ControlMethod", (uint32_t&)ControlMethod, 0);
	REG_ReadLong((char*)"SoundQuality", (uint32_t&)SoundQuality, 1);
	REG_ReadLong((char*)"AutoTarget", (uint32_t&)App.AutoTarget, 1);

	CloseRegistry();
	CheckKeyConflicts();
	return REG_Setup;
}

void SaveSettings() {
	OpenRegistry("Game");
	REG_WriteLong((char*)"Key0", keyboard_layout[1][0]);
	REG_WriteLong((char*)"Key1", keyboard_layout[1][1]);
	REG_WriteLong((char*)"Key2", keyboard_layout[1][2]);
	REG_WriteLong((char*)"Key3", keyboard_layout[1][3]);
	REG_WriteLong((char*)"Key4", keyboard_layout[1][4]);
	REG_WriteLong((char*)"Key5", keyboard_layout[1][5]);
	REG_WriteLong((char*)"Key6", keyboard_layout[1][6]);
	REG_WriteLong((char*)"Key7", keyboard_layout[1][7]);
	REG_WriteLong((char*)"Key8", keyboard_layout[1][8]);
	REG_WriteLong((char*)"Key9", keyboard_layout[1][9]);
	REG_WriteLong((char*)"Key10", keyboard_layout[1][10]);
	REG_WriteLong((char*)"Key11", keyboard_layout[1][11]);
	REG_WriteLong((char*)"Key12", keyboard_layout[1][12]);
	REG_WriteLong((char*)"Key13", keyboard_layout[1][13]);
	REG_WriteLong((char*)"Key14", keyboard_layout[1][14]);
	REG_WriteLong((char*)"Key15", keyboard_layout[1][15]);
	REG_WriteLong((char*)"Key16", keyboard_layout[1][16]);
	REG_WriteLong((char*)"Key17", keyboard_layout[1][17]);
	REG_WriteBool((char*)"UseGamepad", use_gamepad);
	REG_WriteLong((char*)"ControlMethod", ControlMethod);
	REG_WriteLong((char*)"MusicVolume", MusicVolume);
	REG_WriteLong((char*)"SFXVolume", SFXVolume);
	REG_WriteLong((char*)"SoundQuality", SoundQuality);
	REG_WriteLong((char*)"AutoTarget", App.AutoTarget);
	CloseRegistry();

	OpenRegistry("System");
	REG_WriteLong((char*)"VideoWidth", App.VideoWidth);
	REG_WriteLong((char*)"VideoHeight", App.VideoHeight);
	REG_WriteBool((char*)"Window", (App.dx.Flags & DXF_WINDOWED) != 0);
	REG_WriteBool((char*)"BumpMap", App.BumpMapping);
	REG_WriteBool((char*)"Filter", App.Filtering);
	CloseRegistry();
}

bool REG_KeyWasCreated() {
	return section_just_created;
}

