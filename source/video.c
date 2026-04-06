#include "util.h"
#include "event.h"
#include "video.h"
#include "config.h"
#include "window.h"
#include "backend.h"

Video video;

static void EventHandler(Event* e) {
	Window* win = &video.windows[e->windowResize.window]; // TODO: check

	win->aWidth  = e->windowResize.width;
	win->aHeight = e->windowResize.height;
	Backend_OnWindowResize();

	win->width  = win->aWidth  / globalConfig.scale2D;
	win->height = win->aHeight / globalConfig.scale2D;
}

void Video_Init(const char* gameName) {
	Backend_Init(true);

	int w;
	int h;

	#ifdef PLATFORM_PSVITA
		w = 960;
		h = 544;
	#elif defined(PLATFORM_3DS)
		w = 400;
		h = 240;
	#else
		w = 640;
		h = 480;
	#endif

	video.windows[0]        = Window_Create(gameName, w, h);
	video.windows[0].width  = w / globalConfig.scale2D;
	video.windows[0].height = h / globalConfig.scale2D;
	Log("Created window 0");

	#ifdef PLATFORM_3DS
		w = 320;
		h = 240;

		video.windows[1]        = Window_Create(gameName, w, h);
		video.windows[1].width  = w / globalConfig.scale2D;
		video.windows[1].height = h / globalConfig.scale2D;
		Log("Created window 1");
	#endif

	Backend_Init(false);
	Backend_SetTarget(&video.windows[0]);

	Event_AddHandler(AE_EVENT_WINDOW_RESIZE, &EventHandler);
}

void Video_Free(void) {
	Backend_Free();
	for (int i = 0; i < WIN_NUM; ++ i) {
		Window_Free(&video.windows[i]);
	}
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
