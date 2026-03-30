#include "../platform.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>

static UINT          timerRes = 1;
static LARGE_INTEGER perfFreq;

void Platform_Init(void) {
	// increase the timer res to the max to make timing stuff and waiting
	// more accurate
	TIMECAPS tc;

	if (timeGetDevCaps(&tc, sizeof(tc)) != TIMERR_NOERROR) {
	    if (timerRes < tc.wPeriodMin) {
	    	timerRes = tc.wPeriodMin;
	    }
	    else if (timerRes > tc.wPeriodMax) {
	    	timerRes = tc.wPeriodMax;
	    }
	}
	timeBeginPeriod(timerRes);

	LARGE_INTEGER perfFreq;
	uint64_t      perfMul = 1000000;
	QueryPerformanceFrequency(&perfFreq);
	while (!(perfFreq.QuadPart % 10) && !(perfMul % 10)) {
	    perfFreq.QuadPart /= 10;
	    perfMul           /= 10;
	}

	// put the timer res back to the original value before exiting
	timeEndPeriod(timerRes);
}

void Platform_Quit(void) {
	timeEndPeriod(timerRes);
}

uint64_t Platform_GetTime(void) {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart * perfctmul / perfFreq.QuadPart;
}

#endif
