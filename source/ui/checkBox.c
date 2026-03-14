#include "checkBox.h"
#include "../engine.h"
#include "../mem.h"
#include "../util.h"
#include "../input.h"
#include "../theme.h"

typedef struct {
	bool* value;
} UI_CheckBox;

static void Free(UI_Element* e) {
	(void) e;
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	UI_CheckBox* data = (UI_CheckBox*) e->data;
	Rect         rect = UI_ContainerGetRect(container);

	Rect btnRect = (Rect) {
		rect.x + e->x, rect.y + e->y, e->w, e->h
	};

	Backend_RenderRectOL(btnRect, (Colour) {255, 255, 255, 255});

	btnRect.x += 3;
	btnRect.y += 3;
	btnRect.w -= 6;
	btnRect.h -= 6;

	if (*(data->value)) {
		Backend_RenderRect(btnRect, (Colour) {255, 255, 255, 255});
	}
}

static void OnClick(UI_Container* cont, UI_Element* e, uint8_t button, bool down) {
	(void) cont;

	if ((button != 0) || !down) return;

	UI_CheckBox* data = (UI_CheckBox*) e->data;

	*(data->value) = !*(data->value);
}

static bool OnEvent(UI_Container* cont, UI_Element* e, Event* ev, bool focus) {
	(void) cont;
	(void) focus;
	(void) e;
	(void) ev;

	return false;
}

UI_Element UI_NewCheckBox(bool* value) {
	UI_Element ret;
	ret.fixedWidth      = 16;
	ret.data            = SafeMalloc(sizeof(UI_CheckBox));
	ret.preferredHeight = 16;
	ret.free            = &Free;
	ret.render          = &Render;
	ret.onClick         = &OnClick;
	ret.onEvent         = &OnEvent;

	UI_CheckBox* data = (UI_CheckBox*) ret.data;
	data->value       = value;
	return ret;
}
