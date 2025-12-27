#include "app.h"
#include "text.h"
#include "game.h"
#include "util.h"
#include "video.h"
#include "camera.h"
#include "player.h"
#include "backend.h"
#include "audio.h"

GameBaseConfig gameBaseConfig;

AudioEmitter emitters2d[1];
AudioEmitter emitters3d[2];

void Game_Init(void) {
	Map_Init();
	startAudio();
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera.pos.x = 0.0;
	camera.pos.y = 0.0;
	camera.pos.z = 0.0;
	camera.pitch = 0.0;
	camera.yaw   = 0.0;
	camera.roll  = 0.0;

	setAudioEnv(
		AUDIOENVMASK_REVERB,
		&(struct audioenv) {.reverb = {0.07, 0.65, 1.0, 0.1, 0.25, 0.25}},
		AUDIOENVMASK_ALL
	);

	emitters2d[0] = new2DAudioEmitter(
		AUDIOPRIO_DEFAULT, -1, 0,
		0, NULL
	);
	Resource* resource = Resources_GetRes(":base/sfx/air1.ogg", 0);
	if (resource) {
		Audio_Play2DSound(
			emitters2d[0], resource,
			AUDIOPRIO_DEFAULT, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP,
			AUDIOFXMASK_SPEED | AUDIOFXMASK_VOL,
			&(struct audiofx) {.speed = 0.3f, .vol = {0.65f, 0.65f}}
		);
		Resources_FreeRes(resource);
	}

	emitters3d[0] = new3DAudioEmitter(
		AUDIOPRIO_DEFAULT, -1, 0,
		0, NULL,
		AUDIO3DFXMASK_POS, &(struct audio3dfx){.pos = {-5.0f, -0.3f, 9.0f}}
	);
	emitters3d[1] = new3DAudioEmitter(
		AUDIOPRIO_DEFAULT, -1, 0,
		0, NULL,
		AUDIO3DFXMASK_POS, &(struct audio3dfx){.pos = {6.0f, -0.3f, 4.0f}}
	);

	resource = Resources_GetRes(":base/sfx/drip1.ogg", 0);

	if (resource) {
		Audio_Play3DSound(
			emitters3d[0], resource,
			AUDIOPRIO_DEFAULT, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP,
			AUDIOFXMASK_SPEED, &(struct audiofx){.speed = 1.56521f}
		);
		Audio_Play3DSound(
			emitters3d[1], resource,
			AUDIOPRIO_DEFAULT, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP,
			AUDIOFXMASK_SPEED, &(struct audiofx){.speed = 1.10435f}
		);
		Resources_FreeRes(resource);
	}

	gameBaseConfig.sensitivity = 7.5;

	Player_Init();
}

void Game_Free(void) {
	SDL_SetRelativeMouseMode(SDL_FALSE);
	stopAudio();
	Map_Free();
}

void Game_Update(bool top) {
	if (!top || app.console) return;

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	// static const float sensitivity = 180.0;
	float speed = player.speed;

	if (!FloatEqual(player.pos.y, player.sector->floor, 0.01)) {
		speed = player.airSpeed;
	}

	bool  moved  = false;
	FVec3 oldPos = camera.pos;

	if (keys[SDL_SCANCODE_W]) {
		player.acc.z += CosDeg(player.yaw) * speed * app.delta;
		player.acc.x += SinDeg(player.yaw) * speed * app.delta;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_A]) {
		player.acc.z += CosDeg(player.yaw - 90) * speed * app.delta;
		player.acc.x += SinDeg(player.yaw - 90) * speed * app.delta;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_S]) {
		player.acc.z += CosDeg(player.yaw + 180) * speed * app.delta;
		player.acc.x += SinDeg(player.yaw + 180) * speed * app.delta;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_D]) {
		player.acc.z += CosDeg(player.yaw + 90) * speed * app.delta;
		player.acc.x += SinDeg(player.yaw + 90) * speed * app.delta;
		moved         = true;
	}
	if (keys[SDL_SCANCODE_P]) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	if (keys[SDL_SCANCODE_O]) {
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}

	if (keys[SDL_SCANCODE_A]) {
		camera.roll = -2.0;
	}
	else if (keys[SDL_SCANCODE_D]) {
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

	struct audioplayerdata* playeraud = &audiostate.playerdata.data[0];
	playeraud->pos[0] = camera.pos.x;
	playeraud->pos[1] = camera.pos.y;
	playeraud->pos[2] = camera.pos.z;
	playeraud->rotsin[0] = SinDeg(-camera.pitch);
	playeraud->rotsin[1] = SinDeg(-camera.yaw);
	playeraud->rotsin[2] = SinDeg(-camera.roll);
	playeraud->rotcos[0] = CosDeg(-camera.pitch);
	playeraud->rotcos[1] = CosDeg(-camera.yaw);
	playeraud->rotcos[2] = CosDeg(-camera.roll);

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
			if (PointInLine(intersect, a1, a2)) {
				player.sector = &map.sectors[wall->portalSector];
				camera.sector = &map.sectors[wall->portalSector];

				if (player.sector->floor > player.pos.y) {
					player.pos.y = player.sector->floor;
					player.vel.y = 0.0;
				}
			}
		}
	};
}

void Game_HandleEvent(Event* e) {
	switch (e->type) {
		case AE_EVENT_KEY_DOWN: {
			switch (e->key.key) {
				case AE_KEY_SPACE: {
					if (FloatEqual(player.sector->floor, player.pos.y, 0.05)) {
						player.acc.y        = player.jumpSpeed;
						player.skipFriction = true;
					}
					break;
				}
				default: break;
			}

			break;
		}
		case AE_EVENT_MOUSE_MOVE: {
			player.yaw +=
				(float) e->mouseMove.xRel * gameBaseConfig.sensitivity * app.delta;
			player.pitch -=
				(float) e->mouseMove.yRel * gameBaseConfig.sensitivity * app.delta;

			if (player.pitch >  90.0) player.pitch =  90.0;
			if (player.pitch < -90.0) player.pitch = -90.0;
			break;
		}
		default: break;
	}
}

void Game_Render(void) {
	Player_FPCamera();
	Backend_RenderScene();

	/*static char text[80];
	snprintf(text, 80, "FPS: %d", (int) (1 / app.delta));
	Text_Render(&app.font, text, 8, 8);

	snprintf(text, 80, "X: %.3f", player.pos.x);
	Text_Render(&app.font, text, 8, 8 + 16);
	snprintf(text, 80, "Y: %.3f", player.pos.y);
	Text_Render(&app.font, text, 8, 8 + (16 * 2));
	snprintf(text, 80, "Z: %.3f", player.pos.z);
	Text_Render(&app.font, text, 8, 8 + (16 * 3));
	snprintf(text, 80, "Sector: %d", (int) (camera.sector - map.sectors));
	Text_Render(&app.font, text, 8, 8 + (16 * 4));
	snprintf(text, 80, "Camera: %.3f %.3f %.3f", camera.pos.x, camera.pos.y, camera.pos.z);
	Text_Render(&app.font, text, 8, 8 + (16 * 5));
	snprintf(text, 80, "Player rot: %.3f %.3f", player.yaw, player.pitch);
	Text_Render(&app.font, text, 8, 8 + (16 * 6));
	snprintf(text, 80, "Sector floor: %.3f", camera.sector->floor);
	Text_Render(&app.font, text, 8, 8 + (16 * 7));
	snprintf(text, 80, "Velocity: %.3f %.3f %.3f", player.vel.x, player.vel.y, player.vel.z);
	Text_Render(&app.font, text, 8, 8 + (16 * 8));
	snprintf(text, 80, "Delta time: %.3f", app.delta);
	Text_Render(&app.font, text, 8, 8 + (16 * 9));
	snprintf(
		text, 80, "Grounded: %s",
		FloatEqual(player.sector->floor, player.pos.y, 0.05)? "true" : "false"
	);
	Text_Render(&app.font, text, 8, 8 + (16 * 10));*/
}
