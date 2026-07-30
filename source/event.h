#ifndef AE_EVENT_H
#define AE_EVENT_H

#include "window.h"
#include "keyboard.h"

enum {
	// external events
	AE_EVENT_NONE              = 0,
	AE_EVENT_MOUSE_MOVE        = 1,
	AE_EVENT_MOUSE_BUTTON_DOWN = 2,
	AE_EVENT_MOUSE_BUTTON_UP   = 3,
	AE_EVENT_KEY_DOWN          = 4,
	AE_EVENT_KEY_UP            = 5,
	AE_EVENT_QUIT              = 6,
	AE_EVENT_WINDOW_RESIZE     = 7,
	AE_EVENT_TEXT_INPUT        = 8,

	// engine events
	AE_EVENT_ENTITY_SPAWN      = 9,
	AE_EVENT_ENTITY_REMOVED    = 10,
	AE_EVENT_CLIENT_MAP_LOADED = 11,
	AE_EVENT_CLIENT_KICKED     = 12
};

typedef uint8_t Event_Type;

typedef struct {
	Event_Type type;
	int        window;
	int        x;
	int        y;
	int        xRel;
	int        yRel;
} Event_MouseMove;

typedef struct {
	Event_Type type;
	int        window;
	uint8_t    button;
	int        x;
	int        y;
} Event_MouseButton;

typedef struct {
	Event_Type type;
	Key        key;
} Event_Key;

typedef struct {
	Event_Type type;
	int        window;
	int        width;
	int        height;
} Event_WindowResize;

typedef struct {
	Event_Type type;
	char       input[60];
} Event_TextInput;

typedef struct {
	Event_Type type;
	size_t     idx;
} Event_EntitySpawn;

typedef struct {
	Event_Type type;
	size_t     idx;
} Event_EntityRemoved;

typedef struct {
	Event_Type type;
	char       message[257];
} Event_ClientKick;

typedef union {
	Event_Type         type;
	Event_MouseMove    mouseMove;
	Event_MouseButton  mouseButton;
	Event_Key          key;
	Event_WindowResize windowResize;
	Event_TextInput    textInput;
	Event_ClientKick   clientKick;
} Event;

extern Event* events;
extern size_t eventsSize;

typedef void (*Event_Handler)(Event* event);

void Event_Init(void);
void Event_Free(void);
void Event_Add(Event* e);
void Event_Update(void);
void Event_PrepareExternal(void);
bool Event_PollExternal(Event* event); // defined in source/event/*.c
bool Event_Available(void);
bool Event_Poll(Event* e);
void Event_StartTextInput(void); // defined in source/event/*.c
void Event_StopTextInput(void); // defined in source/event/*.c
void Event_AddHandler(Event_Type type, Event_Handler func);

#endif
