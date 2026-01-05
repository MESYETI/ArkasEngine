#include "button.h"
#include "../app.h"
#include "../mem.h"
#include "../util.h"
#include "../input.h"

static void Free(UI_Element* e) {
	UI_Button* data = (UI_Button*) e->data;
	free(data->label);
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;


	UI_Button* data = (UI_Button*) e->data;
	Rect rect       = UI_ContainerGetRect(container);

	if (!input.mousePressed) {
		data->pressed = false;
	}

	Rect btnRect = (Rect) {
		rect.x + e->x, rect.y + e->y, e->w, e->h
	};

	UI_RenderBG(1, btnRect, data->pressed);

	Vec2 textSize = {
		app.font.charWidth  * strlen(data->label), app.font.charHeight
	};

	Text_Render(
		&app.font, data->label,
		rect.x + e->x + ((e->w / 2) - (textSize.x / 2)),
		rect.y + e->y + ((e->h / 2) - (textSize.y / 2))
	);

	// TODO: render outline if mouse is hovered over button
}

static void OnClick(UI_Element* e, uint8_t button, bool down) {
	UI_Button* data = (UI_Button*) e->data;

	if (down) {
		data->pressed = true;
	}
	else if (data->pressed) {
		data->pressed = false;

		if (data->onClick) {
			data->onClick(data, button);
		}
	}
}

static bool OnEvent(UI_Element* e, Event* ev, bool focus) {
	(void) focus;

	if (ev->type == AE_EVENT_MOUSE_BUTTON_UP) {
		UI_Button* data = (UI_Button*) e->data;
		data->pressed = false;
	}

	return false;
}

UI_Element UI_NewButton(const char* label, bool fixed, UI_ButtonFunc onClick) {
	UI_Element ret;
	ret.fixedWidth      = fixed? strlen(label) * app.font.charWidth : 0;
	ret.data            = SafeMalloc(sizeof(UI_Button));
	ret.preferredHeight = app.font.charHeight + 8;
	ret.free            = &Free;
	ret.render          = &Render;
	ret.onClick         = &OnClick;
	ret.onEvent         = &OnEvent;

	UI_Button* data = (UI_Button*) ret.data;
	data->label     = NewString(label);
	data->onClick   = onClick;
	data->pressed   = false;
	return ret;
}
