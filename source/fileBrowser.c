#include "ui.h"
#include "app.h"
#include "mem.h"
#include "video.h"
#include "backend.h"
#include "resources.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "ui/listBox.h"
#include "ui/textInput.h"
#include "fileBrowser.h"

static char fileName[128];

static const char* driveSelected;
static const char* fileSelected;

static UI_ListBoxItem* driveList;

static void Init(Scene* scene) {
	UI_ManagerInit(&scene->ui, 1);

	UI_Container* container = UI_ManagerAddContainer(&scene->ui, 640);
	UI_ContainerCenterX(container);
	UI_ContainerCenterY(container);
	UI_ContainerSetPadding(container, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(container, 24);

	UI_RowAddElement(row, UI_NewLabel(&app.font, "File name:", UI_LABEL_FIXED));
	UI_RowAddElement(row, UI_NewTextInput(fileName, sizeof(fileName)));
	UI_RowFinish(row, true);

	// make drive list
	driveList = SafeMalloc(resources.drivesNum * sizeof(UI_ListBoxItem));

	for (size_t i = 0; i < resources.drivesNum; ++ i) {
		driveList[i] = (UI_ListBoxItem) {resources.drives[i]->name};
	}

	static UI_ListBoxItem files[] = {
		{"file1.txt"},
		{"file2.txt"},
		{"loves_labours_lost.txt"}
	};

	row = UI_ContainerAddRow(container, 412);
	UI_RowAddElement(row, UI_NewListBox(
		driveList, resources.drivesNum, &driveSelected, 100
	));
	UI_RowAddElement(row, UI_NewListBox(files, 3, &fileSelected, 0));
	UI_RowFinish(row, false);

	row = UI_ContainerAddRow(container, 24);
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowAddElement(row, UI_NewButton("Save", false, NULL));
	UI_RowAddElement(row, UI_NewButton("Cancel", false, NULL));
	UI_RowFinish(row, true);
}

static void Free(Scene* scene) {
	UI_ManagerFree(&scene->ui);
}

static bool HandleEvent(Scene* scene, Event* e) {
	UI_ManagerHandleEvent(&scene->ui, e);
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

Scene FileBrowserScene(void) {
	Scene ret;
	ret.type        = SCENE_TYPE_OTHER;
	ret.name        = "File Browser";
	ret.init        = &Init;
	ret.free        = &Free;
	ret.handleEvent = &HandleEvent;
	ret.update      = &Update;
	ret.render      = &Render;
	return ret;
}
