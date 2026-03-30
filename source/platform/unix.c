#include "../platform.h"

#if defined(PLATFORM_UNIX) || defined(PLATFORM_LINUX)
#include <time.h>

void Platform_Init(void) {
	
}

void Platform_Quit(void) {
	
}

uint64_t Platform_GetTime(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec * 1000000 + time.tv_nsec / 1000;
}

#endif
