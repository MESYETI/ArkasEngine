#ifndef AE_UI_LABEL_H
#define AE_UI_LABEL_H

#include "../ui.h"
#include "../text.h"

enum {
	UI_LABEL_CENTERED = 1
};

UI_Element UI_NewLabel(Font* font, const char* label, int opt);

#endif
