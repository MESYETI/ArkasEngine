#include "../app.h"
#include "../mem.h"
#include "../text.h"
#include "../backend.h"
#include "textInput.h"

typedef struct {
	char*  dest;
	size_t size;
	size_t cursor;
} UI_TextInput;

static void Free(UI_Element* e) {
	(void) e;
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	UI_TextInput* data  = (UI_TextInput*) e->data;
	Rect          rect  = UI_ContainerGetRect(container);
	Rect          eRect = (Rect) {rect.x + e->x, rect.y + e->y, e->w, e->h};

	UI_RenderBG(1, eRect, 0);

	Vec2 textSize = {
		app.font.charWidth * strlen(data->dest), app.font.charHeight
	};

	Vec2 textPos = {
		rect.x + e->x + 4, rect.y + e->y + ((e->h / 2) - (textSize.y / 2))
	};

	Text_Render(&app.font, data->dest, textPos.x, textPos.y);

	if (focus) {
		Backend_VLine(
			textPos.x + (app.font.charWidth * ((int) data->cursor)), textPos.y,
			1, app.font.charHeight, (Colour) {255, 255, 255}
		);
	}
}

static bool OnEvent(UI_Container* cont, UI_Element* e, Event* ev, bool focus) {
	(void) cont;

	if (!focus) return false;

	UI_TextInput* data = (UI_TextInput*) e->data;

	switch (ev->type) {
		case AE_EVENT_TEXT_INPUT: {
			if (strlen(ev->textInput.input) + strlen(data->dest) >= data->size - 1) {
				return true;
			}

			strcat(data->dest, ev->textInput.input);
			data->cursor += strlen(ev->textInput.input);
			return true;
		}
	}

	return false;
}

UI_Element UI_NewTextInput(char* dest, size_t size) {
	UI_Element ret;
	ret.fixedWidth      = 0;
	ret.data            = SafeMalloc(sizeof(UI_TextInput));
	ret.preferredHeight = app.font.charHeight + 8;
	ret.free            = &Free;
	ret.onClick         = NULL;
	ret.render          = &Render;
	ret.onEvent         = &OnEvent;

	UI_TextInput* data = (UI_TextInput*) ret.data;
	data->dest         = dest;
	data->size         = size;
	data->cursor       = 0;
	return ret;
}
