#include "../platform.h"

#ifdef PLATFORM_3DS
#include <3ds.h>

void Platform_Init(void) {
	
}

void Platform_Quit(void) {
	
}

uint64_t Platform_GetTime(void) {
	return osGetTime() * 1000;
}

#endif
