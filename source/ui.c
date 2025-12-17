/*#include "ui.h"

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

UI_Container* UI_ManagerAddContainer(UI_Manager* man, int x, int y, int w) {
	for (size_t i = 0; i < man->containerLen; ++ i) {
		if (man->containers[i].active) continue;

		man->containers[i] = (UI_Container) {
			.active = true,
			.x = x, .y = y, .w = w, .h = 0,
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

UI_Row* UI_ContainerAddRow(UI_Container* container, int height) {
	container->rows = SafeRealloc(container->rows, container->rowAmount + 1);
	++ container->rowAmount;

	container->h += height;

	container->rows[container->rowAmount - 1].container = container;

	container->rows[container->rowAmount - 1] = (UI_Row) {0, height, NULL};
	return &container->rows[container->rowAmount - 1];
}

void UI_ContainerRender(UI_Container* container, bool focus) {
	for (size_t row = 0; row < container->rowAmount; ++ row) {
		UI_Row* row = &container->rows[row];

		for (size_t col = 0; col < row->elemAmount; ++ col) {
			UI_Element* elem = &row->elems[col];

			elem->render(elem, focus && container->focus == elem);
		}
	}
}

UI_Element* UI_RowAddElement(UI_Row* row, UI_Element element) {
	row->elems = SafeRealloc(row->elems, row->elemAmount + 1);
	++ row->elemAmount;

	row->elems[row->elemAmount - 1] = element;
	return &row->elems[row->elemAmount - 1];
}

void UI_RowFinish(UI_Row* row) {
	size_t usableSpace = row->container->w;
	size_t resizable   = 0;

	for (size_t i = 0; i < row->elemAmount; ++ i) {
		if (row->elems[i].fixedWidth == 0) {
			usableSpace -= row->elems[i].w;
			++ resizable;
		}
	}

	int x = row->container->x;

	for (size_t i = 0; i < row->elemAmount; ++ i) {
		row->elems[i].x = x;
		row->elems[i].y = row->container->y;
		row->elems[i].w = row->elems[i].fixedWidth == 0?
			row->elems[i].fixedWidth : usableSpace / resizable;
		row->elems[i].h = row->height;

		x += row->elems[i].w;
	}
}
*/
