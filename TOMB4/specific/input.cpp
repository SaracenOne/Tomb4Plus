#include "../tomb4/pch.h"
#include "input.h"
#include "dxshell.h"
#include "../game/sound.h"
#include "../game/newinv.h"
#include "../game/lara_states.h"
#include "../game/control.h"
#include "../game/camera.h"
#include "LoadSave.h"
#include "winmain.h"
#include "../game/lara.h"
#include "../game/savegame.h"
#include "../game/gameflow.h"
#include "../game/spotcam.h"
#include "../tomb4/tomb4.h"

#include "../game/trng/trng.h"
#include "../game/trng/trng_extra_state.h"

#include "function_stubs.h"

#ifdef USE_SDL
int convert_sdl_scancode_to_tomb_keycode(int scancode) {
	switch (scancode) {
	case SDL_SCANCODE_UNKNOWN:
		return 0x00; // 
	case SDL_SCANCODE_A:
		return T4P_KEY_A;
	case SDL_SCANCODE_B:
		return T4P_KEY_B;
	case SDL_SCANCODE_C:
		return T4P_KEY_C;
	case SDL_SCANCODE_D:
		return T4P_KEY_D;
	case SDL_SCANCODE_E:
		return T4P_KEY_E;
	case SDL_SCANCODE_F:
		return T4P_KEY_F;
	case SDL_SCANCODE_G:
		return T4P_KEY_G;
	case SDL_SCANCODE_H:
		return T4P_KEY_H;
	case SDL_SCANCODE_I:
		return T4P_KEY_I;
	case SDL_SCANCODE_J:
		return T4P_KEY_J;
	case SDL_SCANCODE_K:
		return T4P_KEY_K;
	case SDL_SCANCODE_L:
		return T4P_KEY_L;
	case SDL_SCANCODE_M:
		return T4P_KEY_M;
	case SDL_SCANCODE_N:
		return T4P_KEY_N;
	case SDL_SCANCODE_O:
		return T4P_KEY_O;
	case SDL_SCANCODE_P:
		return T4P_KEY_P;
	case SDL_SCANCODE_Q:
		return T4P_KEY_Q;
	case SDL_SCANCODE_R:
		return T4P_KEY_R;
	case SDL_SCANCODE_S:
		return T4P_KEY_S;
	case SDL_SCANCODE_T:
		return T4P_KEY_T;
	case SDL_SCANCODE_U:
		return T4P_KEY_U;
	case SDL_SCANCODE_V:
		return T4P_KEY_V;
	case SDL_SCANCODE_W:
		return T4P_KEY_W;
	case SDL_SCANCODE_X:
		return T4P_KEY_X;
	case SDL_SCANCODE_Y:
		return T4P_KEY_Y;
	case SDL_SCANCODE_Z:
		return T4P_KEY_Z;

	case SDL_SCANCODE_1:
		return T4P_KEY_1;
	case SDL_SCANCODE_2:
		return T4P_KEY_2;
	case SDL_SCANCODE_3:
		return T4P_KEY_3;
	case SDL_SCANCODE_4:
		return T4P_KEY_4;
	case SDL_SCANCODE_5:
		return T4P_KEY_5;
	case SDL_SCANCODE_6:
		return T4P_KEY_6;
	case SDL_SCANCODE_7:
		return T4P_KEY_7;
	case SDL_SCANCODE_8:
		return T4P_KEY_8;
	case SDL_SCANCODE_9:
		return T4P_KEY_9;
	case SDL_SCANCODE_0:
		return T4P_KEY_0;

	case SDL_SCANCODE_RETURN:
		return T4P_KEY_RETURN;
	case SDL_SCANCODE_ESCAPE:
		return T4P_KEY_ESCAPE;
	case SDL_SCANCODE_BACKSPACE:
		return T4P_KEY_BACK;
	case SDL_SCANCODE_TAB:
		return T4P_KEY_TAB;
	case SDL_SCANCODE_SPACE:
		return T4P_KEY_SPACE;

	case SDL_SCANCODE_RIGHT:
		return T4P_KEY_RIGHT;
	case SDL_SCANCODE_LEFT:
		return T4P_KEY_LEFT;
	case SDL_SCANCODE_DOWN:
		return T4P_KEY_DOWN;
	case SDL_SCANCODE_UP:
		return T4P_KEY_UP;

	case SDL_SCANCODE_MINUS:
		return T4P_KEY_MINUS;
	case SDL_SCANCODE_EQUALS:
		return T4P_KEY_EQUALS;
	case SDL_SCANCODE_LEFTBRACKET:
		return T4P_KEY_LBRACKET;
	case SDL_SCANCODE_RIGHTBRACKET:
		return T4P_KEY_RBRACKET;
	case SDL_SCANCODE_BACKSLASH:
		return T4P_KEY_BACKSLASH;

	case SDL_SCANCODE_LCTRL:
		return T4P_KEY_LCONTROL;
	case SDL_SCANCODE_LSHIFT:
		return T4P_KEY_LSHIFT;
	case SDL_SCANCODE_LALT:
		return T4P_KEY_LALT;
	case SDL_SCANCODE_LGUI:
		return T4P_KEY_LWIN;
	case SDL_SCANCODE_RCTRL:
		return T4P_KEY_RCONTROL;
	case SDL_SCANCODE_RSHIFT:
		return T4P_KEY_RSHIFT;
	case SDL_SCANCODE_RALT:
		return T4P_KEY_RALT;
	case SDL_SCANCODE_RGUI:
		return T4P_KEY_RWIN;

	case SDL_SCANCODE_COMMA:
		return T4P_KEY_COMMA;
	case SDL_SCANCODE_PERIOD:
		return T4P_KEY_PERIOD;


	case SDL_SCANCODE_KP_1:
		return T4P_KEY_NUMPAD1;
	case SDL_SCANCODE_KP_2:
		return T4P_KEY_NUMPAD2;
	case SDL_SCANCODE_KP_3:
		return T4P_KEY_NUMPAD3;
	case SDL_SCANCODE_KP_4:
		return T4P_KEY_NUMPAD4;
	case SDL_SCANCODE_KP_5:
		return T4P_KEY_NUMPAD5;
	case SDL_SCANCODE_KP_6:
		return T4P_KEY_NUMPAD6;
	case SDL_SCANCODE_KP_7:
		return T4P_KEY_NUMPAD7;
	case SDL_SCANCODE_KP_8:
		return T4P_KEY_NUMPAD8;
	case SDL_SCANCODE_KP_9:
		return T4P_KEY_NUMPAD9;
	case SDL_SCANCODE_KP_0:
		return T4P_KEY_NUMPAD0;
	case SDL_SCANCODE_KP_PERIOD:
		return T4P_KEY_NUMPADPERIOD;
	case SDL_SCANCODE_KP_PLUS:
		return T4P_KEY_ADD;
	case SDL_SCANCODE_KP_MINUS:
		return T4P_KEY_MINUS;
	case SDL_SCANCODE_KP_DIVIDE:
		return T4P_KEY_DIVIDE;
	case SDL_SCANCODE_KP_MULTIPLY:
		return T4P_KEY_MULTIPLY;
	case SDL_SCANCODE_KP_ENTER:
		return T4P_KEY_NUMPADENTER;

	case SDL_SCANCODE_CAPSLOCK:
		return T4P_KEY_CAPS_LOCK;

	case SDL_SCANCODE_INSERT:
		return T4P_KEY_INSERT;
	case SDL_SCANCODE_HOME:
		return T4P_KEY_HOME;
	case SDL_SCANCODE_PAGEUP:
		return T4P_KEY_PAGE_UP;
	case SDL_SCANCODE_DELETE:
		return T4P_KEY_DELETE;
	case SDL_SCANCODE_END:
		return T4P_KEY_END;
	case SDL_SCANCODE_PAGEDOWN:
		return T4P_KEY_PAGE_DOWN;

	case SDL_SCANCODE_SEMICOLON:
		return T4P_KEY_SEMICOLON;
	case SDL_SCANCODE_SLASH:
		return T4P_KEY_SLASH;
	case SDL_SCANCODE_APOSTROPHE:
		return T4P_KEY_APOSTROPHE;
	case SDL_SCANCODE_GRAVE:
		return T4P_KEY_GRAVE;

	case SDL_SCANCODE_NONUSBACKSLASH:
		return T4P_KEY_OEM_102; // This might have issues with different keyboard layouts.

	default:
		return 0x00;
	}
};

int convert_tomb_keycode_to_sdl_scancode(int tomb_keycode) {
	switch (tomb_keycode) {
	case T4P_KEY_A:
		return SDL_SCANCODE_A;
	case T4P_KEY_B:
		return SDL_SCANCODE_B;
	case T4P_KEY_C:
		return SDL_SCANCODE_C;
	case T4P_KEY_D:
		return SDL_SCANCODE_D;
	case T4P_KEY_E:
		return SDL_SCANCODE_E;
	case T4P_KEY_F:
		return SDL_SCANCODE_F;
	case T4P_KEY_G:
		return SDL_SCANCODE_G;
	case T4P_KEY_H:
		return SDL_SCANCODE_H;
	case T4P_KEY_I:
		return SDL_SCANCODE_I;
	case T4P_KEY_J:
		return SDL_SCANCODE_J;
	case T4P_KEY_K:
		return SDL_SCANCODE_K;
	case T4P_KEY_L:
		return SDL_SCANCODE_L;
	case T4P_KEY_M:
		return SDL_SCANCODE_M;
	case T4P_KEY_N:
		return SDL_SCANCODE_N;
	case T4P_KEY_O:
		return SDL_SCANCODE_O;
	case T4P_KEY_P:
		return SDL_SCANCODE_P;
	case T4P_KEY_Q:
		return SDL_SCANCODE_Q;
	case T4P_KEY_R:
		return SDL_SCANCODE_R;
	case T4P_KEY_S:
		return SDL_SCANCODE_S;
	case T4P_KEY_T:
		return SDL_SCANCODE_T;
	case T4P_KEY_U:
		return SDL_SCANCODE_U;
	case T4P_KEY_V:
		return SDL_SCANCODE_V;
	case T4P_KEY_W:
		return SDL_SCANCODE_W;
	case T4P_KEY_X:
		return SDL_SCANCODE_X;
	case T4P_KEY_Y:
		return SDL_SCANCODE_Y;
	case T4P_KEY_Z:
		return SDL_SCANCODE_Z;

	case T4P_KEY_1:
		return SDL_SCANCODE_1;
	case T4P_KEY_2:
		return SDL_SCANCODE_2;
	case T4P_KEY_3:
		return SDL_SCANCODE_3;
	case T4P_KEY_4:
		return SDL_SCANCODE_4;
	case T4P_KEY_5:
		return SDL_SCANCODE_5;
	case T4P_KEY_6:
		return SDL_SCANCODE_6;
	case T4P_KEY_7:
		return SDL_SCANCODE_7;
	case T4P_KEY_8:
		return SDL_SCANCODE_8;
	case T4P_KEY_9:
		return SDL_SCANCODE_9;
	case T4P_KEY_0:
		return SDL_SCANCODE_0;

	case T4P_KEY_RETURN:
		return SDL_SCANCODE_RETURN;
	case T4P_KEY_ESCAPE:
		return SDL_SCANCODE_ESCAPE;
	case T4P_KEY_BACK:
		return SDL_SCANCODE_BACKSPACE;
	case T4P_KEY_TAB:
		return SDL_SCANCODE_TAB;
	case T4P_KEY_SPACE:
		return SDL_SCANCODE_SPACE;

	case T4P_KEY_RIGHT:
		return SDL_SCANCODE_RIGHT;
	case T4P_KEY_LEFT:
		return SDL_SCANCODE_LEFT;
	case T4P_KEY_DOWN:
		return SDL_SCANCODE_DOWN;
	case T4P_KEY_UP:
		return SDL_SCANCODE_UP;

	case T4P_KEY_MINUS:
		return SDL_SCANCODE_MINUS;
	case T4P_KEY_EQUALS:
		return SDL_SCANCODE_EQUALS;
	case T4P_KEY_LBRACKET:
		return SDL_SCANCODE_LEFTBRACKET;
	case T4P_KEY_RBRACKET:
		return SDL_SCANCODE_RIGHTBRACKET;
	case T4P_KEY_BACKSLASH:
		return SDL_SCANCODE_BACKSLASH;

	case T4P_KEY_LCONTROL:
		return SDL_SCANCODE_LCTRL;
	case T4P_KEY_LSHIFT:
		return SDL_SCANCODE_LSHIFT;
	case T4P_KEY_LALT:
		return SDL_SCANCODE_LALT;
	case T4P_KEY_LWIN:
		return SDL_SCANCODE_LGUI;
	case T4P_KEY_RCONTROL:
		return SDL_SCANCODE_RCTRL;
	case T4P_KEY_RSHIFT:
		return SDL_SCANCODE_RSHIFT;
	case T4P_KEY_RALT:
		return SDL_SCANCODE_RALT;
	case T4P_KEY_RWIN:
		return SDL_SCANCODE_RGUI;

	case T4P_KEY_COMMA:
		return SDL_SCANCODE_COMMA;
	case T4P_KEY_PERIOD:
		return SDL_SCANCODE_PERIOD;

	case T4P_KEY_NUMPAD1:
		return SDL_SCANCODE_KP_1;
	case T4P_KEY_NUMPAD2:
		return SDL_SCANCODE_KP_2;
	case T4P_KEY_NUMPAD3:
		return SDL_SCANCODE_KP_3;
	case T4P_KEY_NUMPAD4:
		return SDL_SCANCODE_KP_4;
	case T4P_KEY_NUMPAD5:
		return SDL_SCANCODE_KP_5;
	case T4P_KEY_NUMPAD6:
		return SDL_SCANCODE_KP_6;
	case T4P_KEY_NUMPAD7:
		return SDL_SCANCODE_KP_7;
	case T4P_KEY_NUMPAD8:
		return SDL_SCANCODE_KP_8;
	case T4P_KEY_NUMPAD9:
		return SDL_SCANCODE_KP_9;
	case T4P_KEY_NUMPAD0:
		return SDL_SCANCODE_KP_0;
	case T4P_KEY_NUMPADPERIOD:
		return SDL_SCANCODE_KP_PERIOD;
	case T4P_KEY_ADD:
		return SDL_SCANCODE_KP_PLUS;
	case T4P_KEY_SUBTRACT:
		return SDL_SCANCODE_KP_MINUS;
	case T4P_KEY_DIVIDE:
		return SDL_SCANCODE_KP_DIVIDE;
	case T4P_KEY_MULTIPLY:
		return SDL_SCANCODE_KP_MULTIPLY;
	case T4P_KEY_NUMPADENTER:
		return SDL_SCANCODE_KP_ENTER;

	case T4P_KEY_CAPS_LOCK:
		return SDL_SCANCODE_CAPSLOCK;

	case T4P_KEY_INSERT:
		return SDL_SCANCODE_INSERT;
	case T4P_KEY_HOME:
		return SDL_SCANCODE_HOME;
	case T4P_KEY_PAGE_UP:
		return SDL_SCANCODE_PAGEUP;
	case T4P_KEY_DELETE:
		return SDL_SCANCODE_DELETE;
	case T4P_KEY_END:
		return SDL_SCANCODE_END;
	case T4P_KEY_PAGE_DOWN:
		return SDL_SCANCODE_PAGEDOWN;

	case T4P_KEY_SEMICOLON:
		return SDL_SCANCODE_SEMICOLON;
	case T4P_KEY_SLASH:
		return SDL_SCANCODE_SLASH;
	case T4P_KEY_APOSTROPHE:
		return SDL_SCANCODE_APOSTROPHE;
	case T4P_KEY_GRAVE:
		return SDL_SCANCODE_GRAVE;

	case T4P_KEY_F1:
		return SDL_SCANCODE_F1;
	case T4P_KEY_F2:
		return SDL_SCANCODE_F2;
	case T4P_KEY_F3:
		return SDL_SCANCODE_F3;
	case T4P_KEY_F4:
		return SDL_SCANCODE_F4;
	case T4P_KEY_F5:
		return SDL_SCANCODE_F5;
	case T4P_KEY_F6:
		return SDL_SCANCODE_F6;
	case T4P_KEY_F7:
		return SDL_SCANCODE_F7;
	case T4P_KEY_F8:
		return SDL_SCANCODE_F8;
	case T4P_KEY_F9:
		return SDL_SCANCODE_F9;
	case T4P_KEY_F10:
		return SDL_SCANCODE_F10;
	case T4P_KEY_F11:
		return SDL_SCANCODE_F11;
	case T4P_KEY_F12:
		return SDL_SCANCODE_F12;

	case T4P_KEY_OEM_102: // This might have issues with different keyboard layouts.
		return SDL_SCANCODE_NONUSBACKSLASH;

	default:
		return SDL_SCANCODE_UNKNOWN;
	}
}
#else 
int convert_sdl_scancode_to_tomb_keycode(int scancode) {
	return scancode;
};

int convert_tomb_keycode_to_sdl_scancode(int tomb_keycode) {
	return tomb_keycode;
}
#endif

void UpdateGamepad()
{
#ifdef USE_SDL
	if (controller) 
	{
		if (SDL_GameControllerGetAttached(controller))
		{
			return;
		}
		else
		{
			SDL_GameControllerClose(controller);
			Log(2, "Gamepad %s disconnected.", controller_name);
			controller = nullptr;
		}
	}

	if (!controller)
	{
		int controller_count = SDL_NumJoysticks();
		for (int i = 0; i < controller_count; i++)
		{
			controller_name = SDL_GameControllerNameForIndex(i);
			controller_type = SDL_GameControllerTypeForIndex(i);
			if (SDL_IsGameController(i)) {
				controller = SDL_GameControllerOpen(i);
				if (SDL_GameControllerGetAttached(controller)) {
					Log(2, "Gamepad %s connected.", controller_name);
				} else {
					SDL_GameControllerClose(controller);
					controller = nullptr;
				}
			}
		}
	}
#endif
};

void InputInit()
{
	UpdateGamepad();
}

void InputShutdown()
{
#ifdef USE_SDL
	if (controller)
	{
		if (SDL_GameControllerGetAttached(controller))
		{
			return;
		} else {
			SDL_GameControllerClose(controller);
			controller = nullptr;
		}
	}
#endif
};

const char* KeyboardButtons[272] =
{
	0,
	"Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Bksp",
	"Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "<", ">", "Return",
	"Ctrl", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
	"Shift", "#", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift",
	"Padx", "Alt", "Space", "Caps", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Nmlk", 0,
	"Pad7", "Pad8", "Pad9", "Pad-",
	"Pad4", "Pad5", "Pad6", "Pad+",
	"Pad1", "Pad2", "Pad3",
	"Pad0", "Pad.", 0, 0, "\\", 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Enter", "Ctrl", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Shift", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Pad/", 0, 0, "Alt", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Home", "Up", "PgUp", 0, "Left", 0, "Right", 0, "End", "Down", "PgDn", "Ins", "Del",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Joy 1", "Joy 2", "Joy 3", "Joy 4", "Joy 5", "Joy 6", "Joy 7", "Joy 8",
	"Joy 9", "Joy 10", "Joy 11", "Joy 12", "Joy 13", "Joy 14", "Joy 15", "Joy 16"
};

const char* GermanKeyboard[272] =
{
	0,
	"Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "=", "`", "Entf",
	"Tab", "Q", "W", "E", "R", "T", "Z", "U", "I", "O", "P", "~u", "x", "Enter",
	"Strg", "A", "S", "D", "F", "G", "H", "J", "K", "L", "~o", "~a", "( ",
	"Shift", "#", "Y", "X", "C", "V", "B", "N", "M", ",", ".", "-", "Shift",
	"Num x", "Alt", "Leert.", "Caps", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Nmlk", 0,
	"Num7", "Num8", "Num9", "Num-",
	"Num4", "Num5", "Num6", "Num+",
	"Num1", "Num2", "Num3",
	"Num0", "Num.", 0, 0, ">", 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Enter", "Strg", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Shift", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Num/", 0, 0, "Alt", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Pos1", "Hoch", "BdAuf", 0, "Links", 0, "Rechts", 0, "Ende", "Runter", "BdAb", "Einfg", "Entf",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Joy 1", "Joy 2", "Joy 3", "Joy 4", "Joy 5", "Joy 6", "Joy 7", "Joy 8",
	"Joy 9", "Joy 10", "Joy 11", "Joy 12", "Joy 13", "Joy 14", "Joy 15", "Joy 16"
};

bool use_gamepad = true;
short keyboard_layout[INPUT_LAYOUT_COUNT][INPUT_ACTION_COUNT] =
{
	{ T4P_KEY_UP, T4P_KEY_DOWN, T4P_KEY_LEFT, T4P_KEY_RIGHT, T4P_KEY_PERIOD, T4P_KEY_SLASH, T4P_KEY_RSHIFT, T4P_KEY_RALT, T4P_KEY_RCONTROL,
	T4P_KEY_SPACE, T4P_KEY_COMMA, T4P_KEY_NUMPAD0, T4P_KEY_END, T4P_KEY_ESCAPE, T4P_KEY_DELETE, T4P_KEY_PAGE_DOWN, T4P_KEY_P, T4P_KEY_RETURN,
	T4P_KEY_ESCAPE },

	{ T4P_KEY_UP, T4P_KEY_DOWN, T4P_KEY_LEFT, T4P_KEY_RIGHT, T4P_KEY_PERIOD, T4P_KEY_SLASH, T4P_KEY_RSHIFT, T4P_KEY_RALT, T4P_KEY_RCONTROL,
	T4P_KEY_SPACE, T4P_KEY_COMMA, T4P_KEY_NUMPAD0, T4P_KEY_END, T4P_KEY_ESCAPE, T4P_KEY_DELETE, T4P_KEY_PAGE_DOWN, T4P_KEY_P, T4P_KEY_RETURN,
	T4P_KEY_ESCAPE }
};

long conflict[INPUT_ACTION_COUNT];
long input;
long linput;
long dbinput;
long inputBusy;
short ammo_change_timer = 0;
char ammo_change_buf[12];

#ifndef USE_SDL
static long joy_x;
static long joy_y;
static long joy_fire;
#endif

bool IsKeyPressed(int t4p_key)
{
#ifdef USE_SDL
	int sdl_scancode = convert_tomb_keycode_to_sdl_scancode(t4p_key);
	if (sdl_scancode < keymap_count) {
		return keymap[sdl_scancode];
	}
	return false;
#else
	if (t4p_key < keymap_count) {
		return keymap[t4p_key];
	}
#endif
}

static void DoWeaponHotkey()	//adds extra checks and does ammo type swaps..
{
	short state;
	bool goin;

	if (!lara_item) {
		goin = 0;
	} else {
		state = lara_item->current_anim_state;
		goin = !(gfLevelFlags & GF_YOUNGLARA) && (lara.water_status == LW_ABOVE_WATER || lara.water_status == LW_WADE) && !bDisableLaraControl &&
			(state != AS_ALL4S && state != AS_CRAWL && state != AS_ALL4TURNL && state != AS_ALL4TURNR && state != AS_CRAWLBACK &&
				state != AS_CRAWL2HANG && state != AS_DUCK && state != AS_DUCKROTL && state != AS_DUCKROTR);
	}

	if (!goin)
		return;

	else if (IsKeyPressed(T4P_KEY_1))
	{
		if (!(lara.pistols_type_carried & W_PRESENT))
			return;

		lara.request_gun_type = WEAPON_PISTOLS;

		if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_PISTOLS)
			lara.gun_status = LG_DRAW_GUNS;
	}
	else if (IsKeyPressed(T4P_KEY_2))
	{
		if (!(lara.shotgun_type_carried & W_PRESENT))
			return;

		lara.request_gun_type = WEAPON_SHOTGUN;

		if (lara.gun_type == WEAPON_SHOTGUN)
		{
			if (lara.gun_status == LG_NO_ARMS)
				lara.gun_status = LG_DRAW_GUNS;
			else if (lara.gun_status == LG_READY && !ammo_change_timer)
			{
				if (!tomb4.ammotype_hotkeys)
					return;

				memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

				if (lara.shotgun_type_carried & W_AMMO2)
				{
					lara.shotgun_type_carried &= ~W_AMMO2;
					lara.shotgun_type_carried |= W_AMMO1;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Normal");
				}
				else if (lara.shotgun_type_carried & W_AMMO1)
				{
					lara.shotgun_type_carried &= ~W_AMMO1;
					lara.shotgun_type_carried |= W_AMMO2;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Wideshot");
				}
			}
		}
	}
	else if (IsKeyPressed(T4P_KEY_3))
	{
		if (!(lara.uzis_type_carried & W_PRESENT))
			return;

		lara.request_gun_type = WEAPON_UZI;

		if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_UZI)
			lara.gun_status = LG_DRAW_GUNS;
	}
	else if (IsKeyPressed(T4P_KEY_4))
	{
		if (!(lara.sixshooter_type_carried & W_PRESENT))
			return;

		lara.request_gun_type = WEAPON_REVOLVER;

		if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_REVOLVER)
			lara.gun_status = LG_DRAW_GUNS;
	}
	else if (IsKeyPressed(T4P_KEY_5))
	{
		if (!(lara.grenade_type_carried & W_PRESENT))
			return;

		lara.request_gun_type = WEAPON_GRENADE;

		if (lara.gun_type == WEAPON_GRENADE)
		{
			if (lara.gun_status == LG_NO_ARMS)
				lara.gun_status = LG_DRAW_GUNS;
			else if (lara.gun_status == LG_READY && !ammo_change_timer)
			{
				if (!tomb4.ammotype_hotkeys)
					return;

				memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

				if (lara.grenade_type_carried & W_AMMO3)
				{
					lara.grenade_type_carried &= ~W_AMMO3;
					lara.grenade_type_carried |= W_AMMO2;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Super");
				}
				else if (lara.grenade_type_carried & W_AMMO2)
				{
					lara.grenade_type_carried &= ~W_AMMO2;
					lara.grenade_type_carried |= W_AMMO1;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Normal");
				}
				else if (lara.grenade_type_carried & W_AMMO1)
				{
					lara.grenade_type_carried &= ~W_AMMO1;
					lara.grenade_type_carried |= W_AMMO3;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Flash");
				}
			}
		}
	}
	else if (IsKeyPressed(T4P_KEY_6))
	{
		if (!(lara.crossbow_type_carried & W_PRESENT))
			return;

		lara.request_gun_type = WEAPON_CROSSBOW;

		if (lara.gun_type == WEAPON_CROSSBOW)
		{
			if (lara.gun_status == LG_NO_ARMS)
				lara.gun_status = LG_DRAW_GUNS;
			else if (lara.gun_status == LG_READY && !ammo_change_timer)
			{
				if (!tomb4.ammotype_hotkeys)
					return;

				memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

				if (lara.crossbow_type_carried & W_AMMO3)
				{
					lara.crossbow_type_carried &= ~W_AMMO3;
					lara.crossbow_type_carried |= W_AMMO2;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Poison");
				}
				else if (lara.crossbow_type_carried & W_AMMO2)
				{
					lara.crossbow_type_carried &= ~W_AMMO2;
					lara.crossbow_type_carried |= W_AMMO1;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Normal");
				}
				else if (lara.crossbow_type_carried & W_AMMO1)
				{
					lara.crossbow_type_carried &= ~W_AMMO1;
					lara.crossbow_type_carried |= W_AMMO3;
					ammo_change_timer = 30;
					sprintf(ammo_change_buf, "Explosive");
				}
			}
		}
	}
}

#ifdef USE_SDL

#define DEFAULT_AXIS_BUTTON_DEAD_ZONE (0x7fff / 2)

enum GamepadBindingType {
	TYPE_NONE,
	TYPE_BUTTON,
	TYPE_AXIS_BUTTON_POSITIVE,
	TYPE_AXIS_BUTTON_NEGATIVE
};

struct GamepadButtonBinding {
	GamepadBindingType binding_type;
	int value;
};

#define MAX_GAMEPAD_BINDINGS_PER_GROUP 2

struct GamepadButtonGroup {
	GamepadButtonBinding bindings[MAX_GAMEPAD_BINDINGS_PER_GROUP];
};

GamepadButtonGroup default_controller_binding[INPUT_ACTION_COUNT] = {
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_UP, TYPE_AXIS_BUTTON_NEGATIVE, SDL_CONTROLLER_AXIS_LEFTY}, // FORWARD
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_DOWN, TYPE_AXIS_BUTTON_POSITIVE, SDL_CONTROLLER_AXIS_LEFTY}, // BACKWARDS
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_LEFT, TYPE_AXIS_BUTTON_NEGATIVE, SDL_CONTROLLER_AXIS_LEFTX}, // LEFT
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, TYPE_AXIS_BUTTON_POSITIVE, SDL_CONTROLLER_AXIS_LEFTX}, // RIGHT
	{TYPE_AXIS_BUTTON_POSITIVE, SDL_CONTROLLER_AXIS_TRIGGERLEFT, TYPE_NONE, 0}, // DUCK
	{TYPE_AXIS_BUTTON_POSITIVE, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, TYPE_NONE, 0}, // SPRINT
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, TYPE_NONE, 0}, // WALK
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_X, TYPE_NONE, 0}, // JUMP
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_A, TYPE_NONE, 0}, // INTERACT
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_Y, TYPE_NONE, 0}, // DRAW
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_RIGHTSTICK, TYPE_NONE, 0}, // FLARE
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, TYPE_NONE, 0}, // LOOK
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_B, TYPE_NONE, 0}, // ROLL
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_START, TYPE_NONE, 0}, // OPTION,
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_INVALID, TYPE_NONE, 0}, // SIDESTEP_LEFT,
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_INVALID, TYPE_NONE, 0}, // SIDESTEP_RIGHT,
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_BACK, TYPE_NONE, 0}, // PAUSE,
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_A, TYPE_NONE, 0}, // SELECT,
	{TYPE_BUTTON, SDL_CONTROLLER_BUTTON_START, TYPE_NONE, 0}, // UNSELECT,
};

bool IsGamepadButtonPressed(SDL_GameControllerButton button)
{
	if (button >= SDL_CONTROLLER_BUTTON_MAX || button <= SDL_CONTROLLER_BUTTON_INVALID)
	{
		return false;
	}

	return SDL_GameControllerGetButton(controller, button);
}

bool IsGamepadActionPressed(InputAction current_action) {
	if (current_action >= INPUT_ACTION_COUNT || current_action < 0)
	{
		return false;
	}

	GamepadButtonGroup* binding_group = &default_controller_binding[current_action];

	for (int i = 0; i < MAX_GAMEPAD_BINDINGS_PER_GROUP; i++) {
		GamepadButtonBinding* binding = &binding_group->bindings[i];

		if (binding->binding_type == TYPE_BUTTON) {
			if (IsGamepadButtonPressed((SDL_GameControllerButton)binding->value)) {
				return true;
			}
		} else if (binding->binding_type == TYPE_AXIS_BUTTON_POSITIVE || binding->binding_type == TYPE_AXIS_BUTTON_NEGATIVE) {
			Sint16 axis = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)binding->value);

			if (binding->binding_type == TYPE_AXIS_BUTTON_POSITIVE)
			{
				if (axis >= DEFAULT_AXIS_BUTTON_DEAD_ZONE)
				{
					return true;
				}
			} else {
				if (axis <= -DEFAULT_AXIS_BUTTON_DEAD_ZONE)
				{
					return true;
				}
			}
		}
	}

	return false;
}
#endif


bool IsActionPressed(InputAction current_action)
{
	short key;

	if (current_action >= INPUT_ACTION_COUNT || current_action < 0)
	{
		return false;
	}

#ifdef USE_SDL
	if (use_gamepad)
	{
		if (IsGamepadActionPressed(current_action))
			return true;
	}

	if (!keymap)
		return false;

	key = convert_tomb_keycode_to_sdl_scancode(keyboard_layout[1][current_action]);
#else 
	key = layout[1][number];
#endif

	if (key < keymap_count)
	{
		if (keymap[key])
			return true;

		switch (key)
		{
#ifdef USE_SDL
		case SDL_SCANCODE_RCTRL:
			return keymap[SDL_SCANCODE_LCTRL];

		case SDL_SCANCODE_LCTRL:
			return keymap[SDL_SCANCODE_RCTRL];

		case SDL_SCANCODE_RSHIFT:
			return keymap[SDL_SCANCODE_LSHIFT];

		case SDL_SCANCODE_LSHIFT:
			return keymap[SDL_SCANCODE_RSHIFT];

		case SDL_SCANCODE_RALT:
			return keymap[SDL_SCANCODE_LALT];

		case SDL_SCANCODE_LALT:
			return keymap[SDL_SCANCODE_RALT];
#else
		case DIK_RCONTROL:
			return keymap[DIK_LCONTROL];

		case DIK_LCONTROL:
			return keymap[DIK_RCONTROL];

		case DIK_RSHIFT:
			return keymap[DIK_LSHIFT];

		case DIK_LSHIFT:
			return keymap[DIK_RSHIFT];

		case DIK_RMENU:
			return keymap[DIK_LMENU];

		case DIK_LMENU:
			return keymap[DIK_RMENU];
#endif
		}
	}
#ifndef USE_SDL
	else if (joy_fire & (1 << key))
		return true;
#endif

	if (conflict[current_action])
		return false;

	key = keyboard_layout[0][current_action];

	if (keymap[key])
		return true;

	switch (key)
	{
#ifdef USE_SDL
	case SDL_SCANCODE_RCTRL:
		return keymap[SDL_SCANCODE_LCTRL];

	case SDL_SCANCODE_LCTRL:
		return keymap[SDL_SCANCODE_RCTRL];

	case SDL_SCANCODE_RSHIFT:
		return keymap[SDL_SCANCODE_LSHIFT];

	case SDL_SCANCODE_LSHIFT:
		return keymap[SDL_SCANCODE_RSHIFT];

	case SDL_SCANCODE_RALT:
		return keymap[SDL_SCANCODE_LALT];

	case SDL_SCANCODE_LALT:
		return keymap[SDL_SCANCODE_RALT];
	}
#else
	case DIK_RCONTROL:
		return keymap[DIK_LCONTROL];

	case DIK_LCONTROL:
		return keymap[DIK_RCONTROL];

	case DIK_RSHIFT:
		return keymap[DIK_LSHIFT];

	case DIK_LSHIFT:
		return keymap[DIK_RSHIFT];

	case DIK_RMENU:
		return keymap[DIK_LMENU];

	case DIK_LMENU:
		return keymap[DIK_RMENU];
	}
#endif

	return false;
}

long S_UpdateInput()
{
	static long LookCnt;
	static long med_hotkey_timer;
	short state;
	static bool flare_no_db = 0;
	bool debounce;

	debounce = SetDebounce;
#ifdef USE_SDL
	keymap = SDLReadKeyboard(keymap);
	if (!keymap) {
		return 0;
	}
#else
	DXReadKeyboard(keymap);
	if (appIsUnfocused) {
		memset(&keymap, 0, 256);
	}
#endif

#ifndef USE_SDL
	if (ControlMethod == 1)
		joy_fire = ReadJoystick(joy_x, joy_y);
#endif

	linput = 0;

#ifndef USE_SDL
	if (ControlMethod == 1)
	{
		if (joy_x < -8)
			linput = IN_LEFT;
		else if (joy_x > 8)
			linput = IN_RIGHT;

		if (joy_y > 8)
			linput |= IN_BACK;
		else if (joy_y < -8)
			linput |= IN_FORWARD;
	}
#endif

	if (IsActionPressed(INPUT_ACTION_FORWARD))
		linput |= IN_FORWARD;

	if (IsActionPressed(INPUT_ACTION_BACK))
		linput |= IN_BACK;

	if (IsActionPressed(INPUT_ACTION_LEFT))
		linput |= IN_LEFT;

	if (IsActionPressed(INPUT_ACTION_RIGHT))
		linput |= IN_RIGHT;

	if (IsActionPressed(INPUT_ACTION_DUCK))
		linput |= IN_DUCK;

	if (IsActionPressed(INPUT_ACTION_SPRINT))
		linput |= IN_SPRINT;

	if (IsActionPressed(INPUT_ACTION_WALK))
		linput |= IN_WALK;

	if (IsActionPressed(INPUT_ACTION_JUMP))
		linput |= IN_JUMP;

	if (IsActionPressed(INPUT_ACTION_INTERACT))
		linput |= IN_SELECT | IN_ACTION;

	if (IsActionPressed(INPUT_ACTION_DRAW))
		linput |= IN_DRAW;

	if (IsActionPressed(INPUT_ACTION_FLARE))
		linput |= IN_FLARE;

	if (IsActionPressed(INPUT_ACTION_LOOK))
		linput |= IN_LOOK;

	if (IsActionPressed(INPUT_ACTION_ROLL))
		linput |= IN_ROLL;

	if (IsActionPressed(INPUT_ACTION_OPTION))
		linput |= IN_OPTION;

	if (IsActionPressed(INPUT_ACTION_SIDESTEP_LEFT))
		linput |= IN_WALK | IN_LEFT;

	if (IsActionPressed(INPUT_ACTION_SIDESTEP_RIGHT))
		linput |= IN_WALK | IN_RIGHT;

	if (IsActionPressed(INPUT_ACTION_PAUSE))
		linput |= IN_PAUSE;

	if (IsActionPressed(INPUT_ACTION_SELECT))
		linput |= IN_SELECT;

	if (IsActionPressed(INPUT_ACTION_UNSELECT))
		linput |= IN_DESELECT;

	linput = NGValidateAgainstBlockedInput(linput);
	linput = NGApplySimulatedInput(linput);

	if (linput & IN_FLARE) {
		linput &= ~IN_FLARE;

		if (!flare_no_db)
		{
			state = lara_item->current_anim_state;

			if (state == AS_ALL4S || state == AS_CRAWL || state == AS_ALL4TURNL ||
				state == AS_ALL4TURNR || state == AS_CRAWLBACK || state == AS_CRAWL2HANG)
			{
				SoundEffect(SFX_LARA_NO, 0, SFX_ALWAYS);
				flare_no_db = 1;
			}
			else
			{
				linput |= IN_FLARE;
				flare_no_db = 0;
			}
		}
	} else {
		flare_no_db = 0;
	}

	if (lara.gun_status == LG_READY)
	{
		savegame.AutoTarget = (uchar)App.AutoTarget;

		if (linput & IN_LOOK)
		{
			if (LookCnt >= 6)
				LookCnt = 100;
			else
			{
				linput &= ~IN_LOOK;
				LookCnt++;
			}
		}
		else
		{
			if (LookCnt && LookCnt != 100)
				linput |= IN_TARGET;

			LookCnt = 0;
		}
	}

	if (NGValidateInputWeaponHotkeys())
		DoWeaponHotkey();

	if (IsKeyPressed(T4P_KEY_0))
	{
		if (!med_hotkey_timer)
		{
			if (lara_item->hit_points > 0 && lara_item->hit_points < 1000 || lara.poisoned)
			{
				if (lara.num_small_medipack)
				{
					if (lara.num_small_medipack != -1)
						lara.num_small_medipack--;

					lara.dpoisoned = 0;
					lara_item->hit_points += 500;
					SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
					savegame.Game.HealthUsed++;
					NGSetUsedSmallMedipack();

					if (lara_item->hit_points > 1000)
						lara_item->hit_points = 1000;

					if (InventoryActive && !lara.num_small_medipack)
					{
						construct_object_list();

						if (lara.num_large_medipack)
							setup_objectlist_startposition(INV_BIGMEDI_ITEM);
						else
							setup_objectlist_startposition(INV_MEMCARD_LOAD_ITEM);
					}

					med_hotkey_timer = 15;
				}
			}
		}
	}

	if (IsKeyPressed(T4P_KEY_9))
	{
		if (!med_hotkey_timer)
		{
			if (lara_item->hit_points > 0 && lara_item->hit_points < 1000 || lara.poisoned)
			{
				if (lara.num_large_medipack)
				{
					if (lara.num_large_medipack != -1)
						lara.num_large_medipack--;

					lara.dpoisoned = 0;
					lara_item->hit_points = 1000;
					SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
					savegame.Game.HealthUsed++;
					NGSetUsedLargeMedipack();

					med_hotkey_timer = 15;

					if (InventoryActive && !lara.num_large_medipack)
					{
						construct_object_list();

						if (lara.num_small_medipack)
							setup_objectlist_startposition(INV_SMALLMEDI_ITEM);
						else
							setup_objectlist_startposition(INV_MEMCARD_LOAD_ITEM);
					}
				}
			}
		}
	}
	else if (med_hotkey_timer)
		med_hotkey_timer--;

	if (linput & IN_WALK && !(linput & (IN_FORWARD | IN_BACK)))
	{
		if (linput & IN_LEFT)
			linput = (linput & ~IN_LEFT) | IN_LSTEP;
		else if (linput & IN_RIGHT)
			linput = (linput & ~IN_RIGHT) | IN_RSTEP;
	}

	if (linput & IN_FORWARD && linput & IN_BACK)
		linput |= IN_ROLL;

	if (linput & IN_ROLL && BinocularRange)
		linput &= ~IN_ROLL;

	if ((linput & (IN_RIGHT | IN_LEFT)) == (IN_RIGHT | IN_LEFT))
		linput -= IN_RIGHT | IN_LEFT;

	if (debounce)
		dbinput = inputBusy;

	if (gfGameMode == GF_GAME_MODE_LEVEL && Gameflow->LoadSaveEnabled)
	{
		if (IsKeyPressed(T4P_KEY_F5)) {
			if (NGValidateInputSavegame()) {
				linput |= IN_SAVE;
			}
		}
		
		if (NGIsSimulatingInputSavegame()) {
			linput |= IN_SAVE;
		}

		if (IsKeyPressed(T4P_KEY_F6)) {
			if (NGValidateInputLoadgame()) {
				linput |= IN_LOAD;
			}
		}
		
		if (NGIsSimulatingInputLoadgame()) {
			linput |= IN_LOAD;
		}
	}


#ifndef USE_BGFX
#if 0
	if (IsKeyPressed(T4P_KEY_APOSTROPHE))
		DXSaveScreen(App.dx.lpBackBuffer, "Tomb");
#endif
#endif

	inputBusy = linput;

	if (lara.Busy)
	{
		linput &= IN_PAUSE | IN_LOOK | IN_OPTION | IN_RIGHT | IN_LEFT | IN_BACK | IN_FORWARD;

		if (linput & IN_FORWARD && linput & IN_BACK)
			linput ^= IN_BACK;
	}

	if (debounce)
		dbinput = inputBusy & (dbinput ^ inputBusy);

	NGClearSimulatedSingleInputForFrame();

	input = linput;
	return 1;
}

#ifndef USE_SDL
long ReadJoystick(long& x, long& y)
{
	JOYINFOEX joystick;
	static JOYCAPS caps;
	static long unavailable = 1;

	joystick.dwSize = sizeof(JOYINFOEX);
	joystick.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNBUTTONS;

	if (joyGetPosEx(0, &joystick) != JOYERR_NOERROR)
	{
		unavailable = 1;
		x = 0;
		y = 0;
		return 0;
	}

	if (unavailable)
	{
		if (joyGetDevCaps(JOYSTICKID1, &caps, sizeof(caps)) != JOYERR_NOERROR)
		{
			x = 0;
			y = 0;
			return 0;
		}
		else
			unavailable = 0;
	}

	x = (joystick.dwXpos << 5) / (caps.wXmax - caps.wXmin) - 16;
	y = (joystick.dwYpos << 5) / (caps.wYmax - caps.wYmin) - 16;
	return joystick.dwButtons;
}
#endif
