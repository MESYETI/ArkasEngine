#include <string.h>
#include "../event.h"
#include "../types.h"
#include "../config.h"

#ifdef PLATFORM_3DS
#include <3ds.h>

// touchscreen
static bool touchDown = false;
static Vec2 lastTouch;

void Event_PrepareExternal(void) {
	hidScanInput();
}

bool Event_PollExternal(Event* event) {
	// home button/power/whatever
	if (!aptMainLoop()) {
		event->type = AE_EVENT_QUIT;
		return true;
	}

	// touchscreen
	touchPosition touchPos;
	hidTouchRead(&touchPos);
	if ((touchPos.px != 0) || (touchPos.py != 0)) {
		Vec2 thisPos = (Vec2) {touchPos.px, touchPos.py};
		bool same    = (thisPos.x == lastTouch.x) && (thisPos.y == lastTouch.y);

		if (touchDown && !same) {
			event->mouseMove = (Event_MouseMove) {
				AE_EVENT_MOUSE_MOVE, touchPos.px, touchPos.py,
				thisPos.x - lastTouch.x, thisPos.y - lastTouch.y
			};
			lastTouch = thisPos;
			touchDown = true;
			return true;
		}
		else if (!touchDown) {
			event->mouseButton = (Event_MouseButton) {
				AE_EVENT_MOUSE_BUTTON_DOWN, 0, touchPos.px, touchPos.py
			};
			lastTouch = (Vec2) {-1, -1};
			touchDown = true;
			return true;
		}
	}
	else if (touchDown) {
		touchDown = false;
		event->mouseButton = (Event_MouseButton) {
			AE_EVENT_MOUSE_BUTTON_UP, 0, lastTouch.x, lastTouch.y
		};
		return true;
	}

	return false;
}

void Event_StartTextInput(void) {
	SwkbdState  keyboard;
	char        buf[60];
	SwkbdButton button;

	buf[0] = 0;

	swkbdInit(&keyboard, SWKBD_TYPE_NORMAL, 2, -1);
	swkbdSetInitialText(&keyboard, buf);
	swkbdSetHintText(&keyboard, "Enter text");
	swkbdSetButton(&keyboard, SWKBD_BUTTON_LEFT,  "Discard", false);
	swkbdSetButton(&keyboard, SWKBD_BUTTON_RIGHT, "OK", true);
	button = swkbdInputText(&keyboard, buf, sizeof(buf));

	if (button == SWKBD_BUTTON_RIGHT) {
		Event e;
		e.type  = AE_EVENT_TEXT_INPUT;
		strcpy(e.textInput.input, buf);
		Event_Add(e);
	}
}

void Event_StopTextInput(void) {
	
}

#endif
