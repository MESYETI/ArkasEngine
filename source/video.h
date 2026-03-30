#ifndef AE_VIDEO_H
#define AE_VIDEO_H

#include "common.h"
#include "window.h"

#undef main

typedef struct {
	uint8_t r, g, b, a;
} Colour;

typedef struct {
	Window window;
	int    width, height;
	int    aWidth, aHeight;
} Video;

extern Video video;

void   Video_Init(const char* gameName);
void   Video_Free(void);
Colour Video_MultiplyColour(Colour colour, float by);

#endif
