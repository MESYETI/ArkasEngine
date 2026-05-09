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
		MProjSector* sect = &proj->sectors[i];

		map.sectors[i] = (Sector) {
			pointsNum, sect->pointsLen, sect->ceiling, sect->floor,
			(FVec2) {0, 0}, (FVec2) {0, 0}, false, false,
			Resources_GetRes("base:3p_textures/concrete1.png", 0),
			Resources_GetRes("base:3p_textures/concrete1.png", 0),
			NULL, 0 // entities
		};

		for (size_t j = 0; j < proj->sectors[i].pointsLen; ++ j, ++ pointsNum) {
			MProjPoint* point = &proj->sectors[i].points[j];

			// TODO: bound checking
			map.points[pointsNum] = (MapPoint) {point->pos};
			map.walls[pointsNum]  = (Wall) {
				false, point->portal, point->portalIdx,
				Resources_GetRes("base:3p_textures/rock2.png", 0), (FVec2) {0, 0}
			};
		}
	}
}

bool MapProj_Open(MProject* proj, Stream* stream) {
	uint16_t ver = Stream_Read16(stream);

	if (ver != 0) {
		Log("Map project file too new for this Arkas Engine version");
		return false;
	}

	uint32_t sectorNum   = Stream_Read32(stream);
	uint32_t stringsSize = Stream_Read32(stream);

	char* strings        = SafeMalloc(stringsSize + 1);
	strings[stringsSize] = 0;

	proj->sectors    = SafeMalloc(sectorNum * sizeof(MProjSector));
	proj->sectorsLen = sectorNum;

	if (Stream_Read(stream, stringsSize, strings) != stringsSize) {
		Log("Failed to read string table");
		free(proj->sectors);
		proj->sectorsLen = 0;
		free(strings);
		return false;
	}

	for (uint32_t i = 0; i < sectorNum; ++ i) {
		MProjSector* sector = &proj->sectors[i];

		sector->ceiling = Stream_ReadFloat(stream);
		sector->floor   = Stream_ReadFloat(stream);

		// TODO: textures
		Stream_Read32(stream);
		Stream_Read32(stream);
		Stream_Read32(stream);

		uint32_t wallsNum = Stream_Read32(stream);

		sector->pointsLen = (size_t) wallsNum;
		sector->points    = SafeMalloc(wallsNum * sizeof(MProjPoint));

		for (uint32_t j = 0; j < wallsNum; ++ j) {
			MProjPoint* point = &sector->points[j];

			point->pos.x = Stream_ReadFloat(stream);
			point->pos.y = Stream_ReadFloat(stream);

			uint32_t portal = Stream_Read32(stream);

			if (portal == 0xFFFFFFFF) {
				point->portal = false;
			}
			else {
				point->portal    = true;
				point->portalIdx = portal;

				if (portal >= sectorNum) {
					Error("Broken project file");
				}
			}

			// TODO: textures
			Stream_Read32(stream);
		}
	}

	return true;
}

void MapProj_Save(MProject* proj, Stream* stream) {
	Stream_Write16(stream, 0); // version
	Stream_Write32(stream, (uint32_t) proj->sectorsLen);
	Stream_Write32(stream, 0); // string table

	// TODO: string table

	for (size_t i = 0; i < proj->sectorsLen; ++ i) {
		MProjSector* sector = &proj->sectors[i];

		Stream_WriteFloat(stream, sector->ceiling);
		Stream_WriteFloat(stream, sector->floor);

		// TODO: textures
		Stream_Write32(stream, 0);
		Stream_Write32(stream, 0);
		Stream_Write32(stream, 0);

		Stream_Write32(stream, (uint32_t) sector->pointsLen);

		for (size_t j = 0; j < sector->pointsLen; ++ j) {
			MProjPoint* point = &sector->points[j];

			Stream_WriteFloat(stream, point->pos.x);
			Stream_WriteFloat(stream, point->pos.y);
			Stream_Write32(
				stream, point->portal? (uint32_t) point->portalIdx : 0xFFFFFFFF
			);
			Stream_Write32(stream, 0xFFFFFFFF); // TODO: textures
		}
	}
}

MProjSector* MapProj_NewSector(MProject* proj) {
	++ proj->sectorsLen;
	proj->sectors = SafeRealloc(proj->sectors, proj->sectorsLen * sizeof(MProjSector));

	MProjSector* sect = &proj->sectors[proj->sectorsLen - 1];
	sect->points      =  NULL;
	sect->pointsLen   =  0;
	sect->floor       = -0.5;
	sect->ceiling     =  0.5;
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
	bool inside = false;

	for (size_t i = 0; i < sect->pointsLen; ++ i) {
		FVec2 a = sect->points[i].pos;
		FVec2 b = i == sect->pointsLen - 1?
			sect->points[0].pos : sect->points[i + 1].pos;

		if (
			(point.y > MIN(a.y, b.y)) && (point.y <= MAX(a.y, b.y)) &&
			(point.x <= MAX(a.x, b.x))
		) {
			float intersect = (point.y - a.y) * (b.x - a.x) / (b.y - a.y) + a.x;

			if ((a.x == b.x) || (point.x <= intersect)) {
				inside = !inside;
			}
		}
	}

	return inside;
}

void MapProj_AddPoint(MProjSector* sect, MProjPoint point) {
	++ sect->pointsLen;
	sect->points = SafeRealloc(sect->points, sect->pointsLen * sizeof(MProjPoint));

	sect->points[sect->pointsLen - 1] = point;
}
