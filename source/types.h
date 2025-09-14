#ifndef AE_TYPES_H
#define AE_TYPES_H

#include "common.h"

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	float x, y;
} FVec2;

typedef struct {
	int x, y, w, h;
} Rect;

typedef struct {
	float x, y, w, h;
} FRect;

typedef struct {
	float x, y, z;
} FVec3;

#endif
