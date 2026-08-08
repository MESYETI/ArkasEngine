#include <assert.h>
#include <string.h>
#include "mem.h"
#include "log.h"
#include "profiler.h"
#include "platform.h"

void Profiler_Init(Profiler* profiler, uint64_t timers) {
	profiler->times    = SafeMalloc(timers * sizeof(uint64_t));
	profiler->timesNum = (size_t) timers;
}

void Profiler_Free(Profiler* profiler) {
	if (profiler->times) {
		free(profiler->times);
		profiler->times = NULL;
	}

	profiler->timesNum = 0;
}

void Profiler_BeginTimer(Profiler* profiler, uint64_t idx) {
	assert(((size_t) idx) < profiler->timesNum);

	profiler->timeIdx = idx;
	profiler->timer   = Platform_GetTime();
}

void Profiler_FinishTimer(Profiler* profiler, uint64_t idx) {
	assert(((size_t) idx) < profiler->timesNum);

	uint64_t time = Platform_GetTime() - profiler->timer;

	profiler->times[idx] = time;
}

void Profiler_PrintToLog(Profiler* profiler, const char** columns) {
	Log("Profiler");
	Log("========");

	size_t maxLen = 0;

	for (size_t i = 0; i < profiler->timesNum; ++ i) {
		size_t len = strlen(columns[i]);

		if (len > maxLen) {
			maxLen = len;
		}
	}

	for (size_t i = 0; i < profiler->timesNum; ++ i) {
		double ms = ((double) profiler->times[i]) / 1000000;

		Log("%*s: %.9f", (int) maxLen, columns[i], ms);
	}
}
