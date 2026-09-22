#include "text.h"
#include "game.h"
#include "util.h"
#include "audio.h"
#include "video.h"
#include "entity.h"
#include "client.h"
#include "camera.h"
#include "config.h"
#include "engine.h"
#include "player.h"
#include "backend.h"
#include "ui/button.h"
#include "ui/chatLog.h"
#include "ui/textInput.h"

GameBaseConfig gameBaseConfig = {
	.debugInfoLevel = 0,
	.noclip         = false,
	.forward        = INPUT_BIND_NONE,
	.left           = INPUT_BIND_NONE,
	.backward       = INPUT_BIND_NONE,
	.right          = INPUT_BIND_NONE,
	.jump           = INPUT_BIND_NONE
};

GameBase gameBase;

static void OnChatFocus(UI_Container* cont, bool focus) {
	cont->hidden            = !focus;
	cont->manager->priority = focus;
}

void GameBase_Init(Scene* scene) {
	//Map_Init();
	Audio_StartAudio();

	camera.pos.x = 0.0;
	camera.pos.y = 0.0;
	camera.pos.z = 0.0;
	camera.pitch = 0.0;
	camera.yaw   = 0.0;
	camera.roll  = 0.0;
	// Player_Init();

	gameBase.mapLoaded = false;

	client.reportPos = true;

	gameBaseConfig.sensitivity = 7.5;

	gameBase.chatInput[0] = 0;

	scene->ui = UI_ManagerInit(32);

	// width = 200
	gameBase.chatCont = UI_ManagerAddContainer(scene->ui, engine.font.charWidth * 64, NULL);
	UI_ContainerAlignLeft(gameBase.chatCont, 8);
	UI_ContainerCenterY(gameBase.chatCont);
	UI_ContainerSetPadding(gameBase.chatCont, 5, 5, 5, 5);
	gameBase.chatCont->hidden  = true;
	gameBase.chatCont->hasBG   = true;
	gameBase.chatCont->opacity = 128;
	gameBase.chatCont->onFocus = &OnChatFocus;

	UI_Row* row = UI_ContainerAddSingleElemRow(gameBase.chatCont, 0, UI_NewChatLog(8));
	UI_RowUpdate(row);

	gameBase.chatLog = &row->elems[0];

	row = UI_ContainerAddRow(gameBase.chatCont, 0);
	UI_RowAddElement(row, UI_NewTextInput(gameBase.chatInput, sizeof(gameBase.chatInput)));
	UI_RowAddElement(row, UI_NewButton("Send", true, NULL));
	UI_RowUpdate(row);

	gameBase.chatInputElem = &row->elems[1];

	UI_ChatLogAddMsg(gameBase.chatLog, "mesyeti: Happy manul Monday");
}

void GameBase_Free(Scene* scene) {
	Audio_StopAudio();
	Map_Free();

	UI_ManagerFree(scene->ui);

	client.reportPos = false;
}

void GameBase_Update(Scene* scene, bool top) {
	(void) scene;

	if (!top || engine.console) return;

	if (!map.active) {
		return;
	}

	if (map.active && !gameBase.mapLoaded) {
		Player_Init();
		gameBase.mapLoaded = true;
	}

	Window_SetRelativeMouseMode(!scene->ui->priority);

	Entity* playerEnt = Entities_Get(player.entityIdx);

	// static const float sensitivity = 180.0;
	float speed = player.speed;

	if (!FloatEqual(playerEnt->pos.y, playerEnt->sector->floor, 0.01)) {
		speed = player.airSpeed;
	}

	// bool  moved  = false;
	FVec3 oldPos = camera.pos;

	if (!scene->ui->priority) {
		if (Input_BindPressed(gameBaseConfig.forward)) {
			player.acc.z += CosDeg(playerEnt->dir.yaw) * speed * engine.delta;
			player.acc.x += SinDeg(playerEnt->dir.yaw) * speed * engine.delta;
			// moved         = true;
		}
		if (Input_BindPressed(gameBaseConfig.left)) {
			player.acc.z += CosDeg(playerEnt->dir.yaw - 90) * speed * engine.delta;
			player.acc.x += SinDeg(playerEnt->dir.yaw - 90) * speed * engine.delta;
			// moved         = true;
		}
		if (Input_BindPressed(gameBaseConfig.backward)) {
			player.acc.z += CosDeg(playerEnt->dir.yaw + 180) * speed * engine.delta;
			player.acc.x += SinDeg(playerEnt->dir.yaw + 180) * speed * engine.delta;
			// moved         = true;
		}
		if (Input_BindPressed(gameBaseConfig.right)) {
			player.acc.z += CosDeg(playerEnt->dir.yaw + 90) * speed * engine.delta;
			player.acc.x += SinDeg(playerEnt->dir.yaw + 90) * speed * engine.delta;
			// moved         = true;
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
				playerEnt->sector = &map.sectors[wall->portalSector];
				camera.sector     = &map.sectors[wall->portalSector];

				// if (
				// 	(player.sector->floor > player.pos.y) ||
				// 	(player.sector->floor - player.pos.y <= 0.6)
				// ) {
				// 	player.pos.y = player.sector->floor;
				// 	player.vel.y = 0.0;
				// }

				playerEnt->pos.x -= wall->portalOff.x;
				playerEnt->pos.z -= wall->portalOff.y;
			}
		}
	};

	// check collision with walls
	if (gameBaseConfig.noclip) return;
	for (size_t i = 0; i < playerEnt->sector->length; ++ i) {
		size_t idx  = playerEnt->sector->start + i;
		Wall*  wall = &map.walls[idx];

		if (wall->isPortal) continue; // TODO: solid portals

		FVec2 a = (FVec2) {map.points[idx].pos.x, map.points[idx].pos.y};
		FVec2 b;
		size_t bIdx;

		bIdx = i == playerEnt->sector->length - 1? playerEnt->sector->start : idx + 1;
		b    = (FVec2) {map.points[bIdx].pos.x, map.points[bIdx].pos.y};

		float dist = LinePointDistance(a, b, (FVec2) {playerEnt->pos.x, playerEnt->pos.z});
		if (dist < 0.2) {
			float angleToWall = GetAngle(a, b) + 90.0;
			FVec2 wallPoint;

			wallPoint.x = playerEnt->pos.x + (CosDeg(angleToWall) * dist);
			wallPoint.y = playerEnt->pos.z + (SinDeg(angleToWall) * dist);

			if (!PointInLine(wallPoint, a, b)) {
				continue;
			}

			float angleToPlayer = GetAngle(
				wallPoint, (FVec2) {playerEnt->pos.x, playerEnt->pos.z}
			);

			playerEnt->pos.x = wallPoint.x + (CosDeg(angleToPlayer) * 0.2);
			playerEnt->pos.z = wallPoint.y + (SinDeg(angleToPlayer) * 0.2);
		}
	}
}

void GameBase_HandleEvent(Scene* scene, Event* e) {
	if (!map.active) return;

	if (UI_ManagerHandleEvent(scene->ui, e)) return;

	Entity* playerEnt = Entities_Get(player.entityIdx);

	switch (e->type) {
		case AE_EVENT_KEY_DOWN: {
			if (scene->ui->priority) {
				break;
			}

			if (Input_MatchBind(gameBaseConfig.jump, e)) {
				if (FloatEqual(playerEnt->sector->floor, playerEnt->pos.y, 0.05)) {
					player.acc.y        = player.jumpSpeed;
					player.skipFriction = true;
				}
			}
			if (Input_MatchBind(gameBaseConfig.chat, e)) {
				scene->ui->priority = true;
				scene->ui->focus    = gameBase.chatCont;

				gameBase.chatCont->focus  = gameBase.chatInputElem;
				gameBase.chatCont->hidden = false;
				Event_StartTextInput();
			}

			break;
		}
		case AE_EVENT_MOUSE_MOVE: {
			if (scene->ui->priority) {
				break;
			}

			playerEnt->dir.yaw +=
				(float) e->mouseMove.xRel * globalConfig.scale2D *
				gameBaseConfig.sensitivity * engine.delta;
			playerEnt->dir.pitch -=
				(float) e->mouseMove.yRel * globalConfig.scale2D *
				gameBaseConfig.sensitivity * engine.delta;

			if (playerEnt->dir.pitch >  90.0) playerEnt->dir.pitch =  90.0;
			if (playerEnt->dir.pitch < -90.0) playerEnt->dir.pitch = -90.0;
			break;
		}
		default: break;
	}
}

void GameBase_Render(Scene* scene) {
	if (!map.active) return;

	Player_FPCamera();
	Backend_RenderScene();

	int videoWidth  = video.windows[0].width;
	int videoHeight = video.windows[0].height;

    int    csLen = (int) (((float) videoHeight) / 96);
    Colour csCol = (Colour) {255, 255, 255, 255};

    Backend_VLine((videoWidth / 2) - 1, (videoHeight / 2) - csLen, 2, csLen * 2, csCol);
    Backend_HLine((videoWidth / 2) - csLen, (videoHeight / 2) - 1, 2, csLen * 2, csCol);

    UI_ManagerRender(scene->ui);

    if (gameBaseConfig.debugInfoLevel == 0) return;

    Entity* playerEnt = Entities_Get(player.entityIdx);

	static char text[80];
	snprintf(text, 80, "FPS: %d", engine.fps);
	Text_Render(&engine.font, text, 8, 8);

	snprintf(text, 80, "X: %.3f", playerEnt->pos.x);
	Text_Render(&engine.font, text, 8, 8 + 16);
	snprintf(text, 80, "Y: %.3f", playerEnt->pos.y);
	Text_Render(&engine.font, text, 8, 8 + (16 * 2));
	snprintf(text, 80, "Z: %.3f", playerEnt->pos.z);
	Text_Render(&engine.font, text, 8, 8 + (16 * 3));
	snprintf(text, 80, "Sector: %d", (int) (camera.sector - map.sectors));

	if (!map.active || (gameBaseConfig.debugInfoLevel == 1)) return;

	Text_Render(&engine.font, text, 8, 8 + (16 * 4));
	snprintf(text, 80, "Camera: %.3f %.3f %.3f", camera.pos.x, camera.pos.y, camera.pos.z);
	Text_Render(&engine.font, text, 8, 8 + (16 * 5));
	snprintf(text, 80, "Player rot: %.3f %.3f", playerEnt->dir.yaw, playerEnt->dir.pitch);
	Text_Render(&engine.font, text, 8, 8 + (16 * 6));
	snprintf(text, 80, "Sector floor: %.3f", camera.sector? 0.0f : camera.sector->floor);
	Text_Render(&engine.font, text, 8, 8 + (16 * 7));
	snprintf(text, 80, "Velocity: %.3f %.3f %.3f", playerEnt->vel.x, playerEnt->vel.y, playerEnt->vel.z);
	Text_Render(&engine.font, text, 8, 8 + (16 * 8));
	snprintf(text, 80, "Delta time: %.3f", engine.delta);
	Text_Render(&engine.font, text, 8, 8 + (16 * 9));
	snprintf(
		text, 80, "Grounded: %s",
		FloatEqual(playerEnt->sector->floor, playerEnt->pos.y, 0.05)? "true" : "false"
	);
	Text_Render(&engine.font, text, 8, 8 + (16 * 10));
}
