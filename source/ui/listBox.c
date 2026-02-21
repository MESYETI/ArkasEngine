#include "../engine.h"
#include "../mem.h"
#include "../util.h"
#include "../text.h"
#include "../input.h"
#include "../theme.h"
#include "../backend.h"
#include "listBox.h"

typedef struct {
	UI_ListBoxItem* list;
	size_t          len;
	const char**    selected;
	UI_Element*     scrollbar;

	UI_ListBoxOnClick onClick;
} UI_ListBox;

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	UI_ListBox* data = (UI_ListBox*) e->data;
	Rect cRect       = UI_ContainerGetRect(container);
	Rect eRect = (Rect) {
		cRect.x + e->x, cRect.y + e->y, e->w, e->h
	};

	size_t start = 0;

	if (data->scrollbar) {
		float scroll = UI_GetScrollBarPercent(data->scrollbar);

		start = (size_t) (scroll * ((float) data->len));
	}

	Backend_RenderRect(eRect, theme.bg[1]);

	for (size_t i = 0; i < data->len - start; ++ i) {
		size_t idx = start + i;

		Rect rect = (Rect) {
			eRect.x, eRect.y + (((int) i) * (engine.font.charHeight + 8)),
			eRect.w, engine.font.charHeight + 8
		};

		if (*data->selected == data->list[idx].label) {
			Backend_RenderRect(rect, Video_MultiplyColour(theme.bg[1], 1.5));
		}

		Text_Render(&engine.font, data->list[idx].label, rect.x + 4, rect.y + 4);
	}

	UI_RenderBorder(0, eRect, true);
}

static void OnClick(UI_Container* cont, UI_Element* e, uint8_t button, bool down) {
	UI_ListBox* data = (UI_ListBox*) e->data;

	if ((button != 0) || !down) return;

	Rect cRect = UI_ContainerGetRect(cont);
	Rect eRect = (Rect) {
		cRect.x + e->x, cRect.y + e->y, e->w, e->h
	};

	*data->selected = NULL;

	for (size_t i = 0; i < data->len; ++ i) {
		Rect rect = (Rect) {
			eRect.x, eRect.y + (((int) i) * (engine.font.charHeight + 8)),
			eRect.w, engine.font.charHeight + 8
		};

		if (PointInRect(input.mousePos, rect)) {
			*data->selected = data->list[i].label;

			if (data->onClick) {
				data->onClick();
			}
			break;
		}
	}
}

UI_Element UI_NewListBox(
	UI_ListBoxItem* list, size_t len, const char** selected, int fixedW,
	UI_ListBoxOnClick onClick
) {
	UI_Element ret;
	ret.fixedWidth      = fixedW;
	ret.data            = SafeMalloc(sizeof(UI_ListBox));
	ret.preferredHeight = engine.font.charHeight + 8;
	ret.free            = NULL;
	ret.render          = &Render;
	ret.onClick         = &OnClick;
	ret.onEvent         = NULL;

	UI_ListBox* data = (UI_ListBox*) ret.data;
	data->list       = list;
	data->len        = len;
	data->selected   = selected;
	data->scrollbar  = NULL;
	data->onClick    = onClick;
	*selected        = NULL;
	return ret;
}

void UI_UpdateListBox(UI_Element* elem, UI_ListBoxItem* list, size_t len) {
	UI_ListBox* data = (UI_ListBox*) elem->data;
	data->list       = list;
	data->len        = len;
	*data->selected  = NULL;

	if (data->scrollbar) {
		UI_SetScrollBarPercent(data->scrollbar, 0.0);
	}
}

void UI_ScrollListBox(UI_Element* elem, UI_Element* scrollbar) {
	UI_ListBox* data = (UI_ListBox*) elem->data;
	data->scrollbar  = scrollbar;
}
