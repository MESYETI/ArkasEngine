#include "fs.h"
#include "ui.h"
#include "engine.h"
#include "map.h"
#include "game.h"
#include "util.h"
#include "event.h"
#include "input.h"
#include "theme.h"
#include "scene.h"
#include "video.h"
#include "audio.h"
#include "config.h"
#include "camera.h"
#include "client.h"
#include "player.h"
#include "server.h"
#include "backend.h"
#include "console.h"
#include "platform.h"
#include "resources.h"
#include "variables.h"

Engine engine;

void Engine_Init(const char* gameName, int argc, const char** argv) {
	Platform_Init();
	Log_Init();

	Variables_Add(VAR_FLOAT, "player.ground-friction",  &player.groundFriction, 0);
	Variables_Add(VAR_FLOAT, "player.gravity",          &player.gravity, 0);
	Variables_Add(VAR_FLOAT, "player.speed",            &player.speed, 0);
	Variables_Add(VAR_FLOAT, "player.air-speed",        &player.airSpeed, 0);
	Variables_Add(VAR_FLOAT, "player.jump-speed",       &player.jumpSpeed, 0);
	Variables_Add(VAR_FLOAT, "game.sensitivity",        &gameBaseConfig.sensitivity, 0);
	Variables_Add(VAR_FLOAT, "game.music-volume",       &gameBaseConfig.musicVolume, 0);
	Variables_Add(VAR_BOOL,  "echo",                    &console.echo, 0);
	Variables_Add(VAR_INT,   "engine.scale-2D",         &globalConfig.scale2D, 0);
	Variables_Add(VAR_BOOL,  "engine.skybox-filtering", &gameBaseConfig.skyboxFiltering, 0);
	Variables_Add(VAR_STR,   "engine.backend",          &backendOptions.name, 20);
	Variables_Add(VAR_INT,   "engine.debug-level",      &gameBaseConfig.debugInfoLevel, 0);
	Variables_Add(VAR_BOOL,  "engine.noclip",           &gameBaseConfig.noclip, 0);
	Variables_Add(VAR_BOOL,  "server.inet",             &serverConf.inet, 0);
	Variables_Add(VAR_INT,   "server.inet-port",        &serverConf.inetPort, 0);
	Variables_Add(VAR_BOOL,  "server.local",            &serverConf.local, 0);
	Variables_Add(VAR_STR,   "client.username",         &client.name, sizeof(client.name));

	engine.server = false;
	for (int i = 1; i < argc; ++ i) {
		if (strcmp(argv[i], "--server") == 0) {
			engine.server = true;
		}
	}

	// make game engine folders
	MakeDir(AE_LOCATION "game",        true);
	MakeDir(AE_LOCATION "game/extra",  true);
	MakeDir(AE_LOCATION "game/net",    true);
	MakeDir(AE_LOCATION "game/maps",   true);
	MakeDir(AE_LOCATION "screenshots", true);

	Console_Init();
	Log("Arkas Engine WIP");
	Log("Made by MESYETI in 2025");
	Resources_Init();

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Failed to initialise SDL2: %s\n", SDL_GetError());
		exit(1);
	}

	if (!FileExists(AE_LOCATION "startup.cmd")) {
		Log("Generating startup.cmd");

		WriteFile_(AE_LOCATION "startup.cmd",
			"@set echo false\n"
			"run gen_options.cmd\n"
			"@set echo true\n"
		);
	}
	if (!FileExists(AE_LOCATION "gen_options.cmd")) {
		Log("Generating gen_options.cmd");

		SaveDefaultConfig();
	}

	// run script

	Log("Arkas Engine is now running as a server");

	engine.running = true;

	if (engine.server) {
		Log("Running server startup...");

		if (!Console_RunFile(AE_LOCATION "server.cmd")) {
			Log("Failed to run server startup");
		}
		return;
	}
	else {
		Log("Running game startup...");

		if (!Console_RunFile(AE_LOCATION "startup.cmd")) {
			Log("Failed to run startup");
		}
	}

	Video_Init(gameName);
	SceneManager_Init();
	Audio_Init();
	Theme_Init();
	UI_Init();

	bool success;
	engine.font    = Text_LoadFont("builtin:font.png", &success);
	engine.fps     = 0;

	if (!success) {
		Error("Failed to load font");
	}
}

void Engine_Free(void) {
	Log("Goodbye!");

	SDL_Quit();

	if (server.running) {
		Server_Free();
	}

	if (engine.server) return;

	Input_Free();
	Audio_Free();
	SceneManager_Free();
	Text_FreeFont(&engine.font);
	Resources_Free();
	Video_Free();
	Event_Free();
}

void Engine_Update(void) {
	static uint64_t oldFrameTime = 0;

	uint64_t newFrameTime  = SDL_GetTicks64();
	uint64_t frameTimeDiff = newFrameTime - oldFrameTime;
	engine.delta           = frameTimeDiff / 1000.0f;

	static float fpsTimer = 0.0;
	static int   frames   = 0;

	++ frames;
	fpsTimer += engine.delta;

	if (fpsTimer >= 1.0) {
		fpsTimer = 0.0;
		engine.fps  = frames;
		frames   = 0;
	}

	Event e;
	while (Event_Poll(&e)) {
		Input_HandleEvent(&e);

		switch (e.type) {
			case AE_EVENT_KEY_DOWN: {
				switch (e.key.key) {
					case AE_KEY_GRAVE: {
						if (!engine.console) {
							Console_Begin();
							engine.console = true;
						}
						break;
					}
					case AE_KEY_ESCAPE: {
						if (engine.console) {
							Console_End();
							engine.console = false;
						}
						break;
					}
					default: break;
				}
				break;
			}
			case AE_EVENT_QUIT: engine.running = false; break;
		}

		if (engine.server) continue;

		SceneManager_HandleEvent(&e);

		if (engine.console) {
			Console_HandleEvent(&e);
		}
	}

	if (server.running) {
		Server_Update();
	}
	if (client.running) {
		Client_Update();
	}

	if (engine.server) return;

	SceneManager_Update();

	Audio_Update();

	Backend_Begin();
	SceneManager_Render();

	if (engine.console) {
		Console_Render();
	}

	Backend_FinishRender();
	oldFrameTime = newFrameTime;
}
