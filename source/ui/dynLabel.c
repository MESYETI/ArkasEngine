#include "dynLabel.h"
#include "../app.h"
#include "../mem.h"
#include "../util.h"

typedef struct {
	DynLabelFunc func;
	bool         centered;
	Font*        font;
} LabelData;

static void Free(UI_Element* e) {
	(void) e;
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	LabelData* data = (LabelData*) e->data;
	Rect       rect = UI_ContainerGetRect(container);

	const char* label = data->func();

	int x = rect.x + e->x;

	if (data->centered) {
		int width = strlen(label) * data->font->charWidth;

		x += (e->h / 2) - (width / 2);
	}

	Text_Render(
		data->font, label, x,
		rect.y + e->y + ((e->h / 2) - (data->font->charHeight / 2))
	);
}

UI_Element UI_NewDynLabel(Font* font, DynLabelFunc func, int opt) {
	UI_Element ret;
	ret.fixedWidth      = 0;
	ret.data            = SafeMalloc(sizeof(LabelData));
	ret.preferredHeight = font->charHeight;
	ret.free            = &Free;
	ret.render          = &Render;
	ret.onClick         = NULL;
	ret.onEvent         = NULL;

	LabelData* data = (LabelData*) ret.data;
	data->func      = func;
	data->centered  = (opt & UI_LABEL_CENTERED) != 0;
	data->font      = font;
	return ret;
}
