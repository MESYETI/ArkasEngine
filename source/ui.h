#ifndef AE_UI_H
#define AE_UI_H

#include "video.h"
#include "common.h"

typedef struct UI_Element UI_Element;

struct UI_Element {
	int   fixedWidth; // set to 0 if not fixed
	int   x;
	int   y;
	int   w;
	int   h;
	void* data;

	void (*render)(UI_Element* e, bool focus);
	void (*onClick)(UI_Element* e, uint8_t button);
	void (*onEvent)(UI_Element* e, SDL_Event* e);
};

typedef struct {
	size_t        elemAmount;
	int           height;
	UI_Element*   elems;
	UI_Container* container;
} UI_Row;

enum {
	UI_MODE_FIXED = 0,
	UI_MODE_MIN_EDGE,
	UI_MODE_MAX_EDGE,
	UI_MODE_CENTERED
};
typedef uint8_t UI_Mode;

typedef struct {
	bool    active;
	int     x;
	int     y;
	int     w;
	int     h;
	UI_Mode yMode;
	UI_Mode xMode;

	UI_Row* rows;
	size_t  rowAmount;

	UI_Element* focus;
} UI_Container;

typedef struct {
	UI_Container* containers;
	size_t        containerLen;
	UI_Container* focus;
} UI_Manager;

void          UI_ManagerInit(UI_Manager* man, size_t poolSize);
void          UI_ManagerFree(UI_Manager* man);
UI_Container* UI_ManagerAddContainer(UI_Manager* man, int x, int y, int w);
void          UI_ManagerRender(UI_Manager* man);

UI_Row* UI_ContainerAddRow(UI_Container* container);
void    UI_ContainerRender(UI_Container* container, bool focus);

UI_Element* UI_RowAddElement(UI_Row* row, UI_Element element);
void        UI_RowFinish(UI_Row* row);

#endif
