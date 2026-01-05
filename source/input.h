#ifndef AE_INPUT_H
#define AE_INPUT_H

#include "event.h"
#include "types.h"
#include "keyboard.h"

// mouse stuff
#define AE_MOUSE_BUTTON_1 0
#define AE_MOUSE_BUTTON_2 1
#define AE_MOUSE_BUTTON_3 2

typedef enum {
	AE_INPUT_BIND_KEY = 0,
	AE_INPUT_BIND_MOUSE_BUTTON
} Input_BindType;

typedef struct {
	Input_BindType type;

	Key key;
	Key mod[3];
} Input_KeyBind;

typedef struct {
	Input_BindType type;
	uint8_t        button;
} Input_MouseButtonBind;

typedef union {
	Input_BindType        type;
	Input_KeyBind         key;
	Input_MouseButtonBind mouseButton;
} Input_Bind;

typedef struct {
	Input_Bind* binds;
	size_t      bindAmount;

	bool mousePressed;
	Vec2 mousePos;
} Input_Manager;

typedef size_t Input_BindID;

#define INPUT_BIND_NONE ((size_t) -1)

extern Input_Manager input;

// non-portable
bool Input_KeyPressed(Key key);
bool Input_MouseButtonPressed(uint8_t button);

// portable
// NOTE: NOT SAFE
void         Input_Free(void);
void         Input_PrintBind(char* dest, size_t size, Input_BindID bindID);
bool         Input_MatchBind(Input_BindID bindID, Event* e);
bool         Input_BindPressed(Input_BindID bindID);
Input_BindID Input_AddBind(Input_Bind bind);
Input_BindID Input_AddKeyBind(Key mod[3], Key key);
Input_BindID Input_AddMouseButtonBind(uint8_t button);
void         Input_HandleEvent(Event* e);

#endif
