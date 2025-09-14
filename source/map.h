#ifndef AE_MAP_H
#define AE_MAP_H

#include "types.h"
#include "common.h"
#include "backend.h"

typedef struct {
	FVec2 pos;
} MapPoint;

typedef struct {
	size_t   start;
	size_t   length;
	float    ceiling;
	float    floor;
	Texture* texture;
} Sector;

typedef struct {
	MapPoint* points;
	size_t    pointsLen;
	Sector*   sectors;
	size_t    sectorsLen;
} Map;

extern Map map;

void Map_Init(void);
void Map_Free(void);

#endif
