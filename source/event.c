#include <string.h>
#include <SDL2/SDL.h>
#include "mem.h"
#include "util.h"
#include "event.h"
#include "config.h"
#include "input/sdl.h"

#define EVENTS_AMOUNT 32

static Event events[EVENTS_AMOUNT];

typedef struct {
	Event_Type    type;
	Event_Handler func;
} Handler;

static Handler* handlers   = NULL;
static size_t   handlerNum = 0;

void Event_Init(void) {
	for (size_t i = 0; i < EVENTS_AMOUNT; ++ i) {
		events[i].type = AE_EVENT_NONE;
	}
}

void Event_Free(void) {
	if (handlers) {
		free(handlers);
	}
}

static int FindFree(void) {
	int i;

	for (i = 0; i < EVENTS_AMOUNT; ++ i) {
		if (events[i].type == AE_EVENT_NONE) return i;
	}

	for (i = 0; i < EVENTS_AMOUNT; ++ i) {
		printf("Event %d: %d\n", i, (int) events[i].type);
	}

	Error("Event pool full");
	return -1;
}

void Event_Add(Event e) {
	events[FindFree()] = e;
}

void Event_Update(void) {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		int idx = FindFree();

		switch (e.type) {
			case SDL_MOUSEMOTION: {
				events[idx].mouseMove = (Event_MouseMove) {
					.type = AE_EVENT_MOUSE_MOVE,
					.x    = e.motion.x / globalConfig.scale2D,
					.y    = e.motion.y / globalConfig.scale2D,
					.xRel = e.motion.xrel / globalConfig.scale2D,
					.yRel = e.motion.yrel / globalConfig.scale2D
				};
				break;
			}
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN: {
				uint8_t button;

				switch (e.button.button) {
					case SDL_BUTTON_LEFT:   button = 0; break;
					case SDL_BUTTON_MIDDLE: button = 1; break;
					case SDL_BUTTON_RIGHT:  button = 2; break;
					default:                button = 0;
				}

				events[idx].mouseButton = (Event_MouseButton) {
					.type   = e.type == SDL_MOUSEBUTTONDOWN?
						AE_EVENT_MOUSE_BUTTON_DOWN : AE_EVENT_MOUSE_BUTTON_UP,
					.button = button,
					.x      = e.button.x / globalConfig.scale2D,
					.y      = e.button.y / globalConfig.scale2D
				};
				break;
			}
			case SDL_KEYUP:
			case SDL_KEYDOWN: {
				events[idx].key = (Event_Key) {
					.type = e.type == SDL_KEYDOWN?
						AE_EVENT_KEY_DOWN : AE_EVENT_KEY_UP,
					.key  = Input_SDLScancodeToKey(e.key.keysym.scancode)
				};
				break;
			}
			case SDL_QUIT: {
				events[idx].type = AE_EVENT_QUIT;
				break;
			}
			case SDL_WINDOWEVENT: {
				if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
					events[idx].windowResize = (Event_WindowResize) {
						.type   = AE_EVENT_WINDOW_RESIZE,
						.width  = e.window.data1,
						.height = e.window.data2
					};
				}
				break;
			}
			case SDL_TEXTINPUT: {
				Event_TextInput textInput;
				textInput.type = AE_EVENT_TEXT_INPUT;
				strcpy(textInput.input, e.text.text);

				events[idx].textInput = textInput;
				break;
			}
			default: break;
		}

		if (idx != -1) if (events[idx].type != AE_EVENT_NONE) {
			for (size_t i = 0; i < handlerNum; ++ i) {
				if (handlers[i].type == events[idx].type) {
					handlers[i].func(&events[idx]);
				}
			}
		}
	}
}

bool Event_Available(void) {
	for (size_t i = 0; i < EVENTS_AMOUNT; ++ i) {
		if (events[i].type != AE_EVENT_NONE) {
			return true;
		}
	}

	return false;
}

bool Event_Poll(Event* e) {
	if (!Event_Available()) {
		Event_Update();
	}

	for (size_t i = 0; i < EVENTS_AMOUNT; ++ i) {
		if (events[i].type != AE_EVENT_NONE) {
			*e             = events[i];
			events[i].type = AE_EVENT_NONE;
			return true;
		}
	}

	return false;
}

void Event_StartTextInput(void) {
	SDL_StartTextInput();
}

void Event_StopTextInput(void) {
	SDL_StopTextInput();
}

void Event_AddHandler(Event_Type type, Event_Handler func) {
	Handler handler = (Handler) {type, func};

	++ handlerNum;
	handlers = SafeRealloc(handlers, handlerNum * sizeof(Handler));

	handlers[handlerNum - 1] = handler;
}
