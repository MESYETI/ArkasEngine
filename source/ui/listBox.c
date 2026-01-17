#include "../app.h"
#include "../mem.h"
#include "../text.h"
#include "../input.h"
#include "../theme.h"
#include "../backend.h"
#include "listBox.h"

typedef struct {
	UI_ListBoxItem* list;
	size_t          len;
	const char**    selected;
} UI_ListBox;

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	UI_ListBox* data = (UI_ListBox*) e->data;
	Rect cRect       = UI_ContainerGetRect(container);
	Rect eRect = (Rect) {
		cRect.x + e->x, cRect.y + e->y, e->w, e->h
	};

	Backend_RenderRect(eRect, theme.bg[1]);

	for (size_t i = 0; i < data->len; ++ i) {
		Rect rect = (Rect) {
			eRect.x, eRect.y + (((int) i) * (app.font.charHeight + 8)),
			eRect.w, app.font.charHeight + 8
		};

		if (*data->selected == data->list[i].label) {
			Backend_RenderRect(rect, Video_MultiplyColour(theme.bg[1], 1.5));
		}

		Text_Render(&app.font, data->list[i].label, rect.x + 4, rect.y + 4);
	}

	UI_RenderBorder(0, eRect, true);
}

static void OnClick(UI_Container* cont, UI_Element* e, uint8_t button, bool down) {
	UI_ListBox* data = (UI_ListBox*) e->data;

	if (button != 0) return;

	Rect cRect = UI_ContainerGetRect(cont);
	Rect eRect = (Rect) {
		cRect.x + e->x, cRect.y + e->y, e->w, e->h
	};

	*data->selected = NULL;

	for (size_t i = 0; i < data->len; ++ i) {
		Rect rect = (Rect) {
			eRect.x, eRect.y + (((int) i) * (app.font.charHeight + 8)),
			eRect.w, app.font.charHeight + 8
		};

		if (PointInRect(input.mousePos, rect)) {
			*data->selected = data->list[i].label;
			break;
		}
	}
}

UI_Element UI_NewListBox(
	UI_ListBoxItem* list, size_t len, const char** selected, int fixedW
) {
	UI_Element ret;
	ret.fixedWidth      = fixedW;
	ret.data            = SafeMalloc(sizeof(UI_ListBox));
	ret.preferredHeight = app.font.charHeight + 8;
	ret.free            = NULL;
	ret.render          = &Render;
	ret.onClick         = &OnClick;
	ret.onEvent         = NULL;

	UI_ListBox* data = (UI_ListBox*) ret.data;
	data->list       = list;
	data->len        = len;
	data->selected   = selected;
	*selected        = NULL;
	return ret;
}
