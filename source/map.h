#ifndef AE_MAP_H
#define AE_MAP_H

#include "types.h"
#include "common.h"
#include "resources.h"

typedef struct {
	FVec2 pos;
} MapPoint;

typedef struct {
	bool      isPortal;
	size_t    portalSector;
	Resource* texture;
} Wall;

typedef struct {
	size_t    start;
	size_t    length;
	float     ceiling;
	float     floor;

	Resource* floorTexture;
	Resource* ceilingTexture;
} Sector;

typedef struct {
	char*     name;
	MapPoint* points;
	size_t    pointsLen;
	Wall*     walls;
	size_t    wallsLen;
	Sector*   sectors;
	size_t    sectorsLen;
} Map;

extern Map map;

void Map_Init(void);
void Map_Free(void);
void Map_LoadTest(void);
bool Map_LoadFile(const char* path);
bool Map_SaveFile(const char* path);

#endif
