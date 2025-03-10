#include "../tomb4/pch.h"
#include "../specific/time.h"	//there's some other time.h
#include "function_stubs.h"

#include <SDL.h>

static Uint64 counter, frequency;

int32_t Sync() {
	Uint64 PerformanceCount;
	int32_t n;

	PerformanceCount = SDL_GetPerformanceCounter();
	Uint64 f = (PerformanceCount - counter) / frequency;
	counter += frequency * f;
	n = (int32_t)f;
	return n;
}

void TIME_Reset() {
	counter = SDL_GetPerformanceCounter();
}

bool TIME_Init() {
	Log(2, "TIME_Init");

	Uint64 pfq = SDL_GetPerformanceFrequency();
	if (!pfq)
		return false;

	frequency = pfq / 60;
	TIME_Reset();
	return true;
}