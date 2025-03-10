#pragma once
#include "../global/types.h"

#define WINDOW_DEFAULT_WIDTH 640
#define WINDOW_DEFAULT_HEIGHT 480

extern SDL_Window* sdl_window;

float SDLFrameRate();
void SDLDisplayString(int32_t x, int32_t y, char* string, ...);
void ClearSurfaces();
void SDLSetStyle(bool fullscreen, uint32_t& set);

void* SDLGetNativeWindowHandle(SDL_Window *window);
void* SDLGetNativeDisplayHandle(SDL_Window *window);

extern WINAPP App;
extern char* cutseqpakPtr;
extern int32_t resChangeCounter;
extern bool appIsUnfocused;
