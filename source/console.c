#include "app.h"
#include "util.h"
#include "text.h"
#include "safe.h"
#include "video.h"
#include "common.h"
#include "backend.h"
#include "console.h"
#include "commands.h"

Console console;

void Console_Init(void) {
	for (size_t i = 0; i < 100; ++ i) {
		console.lines[i][0] = 0;
	}

	console.cmds    = NULL;
	console.cmdsLen = 0;
	Commands_Init();
}

void Console_Free(void) {
	free(console.cmds);
}

void Console_WriteLine(char* text) {
	memcpy(&console.lines[1], &console.lines[0], 99 * sizeof(console.lines[0]));
	strncpy(console.lines[0], text, 100);
}

void Console_Begin(void) {
	memset(console.editor, 0, 100);
	SDL_StartTextInput();
}

void Console_End(void) {
	SDL_StopTextInput();
}

static char** ParseCommand(void) {
	char** res = SafeMalloc(1 * sizeof(char*));
	res[0]     = NULL;

	bool inString = false;
	static char reading[100];
	reading[0] = 0;

	for (size_t i = 0; i <= strlen(console.editor); ++ i) {
		if (inString) {
			switch (console.editor[i]) {
				case '"': {
					AppendStrArray(res, NewString(reading));
					reading[0] = 0;
					inString   = false;
					break;
				}
				case '\\': {
					++ i;
					if (i >= strlen(console.editor)) {
						free(res);
						return NULL;
					}

					char add;
					switch (console.editor[i]) {
						case '\\': add = '\\'; break;
						case '\"': add = '\"'; break;
						default: {
							free(res);
							return NULL;
						}
					}

					if (strlen(reading) >= 98) {
						free(res);
						return NULL;
					}
					strncat(reading, &add, 1);
					break;
				}
				default: {
					if (strlen(reading) >= 98) {
						free(res);
						return NULL;
					}

					strncat(reading, &console.editor[i], 1);
				}
			}
		}
		else {
			switch (console.editor[i]) {
				case '\0':
				case '\t':
				case ' ': {
					if (reading[0] != 0) {
						AppendStrArray(res, NewString(reading));
						reading[0] = 0;
					}
					break;
				}
				case '"': {
					inString = true;
					break;
				}
				default: {
					if (strlen(reading) >= 98) {
						free(res);
						return NULL;
					}

					strncat(reading, &console.editor[i], 1);
				}
			}
		}
	}

	return res;
}

static void RunCommand(void) {
	Log("> %s", console.editor);
	char** parts = ParseCommand();
	console.editor[0] = 0;

	if (parts == NULL) {
		Log("Invalid command");
		return;
	}

	for (size_t i = 0; i < console.cmdsLen; ++ i) {
		if (strcmp(console.cmds[i].name, parts[0]) == 0) {
			console.cmds[i].func(StrArrayLength(parts) - 1, &parts[1]);
			return;
		}
	}

	Log("Command '%s' not found", parts[0]);
	FreeStrArray(parts);
}

void Console_AddCommand(ConsoleCommand cmd) {
	console.cmds = SafeRealloc(
		console.cmds, (console.cmdsLen + 1) * sizeof(ConsoleCommand)
	);

	console.cmds[console.cmdsLen] = cmd;
	++ console.cmdsLen;
}

void Console_HandleEvent(SDL_Event* e) {
	switch (e->type) {
		case SDL_TEXTINPUT: {
			strncat(
				console.editor, e->text.text, 99 - strlen(console.editor)
			);
			break;
		}
		case SDL_KEYDOWN: {
			if (
				(e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE) &&
				(console.editor[0] != 0)
			) {
				console.editor[strlen(console.editor) - 1] = 0;
			}
			if (e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				RunCommand();
			}
			break;
		}
		default: break;
	}
}

void Console_Render(void) {
	Backend_Clear(0, 0, 0);
	Backend_Begin2D();

	// some issue where SDL2 would give the backtick as text input even though it
	// was pressed before text input was started
	if (strcmp(console.editor, "`") == 0) {
		console.editor[0] = 0;
	}

	int y = video.height - (app.font.charHeight * 2);

	for (size_t i = 0; i < 100; ++ i) {
		Text_Render(&app.font, console.lines[i], 1, y);
		y -= app.font.charHeight;

		if (y < -app.font.charHeight) {
			break;
		}
	}

	Text_Render(&app.font, "> ", 1, video.height - app.font.charHeight);

	Text_Render(
		&app.font, console.editor, 1 + (app.font.charWidth * 2),
		video.height - app.font.charHeight
	);
}
