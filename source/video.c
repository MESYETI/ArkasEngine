#include "util.h"
#include "video.h"
#include "backend.h"

Video video;

void Video_Init(const char* gameName) {
	video.width  = 640;
	video.height = 480;
	video.window = SDL_CreateWindow(
		gameName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

Colour Video_MultiplyColour(Colour colour, float by) {
	float r = ((float) colour.r) / 255.0;
	float g = ((float) colour.g) / 255.0;
	float b = ((float) colour.b) / 255.0;

	r *= by;
	g *= by;
	b *= by;

	if (r > 1.0) r = 1.0;
	if (g > 1.0) g = 1.0;
	if (b > 1.0) b = 1.0;

	if (r < 0.0) r = 0.0;
	if (g < 0.0) g = 0.0;
	if (b < 0.0) b = 0.0;

	return (Colour) {
		(uint8_t) (r * 255.0),
		(uint8_t) (g * 255.0),
		(uint8_t) (b * 255.0)
	};
}
