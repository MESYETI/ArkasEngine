#include "app.h"
#include "map.h"
#include "util.h"
#include "scene.h"
#include "video.h"
#include "camera.h"
#include "backend.h"
#include "console.h"

App app;

void App_Init(void) {
	Log("Arkas Engine WIP");
	Log("Made by MESYETI in 2025");

	Video_Init();
	SceneManager_Init();

	app.running = true;
	app.font = Text_LoadFont("font.png");
}

void App_Free(void) {
	SceneManager_Free();
	Text_FreeFont(&app.font);
	Video_Free();
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
			case SDL_MOUSEMOTION: {
				camera.yaw   += (float) e.motion.xrel * 7.5 * app.delta;
				camera.pitch -= (float) e.motion.yrel * 7.5 * app.delta;

				if (camera.pitch >  90.0) camera.pitch =  90.0;
				if (camera.pitch < -90.0) camera.pitch = -90.0;
				break;
			}
		}

		SceneManager_HandleEvent(&e);

		if (app.console) {
			Console_HandleEvent(&e);
		}
	}

	SceneManager_Update();

	Backend_Begin();
	SceneManager_Render();

	if (app.console) {
		Console_Render();
	}

	Backend_FinishRender();
	oldFrameTime = newFrameTime;
}
