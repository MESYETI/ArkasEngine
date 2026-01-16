#ifndef AE_APP_H
#define AE_APP_H

#include "text.h"
#include "common.h"

// TODO: rename to Engine
typedef struct {
	bool  running;
	float delta;
	Font  font;
	bool  console;
	int   fps;
} App;

extern App app;

void App_Init(void);
void App_Free(void);
void App_Update(void);

#endif
