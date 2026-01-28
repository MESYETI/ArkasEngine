#ifndef AE_VIDEO_H
#define AE_VIDEO_H

#include <SDL2/SDL.h>
#include "common.h"

#undef main

typedef struct {
	uint8_t r, g, b;
} Colour;

typedef struct {
	SDL_Window* window;
	int         width, height;
} Video;

extern Video video;

void   Video_Init(const char* gameName);
void   Video_Free(void);
Colour Video_MultiplyColour(Colour colour, float by);

#endif
