#include "engine.h"
#include "backend.h"
#include "resources.h"
#include "splashScreen.h"

static float     timer;
static Resource* image;
static float     timeAmount;

static SplashScreenCallback callback;
static bool                 ended;

static void Init(Scene* scene) {
	(void) scene;
	timer = 0;
}

static void Free(Scene* scene) {
	(void) scene;

	if (image) {
		Resources_FreeRes(image);
	}
}

static bool HandleEvent(Scene* scene, Event* e, bool top) {
	(void) scene;
	(void) e;
	(void) top;
	return false;
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;

	timer += engine.delta;
}

static void Render(Scene* scene, bool top) {
	(void) scene;
	(void) top;

	Backend_Begin2D();

	if (!image) return;

	Vec2 dim = Backend_GetTextureSize(image->v.texture);

	float mult = ((float) video.windows[0].height) / ((float) dim.y);

	Rect dest;
	dest.h = (int) (((float) dim.y) * mult);
	dest.w = (int) (((float) dim.x) * mult);
	dest.y = 0;
	dest.x = (video.windows[0].width / 2) - (dest.w / 2);

	Backend_DrawTexture(image->v.texture, NULL, NULL, &dest);

	if ((timer > timeAmount) && !ended) {
		callback();
		ended = true;
	}
}

Scene NewSplashScreen(const char* path, SplashScreenCallback p_callback, float p_time) {
	Scene ret;
	ret.type        = SCENE_TYPE_OTHER;
	ret.name        = "Splash Screen";
	ret.init        = &Init;
	ret.free        = &Free;
	ret.handleEvent = &HandleEvent;
	ret.update      = &Update;
	ret.render      = &Render;

	callback   = p_callback;
	image      = Resources_GetRes(path, 0);
	timeAmount = p_time;
	ended      = false;
	return ret;
}
