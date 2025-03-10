#pragma once
#include <string>
#include "../global/types.h"

void CLSetup(char* cmd);
void CLNoFMV(char* cmd);
void CLPath(char* cmd);

#ifdef _WIN32
void InitDSDevice(HWND dlg, HWND hwnd);
void InitTFormats(HWND dlg, HWND hwnd);
void InitResolution(HWND dlg, HWND hwnd, bool resetvms);
void InitD3DDevice(HWND dlg, HWND hwnd);
void InitDDDevice(HWND dlg, HWND hwnd);
#endif

char* MapASCIIToANSI(char* s, char* d);

extern bool fmvs_disabled;