#ifndef AE_EVENT_H
#define AE_EVENT_H

#include "keyboard.h"

typedef enum {
	AE_EVENT_NONE              = 0,
	AE_EVENT_MOUSE_MOVE        = 1,
	AE_EVENT_MOUSE_BUTTON_DOWN = 2,
	AE_EVENT_MOUSE_BUTTON_UP   = 3,
	AE_EVENT_KEY_DOWN          = 4,
	AE_EVENT_KEY_UP            = 5,
	AE_EVENT_QUIT              = 6,
	AE_EVENT_WINDOW_RESIZE     = 7,
	AE_EVENT_TEXT_INPUT        = 8
} AE_EventType;

typedef struct {
	AE_EventType type;
	int          x;
	int          y;
	int          xRel;
	int          yRel;
} AE_MouseMoveEvent;

typedef struct {
	AE_EventType type;
	uint8_t      button;
} AE_MouseButtonEvent;

typedef struct {
	AE_EventType type;
	AE_Key       key;
} AE_KeyEvent;

typedef struct {
	AE_EventType type;
	int          width;
	int          height;
} AE_WindowResizeEvent;

typedef struct {
	AE_EventType type;
	char         input[32];
} AE_TextInputEvent;

typedef union {
	AE_EventType         type;
	AE_MouseMoveEvent    mouseMove;
	AE_MouseButtonEvent  mouseButton;
	AE_KeyEvent          key;
	AE_WindowResizeEvent windowResize;
	AE_TextInputEvent    textInput;
} AE_Event;

void AE_InitEvents(void);
void AE_AddEvent(AE_Event e);
void AE_UpdateEvents(void);
bool AE_EventsAvailable(void);
bool AE_PollEvents(AE_Event* e);

#endif
