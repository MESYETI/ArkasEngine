#ifndef AE_ENGINE_H
#define AE_ENGINE_H

#include "text.h"
#include "common.h"

typedef struct {
	bool  running;
	float delta;
	Font  font;
	bool  console;
	int   fps;
	bool  server; // headless instance of Arkas Engine, running as a server
} Engine;

extern Engine engine;

void Engine_Init(const char* gameName, int argc, const char** argv);
void Engine_Free(void);
void Engine_Update(void);

#endif
