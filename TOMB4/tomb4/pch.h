// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#ifndef MA_AUDIO_SAMPLES
#define ENABLE_MINIMUM_DX_FUNCTIONS
#endif

// add headers that you want to pre-compile here
#ifdef _WIN32
#define DIRECTINPUT_VERSION 0x0800
#define _USE_MATH_DEFINES
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#ifdef ENABLE_MINIMUM_DX_FUNCTIONS
#include <d3d.h>
#include <d3dtypes.h>
#endif
#include <cmath>
#include <stdio.h>
#include <process.h>
#if !defined(MA_AUDIO_SAMPLES) || !defined(MA_AUDIO_ENGINE)
#define DIRECTSOUND_VERSION	0x0800
#define XAUDIO2_HELPER_FUNCTIONS
#include <dsound.h>
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <MSAcm.h>
#endif
#endif

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/platform.h>

#include "../resource.h"

#endif //PCH_H
