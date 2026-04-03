#include "../event.h"
#include "../types.h"
#include "../config.h"

#ifdef PLATFORM_3DS
#include <3ds.h>

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
			AE_EVENT_MOUSE_BUTTON_UP, 0, touchPos.px, touchPos.py
		};
		return true;
	}

	return false;
}

void Event_StartTextInput(void) {
	
}

void Event_StopTextInput(void) {
	
}

#endif
