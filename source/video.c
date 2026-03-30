#include "util.h"
#include "event.h"
#include "video.h"
#include "config.h"
#include "window.h"
#include "backend.h"

Video video;

static void EventHandler(Event* e) {
	video.aWidth  = e->windowResize.width;
	video.aHeight = e->windowResize.height;
	Backend_OnWindowResize();

	video.width  = video.aWidth / globalConfig.scale2D;
	video.height = video.aHeight / globalConfig.scale2D;
}

void Video_Init(const char* gameName) {
	Backend_Init(true);

	#ifdef AE_PLATFORM_PSVITA
		video.aWidth  = 960;
		video.aHeight = 544;
	#else
		video.aWidth  = 640;
		video.aHeight = 480;
	#endif
	video.width  = video.aWidth  / globalConfig.scale2D;
	video.height = video.aHeight / globalConfig.scale2D;

	video.window = Window_Create(gameName, video.aWidth, video.aHeight);
	Log("Created window");

	Backend_Init(false);

	Event_AddHandler(AE_EVENT_WINDOW_RESIZE, &EventHandler);
}

void Video_Free(void) {
	Backend_Free();
	Window_Free(&video.window);
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
		(uint8_t) (b * 255.0),
		colour.a
	};
}
