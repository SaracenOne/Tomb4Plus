#pragma once
#include "../global/vars.h"

void inject_drawroom(bool replace);

void ProjectVerts(long nVerts, D3DTLVERTEX* v, short* clip);
void ProjectWaterVerts(long nVerts, D3DTLVERTEX* v, short* clip);
void ProjectShoreVerts(long nVerts, D3DTLVERTEX* v, short* clip);
void ProcessRoomData(ROOM_INFO* r);

#define InitBuckets	( (void(__cdecl*)()) 0x004728B0 )
#define CreateVertexNormals	( (void(__cdecl*)(ROOM_INFO*)) 0x00472F50 )
