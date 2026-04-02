#include "../event.h"
#include "../config.h"

#ifdef PLATFORM_3DS
#include <3ds.h>

bool Event_PollExternal(Event* event) {
	if (!aptMainLoop()) {
		event->type = AE_EVENT_QUIT;
		return true;
	}
	return false;
}

void Event_StartTextInput(void) {
	
}

void Event_StopTextInput(void) {
	
}

#endif
