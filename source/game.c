#include "app.h"
#include "text.h"
#include "game.h"
#include "util.h"
#include "video.h"
#include "camera.h"
#include "backend.h"

void Game_Init(void) {
	Map_Init();
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera.pos.x = 0.0;
	camera.pos.y = 0.0;
	camera.pos.z = 0.0;
	camera.pitch = 0.0;
	camera.yaw   = 0.0;
	camera.roll  = 0.0;
}

void Game_Free(void) {
	Map_Free();
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Game_Update(bool top) {
	if (!top || app.console) return;

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	static const float sensitivity = 180.0;
	static const float speed       = 2.0;

	bool  moved  = false;
	FVec3 oldPos = camera.pos;

	if (keys[SDL_SCANCODE_W]) {
		camera.pos.z += CosDeg(camera.yaw) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw) * app.delta * speed;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_A]) {
		camera.pos.z += CosDeg(camera.yaw - 90) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw - 90) * app.delta * speed;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_S]) {
		camera.pos.z += CosDeg(camera.yaw + 180) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw + 180) * app.delta * speed;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_D]) {
		camera.pos.z += CosDeg(camera.yaw + 90) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw + 90) * app.delta * speed;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_SPACE]) {
		camera.pos.y += app.delta * 5.0;
	}
	if (keys[SDL_SCANCODE_LSHIFT]) {
		camera.pos.y -= app.delta * 5.0;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		camera.yaw += app.delta * sensitivity;
	}
	if (keys[SDL_SCANCODE_LEFT]) {
		camera.yaw -= app.delta * sensitivity;
	}
	if (keys[SDL_SCANCODE_P]) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	if (keys[SDL_SCANCODE_O]) {
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}

	if (moved) {
		// camera
		FVec2 a1 = (FVec2) {oldPos.x,     oldPos.z};
		FVec2 a2 = (FVec2) {camera.pos.x, camera.pos.z};

		for (size_t i = 0; i < camera.sector->length; ++ i) {
			size_t idx  = camera.sector->start + i;
			Wall*  wall = &map.walls[idx];

			if (!wall->isPortal) continue;

			FVec2 b1 = (FVec2) {map.points[idx].pos.x, map.points[idx].pos.y};
			FVec2 b2;
			size_t b2Idx;

			b2Idx = i == camera.sector->length - 1? camera.sector->start : idx + 1;
			b2    = (FVec2) {map.points[b2Idx].pos.x, map.points[b2Idx].pos.y};

			FVec2 intersect = LineIntersect(a1, a2, b1, b2);

			// check if camera is inside the sector
			// this prevents an issue where you walk into another sector but
			// the portal in that sector puts you back in the old sector
			if (PointLineSide(a2, b1, b2) < 0) continue;

			// add a check for the wall line if you want to support concave sectors
			if (PointInLine(intersect, a1, a2)) {
				camera.sector = &map.sectors[wall->portalSector];
			}
		}
	}
}

void Game_Render(void) {
	Backend_RenderScene();

	static char text[80];
	snprintf(text, 80, "FPS: %d", (int) (1 / app.delta));
	Text_Render(&app.font, text, 8, 8);

	snprintf(text, 80, "X: %g", camera.pos.x);
	Text_Render(&app.font, text, 8, 24);
	snprintf(text, 80, "Y: %g", camera.pos.y);
	Text_Render(&app.font, text, 8, 40);
	snprintf(text, 80, "Z: %g", camera.pos.z);
	Text_Render(&app.font, text, 8, 56);
	snprintf(text, 80, "Sector: %d", (int) (camera.sector - map.sectors));
	Text_Render(&app.font, text, 8, 72);
}
