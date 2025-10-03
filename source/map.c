#include <stdio.h>
#include "map.h"
#include "file.h"
#include "util.h"
#include "mem.h"
#include "camera.h"
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
	}
	if (map.walls != NULL) {
		free(map.walls);
	}
	if (map.sectors != NULL) {
		free(map.sectors);
	}

	map.pointsLen  = 0;
	map.wallsLen   = 0;
	map.sectorsLen = 0;

	if (map.name != NULL) {
		free(map.name);
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
	map.walls[0]  = (Wall) {false, 0, NULL};
	map.walls[1]  = (Wall) {true,  1, NULL};
	map.walls[2]  = (Wall) {false, 0, NULL};
	map.walls[3]  = (Wall) {false, 0, NULL};
	map.walls[4]  = (Wall) {false, 0, NULL};
	map.walls[5]  = (Wall) {false, 0, NULL};
	map.walls[6]  = (Wall) {false, 0, NULL};
	map.walls[7]  = (Wall) {false, 0, NULL};
	map.walls[8]  = (Wall) {false, 0, NULL};
	map.walls[9]  = (Wall) {true,  0, NULL};
	map.walls[10] = (Wall) {false, 0, NULL};
	map.walls[11] = (Wall) {false, 0, NULL};

	for (size_t i = 0; i < map.wallsLen; ++ i) {
		map.walls[i].texture = Resources_GetRes(":base/3p_textures/rock1.png");
	}

	map.sectors    = SafeMalloc(2 * sizeof(Sector));
	map.sectorsLen = 2;

	map.sectors[0] = (Sector) {0, 6, 50, -0.5, NULL, NULL};
	map.sectors[1] = (Sector) {6, 6, 10, -0.3, NULL, NULL};

	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		map.sectors[i].floorTexture   = Resources_GetRes(":base/3p_textures/grass3.png");
		map.sectors[i].ceilingTexture = Resources_GetRes(":base/3p_textures/rock2.png");
	}

	camera.sector = &map.sectors[0];
	player.sector = &map.sectors[0];
}

bool Map_LoadFile(const char* path) {
	FILE* file = fopen(path, "rb+");

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

	if (file == NULL) return false;
	map.pointsLen  = File_Read32(file);
	map.wallsLen   = File_Read32(file);
	map.sectorsLen = File_Read32(file);

	map.points  = SafeMalloc(map.pointsLen  * sizeof(MapPoint));
	map.walls   = SafeMalloc(map.wallsLen   * sizeof(Wall));
	map.sectors = SafeMalloc(map.sectorsLen * sizeof(Sector));

	size_t stringsLen  = File_Read32(file);
	char** stringTable = SafeMalloc((stringsLen + 1) * sizeof(char*));

	stringTable[stringsLen] = NULL;

	// read strings
	for (size_t i = 0; i < stringsLen; ++ i) {
		stringTable[i] = File_ReadString(file);
	}

	// read points
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		map.points[i].pos.x = File_ReadFloat(file);
		map.points[i].pos.y = File_ReadFloat(file);
	}

	// read walls
	for (size_t i = 0; i < map.wallsLen; ++ i) {
		map.walls[i].isPortal     = File_Read8(file) != 0;
		map.walls[i].portalSector = File_Read32(file);

		if (map.walls[i].portalSector >= map.sectorsLen) {
			Error("Out of bounds wall portal offset");
		}

		size_t texture = File_Read32(file);
		if (texture >= stringsLen) {
			Error("Out of bounds string table offset");
		}

		map.walls[i].texture = Resources_GetRes(stringTable[texture]);
	}

	// read sectors
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		map.sectors[i].start   = File_Read32(file);
		map.sectors[i].length  = File_Read32(file);
		map.sectors[i].ceiling = File_ReadFloat(file);
		map.sectors[i].floor   = File_ReadFloat(file);
		// map.sectors[i].texture = texture;

		uint32_t floorTexture = File_Read32(file);
		uint32_t ceilTexture  = File_Read32(file);

		if ((floorTexture >= stringsLen) || (ceilTexture >= stringsLen)) {
			Error("Out of bounds string table offset");
		}
		map.sectors[i].floorTexture   = Resources_GetRes(stringTable[floorTexture]);
		map.sectors[i].ceilingTexture = Resources_GetRes(stringTable[ceilTexture]);
	}

	fclose(file);

	Log("Loaded map");
	camera.sector = &map.sectors[0];

	FreeStrArray(stringTable);
	return true;
}

bool Map_SaveFile(const char* path) {
	FILE* file = fopen(path, "wb");

	if (file == NULL) return false;

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

	File_Write32(file, (uint32_t) map.pointsLen);
	File_Write32(file, (uint32_t) map.wallsLen);
	File_Write32(file, (uint32_t) map.sectorsLen);
	File_Write32(file, StrArrayLength(stringTable));

	// write strings
	for (size_t i = 0; stringTable[i]; ++ i) {
		File_WriteString(file, stringTable[i]);
	}

	// write points
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		File_WriteFloat(file, map.points[i].pos.x);
		File_WriteFloat(file, map.points[i].pos.y);
	}

	// write walls
	for (size_t i = 0; i < map.wallsLen; ++ i) {
		File_Write8(file, map.walls[i].isPortal? 1 : 0);
		File_Write32(file, map.walls[i].portalSector);
		File_Write32(
			file, (uint32_t) StrArrayFind(stringTable, map.walls[i].texture->name)
		);
	}

	// write sectors
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		File_Write32(file, map.sectors[i].start);
		File_Write32(file, map.sectors[i].length);
		File_WriteFloat(file, map.sectors[i].ceiling);
		File_WriteFloat(file, map.sectors[i].floor);
		File_Write32(
			file,
			(uint32_t) StrArrayFind(stringTable, map.sectors[i].floorTexture->name)
		);
		File_Write32(
			file,
			(uint32_t) StrArrayFind(stringTable, map.sectors[i].ceilingTexture->name)
		);
	}

	FreeStrArray(stringTable);

	fclose(file);

	Log("Saved map '%s'", map.name);

	return true;
}
