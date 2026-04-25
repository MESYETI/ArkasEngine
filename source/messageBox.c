#include "engine.h"
#include "messageBox.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "ui/separator.h"

static char* title;
static char* message;

#ifdef PLATFORM_3DS
	#define UI_WIN 1
#else
	#define UI_WIN 0
#endif

static void OKButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	SceneManager_SchedulePop();
}

static void Init(Scene* scene) {
	scene->ui = UI_ManagerInit(1);

	UI_Container* con = UI_ManagerAddContainer(scene->ui, 480, NULL);
	UI_ContainerSetWindow(con, UI_WIN);
	UI_ContainerCenterX(con);
	UI_ContainerCenterY(con);
	UI_ContainerSetPadding(con, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddSingleElemRow(con, 0,
		UI_NewLabel(&engine.font, title, UI_LABEL_CENTERED)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(con, 0, UI_NewSeparator());
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(con, 0,
		UI_NewLabel(&engine.font, message, UI_LABEL_CENTERED)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(con, 0, UI_NewSeparator());
	UI_RowUpdate(row);

	row = UI_ContainerAddRow(con, 0);
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowAddElement(row, UI_NewButton("OK", true, &OKButton));
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowUpdate(row);
}

static void Free(Scene* scene) {
	free(title);
	free(message);
	UI_ManagerFree(scene->ui);
}

static bool HandleEvent(Scene* scene, Event* e, bool top) {
	(void) top;

	UI_ManagerHandleEvent(scene->ui, e);
	return true;
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;
}

static void Render(Scene* scene, bool top) {
	(void) top;

	Backend_Begin2D();
	Backend_EnableAlpha(true);
	Backend_RenderRect(
		(Rect) {0, 0, video.windows[UI_WIN].width, video.windows[UI_WIN].height},
		(Colour) {0, 0, 0, 128}
	);

	UI_ManagerRender(scene->ui);
}

Scene NewMessageBoxScene(const char* p_title, const char* p_message) {
	Scene ret;
	ret.type        = SCENE_TYPE_OTHER;
	ret.name        = "Message Box";
	ret.init        = &Init;
	ret.free        = &Free;
	ret.handleEvent = &HandleEvent;
	ret.update      = &Update;
	ret.render      = &Render;

	title   = NewString(p_title);
	message = NewString(p_message);
	return ret;
}
