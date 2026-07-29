#include <string.h>
#include "mem.h"
#include "util.h"
#include "event.h"
#include "config.h"
#include "input/sdl.h"

Event* events;
size_t eventsSize;

typedef struct {
	Event_Type    type;
	Event_Handler func;
} Handler;

static Handler* handlers   = NULL;
static size_t   handlerNum = 0;

void Event_Init(void) {
	events     = SafeMalloc(64 * sizeof(Event));
	eventsSize = 64;

	for (size_t i = 0; i < eventsSize; ++ i) {
		events[i].type = AE_EVENT_NONE;
	}
}

void Event_Free(void) {
	if (handlers) {
		free(handlers);
	}
}

static Event* FindFree(void) {
	int i;

	for (i = 0; i < eventsSize; ++ i) {
		if (events[i].type == AE_EVENT_NONE) return &events[i];
	}

	size_t old  = eventsSize;
	events      = SafeRealloc(events, eventsSize * 2 * sizeof(Event));
	eventsSize *= 2;

	for (size_t i = old; i < eventsSize; ++ i) {
		events[i].type = AE_EVENT_NONE;
	}

	return &events[old];
}

void Event_Add(Event e) {
	*FindFree() = e;
}

void Event_Update(void) {
	Event e;

	Event_PrepareExternal();
	while (Event_PollExternal(&e)) {
		*FindFree() = e;

		for (size_t i = 0; i < handlerNum; ++ i) {
			if (handlers[i].type == e.type) {
				handlers[i].func(&e);
			}
		}
	}
}

bool Event_Available(void) {
	for (size_t i = 0; i < eventsSize; ++ i) {
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

	for (size_t i = 0; i < eventsSize; ++ i) {
		if (events[i].type != AE_EVENT_NONE) {
			*e             = events[i];
			events[i].type = AE_EVENT_NONE;
			return true;
		}
	}

	return false;
}

void Event_AddHandler(Event_Type type, Event_Handler func) {
	Handler handler = (Handler) {type, func};

	++ handlerNum;
	handlers = SafeRealloc(handlers, handlerNum * sizeof(Handler));

	handlers[handlerNum - 1] = handler;
}
