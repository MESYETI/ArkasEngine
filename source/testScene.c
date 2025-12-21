#include "ui.h"
#include "backend.h"
#include "ui/label.h"
#include "ui/button.h"
#include "testScene.h"

static void Init(Scene* scene) {
	UI_ManagerInit(&scene->ui, 1);

	UI_Container* container = UI_ManagerAddContainer(&scene->ui, 512);
	UI_ContainerCenterX(container);
	UI_ContainerAlignTop(container, 10);
	UI_ContainerSetPadding(container, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(container, 24);

	UI_RowAddElement(row, UI_NewLabel("hello!"));
	UI_RowAddElement(row, UI_NewButton("manul button", false, NULL));
	UI_RowFinish(row, true);
}

static void Free(Scene* scene) {
	UI_ManagerFree(&scene->ui);
}

static bool HandleEvent(Scene* scene, SDL_Event* e) {
	(void) scene;
	(void) e;
	return true;
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;
}

static void Render(Scene* scene) {
	Backend_Begin2D();
	UI_ManagerRender(&scene->ui);
}

Scene TestScene(void) {
	Scene ret;
	ret.type        = SCENE_TYPE_OTHER;
	ret.name        = "Test Scene";
	ret.init        = &Init;
	ret.free        = &Free;
	ret.handleEvent = &HandleEvent;
	ret.update      = &Update;
	ret.render      = &Render;
	return ret;
}
