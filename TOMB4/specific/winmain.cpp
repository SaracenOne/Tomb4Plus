#include "../tomb4/pch.h"
#include "winmain.h"
#include "function_stubs.h"
#include "cmdline.h"
#include "registry.h"
#include "dxshell.h"
#include "../game/text.h"
#include "lighting.h"
#include "function_table.h"
#include "d3dmatrix.h"
#include "3dmath.h"
#include "audio.h"
#include "output.h"
#include "file.h"
#include "../game/gameflow.h"
#include "dxsound.h"
#include "gamemain.h"
#include "fmv.h"
#include "audio.h"
#include "platform.h"

#include "../tomb4/mod_config.h"
#include "../tomb4/tomb4.h"

static COMMANDLINES commandlines[] = {
	{ "NOFMV", 0, &CLNoFMV },
	{ "PATH", 1, &CLPath }
};

WINAPP App;
char* cutseqpakPtr;
int32_t resChangeCounter;
bool appIsUnfocused = false;

#include <SDL.h>

SDL_Window* sdl_window = NULL;
#ifdef _WIN32
WNDPROC originalWndProc;
#endif

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if ENTRY_CONFIG_USE_WAYLAND
#		include <wayland-egl.h>
#	endif
#elif BX_PLATFORM_WINDOWS
#	define SDL_MAIN_HANDLED

#include <bx/os.h>

BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wextern-c-compat")
#include <SDL_syswm.h>
BX_PRAGMA_DIAGNOSTIC_POP()

#endif
#include "input.h"

void SDLProcessCommandLine(int argc, char* argv[]) {
	COMMANDLINES* command;
	char* pCommand;
	char* p;
	char* last;
	size_t l;
	int32_t num;
	char parameter[PARAMETER_MAX_LENGTH];

	GlobalLog("SDLProcessCommandLine");

	num = sizeof(commandlines) / sizeof(commandlines[0]);

	for (int i = 0; i < num; i++) {
		command = &commandlines[i];
		command->code((char*)"_INIT");
	}

	for (int cur_arg = 1; cur_arg < argc; cur_arg++) {
		for (int i = 0; (uint32_t)i < strlen(argv[cur_arg]); i++) {
			if (argv[cur_arg][i] == '=') {
				break;
			}

			if (toupper(argv[cur_arg][i]))
				argv[cur_arg][i] = toupper(argv[cur_arg][i]);
		}
	}

	for (int i = 0; i < num; i++) {
		command = &commandlines[i];
		memset(parameter, 0, sizeof(parameter));

		pCommand = NULL;
		for (int cur_arg = 1; cur_arg < argc; cur_arg++) {
			pCommand = strstr(argv[cur_arg], command->command);
			if (pCommand != NULL) {
				break;
			}
		}

		if (pCommand) {
			if (command->needs_parameter) {
				p = 0;
				l = strlen(pCommand);

				for (int j = 0; (uint32_t)j < l; j++, pCommand++) {
					if (*pCommand != '=')
						continue;

					p = pCommand + 1;
					l = strlen(p);

					for (j = 0; (uint32_t)j < l; j++, p++) {
						if (*p != ' ')
							break;
					}

					last = p;
					l = strlen(last);

					if (l > (PARAMETER_MAX_LENGTH - 8))
						l = (PARAMETER_MAX_LENGTH - 8);

					strncpy(parameter, p, l);
					break;
				}

				command->code(parameter);
			} else
				command->code(0);
		}
	}
}

float SDLFrameRate() {
	double t;
	static float fps;
	static Uint64 time, time_now, counter;
	static Uint8 first_time;

	if (!(first_time & 1)) {
		first_time |= 1;
		time = SDL_GetTicks64();
	}

	counter++;

	if (counter == 10) {
		time_now = SDL_GetTicks64();
		t = (double)(time_now - time) / (double)1000;
		time = (int32_t)time_now;
		fps = float(counter / t);
		counter = 0;
	}

	App.fps = fps;
	return fps;
}

#ifdef _WIN32
LRESULT CALLBACK WinMainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int32_t mouseX, mouseY, mouseB;
	static bool closing;

	switch (uMsg) {
		case WM_MOVE:
			Log(6, "WM_MOVE");
			break;
	}

	LRESULT result = CallWindowProc(originalWndProc, hwnd, uMsg, wParam, lParam);

	return result;
}
#endif

void* SDLGetNativeWindowHandle(SDL_Window *window) {
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		return NULL;
	}

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
	wl_egl_window* win_impl = (wl_egl_window*)SDL_GetWindowData(_window, "wl_egl_window");
	if (!win_impl) {
		int width, height;
		SDL_GetWindowSize(_window, &width, &height);
		struct wl_surface* surface = wmi.info.wl.surface;
		if (!surface)
			return nullptr;
		win_impl = wl_egl_window_create(surface, width, height);
		SDL_SetWindowData(_window, "wl_egl_window", win_impl);
	}
	return (void*)(uintptr_t)win_impl;
#else
	return (void*)wmi.info.x11.window;
#endif
#elif BX_PLATFORM_OSX || BX_PLATFORM_IOS
	return wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
	return wmi.info.win.window;
#elif BX_PLATFORM_ANDROID
	return wmi.info.android.window;
#else
#error "Unsupported platform!"
#endif // BX_PLATFORM_
}


void* SDLGetNativeDisplayHandle(SDL_Window *window) {
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		return NULL;
	}

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
	return wmi.info.wl.display;
#else
	return wmi.info.x11.display;
#endif // ENTRY_CONFIG_USE_WAYLAND
#else
	return NULL;
#endif // BX_PLATFORM_*
}

void SDLDisplayString(int32_t x, int32_t y, char* string, ...) {
	va_list list;
	char buf[4096];

	va_start(list, string);
	vsprintf(buf, string, list);
	PrintString(x, y, 6, buf, 0);
}

void ClearSurfaces() {
}

bool SDLCreateWindow() {
	Uint32 sdl_window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI;

#if INTPTR_MAX == INT64_MAX
	sdl_window = SDL_CreateWindow("Tomb4Plus (64-bit)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, sdl_window_flags);
#elif INTPTR_MAX == INT32_MAX
	sdl_window = SDL_CreateWindow("Tomb4Plus (32-bit)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, sdl_window_flags);
#else
#error Unknown pointer size or missing size macros!
#endif
	if (!sdl_window) {
		return false;
	}
#ifdef _WIN32
	SDL_SysWMinfo wmInfo = {};
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdl_window, &wmInfo);
	App.hWnd = wmInfo.info.win.window;
	App.hInstance = wmInfo.info.win.hinstance;

	// Subclass the window procedure to intercept WM_MOVE messages
	originalWndProc = (WNDPROC)GetWindowLongPtr(App.hWnd, GWLP_WNDPROC);
	SetWindowLongPtr(App.hWnd, GWLP_WNDPROC, (LONG_PTR)WinMainWndProc);

	if (!App.hWnd) {
		return false;
	}
#endif


	return true;
}

void SDLSetStyle(bool fullscreen, uint32_t& set) {
#ifdef _WIN32
	uint32_t style;

	style = GetWindowLong(App.hWnd, GWL_STYLE);

	if (fullscreen)
		style = (style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP;
	else
		style = (style & ~WS_POPUP) | WS_OVERLAPPEDWINDOW;

	style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX | WS_SYSMENU);
#endif

	if (fullscreen)
		SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else
		SDL_SetWindowFullscreen(sdl_window, 0);

#if _WIN32
	if (set)
		set = style;
#endif
}

void SDLProcessEvents() {
	SDL_bool quit = SDL_FALSE;
	SDL_bool is_dragging = SDL_FALSE;

	while (!quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					quit = SDL_TRUE;
					break;
				}
				case SDL_WINDOWEVENT: {
					switch (event.window.event) {
						case SDL_WINDOWEVENT_FOCUS_LOST: {
							if (App.SetupComplete) {
								appIsUnfocused = true;
								if (tomb4.hang_game_thread) {
									Log(5, "Change Video Mode");
									Log(5, "HangGameThread");
									S_PauseAudio();
									S_SoundPauseSamples();

									while (App.dx.InScene) {};
									App.dx.WaitAtBeginScene = 1;
									while (!App.dx.InScene) {};

									Log(5, "Game Thread Suspended");
								}
							}
							break;
						}
						case SDL_WINDOWEVENT_FOCUS_GAINED: {
							if (App.SetupComplete) {
								appIsUnfocused = false;
								if (tomb4.hang_game_thread) {
									App.dx.WaitAtBeginScene = 0;
									Log(5, "Game Thread Resumed");
									S_SoundUnpauseSamples();
									S_UnpauseAudio();
								}
							}

							break;
						}
						case SDL_WINDOWEVENT_MOVED: {
							break;
						}
					}
					break;
				}
				case SDL_CONTROLLERDEVICEADDED: {
					UpdateGamepad();
					break;
				}
				case SDL_CONTROLLERDEVICEREMOVED: {
					UpdateGamepad();
					break;
				}
				default: {
					break;
				}
			}
		}
	}
}

void SDLClose() {
	Log(2, "SDLClose");
	InputShutdown();
	SaveSettings();

#ifdef _WIN32
	CloseHandle(App.mutex);
#endif

	DXFreeInfo(&App.DXInfo);
#if 0
	DXClose();
#endif
	FreeBinkStuff();

	if (!G_dxptr)
		return;

	DXDSClose();

	system_report_stray_allocation();

	SDL_DestroyWindow(sdl_window);

	SDL_Quit();
}

int main(int argc, char* argv[]) {
	char* buf;
	size_t size;

	App.SetupComplete = false;
	App.TextureSize = 256;
	App.BumpMapSize = 256;
	App.BumpMapping = false;
	App.Filtering = false;
	App.Volumetric = true;
	App.SoundDisabled = false;
	App.AutoTarget = false;
	App.VideoWidth = WINDOW_DEFAULT_WIDTH;
	App.VideoHeight = WINDOW_DEFAULT_HEIGHT;

	SDLProcessCommandLine(argc, argv);

	// Tomb4Plus
	T4PlusInit();
	//

	// Tomb4Plus
	if (!LoadGameModConfigFirstPass()) {
		return -1;
	}
	//

	LoadGameflow();

	// Tomb4Plus
	LoadGameModConfigSecondPass();
	//

	SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);

	InputInit();

	if (!SDLCreateWindow()) {
		Log(1, "Unable To Create Window");
		return -1;
	}

#ifdef _WIN32
	DXGetInfo(&App.DXInfo, App.hWnd);
#else
	DXGetInfo(&App.DXInfo);
#endif

	LoadSettings();

	fmvs_disabled = 1; // Disable all FMVs for now.

	App.dx.WaitAtBeginScene = 0;
	App.dx.InScene = 0;
	App.fmv = 0;

	int window_width;
	int window_height;
	int window_bpp;
	window_width = App.VideoWidth;
	window_height = App.VideoHeight;
	window_bpp = 32;

	int rendererWidth = 0;
	int rendererHeight = 0;

	SDL_SetWindowSize(sdl_window, window_width, window_height);
	uint32_t window_flags = 0;
	if (App.StartFlags & DXF_FULLSCREEN) {
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_DisplayMode sdl_dm;
		SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(sdl_window), &sdl_dm);
		rendererWidth = sdl_dm.w;
		rendererHeight = sdl_dm.h;
	} else {
		rendererWidth = window_width;
		rendererHeight = window_height;
		SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	SDL_SetWindowFullscreen(sdl_window, window_flags);

#ifdef _WIN32
	if (!DXCreate(rendererWidth, rendererHeight, window_bpp, App.StartFlags, &App.dx, App.hWnd, WS_OVERLAPPEDWINDOW)) {
#else
	if (!DXCreate(rendererWidth, rendererHeight, window_bpp, App.StartFlags, &App.dx)) {
#endif
		platform_fatal_error(GetFixedStringForTextID(TXT_Failed_To_Setup_DirectX));
		return -1;
	}

	SDL_ShowWindow(sdl_window);

	if (!App.SoundDisabled) {
		DXDSCreate();
		ACMInit();
	}

	cutseqpakPtr = 0;
	buf = 0;
	size = T4PLoadFileAtRelativePath("data/cutseq.pak", &buf);

	if (size && size < UINT32_MAX) {
		cutseqpakPtr = (char*)SYSTEM_MALLOC(*(int32_t*)buf);
		Decompress(cutseqpakPtr, buf + 4, uint32_t(size_t(size - 4) & 0xffffffff), *(int32_t*)buf);
		SYSTEM_FREE(buf);
	}

	MainThread.active = 1;
	MainThread.ended = 0;

	MainThread.handle = SDL_CreateThread(GameMain, "GameMain", (void*)NULL);
	if (MainThread.handle == NULL) {
		printf("SDL_CreateThread failed: %s\n", SDL_GetError());
	}

	SDLProcessEvents();

	MainThread.ended = 1;
	while (MainThread.active) {};

	if (cutseqpakPtr) {
		SYSTEM_FREE(cutseqpakPtr);
	}

#ifdef _WIN32
	SetWindowLongPtr(App.hWnd, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
#endif

	SDLClose();

	return 0;
}