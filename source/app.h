#ifndef AE_APP_H
#define AE_APP_H

#include "text.h"
#include "common.h"

typedef struct {
	bool  running;
	float delta;
	Font  font;
	bool  console;
} App;

extern App app;

void App_Init(void);
void App_Free(void);
void App_Update(void);

#endif
