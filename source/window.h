#ifndef AE_WINDOW_H
#define AE_WINDOW_H

#include "common.h"

#define WINDOW_COMMON \
	int width, height; \
	int aWidth, aHeight

#ifdef AE_WINDOW_SDL2
	#include "window/sdl2.h"
#elif defined(PLATFORM_3DS)
	#include "window/ae3ds.h"
#endif

enum {
	WINDOW_MSG_WARNING = 0,
	WINDOW_MSG_ERROR
};

void   Window_Init(void);
void   Window_Quit(void);
Window Window_Create(const char* name, int width, int height);
void   Window_Free(Window* window);
void   Window_ShowCursor(bool show);
void   Window_SetRelativeMouseMode(bool enable);
void   Window_MessageBox(int type, const char* title, const char* contents);

#endif
