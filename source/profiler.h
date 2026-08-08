#ifndef AE_PROFILER_H
#define AE_PROFILER_H

#include "common.h"

typedef struct {
	uint64_t* times;
	size_t    timesNum;

	uint64_t timer;
	uint64_t timeIdx;
} Profiler;

void Profiler_Init(Profiler* profiler, size_t timers);
void Profiler_Free(Profiler* profiler);
void Profiler_BeginTimer(Profiler* profiler, uint64_t idx);
void Profiler_FinishTimer(Profiler* profiler, uint64_t idx);
void Profiler_PrintToLog(Profiler* profiler, const char** columns);

#endif
