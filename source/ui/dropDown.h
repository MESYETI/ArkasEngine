#ifndef AE_UI_DROP_DOWN_H
#define AE_UI_DROP_DOWN_H

#include "../ui.h"

typedef void (*UI_DropDownButtonFunc)(uint8_t button);

typedef struct {
	const char* label;

	UI_DropDownButtonFunc onClick;
} UI_DropDownButton;

typedef struct {
	char*              label;
	UI_DropDownButton* buttons;
	size_t             numButtons;
	bool               open;
	bool               pressed;
} UI_DropDown;

UI_Element UI_NewDropDown(
	const char* label, UI_DropDownButton* buttons, size_t numButtons, bool fixed
);

#endif
