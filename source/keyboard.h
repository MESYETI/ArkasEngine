#ifndef AE_KEYBOARD_H
#define AE_KEYBOARD_H

#include <SDL2/SDL.h>

#include "common.h"

typedef enum {
	AE_KEY_NONE = 0,

	AE_KEY_A = 1,
	AE_KEY_B = 2,
	AE_KEY_C = 3,
	AE_KEY_D = 4,
	AE_KEY_E = 5,
	AE_KEY_F = 6,
	AE_KEY_G = 7,
	AE_KEY_H = 8,
	AE_KEY_I = 9,
	AE_KEY_J = 10,
	AE_KEY_K = 11,
	AE_KEY_L = 12,
	AE_KEY_M = 13,
	AE_KEY_N = 14,
	AE_KEY_O = 15,
	AE_KEY_P = 16,
	AE_KEY_Q = 17,
	AE_KEY_R = 18,
	AE_KEY_S = 19,
	AE_KEY_T = 20,
	AE_KEY_U = 21,
	AE_KEY_V = 22,
	AE_KEY_W = 23,
	AE_KEY_X = 24,
	AE_KEY_Y = 25,
	AE_KEY_Z = 26,

	AE_KEY_0 = 27,
	AE_KEY_1 = 28,
	AE_KEY_2 = 29,
	AE_KEY_3 = 30,
	AE_KEY_4 = 31,
	AE_KEY_5 = 32,
	AE_KEY_6 = 33,
	AE_KEY_7 = 34,
	AE_KEY_8 = 35,
	AE_KEY_9 = 36,

	AE_KEY_RETURN        = 37,
	AE_KEY_ESCAPE        = 38,
	AE_KEY_BACKSPACE     = 39,
	AE_KEY_TAB           = 40,
	AE_KEY_SPACE         = 41,
	AE_KEY_MINUS         = 42,
	AE_KEY_EQUALS        = 43,
	AE_KEY_LEFT_BRACKET  = 44,
	AE_KEY_RIGHT_BRACKET = 45,
	AE_KEY_BACKSLASH     = 46,
	AE_KEY_SEMICOLON     = 47,
	AE_KEY_APOSTROPHE    = 48,
	AE_KEY_GRAVE         = 49,
	AE_KEY_COMMA         = 50,
	AE_KEY_PERIOD        = 51,
	AE_KEY_SLASH         = 52,
	AE_KEY_CAPS_LOCK     = 53,

	AE_KEY_F1  = 54,
	AE_KEY_F2  = 55,
	AE_KEY_F3  = 56,
	AE_KEY_F4  = 57,
	AE_KEY_F5  = 58,
	AE_KEY_F6  = 59,
	AE_KEY_F7  = 60,
	AE_KEY_F8  = 61,
	AE_KEY_F9  = 62,
	AE_KEY_F10 = 63,
	AE_KEY_F11 = 64,
	AE_KEY_F12 = 65,

	AE_KEY_PRINT_SCREEN = 67,
	AE_KEY_SCROLL_LOCK  = 68,
	AE_KEY_PAUSE        = 69,
	AE_KEY_INSERT       = 70,

	AE_KEY_HOME      = 71,
	AE_KEY_PAGE_UP   = 72,
	AE_KEY_PAGE_DOWN = 73,
	AE_KEY_DELETE    = 74,
	AE_KEY_END       = 75,
	AE_KEY_RIGHT     = 76,
	AE_KEY_LEFT      = 78,
	AE_KEY_DOWN      = 79,
	AE_KEY_UP        = 80,

	AE_KEY_NUM_LOCK = 81,

	AE_KEY_NP_DIVIDE = 82,
	AE_KEY_NP_MULTIPLY = 83,
	AE_KEY_NP_MINUS    = 84,
	AE_KEY_NP_PLUS     = 85,
	AE_KEY_NP_ENTER    = 86,
	AE_KEY_NP_0        = 87,
	AE_KEY_NP_1        = 88,
	AE_KEY_NP_2        = 89,
	AE_KEY_NP_3        = 90,
	AE_KEY_NP_4        = 91,
	AE_KEY_NP_5        = 92,
	AE_KEY_NP_6        = 93,
	AE_KEY_NP_7        = 94,
	AE_KEY_NP_8        = 95,
	AE_KEY_NP_9        = 96,
	AE_KEY_NP_PERIOD   = 97,

	AE_KEY_COUNT
} AE_Key;

AE_Key AE_SDLScancodeToKey(SDL_Scancode key);

#endif
