#include "ui.h"
#include "app.h"
#include "backend.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/textInput.h"
#include "testScene.h"

static char input[128];

static void ManulButton(UI_Button* this, uint8_t button) {
	(void) this;
	(void) button;
	Log("Manul");
	Log("Your input: %s", input);
}

static void Init(Scene* scene) {
	UI_ManagerInit(&scene->ui, 1);

	UI_Container* container = UI_ManagerAddContainer(&scene->ui, 512);
	UI_ContainerCenterX(container);
	UI_ContainerAlignTop(container, 10);
	UI_ContainerSetPadding(container, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(container, 24);

	UI_RowAddElement(row, UI_NewLabel(&app.font, "hello!", 0));
	UI_RowAddElement(row, UI_NewButton("manul button", false, &ManulButton));
	UI_RowFinish(row, true);

	row = UI_ContainerAddRow(container, 24);

	UI_RowAddElement(row, UI_NewTextInput(input, sizeof(input)));
	UI_RowFinish(row, true);

	input[0] = 0;
}

static void Free(Scene* scene) {
	UI_ManagerFree(&scene->ui);
}

static bool HandleEvent(Scene* scene, Event* e) {
	return UI_ManagerHandleEvent(&scene->ui, e);
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
