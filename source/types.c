#include "types.h"

bool PointInRect(Vec2 point, Rect rect) {
	return
		(point.x >= rect.x) && (point.x < rect.x + rect.w) &&
		(point.y >= rect.y) && (point.y < rect.y + rect.h);
}
