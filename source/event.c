#include <string.h>
#include "mem.h"
#include "util.h"
#include "event.h"
#include "config.h"
#include "input/sdl.h"

Event events[EVENTS_AMOUNT];

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
	Event e;

	while (Event_PollExternal(&e)) {
		events[FindFree()] = e;

		for (size_t i = 0; i < handlerNum; ++ i) {
			if (handlers[i].type == e.type) {
				handlers[i].func(&e);
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

void Event_AddHandler(Event_Type type, Event_Handler func) {
	Handler handler = (Handler) {type, func};

	++ handlerNum;
	handlers = SafeRealloc(handlers, handlerNum * sizeof(Handler));

	handlers[handlerNum - 1] = handler;
}
