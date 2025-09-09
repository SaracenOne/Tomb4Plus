#pragma once
#include "../global/types.h"

#define MAX_VOICES 32

// TRLE - bumped from 0x40000 to 0x100000 (4x)
#define DECOMPRESS_BUFFER_LEN (0x100000)
// TRLE - bumped from 256 to 1024
#define MAX_SAMPLE_BUFFERS 1024

bool DXChangeOutputFormat(int32_t nSamplesPerSec, bool force);
void DSChangeVolume(int32_t num, int32_t volume);
void DSAdjustPitch(int32_t num, int32_t pitch);
void DSAdjustPan(int32_t num, int32_t pan);
bool DXSetOutputFormat();
bool DXDSCreate();
bool InitSampleDecompress();
bool FreeSampleDecompress();
bool DXCreateSample(char* data, int32_t size, int32_t samples_per_second, int32_t num);
bool DXCreateSampleADPCM(char* data, int32_t comp_size, int32_t uncomp_size, int32_t num);
void DXStopSample(int32_t num);
bool DSIsChannelPlaying(int32_t num);
int32_t DSGetFreeChannel();
bool DSIsSamplePlaying(int32_t sample_id);
int32_t DXStartSample(int32_t num, int32_t volume, int32_t pitch, int32_t pan, uint32_t flags);
int32_t CalcVolume(int32_t volume);
void S_SoundStopAllSamples();
void S_SoundStopSample(int32_t num);
int32_t S_SoundPlaySample(int32_t num, uint16_t volume, int32_t pitch, int16_t pan);
int32_t S_SoundPlaySampleLooped(int32_t num, uint16_t volume, int32_t pitch, int16_t pan);
void DXFreeSounds();
int32_t S_SoundSampleIsPlayingOnChannel(int32_t num);
void S_SoundSetPanAndVolume(int32_t num, int16_t pan, uint16_t volume);
void S_SoundSetPitch(int32_t num, int32_t pitch);
void S_SetReverbType(int32_t reverb);
void S_SoundPauseSamples();
void S_SoundUnpauseSamples();
void DXDSClose();

extern char* samples_buffer;
