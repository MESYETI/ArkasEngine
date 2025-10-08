#include "app.h"
#include "util.h"
#include "text.h"
#include "mem.h"
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
	console.echo    = true;
	Commands_Init();
}

void Console_Free(void) {
	free(console.cmds);
}

static void ScrollUp(void) {
	memmove(&console.lines[1], &console.lines[0], 99 * sizeof(console.lines[0]));
}

void Console_WriteLine(char* text) {
	ScrollUp();
	int col = 0;

	for (size_t i = 0; i <= strlen(text); ++ i, ++ col) {
		if (text[i] == '\n') {
			console.lines[0][col] = 0;
			col = -1;
			ScrollUp();
		}
		else if (col < 100) {
			console.lines[0][col] = text[i];
		}
	}
}

void Console_Begin(void) {
	memset(console.editor, 0, 100);
	SDL_StartTextInput();
}

void Console_End(void) {
	SDL_StopTextInput();
}

// written by PQCraft after he was ragebaited by my C code
static char** ParseCommand(size_t* argcOut) {
	// USAGE:
	//   argv = ParseCommand(&argc);
	//   ...
	//   free(*argv);
	//   free(argv);

	char* in = console.editor;

	if (*in == '@') ++ in; // skip disable echo character

	char** resArray     = SafeMalloc(1 * sizeof(char*));
	size_t resArrayLen  = 0; // number of strings
	size_t resArraySize = 1; // buffer size

	char*  resString     = SafeMalloc(16);
	size_t resStringLen  = 0;  // data length
	size_t resStringSize = 16; // buffer size

	char   c            = *in;
	bool   inString     = false;
	size_t curStringPos = 0;

	if (!c) goto longBreak;
	while (c == ' ' || c == '\t') { // trim leading whitespace
		++ in;
		c = *in;
		if (!c) goto longBreak;
	}
	++ in;
	goto skipRead; // avoids an extra deref without making the while loop funky

	while (1) {
		c = *in;
		++ in;

		skipRead:
		if (inString) {
			if (c == '"') {
				inString = false;
				continue;
			}
			else if (c == '\\') {
				c = *in;
				++ in;

				switch (c) {
					case '"': break;
					case '\\': break;
					//case 'n': c = '\n'; break; // example escape sequence detection for '\n'

					// for invalid escapes, put down a backslash and then the char
					// (instead of erroring out)
					default: {
						c = '\\';
						-- in;
						break;
					}
				}
			}
			else if (!c) { // string wasn't terminated
				free(resArray);
				free(resString);
				*argcOut = -1;
				return NULL;
			}
		}
		else {
			if (!c) goto isNullChar;
			if (c == ' ' || c == '\t') {
				// gobble up extra whitespace
				c = *in;
				while ((c == ' ') || (c == '\t') || (c == '\n')) {
					++ in;
					c = *in;
				}

				isNullChar:
				if (resArrayLen == resArraySize) {
					resArraySize *= 2;
					resArray = SafeRealloc(resArray, resArraySize * sizeof(char*));
				}

				// mark down the location of the string in the buffer
				resArray[resArrayLen] = (char*) ((uintptr_t) curStringPos);
				++ resArrayLen;

				// add null terminator
				if (resStringLen == resStringSize) {
					resStringSize *= 2;
					resString      = SafeRealloc(resString, resStringSize);
				}

				resString[resStringLen] = 0;
				++ resStringLen;

				if (!c) break;

				curStringPos = resStringLen;
				continue;
			}
			else if (c == '"') {
				inString = true;
				continue;
			}
		}

		if (resStringLen == resStringSize) {
			resStringSize *= 2;
			resString      = SafeRealloc(resString, resStringSize);
		}

		resString[resStringLen] = c;
		++ resStringLen;
	}
	longBreak:

	if (resArrayLen != 0) {
		// downsize arrays
		if (resArrayLen != resArraySize) {
			resArray = SafeRealloc(resArray, resArrayLen * sizeof(char*));
		}
		if (resStringLen != resStringSize) {
			resString = SafeRealloc(resString, resStringLen);
		}

		for (size_t i = 0; i < resArrayLen; ++i) {
			// add the base pointer to all the strings
			resArray[i] += (uintptr_t) resString;
		}

		*argcOut = resArrayLen;
		return resArray;
	}

	free(resArray);
	free(resString);
	*argcOut = 0;
	return NULL;
}

static void RunCommand(void) {
	if (console.echo && (console.editor[0] != '@')) {
		Log("> %s", console.editor);
	}

	size_t argc;
	char** parts = ParseCommand(&argc);
	console.editor[0] = 0;

	if (argc == 0) return;

	if (parts == NULL) {
		Log("Bad syntax");
		return;
	}

	for (size_t i = 0; i < console.cmdsLen; ++ i) {
		if (strcmp(console.cmds[i].name, parts[0]) == 0) {
			console.cmds[i].func(argc - 1, &parts[1]);
			free(*parts);
			free(parts);
			return;
		}
	}

	Log("Command '%s' not found", parts[0]);
	free(*parts);
	free(parts);
}

void Console_AddCommand(ConsoleCommand cmd) {
	console.cmds = SafeRealloc(
		console.cmds, (console.cmdsLen + 1) * sizeof(ConsoleCommand)
	);

	console.cmds[console.cmdsLen] = cmd;
	++ console.cmdsLen;
}

bool Console_RunFile(const char* path) {
	FILE* file = fopen(path, "r");

	if (file == NULL) {
		Log("Failed to open '%s'", path);
		return false;
	}

	char oldEdit[100];
	memcpy(oldEdit, console.editor, 100);

	char* line;

	while (true) {
		line = fgets(console.editor, 100, file);

		if (line == NULL) break;
		line[strlen(line) - 1] = 0;

		RunCommand();
	}

	memcpy(console.editor, oldEdit, 100);

	return true;
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
