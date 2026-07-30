#include "../platform.h"

#if defined(PLATFORM_UNIX) || defined(PLATFORM_LINUX)
#include <time.h>
#include <unistd.h>

void Platform_Init(void) {
	
}

void Platform_Quit(void) {
	
}

uint64_t Platform_GetTime(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec * 1000000 + time.tv_nsec / 1000;
}

void Platform_Sleep(uint32_t ms) {
	struct timespec time;
	time.tv_sec  = 0;
	time.tv_nsec = ms * 1000000;

	nanosleep(&time, NULL);
}

#endif
