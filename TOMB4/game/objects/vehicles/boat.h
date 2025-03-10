#pragma once
#include "../../../global/types.h"

#define RUBBER_BOAT_TOP_SPEED	110
#define RUBBER_BOAT_SLOW_SPEED	(RUBBER_BOAT_TOP_SPEED / 3)
#define RUBBER_BOAT_FAST_SPEED	(RUBBER_BOAT_TOP_SPEED + 75)

#define MOTOR_BOAT_TOP_SPEED	90
#define MOTOR_BOAT_SLOW_SPEED	(MOTOR_BOAT_TOP_SPEED / 3)
#define MOTOR_BOAT_FAST_SPEED	(MOTOR_BOAT_TOP_SPEED + 75)

#define BOAT_OCB_HEADLIGHT (1 << 0)
#define BOAT_OCB_SKIP_HEAVY_TRIGGERS (1 << 1)
#define BOAT_OCB_SKIP_REGULAR_TRIGGERS (1 << 2)
#define BOAT_OCB_LOOK_AROUND (1 << 3)
#define BOAT_OCB_NO_FUEL (1 << 4)
#define BOAT_OCB_FUEL_MANAGEMENT (1 << 5)
#define BOAT_OCB_ANCHORED (1 << 6)
#define BOAT_OCB_SHOW_FUEL_BAR (1 << 7)

struct BOAT_INFO {
	int32_t boat_turn;
	int32_t left_fallspeed;
	int32_t right_fallspeed;
	int16_t tilt_angle;
	int16_t extra_rotation;
	int32_t water;
	int32_t pitch;
	int16_t prop_rot;
	int16_t light_intensity;
};

extern void InitialiseBoat(int16_t item_num);
extern void BoatCollision(int16_t item_num, ITEM_INFO *l, COLL_INFO *coll);
extern void DrawBoat(ITEM_INFO *item);

extern void RubberBoatControl(int16_t item_num);
extern void MotorBoatControl(int16_t item_num);
