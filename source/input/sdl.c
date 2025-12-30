#ifdef AE_USE_SDL2
#include <SDL2/SDL.h>

#include "sdl.h"
#include "../input.h"
#include "../keyboard.h"

Key Input_SDLScancodeToKey(SDL_Scancode key) {
	switch (key) {
		case SDL_SCANCODE_A:             return AE_KEY_A;
		case SDL_SCANCODE_B:             return AE_KEY_B;
		case SDL_SCANCODE_C:             return AE_KEY_C;
		case SDL_SCANCODE_D:             return AE_KEY_D;
		case SDL_SCANCODE_E:             return AE_KEY_E;
		case SDL_SCANCODE_F:             return AE_KEY_F;
		case SDL_SCANCODE_G:             return AE_KEY_G;
		case SDL_SCANCODE_H:             return AE_KEY_H;
		case SDL_SCANCODE_I:             return AE_KEY_I;
		case SDL_SCANCODE_J:             return AE_KEY_J;
		case SDL_SCANCODE_K:             return AE_KEY_K;
		case SDL_SCANCODE_L:             return AE_KEY_L;
		case SDL_SCANCODE_M:             return AE_KEY_M;
		case SDL_SCANCODE_N:             return AE_KEY_N;
		case SDL_SCANCODE_O:             return AE_KEY_O;
		case SDL_SCANCODE_P:             return AE_KEY_P;
		case SDL_SCANCODE_Q:             return AE_KEY_Q;
		case SDL_SCANCODE_R:             return AE_KEY_R;
		case SDL_SCANCODE_S:             return AE_KEY_S;
		case SDL_SCANCODE_T:             return AE_KEY_T;
		case SDL_SCANCODE_U:             return AE_KEY_U;
		case SDL_SCANCODE_V:             return AE_KEY_V;
		case SDL_SCANCODE_W:             return AE_KEY_W;
		case SDL_SCANCODE_X:             return AE_KEY_X;
		case SDL_SCANCODE_Y:             return AE_KEY_Y;
		case SDL_SCANCODE_Z:             return AE_KEY_Z;
		case SDL_SCANCODE_0:             return AE_KEY_0;
		case SDL_SCANCODE_1:             return AE_KEY_1;
		case SDL_SCANCODE_2:             return AE_KEY_2;
		case SDL_SCANCODE_3:             return AE_KEY_3;
		case SDL_SCANCODE_4:             return AE_KEY_4;
		case SDL_SCANCODE_5:             return AE_KEY_5;
		case SDL_SCANCODE_6:             return AE_KEY_6;
		case SDL_SCANCODE_7:             return AE_KEY_7;
		case SDL_SCANCODE_8:             return AE_KEY_8;
		case SDL_SCANCODE_9:             return AE_KEY_9;
		case SDL_SCANCODE_RETURN:        return AE_KEY_RETURN;
		case SDL_SCANCODE_ESCAPE:        return AE_KEY_ESCAPE;
		case SDL_SCANCODE_BACKSPACE:     return AE_KEY_BACKSPACE;
		case SDL_SCANCODE_TAB:           return AE_KEY_TAB;
		case SDL_SCANCODE_SPACE:         return AE_KEY_SPACE;
		case SDL_SCANCODE_MINUS:         return AE_KEY_MINUS;
		case SDL_SCANCODE_EQUALS:        return AE_KEY_EQUALS;
		case SDL_SCANCODE_LEFTBRACKET:   return AE_KEY_LEFT_BRACKET;
		case SDL_SCANCODE_RIGHTBRACKET:  return AE_KEY_RIGHT_BRACKET;
		case SDL_SCANCODE_BACKSLASH:     return AE_KEY_BACKSLASH;
		case SDL_SCANCODE_SEMICOLON:     return AE_KEY_SEMICOLON;
		case SDL_SCANCODE_APOSTROPHE:    return AE_KEY_APOSTROPHE;
		case SDL_SCANCODE_GRAVE:         return AE_KEY_GRAVE;
		case SDL_SCANCODE_COMMA:         return AE_KEY_COMMA;
		case SDL_SCANCODE_PERIOD:        return AE_KEY_PERIOD;
		case SDL_SCANCODE_SLASH:         return AE_KEY_SLASH;
		case SDL_SCANCODE_CAPSLOCK:      return AE_KEY_CAPS_LOCK;
		case SDL_SCANCODE_F1:            return AE_KEY_F1;
		case SDL_SCANCODE_F2:            return AE_KEY_F2;
		case SDL_SCANCODE_F3:            return AE_KEY_F3;
		case SDL_SCANCODE_F4:            return AE_KEY_F4;
		case SDL_SCANCODE_F5:            return AE_KEY_F5;
		case SDL_SCANCODE_F6:            return AE_KEY_F6;
		case SDL_SCANCODE_F7:            return AE_KEY_F7;
		case SDL_SCANCODE_F8:            return AE_KEY_F8;
		case SDL_SCANCODE_F9:            return AE_KEY_F9;
		case SDL_SCANCODE_F10:           return AE_KEY_F10;
		case SDL_SCANCODE_F11:           return AE_KEY_F11;
		case SDL_SCANCODE_F12:           return AE_KEY_F12;
		case SDL_SCANCODE_PRINTSCREEN:   return AE_KEY_PRINT_SCREEN;
		case SDL_SCANCODE_SCROLLLOCK:    return AE_KEY_SCROLL_LOCK;
		case SDL_SCANCODE_PAUSE:         return AE_KEY_PAUSE;
		case SDL_SCANCODE_INSERT:        return AE_KEY_INSERT;
		case SDL_SCANCODE_HOME:          return AE_KEY_HOME;
		case SDL_SCANCODE_PAGEUP:        return AE_KEY_PAGE_UP;
		case SDL_SCANCODE_PAGEDOWN:      return AE_KEY_PAGE_DOWN;
		case SDL_SCANCODE_DELETE:        return AE_KEY_DELETE;
		case SDL_SCANCODE_END:           return AE_KEY_END;
		case SDL_SCANCODE_RIGHT:         return AE_KEY_RIGHT;
		case SDL_SCANCODE_LEFT:          return AE_KEY_LEFT;
		case SDL_SCANCODE_DOWN:          return AE_KEY_DOWN;
		case SDL_SCANCODE_UP:            return AE_KEY_UP;
		case SDL_SCANCODE_NUMLOCKCLEAR:  return AE_KEY_NUM_LOCK;
		case SDL_SCANCODE_KP_DIVIDE:     return AE_KEY_NP_DIVIDE;
		case SDL_SCANCODE_KP_MULTIPLY:   return AE_KEY_NP_MULTIPLY;
		case SDL_SCANCODE_KP_MINUS:      return AE_KEY_NP_MINUS;
		case SDL_SCANCODE_KP_PLUS:       return AE_KEY_NP_PLUS;
		case SDL_SCANCODE_KP_ENTER:      return AE_KEY_NP_ENTER;
		case SDL_SCANCODE_KP_0:          return AE_KEY_NP_0;
		case SDL_SCANCODE_KP_1:          return AE_KEY_NP_1;
		case SDL_SCANCODE_KP_2:          return AE_KEY_NP_2;
		case SDL_SCANCODE_KP_3:          return AE_KEY_NP_3;
		case SDL_SCANCODE_KP_4:          return AE_KEY_NP_4;
		case SDL_SCANCODE_KP_5:          return AE_KEY_NP_5;
		case SDL_SCANCODE_KP_6:          return AE_KEY_NP_6;
		case SDL_SCANCODE_KP_7:          return AE_KEY_NP_7;
		case SDL_SCANCODE_KP_8:          return AE_KEY_NP_8;
		case SDL_SCANCODE_KP_9:          return AE_KEY_NP_9;
		case SDL_SCANCODE_KP_PERIOD:     return AE_KEY_NP_PERIOD;
		default: return 0;
	}
}

SDL_Scancode Input_KeyToSDLScancode(Key key) {
	switch (key) {
		case AE_KEY_A:             return SDL_SCANCODE_A;
		case AE_KEY_B:             return SDL_SCANCODE_B;
		case AE_KEY_C:             return SDL_SCANCODE_C;
		case AE_KEY_D:             return SDL_SCANCODE_D;
		case AE_KEY_E:             return SDL_SCANCODE_E;
		case AE_KEY_F:             return SDL_SCANCODE_F;
		case AE_KEY_G:             return SDL_SCANCODE_G;
		case AE_KEY_H:             return SDL_SCANCODE_H;
		case AE_KEY_I:             return SDL_SCANCODE_I;
		case AE_KEY_J:             return SDL_SCANCODE_J;
		case AE_KEY_K:             return SDL_SCANCODE_K;
		case AE_KEY_L:             return SDL_SCANCODE_L;
		case AE_KEY_M:             return SDL_SCANCODE_M;
		case AE_KEY_N:             return SDL_SCANCODE_N;
		case AE_KEY_O:             return SDL_SCANCODE_O;
		case AE_KEY_P:             return SDL_SCANCODE_P;
		case AE_KEY_Q:             return SDL_SCANCODE_Q;
		case AE_KEY_R:             return SDL_SCANCODE_R;
		case AE_KEY_S:             return SDL_SCANCODE_S;
		case AE_KEY_T:             return SDL_SCANCODE_T;
		case AE_KEY_U:             return SDL_SCANCODE_U;
		case AE_KEY_V:             return SDL_SCANCODE_V;
		case AE_KEY_W:             return SDL_SCANCODE_W;
		case AE_KEY_X:             return SDL_SCANCODE_X;
		case AE_KEY_Y:             return SDL_SCANCODE_Y;
		case AE_KEY_Z:             return SDL_SCANCODE_Z;
		case AE_KEY_0:             return SDL_SCANCODE_0;
		case AE_KEY_1:             return SDL_SCANCODE_1;
		case AE_KEY_2:             return SDL_SCANCODE_2;
		case AE_KEY_3:             return SDL_SCANCODE_3;
		case AE_KEY_4:             return SDL_SCANCODE_4;
		case AE_KEY_5:             return SDL_SCANCODE_5;
		case AE_KEY_6:             return SDL_SCANCODE_6;
		case AE_KEY_7:             return SDL_SCANCODE_7;
		case AE_KEY_8:             return SDL_SCANCODE_8;
		case AE_KEY_9:             return SDL_SCANCODE_9;
		case AE_KEY_RETURN:        return SDL_SCANCODE_RETURN;
		case AE_KEY_ESCAPE:        return SDL_SCANCODE_ESCAPE;
		case AE_KEY_BACKSPACE:     return SDL_SCANCODE_BACKSPACE;
		case AE_KEY_TAB:           return SDL_SCANCODE_TAB;
		case AE_KEY_SPACE:         return SDL_SCANCODE_SPACE;
		case AE_KEY_MINUS:         return SDL_SCANCODE_MINUS;
		case AE_KEY_EQUALS:        return SDL_SCANCODE_EQUALS;
		case AE_KEY_LEFT_BRACKET:  return SDL_SCANCODE_LEFTBRACKET;
		case AE_KEY_RIGHT_BRACKET: return SDL_SCANCODE_RIGHTBRACKET;
		case AE_KEY_BACKSLASH:     return SDL_SCANCODE_BACKSLASH;
		case AE_KEY_SEMICOLON:     return SDL_SCANCODE_SEMICOLON;
		case AE_KEY_APOSTROPHE:    return SDL_SCANCODE_APOSTROPHE;
		case AE_KEY_GRAVE:         return SDL_SCANCODE_GRAVE;
		case AE_KEY_COMMA:         return SDL_SCANCODE_COMMA;
		case AE_KEY_PERIOD:        return SDL_SCANCODE_PERIOD;
		case AE_KEY_SLASH:         return SDL_SCANCODE_SLASH;
		case AE_KEY_CAPS_LOCK:     return SDL_SCANCODE_CAPSLOCK;
		case AE_KEY_F1:            return SDL_SCANCODE_F1;
		case AE_KEY_F2:            return SDL_SCANCODE_F2;
		case AE_KEY_F3:            return SDL_SCANCODE_F3;
		case AE_KEY_F4:            return SDL_SCANCODE_F4;
		case AE_KEY_F5:            return SDL_SCANCODE_F5;
		case AE_KEY_F6:            return SDL_SCANCODE_F6;
		case AE_KEY_F7:            return SDL_SCANCODE_F7;
		case AE_KEY_F8:            return SDL_SCANCODE_F8;
		case AE_KEY_F9:            return SDL_SCANCODE_F9;
		case AE_KEY_F10:           return SDL_SCANCODE_F10;
		case AE_KEY_F11:           return SDL_SCANCODE_F11;
		case AE_KEY_F12:           return SDL_SCANCODE_F12;
		case AE_KEY_PRINT_SCREEN:  return SDL_SCANCODE_PRINTSCREEN;
		case AE_KEY_SCROLL_LOCK:   return SDL_SCANCODE_SCROLLLOCK;
		case AE_KEY_PAUSE:         return SDL_SCANCODE_PAUSE;
		case AE_KEY_INSERT:        return SDL_SCANCODE_INSERT;
		case AE_KEY_HOME:          return SDL_SCANCODE_HOME;
		case AE_KEY_PAGE_UP:       return SDL_SCANCODE_PAGEUP;
		case AE_KEY_PAGE_DOWN:     return SDL_SCANCODE_PAGEDOWN;
		case AE_KEY_DELETE:        return SDL_SCANCODE_DELETE;
		case AE_KEY_END:           return SDL_SCANCODE_END;
		case AE_KEY_RIGHT:         return SDL_SCANCODE_RIGHT;
		case AE_KEY_LEFT:          return SDL_SCANCODE_LEFT;
		case AE_KEY_DOWN:          return SDL_SCANCODE_DOWN;
		case AE_KEY_UP:            return SDL_SCANCODE_UP;
		case AE_KEY_NUM_LOCK:      return SDL_SCANCODE_NUMLOCKCLEAR;
		case AE_KEY_NP_DIVIDE:     return SDL_SCANCODE_KP_DIVIDE;
		case AE_KEY_NP_MULTIPLY:   return SDL_SCANCODE_KP_MULTIPLY;
		case AE_KEY_NP_MINUS:      return SDL_SCANCODE_KP_MINUS;
		case AE_KEY_NP_PLUS:       return SDL_SCANCODE_KP_PLUS;
		case AE_KEY_NP_ENTER:      return SDL_SCANCODE_KP_ENTER;
		case AE_KEY_NP_0:          return SDL_SCANCODE_KP_0;
		case AE_KEY_NP_1:          return SDL_SCANCODE_KP_1;
		case AE_KEY_NP_2:          return SDL_SCANCODE_KP_2;
		case AE_KEY_NP_3:          return SDL_SCANCODE_KP_3;
		case AE_KEY_NP_4:          return SDL_SCANCODE_KP_4;
		case AE_KEY_NP_5:          return SDL_SCANCODE_KP_5;
		case AE_KEY_NP_6:          return SDL_SCANCODE_KP_6;
		case AE_KEY_NP_7:          return SDL_SCANCODE_KP_7;
		case AE_KEY_NP_8:          return SDL_SCANCODE_KP_8;
		case AE_KEY_NP_9:          return SDL_SCANCODE_KP_9;
		case AE_KEY_NP_PERIOD:     return SDL_SCANCODE_KP_PERIOD;
		default: return 0;
	}
}

bool Input_KeyPressed(Key key) {
	const uint8_t* keyState = SDL_GetKeyboardState(NULL);

	return keyState[Input_KeyToSDLScancode(key)]? true : false;
}

bool Input_MouseButtonPressed(uint8_t button) {
	int x, y;

	return (SDL_GetMouseState(&x, &y) & SDL_BUTTON(button))? true : false;
}

#endif
