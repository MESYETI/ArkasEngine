#ifndef AE_WINDOW_SDL2_H
#define AE_WINDOW_SDL2_H

#include <SDL2/SDL.h>

typedef struct {
	WINDOW_COMMON;

	SDL_Window* window;
} Window;

#endif
