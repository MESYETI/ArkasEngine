#include "fs.h"
#include "app.h"
#include "map.h"
#include "util.h"
#include "theme.h"
#include "scene.h"
#include "video.h"
#include "audio.h"
#include "config.h"
#include "camera.h"
#include "backend.h"
#include "console.h"
#include "resources.h"

App app;

void App_Init(void) {
	// make game engine folders
	MakeDir("game",        true);
	MakeDir("game/extra",  true);
	MakeDir("game/net",    true);
	MakeDir("maps",        true);
	MakeDir("screenshots", true);

	Console_Init();
	Log("Arkas Engine WIP");
	Log("Made by MESYETI in 2025");
	Resources_Init();

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Failed to initialise SDL2: %s\n", SDL_GetError());
		exit(1);
	}

	Video_Init();
	SceneManager_Init();
	Audio_Init();
	Theme_Init();

	bool success;
	app.running = true;
	app.font    = Text_LoadFont(":builtin/font.png", &success);
	app.console = true;

	if (!success) {
		Error("Failed to load font");
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
}

void App_Free(void) {
	Log("Goodbye!");

	Audio_Free();
	SceneManager_Free();
	Text_FreeFont(&app.font);
	Resources_Free();
	Video_Free();
	SDL_Quit();
}

void App_Update(void) {
	static uint64_t oldFrameTime = 0;

	uint64_t newFrameTime  = SDL_GetTicks64();
	uint64_t frameTimeDiff = newFrameTime - oldFrameTime;
	app.delta              = frameTimeDiff / 1000.0f;

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_KEYDOWN: {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_GRAVE: {
						if (!app.console) {
							Console_Begin();
							app.console = true;
						}
						break;
					}
					case SDL_SCANCODE_ESCAPE: {
						if (app.console) {
							Console_End();
							app.console = false;
						}
						break;
					}
					default: break;
				}
				break;
			}
			case SDL_QUIT: app.running = false; break;
			case SDL_WINDOWEVENT: {
				if (
					(e.window.event == SDL_WINDOWEVENT_RESIZED) ||
					(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				) {
					video.width  = e.window.data1;
					video.height = e.window.data2;
					Backend_OnWindowResize();
				}
				break;
			}
		}

		SceneManager_HandleEvent(&e);

		if (app.console) {
			Console_HandleEvent(&e);
		}
	}

	SceneManager_Update();

	Audio_Update();

	Backend_Begin();
	SceneManager_Render();

	if (app.console) {
		Console_Render();
	}

	Backend_FinishRender();
	oldFrameTime = newFrameTime;
}
