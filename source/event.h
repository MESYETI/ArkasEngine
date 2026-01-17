#ifndef Event_H
#define Event_H

#include "keyboard.h"

// i don't use an enum because of compiler warnings about missing cases in switch
// statements for event types
#define AE_EVENT_NONE              0
#define AE_EVENT_MOUSE_MOVE        1
#define AE_EVENT_MOUSE_BUTTON_DOWN 2
#define AE_EVENT_MOUSE_BUTTON_UP   3
#define AE_EVENT_KEY_DOWN          4
#define AE_EVENT_KEY_UP            5
#define AE_EVENT_QUIT              6
#define AE_EVENT_WINDOW_RESIZE     7
#define AE_EVENT_TEXT_INPUT        8

typedef uint8_t Event_Type;

typedef struct {
	Event_Type type;
	int        x;
	int        y;
	int        xRel;
	int        yRel;
} Event_MouseMove;

typedef struct {
	Event_Type type;
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
	int        width;
	int        height;
} Event_WindowResize;

typedef struct {
	Event_Type type;
	char       input[32];
} Event_TextInput;

typedef union {
	Event_Type         type;
	Event_MouseMove    mouseMove;
	Event_MouseButton  mouseButton;
	Event_Key          key;
	Event_WindowResize windowResize;
	Event_TextInput    textInput;
} Event;

void Event_Init(void);
void Event_Add(Event e);
void Event_Update(void);
bool Event_Available(void);
bool Event_Poll(Event* e);
void Event_StartTextInput(void);
void Event_StopTextInput(void);

#endif
