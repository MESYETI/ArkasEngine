#ifndef AE_MAP_H
#define AE_MAP_H

#include "types.h"
#include "common.h"
#include "resources.h"

typedef struct {
	FVec2 pos;
} MapPoint;

typedef struct {
	bool      blank;
	bool      isPortal;
	size_t    portalSector; // TODO: check this when loading a map
	Resource* texture;
} Wall;

typedef struct {
	size_t    start;
	size_t    length;
	float     ceiling;
	float     floor;
	FVec2     floorTexOff;
	FVec2     ceilingTexOff;
	bool      floorBlank;
	bool      ceilingBlank;

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
bool Map_LoadFile(Stream* file, const char* path);
bool Map_SaveFile(Stream* file);
bool Map_PointInMap(FVec2 pos);

#endif
