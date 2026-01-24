#ifndef AE_UI_LIST_BOX_H
#define AE_UI_LIST_BOX_H

#include "../ui.h"

typedef void (*UI_ListBoxOnClick)(void);

typedef struct {
	const char* label;
} UI_ListBoxItem;

UI_Element UI_NewListBox(
	UI_ListBoxItem* list, size_t len, const char** selected, int fixedW,
	UI_ListBoxOnClick onClick
);
void UI_UpdateListBox(UI_Element* elem, UI_ListBoxItem* list, size_t len);

#endif
