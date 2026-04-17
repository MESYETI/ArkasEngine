#ifndef AE_MAP_PROJECT_H
#define AE_MAP_PROJECT_H

#include "types.h"

typedef struct {
	FVec2  pos;
	bool   portal;
	size_t portalIdx;
} MProjPoint;

typedef struct {
	MProjPoint* points;
	size_t      pointsLen;
} MProjSector;

typedef struct {
	MProjSector* sectors;
	size_t       sectorsLen;
} MProject;

void MapProj_Init(MProject* proj);
void MapProj_Free(MProject* proj);
void MapProj_Export(MProject* proj);

MProjSector* MapProj_NewSector(MProject* proj);

void MapProj_AddPoint(MProjSector* sect, MProjPoint point);

#endif
