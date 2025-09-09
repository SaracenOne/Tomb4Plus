#include "../../tomb4/pch.h"

#include "t4plus_mirror.h"

int32_t t4p_mirror_count = 0;
T4PlusMirrorInfo t4p_mirror_info[MAX_MIRRORS];

void T4PResetMirrors() {
	t4p_mirror_count = 0;
}

void T4PInsertMirror(int32_t mirror_room, int32_t pivot_point, T4PlusMirrorDirection direction) {
	if (t4p_mirror_count >= MAX_MIRRORS) {
		platform_fatal_error("MAX_MIRRORS overflow.");
		return;
	}

	t4p_mirror_info[t4p_mirror_count].mirror_room = mirror_room;
	t4p_mirror_info[t4p_mirror_count].pivot_point = pivot_point;
	t4p_mirror_info[t4p_mirror_count].direction = direction;

	t4p_mirror_count++;
}

PHD_VECTOR T4PMirrorVectorOnPlane(T4PlusMirrorInfo* mirror_info, PHD_VECTOR vec) {
	PHD_VECTOR new_vec = vec;

	switch (mirror_info->direction) {
		case T4_MIR_PLANE_X:
			new_vec.x = (mirror_info->pivot_point << 1) - vec.x;
			break;
		case T4_MIR_PLANE_Y:
			new_vec.y = (mirror_info->pivot_point << 1) - vec.y;
			break;
		case T4_MIR_PLANE_Z:
			new_vec.z = (mirror_info->pivot_point << 1) - vec.z;
			break;
	}

	return new_vec;
}

PHD_3DPOS T4PMirrorUnrotated3DPosOnPlane(T4PlusMirrorInfo* mirror_info, PHD_3DPOS pos) {
	PHD_3DPOS new_pos = pos;

	switch (mirror_info->direction) {
		case T4_MIR_PLANE_X:
			new_pos.x_pos = (mirror_info->pivot_point << 1) - pos.x_pos;
			break;
		case T4_MIR_PLANE_Y:
			new_pos.y_pos = (mirror_info->pivot_point << 1) - pos.y_pos;
			break;
		case T4_MIR_PLANE_Z:
			new_pos.z_pos = (mirror_info->pivot_point << 1) - pos.z_pos;
			break;
	}

	return new_pos;
}

PHD_3DPOS T4PMirrorRotated3DPosOnPlane(T4PlusMirrorInfo* mirror_info, PHD_3DPOS pos) {
	PHD_3DPOS new_pos = pos;

	switch (mirror_info->direction) {
		case T4_MIR_PLANE_X:
			new_pos.x_pos = (mirror_info->pivot_point << 1) - pos.x_pos;
			new_pos.y_rot += 0x8000;
			break;
		case T4_MIR_PLANE_Y:
			new_pos.y_pos = (mirror_info->pivot_point << 1) - pos.y_pos;
			new_pos.x_rot += 0x8000;
			new_pos.z_rot += 0x8000;
			break;
		case T4_MIR_PLANE_Z:
			new_pos.z_pos = (mirror_info->pivot_point << 1) - pos.z_pos;
			new_pos.y_rot += 0x8000;
			break;
	}

	return new_pos;
}

PHD_3DPOS T4PMirrorInverted3DPosOnPlane(T4PlusMirrorInfo* mirror_info, PHD_3DPOS pos) {
	PHD_3DPOS new_pos = pos;

	switch (mirror_info->direction) {
		case T4_MIR_PLANE_X:
			new_pos.x_pos = (mirror_info->pivot_point << 1) - pos.x_pos;
			new_pos.x_rot += 0x8000;
			new_pos.y_rot = -pos.y_rot;
			new_pos.z_rot = -pos.z_rot;
			break;
		case T4_MIR_PLANE_Y:
			new_pos.y_pos = (mirror_info->pivot_point << 1) - pos.y_pos;
			new_pos.z_rot = -pos.z_rot;
			new_pos.y_rot += 0x8000;
			new_pos.x_rot = -pos.x_rot;
			break;
		case T4_MIR_PLANE_Z:
			new_pos.z_pos = (mirror_info->pivot_point << 1) - pos.z_pos;
			new_pos.x_rot = -pos.x_rot;
			new_pos.y_rot = -pos.y_rot;
			new_pos.z_rot += 0x8000;
			break;
	}


	return new_pos;
}