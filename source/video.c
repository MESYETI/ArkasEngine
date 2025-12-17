#include "util.h"
#include "video.h"
#include "backend.h"

Video video;

void Video_Init(void) {
	video.width  = 640;
	video.height = 480;
	video.window = SDL_CreateWindow(
		"Arkas Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		video.width, video.height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

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
}
