#include "ui.h"
#include "engine.h"
#include "backend.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/checkBox.h"
#include "ui/textInput.h"
#include "testScene.h"

static char input[128];
static bool thing = false;

static void ManulButton(UI_Button* this, uint8_t button) {
	(void) this;
	(void) button;
	Log("Manul");
	Log("Your input: %s", input);
}

static Vec2 Resizer(UI_Container* cont) {
	(void) cont;
	return (Vec2) {video.windows[0].width, video.windows[0].height};
}

static void Init(Scene* scene) {
	scene->ui = UI_ManagerInit(1);

	UI_Container* container = UI_ManagerAddContainer(
		scene->ui, video.windows[0].width, NULL
	);
	UI_ContainerAlignLeft(container, 0);
	UI_ContainerAlignTop(container, 0);
	container->resizer     = &Resizer;
	container->fixedHeight = video.windows[0].height;
	UI_ContainerSetPadding(container, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(container, 0);

	UI_RowAddElement(row, UI_NewLabel(&engine.font, "hello!", 0));
	UI_RowAddElement(row, UI_NewButton("manul button", false, &ManulButton));
	UI_RowUpdate(row);

	row = UI_ContainerAddRow(container, 0);

	UI_RowAddElement(row, UI_NewTextInput(input, sizeof(input)));
	UI_RowUpdate(row);

	row = UI_ContainerAddRow(container, 0);

	UI_RowAddElement(row, UI_NewLabel(&engine.font, "Click this checkbox:", 0));
	UI_RowAddElement(row, UI_NewCheckBox(&thing));
	UI_RowUpdate(row);

	input[0] = 0;
}

static void Free(Scene* scene) {
	UI_ManagerFree(scene->ui);
}

static bool HandleEvent(Scene* scene, Event* e, bool top) {
	if (!top) return false;

	return UI_ManagerHandleEvent(scene->ui, e);
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;
}

static void Render(Scene* scene, bool top) {
	(void) top;

	Backend_Begin2D();
	UI_ManagerRender(scene->ui);
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
