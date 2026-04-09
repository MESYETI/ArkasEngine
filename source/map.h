#ifndef AE_MAP_H
#define AE_MAP_H

#include "types.h"
#include "common.h"
#include "resources.h"

typedef struct Entity Entity;

typedef struct {
	FVec2 pos;
} MapPoint;

typedef struct {
	bool      blank;
	bool      isPortal;
	size_t    portalSector; // TODO: check this when loading a map
	Resource* texture;
	FVec2     portalOff;
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

	// entities
	Entity** entities;
	size_t   entitiesNum;
} Sector;

typedef struct {
	bool      active;
	char*     name;
	MapPoint* points;
	size_t    pointsLen;
	Wall*     walls;
	size_t    wallsLen;
	Sector*   sectors;
	size_t    sectorsLen;
	Colour    fogColour;
	float     fogDistance;
} Map;

extern Map map;

void Map_Init(void);
void Map_Free(void);
void Map_LoadTest(void);
void Map_LoadTest2(void);
bool Map_LoadFile(Stream* file, const char* path);
bool Map_SaveFile(Stream* file);
void Map_PointInMap(FVec2 pos);
void Map_AddEntity(Entity* entity);
void Map_DetachEntity(Entity* entity);
void Map_DeleteEntity(Entity* entity);

#endif
