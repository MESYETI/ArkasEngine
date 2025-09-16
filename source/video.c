#include "util.h"
#include "video.h"
#include "backend.h"

Video video;

void Video_Init(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Failed to initialise SDL2: %s\n", SDL_GetError());
		exit(1);
	}

	Backend_Init(true);

	video.window = SDL_CreateWindow(
		"Arkas Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	video.width  = 640;
	video.height = 480;

	if (video.window == NULL) {
		fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
		exit(1);
	}

	Log("Created window");

	Backend_Init(false);
}

void Video_Free(void) {
	Backend_Free();
	SDL_DestroyWindow(video.window);
	SDL_Quit();
}
