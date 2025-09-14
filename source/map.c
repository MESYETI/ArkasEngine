#include "map.h"
#include "safe.h"
#include "camera.h"
#include "backend.h"

Map map;

void Map_Init(void) {
	map.points    = SafeMalloc(6 * sizeof(MapPoint));
	map.pointsLen = 6;

	map.points[0] = (MapPoint) {{-3.0,  4.0}};
	map.points[1] = (MapPoint) {{ 1.0,  5.0}};
	map.points[2] = (MapPoint) {{ 3.0,  2.0}};
	map.points[3] = (MapPoint) {{ 2.0, -1.0}};
	map.points[4] = (MapPoint) {{-1.0, -3.0}};
	map.points[5] = (MapPoint) {{-3.0,  1.0}};

	map.sectors    = SafeMalloc(1 * sizeof(Sector));
	map.sectorsLen = 1;

	map.sectors[0] = (Sector) {0, 6, 0.5, -0.5, Backend_LoadTexture("texture.png")};

	camera.sector = &map.sectors[0];
}

void Map_Free(void) {
	map.points    = NULL;
	map.pointsLen = 0;

	Backend_FreeTexture(map.sectors[0].texture);
}
