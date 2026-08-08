#ifndef AE_ENGINE_H
#define AE_ENGINE_H

#include "text.h"
#include "input.h"
#include "common.h"
#include "profiler.h"

enum {
	ENGINE_PROF_EVENTS = 0,
	ENGINE_PROF_SERVER,
	ENGINE_PROF_CLIENT,
	ENGINE_PROF_SCENES,
	ENGINE_PROF_AUDIO,
	ENGINE_PROF_RENDER,

	ENGINE_PROF_NUM
};

typedef struct {
	bool     running;
	float    delta;
	Font     font;
	bool     console;
	int      fps;
	bool     server; // headless instance of Arkas Engine, running as a server
	Profiler profiler;

	Input_BindID profilerBind;
} Engine;

extern Engine engine;

void Engine_Init(const char* gameName, int argc, const char** argv);
void Engine_Free(void);
void Engine_PrintProfiler(void);
void Engine_Update(void);

#endif
