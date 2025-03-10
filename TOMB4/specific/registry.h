#pragma once
#include "../global/types.h"

bool OpenRegistry(const char *SubKeyName);
void REG_CloseKey();
void CloseRegistry();
void REG_WriteLong(char* SubKeyName, uint32_t value);
void REG_WriteBool(char* SubKeyName, bool value);
void REG_WriteString(char* SubKeyName, char* string, int32_t length);
void REG_WriteFloat(char* SubKeyName, float value);
bool REG_ReadLong(char* SubKeyName, uint32_t& value, uint32_t defaultValue);
bool REG_ReadBool(char* SubKeyName, bool& value, bool defaultValue);
bool REG_ReadString(char* SubKeyName, char* value, int32_t length, char* defaultValue);
bool REG_ReadFloat(char* SubKeyName, float& value, float defaultValue);
void LoadDefaultSettings();
bool LoadSettings();
void SaveSettings();
#ifdef _WIN32
bool SaveSetup(HWND hDlg);
#endif
bool REG_KeyWasCreated();
