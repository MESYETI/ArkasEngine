#ifndef AE_VIDEO_H
#define AE_VIDEO_H

#include "common.h"
#include "window.h"

#undef main

#ifdef PLATFORM_3DS
	#define WIN_NUM 2
#else
	#define WIN_NUM 1
#endif

typedef struct {
	uint8_t r, g, b, a;
} Colour;

typedef struct {
	Window windows[WIN_NUM];
} Video;

extern Video video;

void   Video_Init(const char* gameName);
void   Video_Free(void);
Colour Video_MultiplyColour(Colour colour, float by);

#endif
