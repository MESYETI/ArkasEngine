#include "fs.h"
#include "engine.h"
#include "map.h"
#include "util.h"
#include "event.h"
#include "input.h"
#include "theme.h"
#include "scene.h"
#include "video.h"
#include "audio.h"
#include "config.h"
#include "camera.h"
#include "backend.h"
#include "console.h"
#include "resources.h"

Engine engine;

void Engine_Init(const char* gameName) {
	// make game engine folders
	MakeDir("game",        true);
	MakeDir("game/extra",  true);
	MakeDir("game/net",    true);
	MakeDir("game/maps",   true);
	MakeDir("screenshots", true);

	Console_Init();
	Log("Arkas Engine WIP");
	Log("Made by MESYETI in 2025");
	Resources_Init();

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Failed to initialise SDL2: %s\n", SDL_GetError());
		exit(1);
	}

	if (!FileExists("startup.cmd")) {
		Log("Generating startup.cmd");

		WriteFile_("startup.cmd",
			"@set echo false\n"
			"run gen_options.cmd\n"
			"@set echo true\n"
		);
	}
	if (!FileExists("gen_options.cmd")) {
		Log("Generating gen_options.cmd");

		SaveDefaultConfig();
	}

	// run script
	Log("Running startup...");
	if (!Console_RunFile("startup.cmd")) {
		Log("Failed to run startup");
	}

	Video_Init(gameName);
	SceneManager_Init();
	Audio_Init();
	Theme_Init();

	bool success;
	engine.running = true;
	engine.font    = Text_LoadFont(":builtin/font.png", &success);
	engine.fps     = 0;

	if (!success) {
		Error("Failed to load font");
	}
}

void Engine_Free(void) {
	Log("Goodbye!");

	Input_Free();
	Audio_Free();
	SceneManager_Free();
	Text_FreeFont(&engine.font);
	Resources_Free();
	Video_Free();
	SDL_Quit();
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
			case AE_EVENT_WINDOW_RESIZE: {
				video.width  = e.windowResize.width;
				video.height = e.windowResize.height;
				Backend_OnWindowResize();
				break;
			}
		}

		SceneManager_HandleEvent(&e);

		if (engine.console) {
			Console_HandleEvent(&e);
		}
	}

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
