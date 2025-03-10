#pragma once
#include "../global/types.h"

// TRLE - bumped from 8 to 64
#define CAMERA_COUNT 64
#define MAXIMUM_SPOTCAMS CAMERA_COUNT * 4

void SetSplineData(int32_t num, int32_t cam);
void InitialiseSpotCam(int16_t Sequence);
void InitSpotCamSequences();
int32_t Spline(int32_t x, int32_t* knots, int32_t nk);
void CalculateSpotCams();
void SpotcamResetFOV();

extern SPOTCAM SpotCam[];
extern int32_t bTrackCamInit;
extern int16_t LastSequence;
extern int16_t CurrentFov;
extern int16_t number_spotcams;
extern int32_t bUseSpotCam;
extern int32_t bDisableLaraControl;

enum spotcam_flags {
	SP_SNAPCAMERA = (1 << 0),	//snaps the camera to the first spline
	SP_VIGNETTE = (1 << 1),	//?
	SP_LOOPCAMERA = (1 << 2),	//loops the sequence (if past last camera -> go back to first)
	SP_TRACKCAMERA = (1 << 3),	//?
	SP_NODRAWLARA = (1 << 4),	//disable drawing lara
	SP_TARGETLARA = (1 << 5),	//overrides the camera target to lara's position
	SP_SNAPBACK = (1 << 6),	//?
	SP_JUMPTO = (1 << 7),	//cuts to another camera in the same sequence
	SP_HOLD = (1 << 8),	//stops moving, or, "holds" the camera for a given time, the "timer" field
	SP_NOBREAK = (1 << 9),	//don't allow breaking the sequence with the look button
	SP_NOLARACONTROL = (1 << 10),	//disable lara control
	SP_ENABLELARACONTROL = (1 << 11),	//enable lara control
	SP_FADEINSCREEN = (1 << 12),	//do screen fadein
	SP_FADEOUTSCREEN = (1 << 13),	//do screen fadeout
	SP_TESTTRIGGER = (1 << 14),	//test heavy triggers
	SP_FLYBYONESHOT = (1 << 15)	//used in TestTriggers to force the flyby trigger to be one shot
};
