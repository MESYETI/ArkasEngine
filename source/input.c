#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "input.h"

Input_Manager input = {
	.binds      = NULL,
	.bindAmount = 0,

	.mousePressed = false,
	.mousePos     = {0, 0}
};

void Input_Free(void) {
	if (input.binds) {
		free(input.binds);
		input.binds = NULL;
	}

	input.bindAmount = 0;
}

void Input_PrintBind(char* dest, size_t size, Input_BindID bindID) {
	Input_Bind* bind = &input.binds[bindID];

	switch (bind->type) {
		case AE_INPUT_BIND_KEY: {
			snprintf(dest, size, "key %s", Key_ToString(bind->key.key));
			break;
		}
		case AE_INPUT_BIND_MOUSE_BUTTON: {
			snprintf(dest, size, "button %d", bind->mouseButton.button);
			break;
		}
		default: {
			snprintf(dest, size, "???");
			break;
		}
	}
}

bool Input_MatchBind(Input_BindID bindID, Event* e) {
	if (bindID > input.bindAmount) return false;

	Input_Bind* bind = &input.binds[bindID];

	switch (bind->type) {
		case AE_INPUT_BIND_KEY: {
			if ((e->type != AE_EVENT_KEY_DOWN) && (e->type != AE_EVENT_KEY_UP)) {
				return false;
			}

			return bind->key.key == e->key.key; // TODO: modifiers
		}
		case AE_INPUT_BIND_MOUSE_BUTTON: {
			if (
				(e->type != AE_EVENT_MOUSE_BUTTON_DOWN) &&
				(e->type != AE_EVENT_MOUSE_BUTTON_UP)
			) {
				return false;
			}

			return bind->mouseButton.button == e->mouseButton.button;
		}
	}

	return false;
}

bool Input_BindPressed(Input_BindID bindID) {
	if (bindID > input.bindAmount) return false;

	Input_Bind* bind = &input.binds[bindID];

	switch (bind->type) {
		case AE_INPUT_BIND_KEY: {
			return Input_KeyPressed(bind->key.key); // TODO: modifiers
		}
		case AE_INPUT_BIND_MOUSE_BUTTON: {
			return Input_MouseButtonPressed(bind->mouseButton.button);
		}
	}

	return false;
}

Input_BindID Input_AddBind(Input_Bind bind) {
	input.binds = SafeRealloc(input.binds, (input.bindAmount + 1) * sizeof(Input_Bind));
	++ input.bindAmount;

	input.binds[input.bindAmount - 1] = bind;
	return input.bindAmount - 1;
}

Input_BindID Input_AddKeyBind(Key mod[3], Key key) {
	Input_Bind bind;
	bind.type    = AE_INPUT_BIND_KEY;
	bind.key.key = key;
	memcpy(bind.key.mod, mod, sizeof(bind.key.mod));

	return Input_AddBind(bind);
}

Input_BindID Input_AddMouseButtonBind(uint8_t button) {
	Input_Bind bind;
	bind.type               = AE_INPUT_BIND_MOUSE_BUTTON;
	bind.mouseButton.button = button;

	return Input_AddBind(bind);
}

void Input_HandleEvent(Event* e) {
	switch (e->type) {
		case AE_EVENT_MOUSE_MOVE: {
			input.mousePos = (Vec2) {e->mouseMove.x, e->mouseMove.y};
			break;
		}
		case AE_EVENT_MOUSE_BUTTON_DOWN: input.mousePressed = true;  break;
		case AE_EVENT_MOUSE_BUTTON_UP:   input.mousePressed = false; break;
	}
}
