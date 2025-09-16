#ifndef AE_CONSOLE_H
#define AE_CONSOLE_H

#include "video.h"

typedef struct {
	char lines[100][100];
	char editor[100]; // user input
} Console;

extern Console console;

void Console_Init(void);
void Console_WriteLine(char* text);
void Console_Begin(void);
void Console_End(void);
void Console_HandleEvent(SDL_Event* e);
void Console_Render(void);

#endif
