#include "ui.h"
#include "mem.h"
#include "util.h"
#include "theme.h"
#include "video.h"
#include "backend.h"

void UI_ManagerInit(UI_Manager* man, size_t poolSize) {
	man->containers   = SafeMalloc(poolSize * sizeof(UI_Container));
	man->containerLen = poolSize;
	man->focus        = NULL;
	man->priority     = false;

	for (size_t i = 0; i < poolSize; ++ i) {
		man->containers[i].active = false;
	}
}

void UI_ManagerFree(UI_Manager* man) {
	for (size_t i = 0; i < man->containerLen; ++ i) {
		UI_Container* container = &man->containers[i];

		if (!container->active) continue;

		for (size_t j = 0; j < container->rowAmount; ++ j) {
			UI_Row* row = &container->rows[j];

			for (size_t k = 0; k < row->elemAmount; ++ k) {
				UI_Element* elem = &row->elems[k];

				if (elem->free) {
					elem->free(elem);
				}
			}

			if (row->elemAmount > 0) free(container->rows[j].elems);
		}

		if (container->rowAmount > 0) free(container->rows);
	}

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
			.focus     = NULL,
			.manager   = man
		};
		return &man->containers[i];
	}

	return NULL;
}

void UI_ManagerRender(UI_Manager* man) {
	for (size_t i = 0; i < man->containerLen; ++ i) {
		if (!man->containers[i].active) continue;
		UI_ContainerRender(
			&man->containers[i], man->focus == &man->containers[i]
		);
	}
}

static bool SendToFocused(UI_Manager* man, Event* e) {
	if (man->focus) {
		// send events to the focused element first
		UI_Element* focusElem = man->focus->focus;

		if (focusElem) {
			if (focusElem->onEvent) if (focusElem->onEvent(man->focus, focusElem, e, true)) {
				return true;
			}
		}

		for (size_t i = 0; i < man->focus->rowAmount; ++ i) {
			UI_Row* row = &man->focus->rows[i];

			for (size_t j = 0; j < row->elemAmount; ++ j) {
				UI_Element* elem = &row->elems[j];

				if (elem == focusElem) continue;

				if (elem->onEvent) if (elem->onEvent(man->focus, elem, e, false)) {
					return true;
				}
			}
		}
	}

	return false;
}

bool UI_ManagerHandleEvent(UI_Manager* man, Event* e) {
	if (man->priority) {
		if (SendToFocused(man, e)) return true;
	}

	switch (e->type) {
		case AE_EVENT_MOUSE_BUTTON_DOWN:
		case AE_EVENT_MOUSE_BUTTON_UP: {
			int     x      = e->mouseButton.x;
			int     y      = e->mouseButton.y;
			uint8_t button = e->mouseButton.button;

			Vec2 mouse = (Vec2) {x, y};
			bool focus = false;

			for (size_t i = 0; i < man->containerLen; ++ i) {
				UI_Container* container = &man->containers[i];

				if (!container->active) continue;

				Rect rect = UI_ContainerGetRect(container);

				if (!PointInRect(mouse, rect)) {
					continue;
				}

				man->focus = container;
				focus      = true;

				bool elemFocus = false;

				for (size_t j = 0; j < container->rowAmount; ++ j) {
					UI_Row* row = &container->rows[j];

					for (size_t k = 0; k < row->elemAmount; ++ k) {
						UI_Element* elem = &row->elems[k];

						Rect elemRect = {
							rect.x + elem->x, rect.y + elem->y, elem->w, elem->h
						};

						if (PointInRect((Vec2) {x, y}, elemRect)) {
							if (elem->onClick) {
								elem->onClick(
									container, elem, button,
									e->type == AE_EVENT_MOUSE_BUTTON_DOWN
								);
							}

							container->focus = elem;
							return true;
						}
					}
				}

				if (!elemFocus) {
					container->focus = NULL;
				}

				return true;
			}

			if (!focus) man->focus = NULL;
			break;
		}
		case AE_EVENT_MOUSE_MOVE: {
			int     x      = e->mouseMove.x;
			int     y      = e->mouseMove.y;

			Vec2 mouse = (Vec2) {x, y};

			for (size_t i = 0; i < man->containerLen; ++ i) {
				UI_Container* container = &man->containers[i];

				if (!container->active) continue;

				Rect rect = UI_ContainerGetRect(container);

				if (!PointInRect(mouse, rect)) {
					continue;
				}

				bool elemFocus = false;

				for (size_t j = 0; j < container->rowAmount; ++ j) {
					UI_Row* row = &container->rows[j];

					for (size_t k = 0; k < row->elemAmount; ++ k) {
						UI_Element* elem = &row->elems[k];

						Rect elemRect = {
							rect.x + elem->x, rect.y + elem->y, elem->w, elem->h
						};

						if (PointInRect((Vec2) {x, y}, elemRect)) {
							if (elem->onEvent) {
								elem->onEvent(
									container, elem, e,
									(man->focus == container) && (container->focus == elem)
								);
							}
							return true;
						}
					}
				}

				return true;
			}
		}
	}

	// send events to the focused container first
	if (!man->priority) {
		if (SendToFocused(man, e)) return true;
	}

	for (size_t i = 0; i < man->containerLen; ++ i) {
		if (&man->containers[i] == man->focus) continue;
		if (!man->containers[i].active) continue;

		for (size_t j = 0; j < man->containers[i].rowAmount; ++ j) {
			UI_Container* cont = &man->containers[i];

			for (size_t k = 0; k < man->containers[i].rows[j].elemAmount; ++ k) {
				UI_Element* elem = &cont->rows[j].elems[k];

				if (elem->onEvent) if (elem->onEvent(cont, elem, e, false)) {
					return true;
				}
			}
		}
	}

	return false;
}

void UI_RenderBorder(size_t depth, Rect rect, bool swap) {
	Colour bright = Video_MultiplyColour(theme.bg[depth], 1.5);
	Colour dark   = Video_MultiplyColour(theme.bg[depth], 0.5);

	if (swap) {
		Colour temp = dark;

		dark   = bright;
		bright = temp;
	}

	static const int width = 2;

	// dark edges
	Backend_HLine(rect.x, rect.y + rect.h - width, width, rect.w, dark);
	Backend_VLine(rect.x + rect.w - width, rect.y, width, rect.h, dark);

	// bright edges
	Backend_HLine(rect.x, rect.y, width, rect.w, bright);
	Backend_VLine(rect.x, rect.y, width, rect.h, bright);
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
) {;
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

	Backend_RenderRect(rect, theme.bg[0]);
	UI_RenderBorder(0, rect, false);

	for (size_t rowIdx = 0; rowIdx < container->rowAmount; ++ rowIdx) {
		UI_Row* row = &container->rows[rowIdx];

		for (size_t col = 0; col < row->elemAmount; ++ col) {
			UI_Element* elem = &row->elems[col];

			// Backend_RenderRectOL((Rect) {
			// 	rect.x + elem->x, rect.y + elem->y, elem->w, elem->h
			// }, (Colour) {0xFF, 0xFF, 0xFF});

			if (!elem->render) continue;

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

void UI_RowFinish(UI_Row* row, bool autoHeight) {
	int usableSpace = row->container->w - row->container->padRight;

	if (autoHeight) {
		int preferredHeight = 0;

		for (size_t i = 0; i < row->elemAmount; ++ i) {
			if (row->elems[i].preferredHeight > preferredHeight) {
				preferredHeight = row->elems[i].preferredHeight;
			}
		}

		row->height = preferredHeight;
	}

	int resizable = 0;

	for (size_t i = 0; i < row->elemAmount; ++ i) {
		if (row->elems[i].fixedWidth == 0) {
			++ resizable;
		}
		else {
			usableSpace -= row->elems[i].fixedWidth;
		}

		usableSpace -= row->container->padLeft;
	}

	int x = row->container->padLeft;

	for (size_t i = 0; i < row->elemAmount; ++ i) {
		row->elems[i].x = x;
		row->elems[i].y = row->y;

		if (row->elems[i].fixedWidth == 0) {
			row->elems[i].w = row->elems[i].fixedWidth == 0?
				usableSpace / resizable : row->elems[i].fixedWidth;
		}
		else {
			row->elems[i].w = row->elems[i].fixedWidth;
		}

		row->elems[i].h = row->height;

		x += row->elems[i].w + row->container->padLeft;
	}
}
