#include "scrollbar.h"
#include "../engine.h"
#include "../mem.h"
#include "../util.h"
#include "../input.h"
#include "../theme.h"

typedef struct {
	float percent;
	int   buttonHeight;
	int   buttonY;
} UI_ScrollBar;

static void Free(UI_Element* e) {
	UI_ScrollBar* data = (UI_ScrollBar*) e->data;
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	UI_ScrollBar* data = (UI_ScrollBar*) e->data;
	Rect          rect = UI_ContainerGetRect(container);

	Rect eRect = (Rect) {
		rect.x + e->x, rect.y + e->y, e->w, e->h
	};

	Backend_RenderRect(eRect, theme.bg[1]);
	UI_RenderBorder(1, eRect, false);

	Rect button = (Rect) {
		eRect.x, eRect.y + data->buttonY, e->w, data->buttonHeight
	};

	int yOff  = (int) (data->percent * ((float) (eRect.h - data->buttonHeight)));
	button.y += yOff;

	Backend_RenderRect(button, theme.bg[2]);
	UI_RenderBorder(2, button, false);
}

static void OnClick(UI_Container* cont, UI_Element* e, uint8_t button, bool down) {
	(void) cont;

	if (!down) return;

	UI_ScrollBar* data = (UI_ScrollBar*) e->data;
	Rect          rect = UI_ContainerGetRect(cont);

	Rect eRect = (Rect) {
		rect.x + e->x, rect.y + e->y, e->w, e->h
	};
	Rect btnRect = (Rect) {
		eRect.x, eRect.y + data->buttonY, e->w, data->buttonHeight
	};

	cont->manager->priority = true;
	int y = input.mousePos.y - btnRect.y;

	data->percent = ((float) y) / ((float) eRect.h);
}

static bool OnEvent(UI_Container* cont, UI_Element* e, Event* ev, bool focus) {
	(void) cont;
	(void) focus;

	UI_ScrollBar* data = (UI_ScrollBar*) e->data;

	Rect cRect = UI_ContainerGetRect(cont);
	Rect eRect = (Rect) {
		cRect.x + e->x, cRect.y + e->y, e->w, e->h
	};
	Rect button = (Rect) {
		eRect.x, eRect.y + data->buttonY, e->w, data->buttonHeight
	};

	switch (ev->type) {
		case AE_EVENT_MOUSE_MOVE: {
			if (!input.mouseBtn[0]) break;

			int y = input.mousePos.y - button.y;
			data->percent = ((float) y) / ((float) eRect.h);

			if (y < 0.0) {
				data->percent = 0.0;
			}
			else if (data->percent > 1.0) {
				data->percent = 1.0;
			}
			break;
		}
		case AE_EVENT_MOUSE_BUTTON_UP: {
			cont->manager->priority = false;
			break;
		}
	}

	return false;
}

UI_Element UI_NewScrollBar(int buttonHeight) {
	UI_ScrollBar* data = SafeMalloc(sizeof(UI_ScrollBar));
	data->percent      = 0.0;
	data->buttonHeight = buttonHeight;
	data->buttonY      = 0;

	return (UI_Element) {
		.fixedWidth      = 16,
		.data            = data,
		.preferredHeight = 0,
		.free            = &Free,
		.render          = &Render,
		.onClick         = &OnClick,
		.onEvent         = &OnEvent
	};
}
