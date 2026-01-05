#ifndef AE_UI_DYN_LABEL_H
#define AE_UI_DYN_LABEL_H

#include "../ui.h"
#include "../text.h"
#include "label.h"

typedef const char* (*DynLabelFunc)(void);

UI_Element UI_NewDynLabel(Font* font, DynLabelFunc func, int opt);

#endif
