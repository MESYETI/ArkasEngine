#include "app.h"
#include "map.h"
#include "util.h"
#include "video.h"
#include "camera.h"
#include "backend.h"

App app;

void App_Init(void) {
	Video_Init();

	app.running = true;

	camera.pos.x = 0.0;
	camera.pos.y = 0.0;
	camera.pos.z = 0.0;
	camera.pitch = 0.0;
	camera.yaw   = 0.0;
	camera.roll  = 0.0;

	Map_Init();
}

void App_Update(void) {
	static uint64_t oldFrameTime = 0;

	uint64_t newFrameTime  = SDL_GetTicks64();
	uint64_t frameTimeDiff = newFrameTime - oldFrameTime;
	app.delta              = frameTimeDiff / 1000.0f;

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
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
	}

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	static const float sensitivity = 180.0;
	static const float speed       = 2.0;

	if (keys[SDL_SCANCODE_W]) {
		camera.pos.z += CosDeg(camera.yaw) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw) * app.delta * speed;
	}
	if (keys[SDL_SCANCODE_A]) {
		camera.pos.z += CosDeg(camera.yaw - 90) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw - 90) * app.delta * speed;
	}
	if (keys[SDL_SCANCODE_S]) {
		camera.pos.z += CosDeg(camera.yaw + 180) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw + 180) * app.delta * speed;
	}
	if (keys[SDL_SCANCODE_D]) {
		camera.pos.z += CosDeg(camera.yaw + 90) * app.delta * speed;
		camera.pos.x += SinDeg(camera.yaw + 90) * app.delta * speed;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		camera.yaw += app.delta * sensitivity;
	}
	if (keys[SDL_SCANCODE_LEFT]) {
		camera.yaw -= app.delta * sensitivity;
	}

	Backend_RenderScene();
	oldFrameTime = newFrameTime;
}

void App_Free(void) {
	Video_Free();
}
