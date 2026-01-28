#include "ui.h"
#include "engine.h"
#include "mem.h"
#include "backend.h"
#include "resources.h"
#include "imageViewer.h"

typedef struct {
	Resource* res;
} Data;

static void Init(Scene* scene) {
	(void) scene;
}

static void Free(Scene* scene) {
	Data* data = (Data*) scene->data;

	Resources_FreeRes(data->res);
	free(data);
}

static bool HandleEvent(Scene* scene, Event* e) {
	(void) scene;

	if ((e->type == AE_EVENT_KEY_DOWN) && (e->key.key == AE_KEY_Q)) {
		SceneManager_PopScene();
		return true;
	}

	return false;
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;
}

static void Render(Scene* scene) {
	Data* data = (Data*) scene->data;

	Backend_Begin2D();
	Backend_Clear(0, 0, 0);

	Texture* texture = data->res->v.texture;
	Vec2     texSize = Backend_GetTextureSize(texture);

	Rect dest = {
		(video.width  / 2) - (texSize.x / 2),
		(video.height / 2) - (texSize.y / 2),
		texSize.x, texSize.y
	};

	Backend_DrawTexture(texture, NULL, NULL, &dest);
}

Scene ImageViewerScene(const char* path) {
	Scene ret;
	ret.type        = SCENE_TYPE_OTHER;
	ret.name        = "Image Viewer";
	ret.init        = &Init;
	ret.free        = &Free;
	ret.handleEvent = &HandleEvent;
	ret.update      = &Update;
	ret.render      = &Render;

	Data* data = SafeMalloc(sizeof(Data));
	ret.data   = data;
	data->res  = Resources_GetRes(path, 0);

	return ret;
}
