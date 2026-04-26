#include "../ui.h"
#include "../mem.h"
#include "../map.h"
#include "../video.h"
#include "../input.h"
#include "../camera.h"
#include "../engine.h"
#include "../player.h"
#include "../backend.h"
#include "../ui/label.h"
#include "../ui/button.h"
#include "../ui/spacer.h"
#include "../ui/dynLabel.h"
#include "../ui/dropDown.h"
#include "../ui/textInput.h"
#include "sectorEdit.h"

typedef struct {
	char floor[32];
	char ceiling[32];
	char x[32];
	char y[32];
} Options;

static Options options;

static UI_Container* rightCont;

static void SectorApplyChangesButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	// TODO: check NULL

	mapEditor.thisSect->floor   = atof(options.floor);
	mapEditor.thisSect->ceiling = atof(options.ceiling);
}

static void PointApplyChangesButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	// TODO: check NULL

	mapEditor.thisPoint->pos.x = atof(options.x);
	mapEditor.thisPoint->pos.y = atof(options.y);
}

static void SectorTabButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	UI_ContainerRestoreSet(rightCont, 0);
}

static void PointTabButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	UI_ContainerRestoreSet(rightCont, 1);
}

static Vec2 OptionsResizer(UI_Container* cont) {
	return (Vec2) {
		cont->w, video.windows[ME_UI_WIN].height -
		UI_ContainerGetRect(mapEditor.topCont).h -
		UI_ContainerGetRect(mapEditor.bottomCont).h
	};
}

static void Init(UI_Manager* ui) {
	rightCont = UI_ManagerAddContainer(ui, 128, &OptionsResizer);

	rightCont->fixedHeight = video.windows[ME_UI_WIN].height -
		UI_ContainerGetRect(mapEditor.topCont).h -
		UI_ContainerGetRect(mapEditor.bottomCont).h;

	UI_ContainerAlignRight(rightCont, 0);
	UI_ContainerCenterY(rightCont);
	UI_ContainerSetPadding(rightCont, 5, 5, 5, 5);
	UI_ContainerAllocRowSets(rightCont, 2);

	UI_Row* row = UI_ContainerAddRow(rightCont, 0);
	UI_RowAddElement(row, UI_NewButton("Sector", false, &SectorTabButton));
	UI_RowAddElement(row, UI_NewButton("Wall", false, &PointTabButton));
	// UI_RowAddElement(row, UI_NewButton("-", true, &RightContMinimise));
	UI_RowUpdate(row);

	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewLabel(&engine.font, "Floor", 0)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewTextInput(options.floor, sizeof(options.floor))
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewLabel(&engine.font, "Ceiling", 0)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewTextInput(options.ceiling, sizeof(options.ceiling))
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewButton("Floor texture", false, NULL)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewButton("Ceiling texture", false, NULL)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewButton("Wall texture", false, NULL)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewButton("Apply changes", false, &SectorApplyChangesButton)
	);
	UI_RowUpdate(row);

	UI_ContainerSaveSet(rightCont, 0);
	UI_ContainerRestoreSet(rightCont, 1);

	row = UI_ContainerAddRow(rightCont, 0);
	UI_RowAddElement(row, UI_NewButton("Sector", false, &SectorTabButton));
	UI_RowAddElement(row, UI_NewButton("Wall", false, &PointTabButton));
	UI_RowUpdate(row);

	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewLabel(&engine.font, "Start X", 0)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewTextInput(options.x, sizeof(options.x))
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewLabel(&engine.font, "Start Y", 0)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewTextInput(options.y, sizeof(options.y))
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewButton("Wall texture", false, NULL)
	);
	UI_RowUpdate(row);
	row = UI_ContainerAddSingleElemRow(
		rightCont, 0, UI_NewButton("Apply changes", false, &PointApplyChangesButton)
	);
	UI_RowUpdate(row);

	UI_ContainerSaveSet(rightCont, 1);
	UI_ContainerRestoreSet(rightCont, 0);
}

static void OnSectorSelect(void) {
	snprintf(options.floor,   sizeof(options.floor),   "%g", mapEditor.thisSect->floor);
	snprintf(options.ceiling, sizeof(options.ceiling), "%g", mapEditor.thisSect->ceiling);
}

static void OnPointSelect(void) {
	snprintf(options.x, sizeof(options.x), "%g", mapEditor.thisPoint->pos.x);
	snprintf(options.y, sizeof(options.y), "%g", mapEditor.thisPoint->pos.y);
}

MapEditorExt MapEditorExt_SectorEdit(void) {
	return (MapEditorExt) {
		.init           = &Init,
		.free           = NULL,
		.update         = NULL,
		.handleEvent    = NULL,
		.onSectorSelect = &OnSectorSelect,
		.onPointSelect  = &OnPointSelect,
		.render         = NULL
	};
}
