#pragma once
#include "../global/types.h"

#define MAX_VOICES 32

// TRLE - bumped from 0x40000 to 0x100000 (4x)
#define DECOMPRESS_BUFFER_LEN (0x100000)
// TRLE - bumped from 256 to 1024
#define MAX_SAMPLE_BUFFERS 1024

bool DXChangeOutputFormat(long nSamplesPerSec, bool force);
void DSChangeVolume(long num, long volume);
void DSAdjustPitch(long num, long pitch);
void DSAdjustPan(long num, long pan);
bool DXSetOutputFormat();
bool DXDSCreate();
bool InitSampleDecompress();
bool FreeSampleDecompress();
bool DXCreateSample(char* data, long size, int samples_per_second, long num);
bool DXCreateSampleADPCM(char* data, long comp_size, long uncomp_size, long num);
void DXStopSample(long num);
bool DSIsChannelPlaying(long num);
long DSGetFreeChannel();
bool DSIsSamplePlaying(long sample_id);
long DXStartSample(long num, long volume, long pitch, long pan, ulong flags);
long CalcVolume(long volume);
void S_SoundStopAllSamples();
void S_SoundStopSample(long num);
long S_SoundPlaySample(long num, ushort volume, long pitch, short pan);
long S_SoundPlaySampleLooped(long num, ushort volume, long pitch, short pan);
void DXFreeSounds();
long S_SoundSampleIsPlayingOnChannel(long num);
void S_SoundSetPanAndVolume(long num, short pan, ushort volume);
void S_SoundSetPitch(long num, long pitch);
void S_SetReverbType(long reverb);
void S_SoundPauseSamples();
void S_SoundUnpauseSamples();
void DXDSClose();

extern char* samples_buffer;
