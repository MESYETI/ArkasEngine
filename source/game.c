#include "text.h"
#include "game.h"
#include "util.h"
#include "video.h"
#include "camera.h"
#include "config.h"
#include "engine.h"
#include "player.h"
#include "backend.h"
#include "audio.h"

GameBaseConfig gameBaseConfig = {
	.debugInfoLevel = 0,
	.noclip         = false,
	.forward        = INPUT_BIND_NONE,
	.left           = INPUT_BIND_NONE,
	.backward       = INPUT_BIND_NONE,
	.right          = INPUT_BIND_NONE,
	.jump           = INPUT_BIND_NONE
};

AudioEmitter emitters2d[1];
AudioEmitter emitters3d[2];

void GameBase_Init(void) {
	//Map_Init();
	Audio_StartAudio();

	camera.pos.x = 0.0;
	camera.pos.y = 0.0;
	camera.pos.z = 0.0;
	camera.pitch = 0.0;
	camera.yaw   = 0.0;
	camera.roll  = 0.0;

	// setAudioEnv(
	// 	AUDIOENVMASK_REVERB,
	// 	&(struct audioenv) {.reverb = {0.07, 0.65, 1.0, 0.1, 0.25, 0.25}},
	// 	AUDIOENVMASK_ALL
	// );

// 	emitters2d[0] = new2DAudioEmitter(
// 		AUDIOPRIO_DEFAULT, -1, 0,
// 		0, NULL
// 	);
// 	Resource* resource = Resources_GetRes("base:sfx/air1.ogg", 0);
// 	if (resource) {
// 		Audio_Play2DSound(
// 			emitters2d[0], resource,
// 			AUDIOPRIO_DEFAULT, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP,
// 			AUDIOFXMASK_SPEED | AUDIOFXMASK_VOL,
// 			&(struct audiofx) {.speed = 0.3f, .vol = {0.65f, 0.65f}}
// 		);
// 		Resources_FreeRes(resource);
// 	}
// 
// 	emitters3d[0] = new3DAudioEmitter(
// 		AUDIOPRIO_DEFAULT, -1, 0,
// 		0, NULL,
// 		AUDIO3DFXMASK_POS, &(struct audio3dfx){.pos = {-5.0f, -0.3f, 9.0f}}
// 	);
// 	emitters3d[1] = new3DAudioEmitter(
// 		AUDIOPRIO_DEFAULT, -1, 0,
// 		0, NULL,
// 		AUDIO3DFXMASK_POS, &(struct audio3dfx){.pos = {6.0f, -0.3f, 4.0f}}
// 	);
// 
// 	resource = Resources_GetRes("base:sfx/drip1.ogg", 0);
// 
// 	if (resource) {
// 		Audio_Play3DSound(
// 			emitters3d[0], resource,
// 			AUDIOPRIO_DEFAULT, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP,
// 			AUDIOFXMASK_SPEED, &(struct audiofx){.speed = 1.56521f}
// 		);
// 		Audio_Play3DSound(
// 			emitters3d[1], resource,
// 			AUDIOPRIO_DEFAULT, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP,
// 			AUDIOFXMASK_SPEED, &(struct audiofx){.speed = 1.10435f}
// 		);
// 		Resources_FreeRes(resource);
// 	}

	gameBaseConfig.sensitivity = 7.5;

	Player_Init();
}

void GameBase_Free(void) {
	Audio_StopAudio();
	Map_Free();
}

void GameBase_Update(bool top) {
	if (!top || engine.console) return;

	if (!map.active) return;

	// static const float sensitivity = 180.0;
	float speed = player.speed;

	if (!FloatEqual(player.pos.y, player.sector->floor, 0.01)) {
		speed = player.airSpeed;
	}

	bool  moved  = false;
	FVec3 oldPos = camera.pos;

	if (Input_BindPressed(gameBaseConfig.forward)) {
		player.acc.z += CosDeg(player.yaw) * speed * engine.delta;
		player.acc.x += SinDeg(player.yaw) * speed * engine.delta;
		moved         = true;
	}
	if (Input_BindPressed(gameBaseConfig.left)) {
		player.acc.z += CosDeg(player.yaw - 90) * speed * engine.delta;
		player.acc.x += SinDeg(player.yaw - 90) * speed * engine.delta;
		moved         = true;
	}
	if (Input_BindPressed(gameBaseConfig.backward)) {
		player.acc.z += CosDeg(player.yaw + 180) * speed * engine.delta;
		player.acc.x += SinDeg(player.yaw + 180) * speed * engine.delta;
		moved         = true;
	}
	if (Input_BindPressed(gameBaseConfig.right)) {
		player.acc.z += CosDeg(player.yaw + 90) * speed * engine.delta;
		player.acc.x += SinDeg(player.yaw + 90) * speed * engine.delta;
		moved         = true;
	}
	// if (Input_KeyPressed(AE_KEY_P)) {
	// 	SDL_SetRelativeMouseMode(SDL_TRUE);
	// }
	// if (Input_KeyPressed(AE_KEY_O)) {
	// 	SDL_SetRelativeMouseMode(SDL_FALSE);
	// }

	if (Input_BindPressed(gameBaseConfig.left)) {
		camera.roll = -2.0;
	}
	else if (Input_BindPressed(gameBaseConfig.right)) {
		camera.roll = 2.0;
	}
	else {
		camera.roll = 0.0;
	}

	Player_Physics();
	Player_FPCamera();
	player.acc.x = 0.0;
	player.acc.y = 0.0;
	player.acc.z = 0.0;

	{
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
			if (PointInLine(intersect, a1, a2) && PointInLine(intersect, b1, b2)) {
				player.sector = &map.sectors[wall->portalSector];
				camera.sector = &map.sectors[wall->portalSector];

				if (
					(player.sector->floor > player.pos.y) ||
					(player.sector->floor - player.pos.y <= 0.6)
				) {
					player.pos.y = player.sector->floor;
					player.vel.y = 0.0;
				}

				player.pos.x -= wall->portalOff.x;
				player.pos.z -= wall->portalOff.y;
			}
		}
	};

	// check collision with walls
	if (gameBaseConfig.noclip) return;
	for (size_t i = 0; i < player.sector->length; ++ i) {
		size_t idx  = player.sector->start + i;
		Wall*  wall = &map.walls[idx];

		if (wall->isPortal) continue; // TODO: solid portals

		FVec2 a = (FVec2) {map.points[idx].pos.x, map.points[idx].pos.y};
		FVec2 b;
		size_t bIdx;

		bIdx = i == player.sector->length - 1? player.sector->start : idx + 1;
		b    = (FVec2) {map.points[bIdx].pos.x, map.points[bIdx].pos.y};

		float dist = LinePointDistance(a, b, (FVec2) {player.pos.x, player.pos.z});
		if (dist < 0.2) {
			float angleToWall = GetAngle(a, b) + 90.0;
			FVec2 wallPoint;

			wallPoint.x = player.pos.x + (CosDeg(angleToWall) * dist);
			wallPoint.y = player.pos.z + (SinDeg(angleToWall) * dist);

			if (!PointInLine(wallPoint, a, b)) {
				continue;
			}

			float angleToPlayer = GetAngle(
				wallPoint, (FVec2) {player.pos.x, player.pos.z}
			);

			player.pos.x = wallPoint.x + (CosDeg(angleToPlayer) * 0.2);
			player.pos.z = wallPoint.y + (SinDeg(angleToPlayer) * 0.2);
		}
	}
}

void GameBase_HandleEvent(Event* e) {
	if (!map.active) return;

	switch (e->type) {
		case AE_EVENT_KEY_DOWN: {
			if (Input_MatchBind(gameBaseConfig.jump, e)) {
				if (FloatEqual(player.sector->floor, player.pos.y, 0.05)) {
					player.acc.y        = player.jumpSpeed;
					player.skipFriction = true;
				}
			}

			break;
		}
		case AE_EVENT_MOUSE_MOVE: {
			player.yaw +=
				(float) e->mouseMove.xRel * globalConfig.scale2D *
				gameBaseConfig.sensitivity * engine.delta;
			player.pitch -=
				(float) e->mouseMove.yRel * globalConfig.scale2D *
				gameBaseConfig.sensitivity * engine.delta;

			if (player.pitch >  90.0) player.pitch =  90.0;
			if (player.pitch < -90.0) player.pitch = -90.0;
			break;
		}
		default: break;
	}
}

void GameBase_Render(void) {
	if (!map.active) return;

	Player_FPCamera();
	Backend_RenderScene();

	int videoWidth  = video.windows[0].width;
	int videoHeight = video.windows[0].height;

    int    csLen = (int) (((float) videoHeight) / 96);
    Colour csCol = (Colour) {255, 255, 255, 255};

    Backend_VLine((videoWidth / 2) - 1, (videoHeight / 2) - csLen, 2, csLen * 2, csCol);
    Backend_HLine((videoWidth / 2) - csLen, (videoHeight / 2) - 1, 2, csLen * 2, csCol);

    if (gameBaseConfig.debugInfoLevel == 0) return;

	static char text[80];
	snprintf(text, 80, "FPS: %d", engine.fps);
	Text_Render(&engine.font, text, 8, 8);

	snprintf(text, 80, "X: %.3f", player.pos.x);
	Text_Render(&engine.font, text, 8, 8 + 16);
	snprintf(text, 80, "Y: %.3f", player.pos.y);
	Text_Render(&engine.font, text, 8, 8 + (16 * 2));
	snprintf(text, 80, "Z: %.3f", player.pos.z);
	Text_Render(&engine.font, text, 8, 8 + (16 * 3));
	snprintf(text, 80, "Sector: %d", (int) (camera.sector - map.sectors));

	if (!map.active || (gameBaseConfig.debugInfoLevel == 1)) return;

	Text_Render(&engine.font, text, 8, 8 + (16 * 4));
	snprintf(text, 80, "Camera: %.3f %.3f %.3f", camera.pos.x, camera.pos.y, camera.pos.z);
	Text_Render(&engine.font, text, 8, 8 + (16 * 5));
	snprintf(text, 80, "Player rot: %.3f %.3f", player.yaw, player.pitch);
	Text_Render(&engine.font, text, 8, 8 + (16 * 6));
	snprintf(text, 80, "Sector floor: %.3f", camera.sector? 0.0f : camera.sector->floor);
	Text_Render(&engine.font, text, 8, 8 + (16 * 7));
	snprintf(text, 80, "Velocity: %.3f %.3f %.3f", player.vel.x, player.vel.y, player.vel.z);
	Text_Render(&engine.font, text, 8, 8 + (16 * 8));
	snprintf(text, 80, "Delta time: %.3f", engine.delta);
	Text_Render(&engine.font, text, 8, 8 + (16 * 9));
	snprintf(
		text, 80, "Grounded: %s",
		FloatEqual(player.sector->floor, player.pos.y, 0.05)? "true" : "false"
	);
	Text_Render(&engine.font, text, 8, 8 + (16 * 10));
}
