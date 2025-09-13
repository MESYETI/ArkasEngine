#ifndef AE_VIDEO_H
#define AE_VIDEO_H

#include <SDL2/SDL.h>
#include "common.h"

typedef struct {
	int x, y, w, h;
} Rect;

typedef struct {
	uint8_t r, g, b;
} Colour;

typedef struct {
	SDL_Window*   window;
	SDL_GLContext ctx;
	int           width, height;
} Video;

extern Video video;

void Video_Init(void);
void Video_Free(void);

#endif
