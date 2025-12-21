#ifndef AE_UI_BUTTON_H
#define AE_UI_BUTTON_H

#include "../ui.h"

typedef struct UI_Button UI_Button;

typedef void (*UI_ButtonFunc)(UI_Button* this, uint8_t button);

struct UI_Button {
	char*         label;
	UI_ButtonFunc onClick;
};

UI_Element UI_NewButton(const char* label, bool fixed, UI_ButtonFunc onClick);

#endif
