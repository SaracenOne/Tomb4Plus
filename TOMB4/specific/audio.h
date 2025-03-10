#pragma once
#include "../global/types.h"

void ACMSetVolume();
bool ACMInit();
void ACMClose();
void S_CDPlay(int32_t track, int32_t mode);
void S_CDPlayExt(uint8_t track_id, uint8_t channel_id, bool looping, bool restore_old_track);
int32_t S_CDGetTrackID(uint8_t channel_id);
bool S_CDGetChannelIsActive(uint8_t channel_id);
bool S_CDGetChannelIsLooping(uint8_t channel_id);
uint64_t S_CDGetChannelPosition(uint8_t channel_id);
void S_CDStop();
void S_CDStopExt(uint8_t channel_id);
void S_StartSyncedAudio(int32_t track);

void S_CDSetChannelVolume(uint8_t volume, uint8_t channel_id);

void S_AudioUpdate();
void S_PauseAudio();
void S_UnpauseAudio();

void S_Reset();

void S_CDSeek(int channel_id, int64_t frame);

void SetUsingNewAudioSystem(bool enabled);
void SetUsingOldTriggerMode(bool enabled);

bool IsUsingNewAudioSystem();
bool IsUsingOldCDTriggerMode();

#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
extern HACMDRIVER hACMDriver;
#endif

extern uint8_t* wav_file_buffer;
extern uint8_t* ADPCMBuffer;
extern bool acm_ready;

// Used for Von Croy cutscenes
extern int32_t LegacyTrack;
extern int32_t LegacyTrackFlag;
