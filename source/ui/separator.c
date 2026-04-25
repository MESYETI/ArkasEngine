#include "separator.h"
#include "../mem.h"
#include "../util.h"
#include "../theme.h"
#include "../engine.h"

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	Rect rect = UI_ContainerGetRect(container);
	int  x    = rect.x + e->x;

	Backend_HLine(
		x + 2, rect.y + e->y + (e->h / 2), 1, e->w - 4, theme.bg[1]
	);
}

UI_Element UI_NewSeparator(void) {
	UI_Element ret;
	ret.fixedWidth      = 0;
	ret.data            = NULL;
	ret.preferredHeight = 5;
	ret.free            = NULL;
	ret.render          = &Render;
	ret.onClick         = NULL;
	ret.onEvent         = NULL;
	return ret;
}
