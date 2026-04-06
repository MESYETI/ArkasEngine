#include "../event.h"
#include "../video.h"
#include "../config.h"
#include "../input/sdl.h"

#ifdef AE_EVENT_SDL2

void Event_PrepareExternal(void) {
	
}

static int GetWindow(uint32_t id) {
	for (int i = 0; i < WIN_NUM; ++ i) {
		if (SDL_GetWindowID(video.windows[i].window) == id) {
			return i;
		}
	}

	return -1;
}

bool Event_PollExternal(Event* event) {
	SDL_Event e;

	if (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_MOUSEMOTION: {
				event->mouseMove = (Event_MouseMove) {
					.type   = AE_EVENT_MOUSE_MOVE,
					.window = GetWindow(e.motion.windowID),
					.x      = e.motion.x / globalConfig.scale2D,
					.y      = e.motion.y / globalConfig.scale2D,
					.xRel   = e.motion.xrel / globalConfig.scale2D,
					.yRel   = e.motion.yrel / globalConfig.scale2D
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

				event->mouseButton = (Event_MouseButton) {
					.type   = e.type == SDL_MOUSEBUTTONDOWN?
						AE_EVENT_MOUSE_BUTTON_DOWN : AE_EVENT_MOUSE_BUTTON_UP,
					.window = GetWindow(e.button.windowID),
					.button = button,
					.x      = e.button.x / globalConfig.scale2D,
					.y      = e.button.y / globalConfig.scale2D
				};
				break;
			}
			case SDL_KEYUP:
			case SDL_KEYDOWN: {
				event->key = (Event_Key) {
					.type = e.type == SDL_KEYDOWN?
						AE_EVENT_KEY_DOWN : AE_EVENT_KEY_UP,
					.key  = Input_SDLScancodeToKey(e.key.keysym.scancode)
				};
				break;
			}
			case SDL_QUIT: {
				event->type = AE_EVENT_QUIT;
				break;
			}
			case SDL_WINDOWEVENT: {
				if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
					event->windowResize = (Event_WindowResize) {
						.type   = AE_EVENT_WINDOW_RESIZE,
						.window = GetWindow(e.button.windowID),
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

				event->textInput = textInput;
				break;
			}
			default: break;
		}

		return true;
	}

	return false;
}

void Event_StartTextInput(void) {
	SDL_StartTextInput();
}

void Event_StopTextInput(void) {
	SDL_StopTextInput();
}

#endif
