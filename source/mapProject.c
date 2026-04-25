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
			MProjPoint* point = &proj->sectors[i].points[j];

			// TODO: bound checking
			map.points[pointsNum] = (MapPoint) {point->pos};
			map.walls[pointsNum]  = (Wall) {
				false, point->portal, point->portalIdx,
				Resources_GetRes("base:3p_textures/brick1.png", 0), (FVec2) {0, 0}
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

FVec2 MapProj_SectorCenter(MProjSector* sect) {
	FVec2 min = sect->points[0].pos;
	FVec2 max = sect->points[0].pos;

	for (size_t i = 1; i < sect->pointsLen; ++ i) {
		FVec2 point = sect->points[i].pos;

		if (point.x < min.x) min.x = point.x;
		if (point.y < min.y) min.y = point.y;
		if (point.x > max.x) max.x = point.x;
		if (point.y > max.y) max.y = point.y;
	}

	return (FVec2) {
		Lerp(min.x, max.x, 0.5), Lerp(min.y, max.y, 0.5)
	};
}

bool MapProj_PointInSector(MProjSector* sect, FVec2 point) {
	FVec2 center = MapProj_SectorCenter(sect);
	float angle  = GetAngle(point, center);

	FVec2 pointB = {
		point.x + (CosDeg(angle) * 5000.0f), point.y + (SinDeg(angle) * 5000.0f)
	};

	int collisions = 0;
	for (size_t i = 0; i < sect->pointsLen; ++ i) {
		FVec2 a = sect->points[i].pos;
		FVec2 b = i == sect->pointsLen - 1?
			sect->points[0].pos : sect->points[i + 1].pos;

		FVec2 intersection = LineIntersect(point, pointB, a, b);

		if (PointInLine(intersection, a, b) && PointInLine(intersection, point, pointB)) {
			++ collisions;
		}
	}

	return collisions <= 1;
}

void MapProj_AddPoint(MProjSector* sect, MProjPoint point) {
	++ sect->pointsLen;
	sect->points = SafeRealloc(sect->points, sect->pointsLen * sizeof(MProjPoint));

	sect->points[sect->pointsLen - 1] = point;
}
