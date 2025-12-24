#include "ui.h"
#include "app.h"
#include "video.h"
#include "backend.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "testScene.h"

static UI_Container* topCont;
static UI_Container* bottomCont;

static Vec2 camera;

static void Init(Scene* scene) {
	UI_ManagerInit(&scene->ui, 4);

	topCont = UI_ManagerAddContainer(&scene->ui, video.width);
	UI_ContainerAlignLeft(topCont, 0);
	UI_ContainerAlignTop(topCont, 0);
	UI_ContainerSetPadding(topCont, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(topCont, 18);

	UI_RowAddElement(row, UI_NewLabel(&app.font, "Arkas Map Editor", 0));
	UI_RowAddElement(row, UI_NewButton("File", false, NULL));
	UI_RowAddElement(row, UI_NewButton("Edit", false, NULL));
	UI_RowAddElement(row, UI_NewButton("Sector", false, NULL));
	UI_RowAddElement(row, UI_NewButton("Portal", false, NULL));
	UI_RowFinish(row, false);

	bottomCont = UI_ManagerAddContainer(&scene->ui, video.width);
	UI_ContainerAlignLeft(bottomCont, 0);
	UI_ContainerAlignBottom(bottomCont, 0);
	UI_ContainerSetPadding(bottomCont, 5, 5, 5, 5);

	row = UI_ContainerAddRow(bottomCont, 18);

	UI_RowAddElement(row, UI_NewLabel(&app.font, "X: 0.0, Y: 0.0", 0));
	UI_RowAddElement(row, UI_NewSpacer(0));
	UI_RowAddElement(row, UI_NewButton("Play", false, NULL));
	UI_RowFinish(row, false);

	camera = (Vec2) {0, 0};
}

static void Free(Scene* scene) {
	UI_ManagerFree(&scene->ui);
}

static bool HandleEvent(Scene* scene, SDL_Event* e) {
	return UI_ManagerHandleEvent(&scene->ui, e);
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;
}

static void Render(Scene* scene) {
	Backend_Begin2D();

	for (int i = 0; i < video.height; ++ i) {
		if (i % 128 == 0) {
			Backend_HLine(0, i, 1, video.width, (Colour) {0x80, 0x80, 0x80});
		}
		else if (i % 32 == 0) {
			Backend_HLine(0, i, 1, video.width, (Colour) {0x40, 0x40, 0x40});
		}
	}

	for (int i = 0; i < video.width; ++ i) {
		if (i % 128 == 0) {
			Backend_VLine(i, 0, 1, video.height, (Colour) {0x80, 0x80, 0x80});
		}
		else if (i % 32 == 0) {
			Backend_VLine(i, 0, 1, video.height, (Colour) {0x40, 0x40, 0x40});
		}
	}

	UI_ManagerRender(&scene->ui);
}

Scene MapEditorScene(void) {
	Scene ret;
	ret.type        = SCENE_TYPE_OTHER;
	ret.name        = "Map Editor";
	ret.init        = &Init;
	ret.free        = &Free;
	ret.handleEvent = &HandleEvent;
	ret.update      = &Update;
	ret.render      = &Render;
	return ret;
}
