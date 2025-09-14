#include "map.h"
#include "safe.h"
#include "camera.h"
#include "backend.h"

Map map;

void Map_Init(void) {
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

	Texture* texture = Backend_LoadTexture("texture.png");

	map.sectors[0] = (Sector) {0, 6, 0.5, -0.5, texture};
	map.sectors[1] = (Sector) {6, 6, 0.5, -0.5, texture};

	camera.sector = &map.sectors[0];
}

void Map_Free(void) {
	map.points    = NULL;
	map.pointsLen = 0;

	Backend_FreeTexture(map.sectors[0].texture);
}
