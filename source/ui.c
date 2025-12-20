#include "ui.h"
#include "mem.h"
#include "theme.h"
#include "backend.h"

void UI_ManagerInit(UI_Manager* man, size_t poolSize) {
	man->containers   = SafeMalloc(poolSize * sizeof(UI_Container));
	man->containerLen = poolSize;
	man->focus        = NULL;

	for (size_t i = 0; i < poolSize; ++ i) {
		man->containers[i].active = false;
	}
}

void UI_ManagerFree(UI_Manager* man) {
	if (man->containers) {
		free(man->containers);
	}
	man->containerLen = 0;
	man->focus        = NULL;
}

UI_Container* UI_ManagerAddContainer(UI_Manager* man, int w) {
	for (size_t i = 0; i < man->containerLen; ++ i) {
		if (man->containers[i].active) continue;

		man->containers[i] = (UI_Container) {
			.active = true,
			.x = 0, .y = 0, .w = w, .h = 0,
			.padTop = 0, .padBottom = 0, .padLeft = 0, .padRight = 0,
			.yMode = 0, .xMode = 0,
			.rows      = NULL,
			.rowAmount = 0,
			.focus     = NULL
		};
		return &man->containers[i];
	}

	return NULL;
}

void UI_ManagerRender(UI_Manager* man) {
	for (size_t i = 0; i < man->containerLen; ++ i) {
		UI_ContainerRender(
			&man->containers[i], man->focus == &man->containers[i]
		);
	}
}

void UI_ContainerCenterX(UI_Container* container) {
	container->xMode = UI_MODE_CENTERED;
}

void UI_ContainerCenterY(UI_Container* container) {
	container->yMode = UI_MODE_CENTERED;
}

void UI_ContainerAlignLeft(UI_Container* container, int padding) {
	container->xMode = UI_MODE_MIN_EDGE;
	container->x     = padding;
}

void UI_ContainerAlignRight(UI_Container* container, int padding) {
	container->xMode = UI_MODE_MAX_EDGE;
	container->x     = padding;
}

void UI_ContainerAlignTop(UI_Container* container, int padding) {
	container->yMode = UI_MODE_MIN_EDGE;
	container->y     = padding;
}

void UI_ContainerAlignBottom(UI_Container* container, int padding) {
	container->yMode = UI_MODE_MAX_EDGE;
	container->y     = padding;
}

void UI_ContainerFixedPos(UI_Container* container, int x, int y) {
	container->xMode = UI_MODE_FIXED;
	container->x     = x;
	container->yMode = UI_MODE_FIXED;
	container->y     = y;
}

void UI_ContainerSetPadding(
	UI_Container* container, int top, int bottom, int left, int right
) {
	container->padTop    = top;
	container->padBottom = bottom;
	container->padLeft   = left;
	container->padRight  = right;
}

UI_Row* UI_ContainerAddRow(UI_Container* container, int height) {
	container->rows = SafeRealloc(
		container->rows, (container->rowAmount + 1) * sizeof(UI_Row)
	);

	container->h += height;

	Rect rect = UI_ContainerGetRect(container);
	int  y    = rect.h;

	++ container->rowAmount;
	container->rows[container->rowAmount - 1] = (UI_Row) {0, height, y, NULL, container};
	return &container->rows[container->rowAmount - 1];
}

void UI_ContainerRender(UI_Container* container, bool focus) {
	Rect rect = UI_ContainerGetRect(container);

	Backend_RenderRect(rect, theme.bg);

	for (size_t rowIdx = 0; rowIdx < container->rowAmount; ++ rowIdx) {
		UI_Row* row = &container->rows[rowIdx];

		for (size_t col = 0; col < row->elemAmount; ++ col) {
			UI_Element* elem = &row->elems[col];

			elem->render(container, elem, focus && container->focus == elem);
		}
	}
}

Rect UI_ContainerGetRect(UI_Container* container) {
	Rect rect;
	rect.w = container->w;

	rect.h = container->padBottom;
	for (size_t i = 0; i < container->rowAmount; ++ i) {
		rect.h += container->padTop + container->rows[i].height;
	}

	switch (container->yMode) {
		case UI_MODE_FIXED:    rect.y = container->y; break;
		case UI_MODE_MIN_EDGE: rect.y = container->y; break;
		case UI_MODE_MAX_EDGE: rect.y = video.height - rect.h - container->y; break;
		case UI_MODE_CENTERED: rect.y = (video.height / 2) - (rect.h / 2); break;
	}

	switch (container->xMode) {
		case UI_MODE_FIXED:    rect.x = container->x; break;
		case UI_MODE_MIN_EDGE: rect.x = container->x; break;
		case UI_MODE_MAX_EDGE: rect.x = video.width - rect.w - container->x; break;
		case UI_MODE_CENTERED: rect.x = (video.width / 2) - (rect.w / 2); break;
	}

	return rect;
}

UI_Element* UI_RowAddElement(UI_Row* row, UI_Element element) {
	row->elems = SafeRealloc(row->elems, (row->elemAmount + 1) * sizeof(UI_Element));
	++ row->elemAmount;

	row->elems[row->elemAmount - 1] = element;
	return &row->elems[row->elemAmount - 1];
}

void UI_RowFinish(UI_Row* row) {
	int usableSpace = row->container->w -
		row->container->padLeft - row->container->padRight;

	int resizable = 0;

	for (size_t i = 0; i < row->elemAmount; ++ i) {
		if (row->elems[i].fixedWidth == 0) {
			usableSpace -= row->elems[i].w;
			++ resizable;
		}
	}

	int x = row->container->padLeft;

	for (size_t i = 0; i < row->elemAmount; ++ i) {
		row->elems[i].x = x;
		row->elems[i].y = row->y;
		row->elems[i].w = row->elems[i].fixedWidth == 0?
			row->elems[i].fixedWidth : usableSpace / resizable;
		row->elems[i].h = row->height;

		x += row->elems[i].w + row->container->padLeft;
	}
}
