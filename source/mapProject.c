#include "map.h"
#include "mem.h"
#include "mapProject.h"

void MapProj_Init(MProject* proj) {
	proj->sectors    = NULL;
	proj->sectorsLen = 0;
}

void MapProj_Free(MProject* proj) {
	if (proj->sectors) {
		free(proj->sectors);
	}

	proj->sectors    = NULL;
	proj->sectorsLen = 0;
}

void MapProj_Export(MProject* proj) {
	Map_Free();
	Map_Init();

	size_t pointsNum = 0;
	for (size_t i = 0; i < proj->sectorsLen; ++ i) {
		pointsNum += proj->sectors[i].pointsLen;
	}

	map.active     = true;
	map.points     = SafeMalloc(sizeof(MapPoint) * pointsNum);
	map.pointsLen  = pointsNum;
	map.walls      = SafeMalloc(sizeof(Wall) * pointsNum);
	map.wallsLen   = pointsNum;
	map.sectors    = SafeMalloc(sizeof(Sector) * proj->sectorsLen);
	map.sectorsLen = proj->sectorsLen;

	pointsNum = 0;
	for (size_t i = 0; i < proj->sectorsLen; ++ i) {
		map.sectors[i] = (Sector) {
			pointsNum, proj->sectors[i].pointsLen, 0.5, -0.5,
			(FVec2) {0, 0}, (FVec2) {0, 0}, false, false,
			Resources_GetRes("base:3p_textures/grass2.png", 0),
			Resources_GetRes("base:3p_textures/wood3.png", 0),
			NULL, 0 // entities
		};

		for (size_t j = 0; j < proj->sectors[i].pointsLen; ++ j, ++ pointsNum) {
			map.points[pointsNum] = (MapPoint) {proj->sectors[i].points[j].pos};
			map.walls[pointsNum]  = (Wall) {
				false, false, 0, Resources_GetRes("base:3p_textures/brick1.png", 0),
				(FVec2) {0, 0}
			};
		}
	}
}

MProjSector* MapProj_NewSector(MProject* proj) {
	++ proj->sectorsLen;
	proj->sectors = SafeRealloc(proj->sectors, proj->sectorsLen * sizeof(MProjSector));

	MProjSector* sect = &proj->sectors[proj->sectorsLen - 1];
	sect->points      = NULL;
	sect->pointsLen   = 0;
	return sect;
}

void MapProj_AddPoint(MProjSector* sect, MProjPoint point) {
	++ sect->pointsLen;
	sect->points = SafeRealloc(sect->points, sect->pointsLen * sizeof(MProjPoint));

	sect->points[sect->pointsLen - 1] = point;
}
