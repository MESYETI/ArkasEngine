#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "mem.h"
#include "util.h"
#include "video.h"
#include "common.h"
#include "console.h"

float RadToDeg(float rad) {
	return rad * 180 / PI;
}

float DegToRad(float deg) {
	return deg * (PI / 180);
}

float Lerp(float a, float b, float t) {
	return a * (1.0 - t) + (b * t);
}

FVec2 LerpVec(FVec2 a, FVec2 b, float t) {
	return (FVec2) {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
}

float CosDeg(float deg) {
	return cos(DegToRad(deg));
}

float SinDeg(float deg) {
	return sin(DegToRad(deg));
}

bool FloatEqual(float a, float b, float margin) {
	return fabsf(a - b) < margin;
}

float Distance(FVec2 a, FVec2 b) {
	float x = (b.x - a.x) * (b.x - a.x);
	float y = (b.y - a.y) * (b.y - a.y);
	return sqrtf(x + y);
}

float DistanceI(Vec2 a, Vec2 b) {
	return Distance(
		(FVec2) {(float) a.x, (float) a.y}, (FVec2) {(float) b.x, (float) b.y}
	);
}

float GetAngle(FVec2 a, FVec2 b) {
	return RadToDeg(atan2(b.y - a.y, b.x - a.x));
}

#define CROSS_PRODUCT(X1, Y1, X2, Y2) ((X1) * (Y2) - (X2) * (Y1))

static double PointCrossProduct(FVec2 a, FVec2 b) {
	return CROSS_PRODUCT(a.x, a.y, b.x, b.y);
}

FVec2 LineIntersect(FVec2 a1, FVec2 b1, FVec2 b2, FVec2 a2) {
	double vxp1 = PointCrossProduct(a1, b1);
	double vxp2 = PointCrossProduct(a2, b2);
	double vxp3 = CROSS_PRODUCT(a1.x - b1.x, a1.y - b1.y, a2.x - b2.x, a2.y - b2.y);
	return (FVec2) {
		CROSS_PRODUCT(vxp1, a1.x - b1.x, vxp2, a2.x - b2.x) / vxp3,
		CROSS_PRODUCT(vxp1, a1.y - b1.y, vxp2, a2.y - b2.y) / vxp3,
	};
}

float PointLineSide(FVec2 p, FVec2 a, FVec2 b) {
    return CROSS_PRODUCT(b.x - a.x, b.y - a.y, p.x - a.x, p.y - a.y);
}

#define BOUND_MARGIN 0.001
bool PointInLine(FVec2 p, FVec2 a, FVec2 b) {
	if (a.x > b.x) SWAP(float, a.x, b.x);

	if ((p.x < a.x - BOUND_MARGIN) || (p.x > b.x + BOUND_MARGIN)) return false;

	if (a.y > b.y) SWAP(float, a.y, b.y);

	if ((p.y < a.y - BOUND_MARGIN) || (p.y > b.y + BOUND_MARGIN)) return false;

	return true;
}
#undef BOUND_MARGIN

bool RectLineCollision(FVec2 a, FVec2 b, FRect rect, FVec2* res) {
	FVec2 lines[5] = {
		{rect.x, rect.y}, {rect.x + rect.w, rect.y},
		{rect.x + rect.w, rect.y + rect.h}, {rect.x, rect.y + rect.h}
	};
	lines[4] = lines[0];

	FVec2 center = {rect.x + (rect.w / 2), rect.y + (rect.h / 2)};

	for (int i = 0; i < 4; ++ i) {
		FVec2 intersection = LineIntersect(lines[i], lines[i + 1], a, b);

		if (PointInLine(intersection, lines[i], lines[i + 1])) {
			*res = (FVec2) {
				intersection.x - center.x, intersection.y - center.y
			};
			return true;
		}
	}

	return false;
}

float LinePointDistance(FVec2 la, FVec2 lb, FVec2 point) {
	float a = -(lb.y - la.y);
	float b = lb.x - la.x;
	float c = -(a * la.x) - (b * la.y);

	float dist  = (a * point.x) + (b * point.y) + c;
	dist        = fabsf(dist);
	dist       /= sqrtf((a * a) + (b * b));

	return dist;
}

char* NewString(const char* src) {
	char* ret = SafeMalloc(strlen(src) + 1);
	strcpy(ret, src);
	return ret;
}

char* ConcatString(const char* first, const char* second) {
	char* ret = SafeMalloc(strlen(first) + strlen(second) + 1);
	strcpy(ret, first);
	strcat(ret, second);
	return ret;
}

size_t StrArrayLength(char** array) {
	size_t ret = 0;

	while (*array != NULL) {
		++ ret;
		++ array;
	}

	return ret;
}

char** AppendStrArray(char** array, char* string) {
	size_t len = StrArrayLength(array);

	array          = SafeRealloc(array, (len + 2) * sizeof(char*));
	array[len]     = string;
	array[len + 1] = NULL;
	return array;
}

bool StrArrayContains(char** array, char* string) {
	while (true) {
		if (*array == NULL) {
			return false;
		}
		else if (strcmp(*array, string) == 0) {
			return true;
		}

		++ array;
	}
}

size_t StrArrayFind(char** array, char* string) {
	for (size_t i = 0; array[i]; ++ i) {
		if (strcmp(array[i], string) == 0) {
			return i;
		}
	}

	assert(0);
}

void FreeStrArray(char** array) {
	for (size_t i = 0; array[i] != NULL; ++ i) {
		free(array[i]);
	}

	free(array);
}

const char* BaseName(const char* path) {
	char* ret = strrchr(path, '/');

	if (ret) return ret + 1;

	ret = strrchr(path, ':');

	if (ret) return ret + 1;

	return ret;
}
