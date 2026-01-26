#include "dropDown.h"
#include "../app.h"
#include "../mem.h"
#include "../util.h"
#include "../input.h"
#include "../theme.h"

static void Free(UI_Element* e) {
	UI_DropDown* data = (UI_DropDown*) e->data;
	free(data->label);
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	UI_DropDown* data = (UI_DropDown*) e->data;
	Rect rect         = UI_ContainerGetRect(container);

// 	if (!input.mouseBtn[0]) {
// 		data->pressed = false;
// 	}

	Rect btnRect = (Rect) {
		rect.x + e->x, rect.y + e->y, e->w, e->h
	};

	Backend_RenderRect(btnRect, theme.bg[1]);
	UI_RenderBorder(
		1, btnRect, input.mouseBtn[0] && PointInRect(input.mousePos, btnRect)
	);

	Vec2 textSize = {
		app.font.charWidth * strlen(data->label), app.font.charHeight
	};

	Text_Render(
		&app.font, data->label,
		rect.x + e->x + ((e->w / 2) - (textSize.x / 2)),
		rect.y + e->y + ((e->h / 2) - (textSize.y / 2))
	);

	// TODO: render outline if mouse is hovered over button

	if (data->open) {
		Rect rect2  = btnRect;
		rect2.y    += btnRect.h;
		rect2.h     = ((int) data->numButtons) * btnRect.h;

		Backend_RenderRect(rect2, theme.bg[1]);
		UI_RenderBorder(1, rect2, false);

		for (size_t i = 0; i < data->numButtons; ++ i) {
			UI_DropDownButton* button = &data->buttons[i];

			int yOff = ((int) i + 1) * btnRect.h;

			Rect btnRect2  = btnRect;
			btnRect2.y    += yOff;

			// UI_RenderBG(
			// 	1, btnRect2,
			// 	input.mouseBtn[0] && PointInRect(input.mousePos, btnRect2)
			// );

			if (PointInRect(input.mousePos, btnRect2)) {
				Colour bg     = Video_MultiplyColour(theme.bg[1], 1.5);
				Colour bright = Video_MultiplyColour(bg,          1.5);
				Colour dark   = Video_MultiplyColour(bg,          0.5);

				Backend_RenderRect(btnRect2, bg);
				// Backend_HLine(btnRect2.x, btnRect2.y, 1, btnRect2.w, bright);
				// Backend_HLine(
				// 	btnRect2.x, btnRect2.y + btnRect.h - 1, 1, btnRect2.w, dark
				// );
			}

			textSize.x = app.font.charWidth * strlen(button->label);

			Text_Render(
				&app.font, button->label,
				rect.x + e->x + 4,
				rect.y + e->y + ((e->h / 2) - (textSize.y / 2)) + yOff
			);
		}
	}
}

static void OnClick(UI_Container* cont, UI_Element* e, uint8_t button, bool down) {
	UI_DropDown* data = (UI_DropDown*) e->data;

	if (button != 0) return;

	if (down) {
		data->pressed = true;
	}
	else if (data->pressed) {
		data->pressed = false;

		data->open              = !data->open;
		cont->manager->priority = data->open;

		// if (data->onClick) {
		// 	data->onClick(data, button);
		// }

// 		Rect rect = UI_ContainerGetRect(cont);
// 
// 		if (!input.mouseBtn[0]) {
// 			data->pressed = false;
// 		}
// 
// 		Rect btnRect = (Rect) {
// 			rect.x + e->x, rect.y + e->y, e->w, e->h
// 		};
// 
// 		for (size_t i = 0; i < data->numButtons; ++ i) {
// 			UI_Button* iButton = &data->buttons[i];
// 			int        yOff    = ((int) i + 1) * btnRect.h;
// 
// 			Rect btnRect2  = btnRect;
// 			btnRect2.y    += yOff;
// 
// 			if (PointInRect(input.mousePos, btnRect2)) {
// 				if (iButton->onClick) {
// 					iButton->onClick(iButton, button);
// 				}
// 
// 				break;
// 			}
// 		}
	}
}

static bool OnEvent(UI_Container* cont, UI_Element* e, Event* ev, bool focus) {
	(void) focus;

	Rect cRect   = UI_ContainerGetRect(cont);
	Rect btnRect = (Rect) {
		cRect.x + e->x, cRect.y + e->y, e->w, e->h
	};

	UI_DropDown* data = (UI_DropDown*) e->data;
	if (!data->open) return false;

	if (
		(
			(ev->type == AE_EVENT_MOUSE_BUTTON_UP) ||
			(ev->type == AE_EVENT_MOUSE_BUTTON_DOWN)
		) &&
		!PointInRect(input.mousePos, btnRect)
	) {
		data->pressed = false;

		for (size_t i = 0; i < data->numButtons; ++ i) {
			UI_DropDownButton* iButton = &data->buttons[i];

			int yOff = ((int) i + 1) * btnRect.h;

			Rect btnRect2  = btnRect;
			btnRect2.y    += yOff;

			if (PointInRect(input.mousePos, btnRect2)) {
				if (iButton->onClick && ev->type == AE_EVENT_MOUSE_BUTTON_UP) {
					iButton->onClick(ev->mouseButton.button);
					data->open = false;
					cont->manager->priority = false;
				}

				return true;
			}
		}
	}

	return false;
}

UI_Element UI_NewDropDown(
	const char* label, UI_DropDownButton* buttons, size_t numButtons, bool fixed
) {
	UI_Element ret;
	ret.fixedWidth      = fixed? strlen(label) * app.font.charWidth : 0;
	ret.data            = SafeMalloc(sizeof(UI_DropDown));
	ret.preferredHeight = app.font.charHeight + 8;
	ret.free            = &Free;
	ret.render          = &Render;
	ret.onClick         = &OnClick;
	ret.onEvent         = &OnEvent;

	UI_DropDown* data = (UI_DropDown*) ret.data;
	data->label       = NewString(label);
	data->pressed     = false;
	data->buttons     = buttons;
	data->numButtons  = numButtons;
	return ret;
}
