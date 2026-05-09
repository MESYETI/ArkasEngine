#ifndef AE_MAP_EDITOR_H
#define AE_MAP_EDITOR_H

#include "scene.h"
#include "mapProject.h"

enum {
	ME_MODE_EDIT = 0,
	ME_MODE_SECTOR,
	ME_MODE_AUTO_PORTAL
};

typedef struct {
	void (*init)(UI_Manager* ui);
	void (*free)(void);
	void (*update)(void);
	bool (*handleEvent)(Event* e);
	void (*onSectorSelect)(void);
	void (*onPointSelect)(void);
	void (*render)(void);
} MapEditorExt;

typedef struct {
	FVec2 mCamera; // in world coordinates
	bool  align;

	int          editorMode;
	MProjSector* thisSect;
	MProjPoint*  thisPoint;

	MProject project;

	MapEditorExt* extensions;
	size_t        extAmount;

	UI_Container* topCont;
	UI_Container* bottomCont;

	char* savePath;
} MapEditor;

extern MapEditor mapEditor;

void  MapEditor_Init(void);
Scene MapEditor_Scene(void);
void  MapEditor_AddExt(MapEditorExt ext);

#endif
