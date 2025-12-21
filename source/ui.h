#ifndef AE_UI_H
#define AE_UI_H

#include "video.h"
#include "types.h"
#include "common.h"

typedef struct UI_Container UI_Container;
typedef struct UI_Element UI_Element;

struct UI_Element {
	int   fixedWidth; // set to 0 if not fixed
	int   x;
	int   y;
	int   w;
	int   h;
	void* data;
	int   preferredHeight; // minimum height this element should use

	void (*free)(UI_Element* e);
	void (*render)(UI_Container* container, UI_Element* e, bool focus);
	void (*onClick)(UI_Element* e, uint8_t button);
	void (*onEvent)(UI_Element* e, SDL_Event* ev, bool focus);
};

typedef struct {
	size_t        elemAmount;
	int           height;
	int           y;
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

struct UI_Container {
	bool    active;
	int     x;
	int     y;
	int     w;
	int     h;
	int     padTop;
	int     padBottom;
	int     padLeft;
	int     padRight;
	
	UI_Mode yMode;
	UI_Mode xMode;
	UI_Row* rows;
	size_t  rowAmount;

	UI_Element* focus;
};

typedef struct {
	UI_Container* containers;
	size_t        containerLen;
	UI_Container* focus;
} UI_Manager;

void          UI_ManagerInit(UI_Manager* man, size_t poolSize);
void          UI_ManagerFree(UI_Manager* man);
UI_Container* UI_ManagerAddContainer(UI_Manager* man, int w);
void          UI_ManagerRender(UI_Manager* man);
bool          UI_ManagerHandleEvent(UI_Manager* man, SDL_Event* e);

void UI_RenderBG(size_t depth, Rect rect);

void    UI_ContainerCenterX(UI_Container* container);
void    UI_ContainerCenterY(UI_Container* container);
void    UI_ContainerAlignLeft(UI_Container* container, int padding);
void    UI_ContainerAlignRight(UI_Container* container, int padding);
void    UI_ContainerAlignTop(UI_Container* container, int padding);
void    UI_ContainerAlignBottom(UI_Container* container, int padding);
void    UI_ContainerFixedPos(UI_Container* container, int x, int y);

void UI_ContainerSetPadding(
	UI_Container* container, int top, int bottom, int left, int right
);

UI_Row* UI_ContainerAddRow(UI_Container* container, int height);
void    UI_ContainerRender(UI_Container* container, bool focus);
Rect    UI_ContainerGetRect(UI_Container* container);

UI_Element* UI_RowAddElement(UI_Row* row, UI_Element element);
void        UI_RowFinish(UI_Row* row, bool autoHeight);

#endif
