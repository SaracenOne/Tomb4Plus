#pragma once
#include "../../specific/platform.h"
#include "../../global/types.h"

#define MAX_MIRRORS 128

enum T4PlusMirrorDirection {
	T4_MIR_PLANE_X,
	T4_MIR_PLANE_Y,
	T4_MIR_PLANE_Z
};

struct T4PlusMirrorInfo {
	uint32_t mirror_room;
	int pivot_point;
	T4PlusMirrorDirection direction;
};

extern int t4p_mirror_count;
extern T4PlusMirrorInfo t4p_mirror_info[MAX_MIRRORS];

extern void T4PResetMirrors();
extern void T4PInsertMirror(int mirror_room, int pivot_point, T4PlusMirrorDirection direction);
extern PHD_VECTOR T4PMirrorVectorOnPlane(T4PlusMirrorInfo* mirror_info, PHD_VECTOR vec);
extern PHD_3DPOS T4PMirrorUnrotated3DPosOnPlane(T4PlusMirrorInfo* mirror_info, PHD_3DPOS pos);
extern PHD_3DPOS T4PMirrorRotated3DPosOnPlane(T4PlusMirrorInfo* mirror_info, PHD_3DPOS pos);
extern PHD_3DPOS T4PMirrorInverted3DPosOnPlane(T4PlusMirrorInfo* mirror_info, PHD_3DPOS pos);