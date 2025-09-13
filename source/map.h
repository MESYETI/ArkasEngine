#ifndef AE_MAP_H
#define AE_MAP_H

#include "types.h"
#include "common.h"

typedef struct {
	FVec2 pos;
} MapPoint;

typedef struct {
	MapPoint* points;
	size_t    pointsLen;
} Map;

extern Map map;

void Map_Init(void);
void Map_Free(void);

#endif
