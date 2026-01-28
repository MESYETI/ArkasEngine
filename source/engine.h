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
} Engine;

extern Engine engine;

void Engine_Init(const char* gameName);
void Engine_Free(void);
void Engine_Update(void);

#endif
