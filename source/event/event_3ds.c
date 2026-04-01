#include "../event.h"
#include "../config.h"

#ifdef PLATFORM_3DS

bool Event_PollExternal(Event* event) {
	(void) event;
	return false;
}

void Event_StartTextInput(void) {
	
}

void Event_StopTextInput(void) {
	
}

#endif
