#include "ui.h"
#include "engine.h"
#include "mem.h"
#include "video.h"
#include "backend.h"
#include "resources.h"
#include "fileBrowser.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "ui/listBox.h"
#include "ui/dynLabel.h"
#include "ui/textInput.h"
#include "ui/scrollbar.h"

static char fileName[128];
static char cwd[1024];

static const char* driveSelected;
static const char* fileSelected;

static UI_ListBoxItem* driveList;
static UI_ListBoxItem* fileList;
static UI_Element*     filesElem;

static ResourceFile* files     = NULL;
static size_t        filesSize = 0;

static const char* CWDLabel() {
	return cwd;
}

static void UpdateFiles(void) {
	files    = Resources_List(cwd, &filesSize);
	fileList = SafeMalloc(filesSize * sizeof(UI_ListBoxItem));

	for (size_t i = 0; i < filesSize; ++ i) {
		fileList[i] = (UI_ListBoxItem) {BaseName(files[i].fullPath)};
	}

	UI_UpdateListBox(filesElem, fileList, filesSize);
}

static void FileClick(void) {
	for (size_t i = 0; i < filesSize; ++ i) {
		if (strcmp(BaseName(files[i].fullPath), fileSelected) == 0) {
			if (!files[i].dir) return;
		}
	}

	strncat(cwd, "/", sizeof(cwd) - strlen(cwd) - 1);
	strncat(cwd, fileSelected, sizeof(cwd) - strlen(cwd) - 1);

	if (files) {
		Resources_FreeFileList(files, filesSize);
	}

	if (fileList) {
		free(fileList);
	}

	UpdateFiles();
}

static void DriveClick(void) {
	if (files) {
		Resources_FreeFileList(files, filesSize);
		free(fileList);
		fileList = NULL;
	}

	strcpy(cwd, ":");
	strncat(cwd, driveSelected, sizeof(cwd) - 2);
	UpdateFiles();
}

static void UpButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	if (files) {
		Resources_FreeFileList(files, filesSize);
		free(fileList);
		fileList  = NULL;
		filesSize = 0;
		files     = NULL;
	}

	char* end = strrchr(cwd, '/');

	if (end) {
		*end = 0;
		UpdateFiles();
	}
	else {
		cwd[0] = 0;
		UI_UpdateListBox(filesElem, NULL, 0);
	}
}

static void Init(Scene* scene) {
	fileName[0] = 0;
	cwd[0]      = 0;
	files       = NULL;
	filesSize   = 0;

	UI_ManagerInit(&scene->ui, 1);

	UI_Container* container = UI_ManagerAddContainer(&scene->ui, 640);
	UI_ContainerCenterX(container);
	UI_ContainerCenterY(container);
	UI_ContainerSetPadding(container, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(container, 0);

	UI_RowAddElement(row, UI_NewLabel(&engine.font, "File name:", UI_LABEL_FIXED));
	UI_RowAddElement(row, UI_NewTextInput(fileName, sizeof(fileName)));
	UI_RowUpdate(row);

	row = UI_ContainerAddRow(container, 0);

	UI_RowAddElement(row, UI_NewDynLabel(&engine.font, &CWDLabel, 0));
	UI_RowAddElement(row, UI_NewButton("Up", true, &UpButton));
	UI_RowUpdate(row);

	// make drive list
	driveList = SafeMalloc(resources.drivesNum * sizeof(UI_ListBoxItem));

	for (size_t i = 0; i < resources.drivesNum; ++ i) {
		driveList[i] = (UI_ListBoxItem) {resources.drives[i]->name};
	}

	row = UI_ContainerAddRow(container, 391); // 412
	UI_RowAddElement(row, UI_NewListBox(
		driveList, resources.drivesNum, &driveSelected, 100, DriveClick
	));
	filesElem = UI_RowAddElement(
		row, UI_NewListBox(NULL, 0, &fileSelected, 0, FileClick)
	);
	UI_RowAddElement(row, UI_NewScrollBar(40));
	UI_RowUpdate(row);

	row = UI_ContainerAddRow(container, 0);
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowAddElement(row, UI_NewButton("Save", false, NULL));
	UI_RowAddElement(row, UI_NewButton("Cancel", false, NULL));
	UI_RowUpdate(row);
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
