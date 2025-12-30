#include <string.h>
#include <SDL2/SDL.h>
#include "input/sdl.h"
#include "event.h"
#include "util.h"

#define EVENTS_AMOUNT 32

Event events[EVENTS_AMOUNT];

void Event_Init(void) {
	for (size_t i = 0; i < EVENTS_AMOUNT; ++ i) {
		events[i].type = AE_EVENT_NONE;
	}
}

static int FindFree(void) {
	int i;

	for (i = 0; i < EVENTS_AMOUNT; ++ i) {
		if (events[i].type == AE_EVENT_NONE) return i;
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
		switch (e.type) {
			case SDL_MOUSEMOTION: {
				events[FindFree()].mouseMove = (Event_MouseMove) {
					.type = AE_EVENT_MOUSE_MOVE,
					.x    = e.motion.x,
					.y    = e.motion.y,
					.xRel = e.motion.xrel,
					.yRel = e.motion.yrel
				};
				break;
			}
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN: {
				events[FindFree()].mouseButton = (Event_MouseButton) {
					.type   = e.type == SDL_MOUSEBUTTONDOWN?
						AE_EVENT_MOUSE_BUTTON_DOWN : AE_EVENT_MOUSE_BUTTON_UP,
					.button = e.button.button,
					.x      = e.button.x,
					.y      = e.button.y
				};
				break;
			}
			case SDL_KEYUP:
			case SDL_KEYDOWN: {
				events[FindFree()].key = (Event_Key) {
					.type = e.type == SDL_KEYDOWN?
						AE_EVENT_KEY_DOWN : AE_EVENT_KEY_UP,
					.key  = Input_SDLScancodeToKey(e.key.keysym.scancode)
				};
				break;
			}
			case SDL_QUIT: {
				events[FindFree()].type = AE_EVENT_QUIT;
				break;
			}
			case SDL_WINDOWEVENT: {
				if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
					events[FindFree()].windowResize = (Event_WindowResize) {
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

				events[FindFree()].textInput = textInput;
				break;
			}
			default: break;
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
