#ifndef AE_CONSOLE_H
#define AE_CONSOLE_H

#include "util.h"
#include "event.h"
#include "video.h"

typedef struct {
	bool        show; // shown in help menu?
	const char* name;

	FUNCTION_POINTER(void, func, size_t argc, char** argv);
} ConsoleCommand;

typedef struct {
	char lines[100][100];
	char editor[100]; // user input

	ConsoleCommand* cmds;
	size_t          cmdsLen;
	bool            echo;
} Console;

extern Console console;

void Console_Init(void);
void Console_Free(void);
void Console_WriteLine(char* text);
void Console_Begin(void);
void Console_End(void);
void Console_AddCommand(ConsoleCommand cmd);
bool Console_RunFile(const char* path);
void Console_HandleEvent(Event* e);
void Console_Render(void);

#endif
