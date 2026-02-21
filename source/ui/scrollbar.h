#ifndef AE_UI_SCROLLBAR
#define AE_UI_SCROLLBAR

#include "../ui.h"

UI_Element UI_NewScrollBar(int buttonHeight);
float      UI_GetScrollBarPercent(UI_Element* scrollbar);
void       UI_SetScrollBarPercent(UI_Element* scrollbar, float percent);

#endif
