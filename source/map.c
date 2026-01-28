#include <stdio.h>
#include "map.h"
#include "util.h"
#include "mem.h"
#include "camera.h"
#include "stream.h"
#include "player.h"
#include "backend.h"

Map map = {
	NULL,
	NULL, 0, // points
	NULL, 0, // walls
	NULL, 0  // sectors
};

// static Resource* texture; // temp

void Map_Init(void) {
	map.points     = NULL;
	map.pointsLen  = 0;
	map.walls      = NULL;
	map.wallsLen   = 0;
	map.sectors    = NULL;
	map.sectorsLen = 0;
}

void Map_Free(void) {
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		Resources_FreeRes(map.sectors[i].floorTexture);
		Resources_FreeRes(map.sectors[i].ceilingTexture);
	}
	for (size_t i = 0; i < map.wallsLen; ++ i) {
		Resources_FreeRes(map.walls[i].texture);
	}

	if (map.points != NULL) {
		free(map.points);
		map.points = NULL;
	}
	if (map.walls != NULL) {
		free(map.walls);
		map.walls = NULL;
	}
	if (map.sectors != NULL) {
		free(map.sectors);
		map.sectors = NULL;
	}

	map.pointsLen  = 0;
	map.wallsLen   = 0;
	map.sectorsLen = 0;

	if (map.name != NULL) {
		free(map.name);
		map.name = NULL;
	}
}

void Map_LoadTest(void) {
	map.name      = NewString("ae_test");
	map.points    = SafeMalloc(12 * sizeof(MapPoint));
	map.pointsLen = 12;

	// sector 0
	map.points[0] = (MapPoint) {{-7,  4}};
	map.points[1] = (MapPoint) {{-2,  4}};
	map.points[2] = (MapPoint) {{ 2,  4}};
	map.points[3] = (MapPoint) {{ 7,  4}};
	map.points[4] = (MapPoint) {{ 7, -6}};
	map.points[5] = (MapPoint) {{-7, -6}};

	// sector 1
	map.points[6]  = (MapPoint) {{-5,  9}};
	map.points[7]  = (MapPoint) {{ 6,  9}};
	map.points[8]  = (MapPoint) {{ 6,  4}};
	map.points[9]  = (MapPoint) {{ 2,  4}};
	map.points[10] = (MapPoint) {{-2,  4}};
	map.points[11] = (MapPoint) {{-5,  4}};

	map.walls     = SafeMalloc(12 * sizeof(Wall));
	map.wallsLen  = 12;
	map.walls[0]  = (Wall) {false, false, 0, NULL};
	map.walls[1]  = (Wall) {false, true,  1, NULL};
	map.walls[2]  = (Wall) {false, false, 0, NULL};
	map.walls[3]  = (Wall) {false, false, 0, NULL};
	map.walls[4]  = (Wall) {false, false, 0, NULL};
	map.walls[5]  = (Wall) {false, false, 0, NULL};
	map.walls[6]  = (Wall) {false, false, 0, NULL};
	map.walls[7]  = (Wall) {false, false, 0, NULL};
	map.walls[8]  = (Wall) {false, false, 0, NULL};
	map.walls[9]  = (Wall) {false, true,  0, NULL};
	map.walls[10] = (Wall) {false, false, 0, NULL};
	map.walls[11] = (Wall) {false, false, 0, NULL};

	for (size_t i = 0; i < map.wallsLen; ++ i) {
		map.walls[i].texture = Resources_GetRes(":base/3p_textures/rock1.png", 0);
	}

	map.sectors    = SafeMalloc(2 * sizeof(Sector));
	map.sectorsLen = 2;

	map.sectors[0] = (Sector) {
		0, 6, 50, -0.5, (FVec2) {0, 0}, (FVec2) {0, 0}, false, false, NULL, NULL
	};
	map.sectors[1] = (Sector) {
		6, 6, 10, -0.3, (FVec2) {0, 0}, (FVec2) {0, 0}, false, false, NULL, NULL
	};

	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		map.sectors[i].floorTexture =
			Resources_GetRes(":base/3p_textures/grass3.png", 0);
		map.sectors[i].ceilingTexture =
			Resources_GetRes(":base/3p_textures/rock2.png", 0);
	}

	camera.sector = &map.sectors[0];
	player.sector = &map.sectors[0];
}

bool Map_LoadFile(const char* path) {
	FILE* fd = fopen(path, "rb+");

	Stream file = Stream_File(fd, true);

	Map_Free();

	char* baseName = strrchr(path, '/');
	if (baseName == NULL) {
		map.name = NewString(path);
	}
	else {
		map.name = NewString(baseName + 1);
	}

	char* ext = strrchr(map.name, '.');
	if (ext != NULL) {
		*ext = 0;
		map.name = SafeRealloc(map.name, strlen(map.name) + 1);
	}

	Log("Loading '%s'", map.name);

	if (fd == NULL) return false;
	map.pointsLen  = Stream_Read32(&file);
	map.wallsLen   = Stream_Read32(&file);
	map.sectorsLen = Stream_Read32(&file);

	map.points  = SafeMalloc(map.pointsLen  * sizeof(MapPoint));
	map.walls   = SafeMalloc(map.wallsLen   * sizeof(Wall));
	map.sectors = SafeMalloc(map.sectorsLen * sizeof(Sector));

	size_t stringsLen  = Stream_Read32(&file);
	char** stringTable = SafeMalloc((stringsLen + 1) * sizeof(char*));

	stringTable[stringsLen] = NULL;

	// read strings
	for (size_t i = 0; i < stringsLen; ++ i) {
		stringTable[i] = Stream_ReadString(&file);
	}

	// read points
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		map.points[i].pos.x = Stream_ReadFloat(&file);
		map.points[i].pos.y = Stream_ReadFloat(&file);
	}

	// read walls
	for (size_t i = 0; i < map.wallsLen; ++ i) {
		map.walls[i].blank        = false;
		map.walls[i].isPortal     = Stream_Read8(&file) != 0;
		map.walls[i].portalSector = Stream_Read32(&file);

		if (map.walls[i].portalSector >= map.sectorsLen) {
			Error("Out of bounds wall portal offset");
		}

		size_t texture = Stream_Read32(&file);
		if (texture >= stringsLen) {
			Error("Out of bounds string table offset");
		}

		map.walls[i].texture = Resources_GetRes(stringTable[texture], 0);
	}

	// read sectors
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		map.sectors[i].start   = Stream_Read32(&file);
		map.sectors[i].length  = Stream_Read32(&file);
		map.sectors[i].ceiling = Stream_ReadFloat(&file);
		map.sectors[i].floor   = Stream_ReadFloat(&file);
		// map.sectors[i].texture = texture;

		map.sectors[i].floorTexOff   = (FVec2) {0.0, 0.0};
		map.sectors[i].ceilingTexOff = (FVec2) {0.0, 0.0};
		map.sectors[i].floorBlank    = false;
		map.sectors[i].ceilingBlank  = false;

		uint32_t floorTexture = Stream_Read32(&file);
		uint32_t ceilTexture  = Stream_Read32(&file);

		if ((floorTexture >= stringsLen) || (ceilTexture >= stringsLen)) {
			Error("Out of bounds string table offset");
		}
		map.sectors[i].floorTexture   = Resources_GetRes(stringTable[floorTexture], 0);
		map.sectors[i].ceilingTexture = Resources_GetRes(stringTable[ceilTexture], 0);
	}

	Stream_Close(&file);

	Log("Loaded map");
	camera.sector = &map.sectors[0];

	FreeStrArray(stringTable);
	return true;
}

bool Map_SaveFile(const char* path) {
	FILE* fd = fopen(path, "wb");

	if (fd == NULL) return false;

	Stream file = Stream_File(fd, true);

	char** stringTable = SafeMalloc(sizeof(char*));
	*stringTable       = NULL;

	for (size_t i = 0; i < map.wallsLen; ++ i) {
		if (!StrArrayContains(stringTable, map.walls[i].texture->name)) {
			stringTable = AppendStrArray(stringTable, map.walls[i].texture->name);
		}
	}

	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		Sector* sector = &map.sectors[i];

		if (!StrArrayContains(stringTable, sector->floorTexture->name)) {
			stringTable = AppendStrArray(stringTable, sector->floorTexture->name);
		}
		if (!StrArrayContains(stringTable, sector->ceilingTexture->name)) {
			stringTable = AppendStrArray(stringTable, sector->ceilingTexture->name);
		}
	}

	Stream_Write32(&file, (uint32_t) map.pointsLen);
	Stream_Write32(&file, (uint32_t) map.wallsLen);
	Stream_Write32(&file, (uint32_t) map.sectorsLen);
	Stream_Write32(&file, StrArrayLength(stringTable));

	// write strings
	for (size_t i = 0; stringTable[i]; ++ i) {
		Stream_WriteString(&file, stringTable[i]);
	}

	// write points
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		Stream_WriteFloat(&file, map.points[i].pos.x);
		Stream_WriteFloat(&file, map.points[i].pos.y);
	}

	// write walls
	for (size_t i = 0; i < map.wallsLen; ++ i) {
		Stream_Write8(&file, map.walls[i].isPortal? 1 : 0);
		Stream_Write32(&file, map.walls[i].portalSector);
		Stream_Write32(
			&file, (uint32_t) StrArrayFind(stringTable, map.walls[i].texture->name)
		);
	}

	// write sectors
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		Stream_Write32(&file, map.sectors[i].start);
		Stream_Write32(&file, map.sectors[i].length);
		Stream_WriteFloat(&file, map.sectors[i].ceiling);
		Stream_WriteFloat(&file, map.sectors[i].floor);
		Stream_Write32(
			&file,
			(uint32_t) StrArrayFind(stringTable, map.sectors[i].floorTexture->name)
		);
		Stream_Write32(
			&file,
			(uint32_t) StrArrayFind(stringTable, map.sectors[i].ceilingTexture->name)
		);
	}

	FreeStrArray(stringTable);

	Stream_Close(&file);

	Log("Saved map '%s'", map.name);

	return true;
}
