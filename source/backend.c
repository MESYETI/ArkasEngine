#include "backend.h"

BackendOptions backendOptions = {
	.vsync = true,
	.name  = "gl1"
};

void Backend_VLine(int x, int y, int thick, int len, Colour colour) {
	Backend_RenderRect((Rect) {x, y, thick, len}, colour);
}

void Backend_HLine(int x, int y, int thick, int len, Colour colour) {
	Backend_RenderRect((Rect) {x, y, len, thick}, colour);
}

void Backend_RenderRectOL(Rect rect, Colour colour) {
	Backend_HLine(rect.x, rect.y, 1, rect.w, colour);
	Backend_VLine(rect.x, rect.y, 1, rect.h, colour);
	Backend_HLine(rect.x, rect.y + rect.h - 1, 1, rect.w, colour);
	Backend_VLine(rect.x + rect.w - 1, rect.y, 1, rect.h, colour);
}
