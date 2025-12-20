#include "label.h"
#include "../app.h"
#include "../mem.h"
#include "../util.h"

typedef struct {
	char* label;
} LabelData;

static void Free(UI_Element* e) {
	LabelData* data = (LabelData*) e->data;
	free(data->label);
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	LabelData* data = (LabelData*) e->data;
	Rect rect       = UI_ContainerGetRect(container);

	Text_Render(&app.font, data->label, rect.x + e->x, rect.y + e->y);
}

UI_Element UI_NewLabel(const char* label) {
	UI_Element ret;
	ret.fixedWidth      = 0;
	ret.data            = SafeMalloc(sizeof(LabelData));
	ret.preferredHeight = app.font.charHeight;
	ret.free            = &Free;
	ret.render          = &Render;
	ret.onClick         = NULL;
	ret.onEvent         = NULL;

	LabelData* data = (LabelData*) ret.data;
	data->label     = NewString(label);
	return ret;
}
