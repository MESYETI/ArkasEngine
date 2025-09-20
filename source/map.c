#include <stdio.h>
#include "map.h"
#include "file.h"
#include "util.h"
#include "safe.h"
#include "camera.h"
#include "backend.h"

Map map = {
	NULL,
	NULL, 0, // points
	NULL, 0, // walls
	NULL, 0  // sectors
};

static Texture* texture; // temp

void Map_Init(void) {
	texture = Backend_LoadTexture("texture.png");
}

void Map_Free(void) {
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

	Backend_FreeTexture(texture);

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
	map.walls[0]  = (Wall) {false};
	map.walls[1]  = (Wall) {true, 1};
	map.walls[2]  = (Wall) {false};
	map.walls[3]  = (Wall) {false};
	map.walls[4]  = (Wall) {false};
	map.walls[5]  = (Wall) {false};
	map.walls[6]  = (Wall) {false};
	map.walls[7]  = (Wall) {false};
	map.walls[8]  = (Wall) {false};
	map.walls[9]  = (Wall) {true, 0};
	map.walls[10] = (Wall) {false};
	map.walls[11] = (Wall) {false};

	map.sectors    = SafeMalloc(2 * sizeof(Sector));
	map.sectorsLen = 2;

	map.sectors[0] = (Sector) {0, 6, 0.5, -0.5, texture};
	map.sectors[1] = (Sector) {6, 6, 0.5, -0.5, texture};

	camera.sector = &map.sectors[0];
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
	map.pointsLen  = File_Read32Bit(file);
	map.wallsLen   = File_Read32Bit(file);
	map.sectorsLen = File_Read32Bit(file);

	map.points  = SafeMalloc(map.pointsLen  * sizeof(MapPoint));
	map.walls   = SafeMalloc(map.wallsLen   * sizeof(Wall));
	map.sectors = SafeMalloc(map.sectorsLen * sizeof(Sector));

	size_t stringsLen  = File_Read32Bit(file);
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
		map.walls[i].isPortal     = File_ReadByte(file) != 0;
		map.walls[i].portalSector = File_Read32Bit(file);

		size_t texture = File_Read32Bit(file);
		(void) texture;
	}

	// read sectors
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		map.sectors[i].start   = File_Read32Bit(file);
		map.sectors[i].length  = File_Read32Bit(file);
		map.sectors[i].ceiling = File_ReadFloat(file);
		map.sectors[i].floor   = File_ReadFloat(file);
		map.sectors[i].texture = texture;

		uint32_t ceilTexture  = File_Read32Bit(file);
		uint32_t floorTexture = File_Read32Bit(file);
		(void) ceilTexture;
		(void) floorTexture;
	}

	Log("Loaded map");
	camera.sector = &map.sectors[0];

	FreeStrArray(stringTable);
	return true;
}

bool Map_SaveFile(const char* path) {
	FILE* file = fopen(path, "wb");

	if (file == NULL) return false;

	File_Write32Bit(file, (uint32_t) map.pointsLen);
	File_Write32Bit(file, (uint32_t) map.wallsLen);
	File_Write32Bit(file, (uint32_t) map.sectorsLen);
	File_Write32Bit(file, 0);

	// write points
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		File_WriteFloat(file, map.points[i].pos.x);
		File_WriteFloat(file, map.points[i].pos.y);
	}

	// write walls
	for (size_t i = 0; i < map.wallsLen; ++ i) {
		File_WriteByte(file, map.walls[i].isPortal? 1 : 0);
		File_Write32Bit(file, map.walls[i].portalSector);
		File_Write32Bit(file, 0);
	}

	// write sectors
	for (size_t i = 0; i < map.sectorsLen; ++ i) {
		File_Write32Bit(file, map.sectors[i].start);
		File_Write32Bit(file, map.sectors[i].length);
		File_WriteFloat(file, map.sectors[i].ceiling);
		File_WriteFloat(file, map.sectors[i].floor);
		File_Write32Bit(file, 0);
		File_Write32Bit(file, 0);
	}

	Log("Saved map '%s'", map.name);

	return true;
}
