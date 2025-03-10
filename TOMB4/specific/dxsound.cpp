#include "../tomb4/pch.h"
#include "dxsound.h"
#include "dxshell.h"
#include "function_stubs.h"
#include "audio.h"
#include "../game/sound.h"
#include "LoadSave.h"
#include "winmain.h"
#include "../tomb4/tomb4.h"

#pragma warning(push)
#pragma warning(disable : 4838)
#pragma warning(disable : 4309)
static int8_t source_pcm_format[50] = {
	2, 0, 1, 0, 34, 86, 0, 0, (int8_t)147, 43, 0, 0, 0, 2, 4, 0, 32, 0, (int8_t)244, 3, 7, 0, 0, 1, 0, 0, 0, 2, 0, (int8_t)255, 0, 0,
	0, 0, (int8_t)192, 0, 64, 0, (int8_t)240, 0, 0, 0, (int8_t)204, 1, 48, (int8_t)255, (int8_t)136, 1, 24, (int8_t)255
};
#pragma warning(pop)

#ifdef MA_AUDIO_ENGINE
#define STB_VORBIS_HEADER_ONLY // <-- Exclude stb_vorbis' implementation
#include "extras/stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#endif
#include "../tomb4/mod_config.h"
#include "../game/gameflow.h"

char* samples_buffer;
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
ma_engine ma_samples_engine;
static ma_sound ma_voices[MAX_VOICES];
static int ma_voice_active[MAX_VOICES];
static ma_audio_buffer ma_voice_buffers[MAX_VOICES];
static ma_audio_buffer *ma_sample_buffers[MAX_SAMPLE_BUFFERS];
#else
static LPDIRECTSOUNDBUFFER DSPrimary;
static IXAudio2MasteringVoice* XAMaster;
static IUnknown* XAEffect;
static IXAudio2SourceVoice* XA_Voices[MAX_VOICES];
static int XA_Voice_Active[MAX_VOICES];
static float current_voice_samples_per_second = 0.0F;
static XAUDIO2_BUFFER XA_Buffers[MAX_SAMPLE_BUFFERS];
static float XA_SPS[MAX_SAMPLE_BUFFERS];
static MMRESULT mmresult;
static WAVEFORMATEX pcm_format;
static HACMSTREAM hACMStream;
static ACMSTREAMHEADER ACMStreamHeader;
static char* decompressed_samples_buffer;

static XAUDIO2FX_REVERB_I3DL2_PARAMETERS reverb_preset[4] = {
	{50.0F,-1000, -500, 0.0F, 2.31F, 0.64F, -711, 0.012F, -800, 0.017F, 100.0F, 100.0F, 5000.0F}, // Small Room
	{50.0F,-1000, -500, 0.0F, 2.31F, 0.64F, -711, 0.012F, -300, 0.017F, 100.0F, 100.0F, 5000.0F}, // Medium Room
	{50.0F,-1000, -500, 0.0F, 2.31F, 0.64F, -711, 0.012F, 200, 0.017F, 100.0F, 100.0F, 5000.0F}, // Large Room
	{50.0F,-1000, -500, 0.0F, 2.31F, 0.64F, -711, 0.012F, 700, 0.017F, 100.0F, 100.0F, 5000.0F} // Pipe
};
static XAUDIO2FX_REVERB_PARAMETERS reverb_type[4];

static int32_t current_reverb = -1;
#endif

bool DXChangeOutputFormat(int32_t nSamplesPerSec, bool force) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	if (!force && ma_samples_engine.sampleRate == nSamplesPerSec)
		return true;

	S_SoundStopAllSamples();

	ma_engine_config engineConfig = ma_engine_config_init();
	engineConfig.channels = 2;
	engineConfig.sampleRate = nSamplesPerSec;

	ma_engine_uninit(&ma_samples_engine);
	ma_result result = ma_engine_init(&engineConfig, &ma_samples_engine);
	if (result != MA_SUCCESS) {
		return false;
	}

	return true;
#elif _WIN32
	WAVEFORMATEX pcfxFormat = {};
	static int32_t lastSPC;

	if (!force && lastSPC == nSamplesPerSec)
		return 1;

	lastSPC = nSamplesPerSec;
	pcfxFormat.wFormatTag = WAVE_FORMAT_PCM;
	pcfxFormat.nChannels = 2;
	pcfxFormat.nSamplesPerSec = nSamplesPerSec;
	pcfxFormat.nAvgBytesPerSec = 4 * nSamplesPerSec;
	pcfxFormat.nBlockAlign = 4;
	pcfxFormat.wBitsPerSample = 16;
	pcfxFormat.cbSize = 0;
	S_SoundStopAllSamples();

	if (DSPrimary && DXAttempt(DSPrimary->SetFormat(&pcfxFormat)) != DS_OK) {
		Log(1, "Can't set sound output format to %d", pcfxFormat.nSamplesPerSec);
		return 0;
	}

	return 1;
#endif
}

void DSChangeVolume(int32_t num, int32_t volume) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	float miniaudio_volume = ma_volume_db_to_linear(volume / 100.0f);
	miniaudio_volume = ((miniaudio_volume * (10.0f * (miniaudio_volume * 10.0f))) / 10.0f) * 0.1f;

	ma_sound_set_volume(&ma_voices[num], miniaudio_volume);
#elif _WIN32
	float fvolume;

	if (XA_Voices[num]) {
		fvolume = XAudio2DecibelsToAmplitudeRatio(volume / 100.0F);
		XA_Voices[num]->SetChannelVolumes(1, &fvolume, XAUDIO2_COMMIT_NOW);
	}
#endif
}

void DSAdjustPitch(int32_t num, int32_t pitch) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	uint32_t frequency = uint32_t((float)pitch / 65536.0F * 22050.0F);

	if (frequency < 100)
		frequency = 100;
	else if (frequency > 100000)
		frequency = 100000;

	ma_sound_set_pitch(&ma_voices[num], (float)frequency / 22050.0f);
#elif _WIN32
	uint32_t frequency;

	if (XA_Voices[num]) {
		frequency = uint32_t((float)pitch / 65536.0F * current_voice_samples_per_second);

		if (frequency < 100)
			frequency = 100;
		else if (frequency > 100000)
			frequency = 100000;

		XA_Voices[num]->SetFrequencyRatio(frequency / 22050.0F, XAUDIO2_COMMIT_NOW);
	}
#endif
}

void DSAdjustPan(int32_t num, int32_t pan) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	if (pan < 0) {
		if (pan < -0x4000)
			pan = -0x4000 - pan;
	} else if (pan > 0 && pan > 0x4000) {
		pan = 0x8000 - pan;
	}

	pan >>= 4;

	ma_sound_set_pan_mode(&ma_voices[num], ma_pan_mode_pan);
	ma_sound_set_pan(&ma_voices[num], float(pan) / 10000.0f);
#elif _WIN32
	float matrix[2] = {0.0f, 0.0f};

	if (XA_Voices[num]) {
		if (pan < 0) {
			if (pan < -0x4000)
				pan = -0x4000 - pan;
		} else if (pan > 0 && pan > 0x4000)
			pan = 0x8000 - pan;

		pan >>= 4;

		if (!pan) {
			matrix[0] = 1.0F;
			matrix[1] = 1.0F;
		} else if (pan < 0) {
			matrix[0] = 1.0F;
			matrix[1] = XAudio2DecibelsToAmplitudeRatio(pan / 100.0F);
		} else {
			matrix[0] = XAudio2DecibelsToAmplitudeRatio(-pan / 100.0F);
			matrix[1] = 1.0F;
		}

		XA_Voices[num]->SetOutputMatrix(0, 1, 2, matrix, XAUDIO2_COMMIT_NOW);
	}
#endif
}

bool DXSetOutputFormat() {
#ifdef MA_AUDIO_SAMPLES
	return true;
#elif _WIN32
	DSBUFFERDESC desc;

	Log(2, "DXSetOutputFormat");
	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

	if (DXAttempt(App.dx.lpDS->CreateSoundBuffer(&desc, &DSPrimary, 0)) == DS_OK) {
		DXChangeOutputFormat(sfx_frequencies[SoundQuality], 0);
		DSPrimary->Play(0, 0, DSBPLAY_LOOPING);
		return 1;
	}

	Log(1, "Can't Get Primary Sound Buffer");
	return 0;
#endif
}

bool DXDSCreate() {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	Log(2, "DXDSCreate");

	ma_engine_config engineConfig = ma_engine_config_init();
	engineConfig.channels = 2;
	engineConfig.sampleRate = sfx_frequencies[SoundQuality];

	ma_result result = ma_engine_init(&engineConfig, &ma_samples_engine);
	if (result != MA_SUCCESS) {
		return false;
	}

	for (int i = 0; i < MAX_SAMPLE_BUFFERS; i++) {
		if (ma_sample_buffers[i]) {
			ma_sample_buffers[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_VOICES; i++) {
		ma_voice_active[i] = -1;
	}

	sound_active = true;
	return true;
#elif _WIN32
	XAUDIO2_EFFECT_DESCRIPTOR chaind = {};
	XAUDIO2_EFFECT_CHAIN chain = {};

	pcm_format.wFormatTag = WAVE_FORMAT_PCM;
	pcm_format.cbSize = 0;
	pcm_format.nChannels = 1;
	pcm_format.nAvgBytesPerSec = 44100;
	pcm_format.nSamplesPerSec = 22050;
	pcm_format.nBlockAlign = 2;
	pcm_format.wBitsPerSample = 16;
	DXAttempt(DirectSoundCreate8(G_dxinfo->DSInfo[G_dxinfo->nDS].lpGuid, &App.dx.lpDS, 0));
	DXAttempt(App.dx.lpDS->SetCooperativeLevel(App.hWnd, DSSCL_EXCLUSIVE));
	DXSetOutputFormat();
	DXAttempt(XAudio2Create(&App.dx.lpXA, 0, XAUDIO2_DEFAULT_PROCESSOR));
	DXAttempt(XAudio2CreateReverb(&XAEffect, 0));
	chaind.pEffect = XAEffect;
	chaind.InitialState = TRUE;
	chaind.OutputChannels = 2;
	chain.EffectCount = 1;
	chain.pEffectDescriptors = &chaind;
	DXAttempt(App.dx.lpXA->CreateMasteringVoice(&XAMaster, 2, 44100, 0, 0, &chain, AudioCategory_GameEffects));

	for (int i = 0; i < MAX_VOICES; i++)
		DXAttempt(App.dx.lpXA->CreateSourceVoice(&XA_Voices[i], &pcm_format, 0, XAUDIO2_MAX_FREQ_RATIO, 0, 0, 0));

	for (int i = 0; i < 4; i++)
		ReverbConvertI3DL2ToNative(&reverb_preset[i], &reverb_type[i], FALSE);

	for (int i = 0; i < MAX_VOICES; i++) {
		XA_Voice_Active[i] = -1;
	}

	sound_active = true;
	return true;
#endif
}

bool InitSampleDecompress() {
#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
	mmresult = acmStreamOpen(&hACMStream, hACMDriver, (LPWAVEFORMATEX)source_pcm_format, &pcm_format, 0, 0, 0, 0);

	if (mmresult != DS_OK)
		Log(1, "Stream Open %d", mmresult);

	decompressed_samples_buffer = (int8_t*)SYSTEM_MALLOC(DECOMPRESS_BUFFER_LEN);
	samples_buffer = (char*)SYSTEM_MALLOC(DECOMPRESS_BUFFER_LEN + 0x5A);
	memset(&ACMStreamHeader, 0, sizeof(ACMStreamHeader));
	ACMStreamHeader.pbSrc = (uint8_t*)(samples_buffer + 0x5A);
	ACMStreamHeader.cbStruct = 84;
	ACMStreamHeader.cbSrcLength = DECOMPRESS_BUFFER_LEN;
	ACMStreamHeader.cbDstLength = DECOMPRESS_BUFFER_LEN;
	ACMStreamHeader.pbDst = (uint8_t*)decompressed_samples_buffer;
	mmresult = acmStreamPrepareHeader(hACMStream, &ACMStreamHeader, 0);

	if (mmresult != DS_OK)
		Log(1, "Prepare Stream %d", mmresult);
#else
	samples_buffer = (char*)SYSTEM_MALLOC(DECOMPRESS_BUFFER_LEN + 0x5A);
#endif

	return 1;
}

bool FreeSampleDecompress() {
#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
	ACMStreamHeader.cbSrcLength = DECOMPRESS_BUFFER_LEN;
	mmresult = acmStreamUnprepareHeader(hACMStream, &ACMStreamHeader, 0);

	if (mmresult != DS_OK)
		Log(1, "UnPrepare Stream %d", mmresult);

	mmresult = acmStreamClose(hACMStream, 0);

	if (mmresult != DS_OK)
		Log(1, "Stream Close %d", mmresult);

	SYSTEM_FREE(decompressed_samples_buffer);
	SYSTEM_FREE(samples_buffer);
#else
	SYSTEM_FREE(samples_buffer);
#endif

	return 1;
}

bool DXCreateSample(char* data, int32_t size, int samples_per_second, int32_t num) {
	Log(8, "DXCreateSample - %u", num);

#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	MOD_LEVEL_AUDIO_INFO *mod_audio_info = get_game_mod_level_audio_info(gfCurrentLevel);

	ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
	        ma_format_s16,
	        1,
	        size / 2,
	        data,
	        NULL);
	bufferConfig.sampleRate = mod_audio_info->sample_rate;

	ma_result result = ma_audio_buffer_alloc_and_init(&bufferConfig, &ma_sample_buffers[num]);
	if (result != MA_SUCCESS) {
		return false;
	}

	return true;
#elif _WIN32
	if (!App.dx.lpDS)
		return 0;

	if (samples_per_second != 22050)
		Log(1, "Incorrect SamplesPerSec");

	XA_Buffers[num].pAudioData = (BYTE*)SYSTEM_MALLOC(size);

	XA_SPS[num] = (float)get_game_mod_level_audio_info(gfCurrentLevel)->sample_rate;
	if (XA_Buffers[num].pAudioData) {
		memcpy((void*)XA_Buffers[num].pAudioData, data, size);
		XA_Buffers[num].AudioBytes = size;

		return true;
	}

	return false;
#endif
}

bool DXCreateSampleADPCM(char* data, int32_t comp_size, int32_t uncomp_size, int32_t num) {
	Log(8, "DXCreateSampleADPCM");

	bool result = false;

	// Load the ADPCM data into a buffer
	ma_uint8* pPCMData = nullptr;
	ma_uint8* pADPCMData = (ma_uint8*)SYSTEM_MALLOC(comp_size);
	ma_decoder decoder;

	if (pADPCMData) {
		memcpy(pADPCMData, data, comp_size);

		// Create a decoder
		ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 1, 22050);
		if (ma_decoder_init_memory(pADPCMData, comp_size, &config, &decoder) == MA_SUCCESS) {
			// Decode the ADPCM data into PCM
			ma_uint8* pPCMData = (ma_uint8*)SYSTEM_MALLOC(uncomp_size);
			ma_uint64 frameCount;
			ma_result decoder_result = ma_decoder_read_pcm_frames(&decoder, pPCMData, uncomp_size / ma_get_bytes_per_frame(config.format, config.channels), &frameCount);

			if (decoder_result == MA_SUCCESS) {
#ifdef MA_AUDIO_SAMPLES
				// Now you can use the PCM data with MiniAudio
				ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
				        ma_format_s16,
				        1,
				        frameCount,
				        pPCMData,
				        NULL);
				bufferConfig.sampleRate = 22050;

				ma_result audio_buffer_alloc_result = ma_audio_buffer_alloc_and_init(&bufferConfig, &ma_sample_buffers[num]);
				if (audio_buffer_alloc_result == MA_SUCCESS) {
					result = true;
				}
#elif _WIN32
				XA_Buffers[num].pAudioData = (BYTE*)SYSTEM_MALLOC(uncomp_size - 32);
				if (XA_Buffers[num].pAudioData) {
					XA_SPS[num] = 22050;
					memcpy((void*)XA_Buffers[num].pAudioData, pPCMData, uncomp_size - 32);
					XA_Buffers[num].AudioBytes = uncomp_size - 32;

					result = true;
				}
#endif
			}
		} else {
			Log(1, "Failed to initialize decoder.");
		}
	}

	// Clean up
	if (pADPCMData)
		SYSTEM_FREE(pADPCMData);
	if (pPCMData)
		SYSTEM_FREE(pPCMData);

	ma_decoder_uninit(&decoder);

	return result;
}

void DXStopSample(int32_t channel) {
	if (channel >= 0 && channel < MAX_VOICES) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
		ma_sound_stop(& ma_voices[channel]);
		ma_voice_active[channel] = -1;
#elif _WIN32
		if (XA_Voices[channel]) {
			DXAttempt(XA_Voices[channel]->Stop(0, XAUDIO2_COMMIT_NOW));
			DXAttempt(XA_Voices[channel]->FlushSourceBuffers());

			XA_Voice_Active[channel] = -1;
		}
#endif
	}
}

bool DSIsChannelPlaying(int32_t channel) {
	if (channel >= 0 && channel < MAX_VOICES) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
		return ma_voice_active[channel] >= 0;
#elif _WIN32
		XAUDIO2_VOICE_STATE state;

		if (XA_Voices[channel]) {
			XA_Voices[channel]->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

			if (state.BuffersQueued)
				return 1;
		}
#endif
	}

	return false;
}

int32_t DSGetFreeChannel() {
	for (int i = 0; i < MAX_VOICES; i++) {
		if (!DSIsChannelPlaying(i))
			return i;
	}

	return -1;
}

bool DSIsSamplePlaying(int32_t sample_id) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	for (int i = 0; i < MAX_VOICES; i++) {
		if (ma_voice_active[sample_id] == sample_id) {
			return true;
		}
	}
#elif _WIN32
	for (int i = 0; i < MAX_VOICES; i++) {
		if (DSIsChannelPlaying(i)) {
			if (XA_Voice_Active[i] == sample_id) {
				return true;
			}
		}
	}
#endif

	return false;
}


#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
// Resets the voice active flag upon ending so that the channel can be reused.
void ma_sample_end_callback(void* pUserData, ma_sound* pSound) {
	*(int*)pUserData = -1;
}
#endif

int32_t DXStartSample(int32_t num, int32_t volume, int32_t pitch, int32_t pan, uint32_t flags) {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	int32_t channel;

	channel = DSGetFreeChannel();

	if (channel < 0)
		return -1;

	if (!ma_sample_buffers[num])
		return -1;

	ma_sound_uninit(&ma_voices[channel]);

	bool is_looping = flags & 0xff ? MA_TRUE : MA_FALSE;

	ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
	        ma_sample_buffers[num]->ref.format,
	        ma_sample_buffers[num]->ref.channels,
	        ma_sample_buffers[num]->ref.sizeInFrames,
	        ma_sample_buffers[num]->ref.pData,
	        NULL);
	bufferConfig.sampleRate = ma_sample_buffers[num]->ref.sampleRate;

	ma_result buffer_result = ma_audio_buffer_init(&bufferConfig, &ma_voice_buffers[channel]);
	if (buffer_result != MA_SUCCESS) {
		return -1;
	}

	ma_sound_config config = ma_sound_config_init_2(&ma_samples_engine);
	config.pDataSource = &ma_voice_buffers[channel];
	config.flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
	config.pInitialAttachment = NULL;
	config.endCallback = &ma_sample_end_callback;
	config.pEndCallbackUserData = &ma_voice_active[channel];

	ma_result sound_init_result = ma_sound_init_ex(&ma_samples_engine, &config, &ma_voices[channel]);
	if (sound_init_result != MA_SUCCESS) {
		return -1;
	}

	DSChangeVolume(channel, volume);
	DSAdjustPitch(channel, pitch);
	DSAdjustPan(channel, pan);

	ma_sound_set_looping(&ma_voices[channel], is_looping ? MA_TRUE : MA_FALSE);
	ma_audio_buffer_seek_to_pcm_frame(ma_sample_buffers[num], 0);
	ma_sound_set_start_time_in_pcm_frames(&ma_voices[channel], 0);
	ma_sound_start(&ma_voices[channel]);
	ma_voice_active[channel] = num;

	return channel;
#else
	IXAudio2SourceVoice* voice;
	XAUDIO2_BUFFER* buffer;
	int32_t channel;

	channel = DSGetFreeChannel();

	if (channel < 0)
		return -1;

	voice = XA_Voices[channel];
	if (voice) {
		current_voice_samples_per_second = XA_SPS[num];

		DSChangeVolume(channel, volume);
		DSAdjustPitch(channel, pitch);
		DSAdjustPan(channel, pan);
		buffer = &XA_Buffers[num];
		buffer->LoopCount = flags;
		DXAttempt(voice->SubmitSourceBuffer(buffer, 0));
		DXAttempt(voice->Start(0, XAUDIO2_COMMIT_NOW));
		return channel;
	}
	return -1;
#endif
}

int32_t CalcVolume(int32_t volume) {
	int32_t result;

	result = 8000 - int32_t(float(0x7FFF - volume) * 0.30518511F);

	if (result > 0)
		result = 0;
	else if (result < -10000)
		result = -10000;

	result -= (100 - SFXVolume) * 50;

	if (result > 0)
		result = 0;

	if (result < -10000)
		result = -10000;

	return result;
}

void S_SoundStopAllSamples() {
	for (int i = 0; i < MAX_VOICES; i++)
		DXStopSample(i);
}

void S_SoundStopSample(int32_t num) {
	DXStopSample(num);
}

int32_t S_SoundPlaySample(int32_t num, uint16_t volume, int32_t pitch, int16_t pan) {
	return DXStartSample(num, CalcVolume(volume), pitch, pan, 0);
}

int32_t S_SoundPlaySampleLooped(int32_t num, uint16_t volume, int32_t pitch, int16_t pan) {
#ifndef MA_AUDIO_SAMPLES
	return DXStartSample(num, CalcVolume(volume), pitch, pan, XAUDIO2_LOOP_INFINITE);
#elif _WIN32
	return DXStartSample(num, CalcVolume(volume), pitch, pan, 0xff);
#endif
}

void DXFreeSounds() {
	S_SoundStopAllSamples();

#ifdef MA_AUDIO_SAMPLES
	for (int i = 0; i < MAX_SAMPLE_BUFFERS; i++) {
		if (ma_sample_buffers[i]) {
			ma_audio_buffer_uninit_and_free(ma_sample_buffers[i]);
			ma_sample_buffers[i] = nullptr;
		}
	}
#elif _WIN32
	for (int i = 0; i < MAX_SAMPLE_BUFFERS; i++) {
		if (XA_Buffers[i].pAudioData) {
			SYSTEM_FREE((void*)XA_Buffers[i].pAudioData);
			XA_Buffers[i].pAudioData = 0;
		}
	}
#endif
}

int32_t S_SoundSampleIsPlayingOnChannel(int32_t num) {
	if (sound_active && DSIsChannelPlaying(num))
		return 1;

	return 0;
}

void S_SoundSetPanAndVolume(int32_t num, int16_t pan, uint16_t volume) {
	if (sound_active) {
		DSChangeVolume(num, CalcVolume(volume));
		DSAdjustPan(num, pan);
	}
}

void S_SoundSetPitch(int32_t num, int32_t pitch) {
	if (sound_active)
		DSAdjustPitch(num, pitch);
}

void S_SetReverbType(int32_t reverb) {
#ifdef MA_AUDIO_SAMPLES
	return;
#elif _WIN32
	if (App.SoundDisabled)
		return;

	if (tomb4.reverb == REVERB_OFF)
		reverb = 0;

	if (current_reverb != reverb) {
		if (reverb) {
			if (!current_reverb) {
				XAMaster->EnableEffect(0, XAUDIO2_COMMIT_NOW);
				XAMaster->SetVolume(2.0F, XAUDIO2_COMMIT_NOW);
			}

			XAMaster->SetEffectParameters(0, &reverb_type[reverb - 1], sizeof(XAUDIO2FX_REVERB_PARAMETERS), XAUDIO2_COMMIT_NOW);
		} else {
			XAMaster->DisableEffect(0, XAUDIO2_COMMIT_NOW);
			XAMaster->SetVolume(1.0F, XAUDIO2_COMMIT_NOW);
		}

		current_reverb = reverb;
	}
#endif
}

void S_SoundPauseSamples() {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	for (int i = 0; i < MAX_VOICES; i++) {
		if (ma_voice_active[i] >= 0) {
			ma_sound_stop(&ma_voices[i]);
		}
	}
#endif
}

void S_SoundUnpauseSamples() {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	for (int i = 0; i < MAX_VOICES; i++) {
		if (ma_voice_active[i] >= 0) {
			ma_sound_start(&ma_voices[i]);
		}
	}
#endif
}

void DXDSClose() {
#if defined(MA_AUDIO_SAMPLES) && defined(MA_AUDIO_ENGINE)
	Log(2, "DXDSClose");
	ma_engine_uninit(&ma_samples_engine);
#elif _WIN32
	if (App.SoundDisabled)
		return;

	for (int i = 0; i < MAX_VOICES; i++) {
		if (XA_Voices[i]) {
			XA_Voices[i]->DestroyVoice();
			XA_Voices[i] = 0;
		}
	}

	XAMaster->DestroyVoice();
	XAEffect->Release();
	App.dx.lpXA->Release();
#endif
}
