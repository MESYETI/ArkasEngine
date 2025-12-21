#include "backend.h"

void Backend_VLine(int x, int y, int thick, int len, Colour colour) {
	Backend_RenderRect((Rect) {x, y, thick, len}, colour);
}

void Backend_HLine(int x, int y, int thick, int len, Colour colour) {
	Backend_RenderRect((Rect) {x, y, len, thick}, colour);
}
