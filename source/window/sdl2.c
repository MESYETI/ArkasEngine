#include "../util.h"
#include "../window.h"

#ifdef AE_WINDOW_SDL2
#include "sdl2.h"

static bool cursorVisible     = true;
static bool relativeMouseMode = false;

void Window_Init(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		Log("Failed to initialise SDL2: %s", SDL_GetError());
		exit(1);
	}
}

void Window_Quit(void) {
	SDL_Quit();
}

Window Window_Create(const char* name, int width, int height) {
	Window ret;
	ret.window = SDL_CreateWindow(
		name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (ret.window == NULL) {
		Log("Failed to create window: %s", SDL_GetError());
		exit(1);
	}

	return ret;
}

void Window_Free(Window* window) {
	SDL_DestroyWindow(window->window);
}

void Window_ShowCursor(bool show) {
	if (cursorVisible != show) {
		SDL_ShowCursor(show? SDL_ENABLE : SDL_DISABLE);
	}

	cursorVisible = show;
}

void Window_SetRelativeMouseMode(bool enable) {
	if (relativeMouseMode != enable) {
		SDL_SetRelativeMouseMode(enable? SDL_TRUE : SDL_FALSE);
	}

	relativeMouseMode = enable;
}

void Window_MessageBox(int type, const char* title, const char* contents) {
	uint32_t flags;
	switch (type) {
		case WINDOW_MSG_WARNING: flags = SDL_MESSAGEBOX_WARNING; break;
		case WINDOW_MSG_ERROR:   flags = SDL_MESSAGEBOX_ERROR;   break;
	}

	SDL_ShowSimpleMessageBox(flags, title, contents, NULL);
}

#endif
