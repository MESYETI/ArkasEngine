#include "ui.h"
#include "engine.h"
#include "mem.h"
#include "map.h"
#include "video.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "backend.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "ui/dynLabel.h"
#include "ui/dropDown.h"
#include "testScene.h"

enum {
	ME_MODE_EDIT = 0,
	ME_MODE_SECTOR
};

static int editorMode;

typedef struct {
	FVec2 pos;
} EPoint;

typedef struct {
	EPoint* points;
	size_t  pointsLen;
} ESector;

static ESector* thisSect;
static ESector* sectors;
static size_t   sectorsLen;

static UI_Container* topCont;
static UI_Container* bottomCont;

static FVec2 mCamera;

static const char* CoordLabel(void) {
	static char buf[64];
	snprintf(buf, sizeof(buf), "X: %.2f, Y: %.2f", mCamera.x, mCamera.y);

	return (const char*) buf;
}

static const char* ModeLabel(void) {
	switch (editorMode) {
		case ME_MODE_EDIT:   return "Editing";
		case ME_MODE_SECTOR: return "New sector";
		default:             return "???";
	}
}

static void NewSectorButton(uint8_t button) {
	if (button != 0) return;

	editorMode = ME_MODE_SECTOR;

	sectors  = SafeRealloc(sectors, sizeof(ESector) * (sectorsLen + 1));
	thisSect = &sectors[sectorsLen];
	++ sectorsLen;

	thisSect->points    = NULL;
	thisSect->pointsLen = 0;
}

static void FinishSectorButton(uint8_t button) {
	if (button != 0) return;

	thisSect = NULL;
	editorMode = ME_MODE_EDIT;
}

static void PlayButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	Map_Free();
	Map_Init();

	size_t pointsNum = 0;
	for (size_t i = 0; i < sectorsLen; ++ i) {
		pointsNum += sectors[i].pointsLen;
	}

	map.points     = SafeMalloc(sizeof(MapPoint) * pointsNum);
	map.pointsLen  = pointsNum;
	map.walls      = SafeMalloc(sizeof(Wall) * pointsNum);
	map.wallsLen   = pointsNum;
	map.sectors    = SafeMalloc(sizeof(Sector) * sectorsLen);
	map.sectorsLen = sectorsLen;

	pointsNum = 0;
	for (size_t i = 0; i < sectorsLen; ++ i) {
		map.sectors[i] = (Sector) {
			pointsNum, sectors[i].pointsLen, 0.5, -0.5,
			(FVec2) {0, 0}, (FVec2) {0, 0}, false, false,
			Resources_GetRes("base:3p_textures/grass2.png", 0),
			Resources_GetRes("base:3p_textures/wood3.png", 0)
		};
		for (size_t j = 0; j < sectors[i].pointsLen; ++ j, ++ pointsNum) {
			map.points[pointsNum] = (MapPoint) {sectors[i].points[j].pos};
			map.walls[pointsNum]  = (Wall) {
				false, false, 0, Resources_GetRes("base:3p_textures/brick1.png", 0)
			};
		}
	}

	SceneManager_Free();
	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", (UI_Manager) {0}, NULL, NULL, NULL,
		NULL, NULL
	});

	camera.sector = &map.sectors[0];
	player.sector = &map.sectors[0];
}

static void Unimplemented(uint8_t button) {
	(void) button;

	Log("Unimplemented");
}

static void Init(Scene* scene) {
	editorMode = ME_MODE_EDIT;
	thisSect   = NULL;
	sectors    = NULL;
	sectorsLen = 0;

	UI_ManagerInit(&scene->ui, 4);

	topCont = UI_ManagerAddContainer(&scene->ui, video.width);
	UI_ContainerAlignLeft(topCont, 0);
	UI_ContainerAlignTop(topCont, 0);
	UI_ContainerSetPadding(topCont, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(topCont, 18);

	static UI_DropDownButton fileButtons[] = {
		{"New (c+N)", &Unimplemented},
		{"Open (c+O)", &Unimplemented},
		{"Save (c+S)", &Unimplemented},
		{"Save as (c+D)", &Unimplemented}
	};

	static UI_DropDownButton sectorButtons[] = {
		{"New", &NewSectorButton},
		{"Finish", &FinishSectorButton}
	};

	UI_RowAddElement(row, UI_NewLabel(&engine.font, "Arkas Map Editor", 0));
	UI_RowAddElement(row, UI_NewDropDown("File", fileButtons, 4, false));
	UI_RowAddElement(row, UI_NewButton("Edit", false, NULL));
	UI_RowAddElement(row, UI_NewDropDown("Sector", sectorButtons, 2, false));
	UI_RowAddElement(row, UI_NewButton("Portal", false, NULL));
	UI_RowUpdate(row);

	bottomCont = UI_ManagerAddContainer(&scene->ui, video.width);
	UI_ContainerAlignLeft(bottomCont, 0);
	UI_ContainerAlignBottom(bottomCont, 0);
	UI_ContainerSetPadding(bottomCont, 5, 5, 5, 5);

	row = UI_ContainerAddRow(bottomCont, 18);

	UI_RowAddElement(row, UI_NewDynLabel(&engine.font, &CoordLabel, 0));
	UI_RowAddElement(row, UI_NewDynLabel(&engine.font, &ModeLabel, UI_LABEL_CENTERED));
	UI_RowAddElement(row, UI_NewButton("Play", false, &PlayButton));
	UI_RowUpdate(row);

	mCamera = (FVec2) {0, 0};
}

static void Free(Scene* scene) {
	UI_ManagerFree(&scene->ui);
}

static bool HandleEvent(Scene* scene, Event* e) {
	if (UI_ManagerHandleEvent(&scene->ui, e)) return true;

	switch (e->type) {
		case AE_EVENT_MOUSE_BUTTON_DOWN: {
			switch (editorMode) {
				case ME_MODE_SECTOR: {
					++ thisSect->pointsLen;
					thisSect->points = SafeRealloc(
						thisSect->points, sizeof(EPoint) * thisSect->pointsLen
					);

					EPoint* point = &thisSect->points[thisSect->pointsLen - 1];

					point->pos.x = (((float) input.mousePos.x) / 32.0) + mCamera.x;
					point->pos.y = (((float) input.mousePos.y) / 32.0) + mCamera.y;
					break;
				}
				default: break;
			}
			break;
		}
		case AE_EVENT_MOUSE_MOVE: {
			if (input.mouseBtn[2]) {
				mCamera.x += ((float) -e->mouseMove.xRel) / 32.0;
				mCamera.y += ((float) -e->mouseMove.yRel) / 32.0;
				return true;
			}
			else if (input.mouseBtn[0]) {
				if (editorMode == ME_MODE_EDIT) {
					for (size_t i = 0; i < sectorsLen; ++ i) {
						for (size_t j = 0; j < sectors[i].pointsLen; ++ j) {
							EPoint* point = &sectors[i].points[j];

							Vec2 onScreen = (Vec2) {
								.x = ((int) ((point->pos.x - mCamera.x) * 32.0)),
								.y = ((int) ((point->pos.y - mCamera.y) * 32.0))
							};

							if (DistanceI(input.mousePos, onScreen) < 15.0) {
								point->pos.x =
									(((float) input.mousePos.x) / 32.0) + mCamera.x;
								point->pos.y =
									(((float) input.mousePos.y) / 32.0) + mCamera.y;
							}
						}
					}
				}
			}
			break;
		}
	}

	return false;
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;
}

static void Render(Scene* scene) {
	Backend_Begin2D();

	const int unitPx = 32;

	Vec2 cam = {
		(int) (mCamera.x * unitPx),
		(int) (mCamera.y * unitPx)
	};

	for (int i = 0; i < video.height; ++ i) {
		if ((i + cam.y) % (unitPx * 4) == 0) {
			Backend_HLine(0, i, 1, video.width, (Colour) {0x80, 0x80, 0x80});
		}
		else if ((i + cam.y) % unitPx == 0) {
			Backend_HLine(0, i, 1, video.width, (Colour) {0x40, 0x40, 0x40});
		}
	}

	for (int i = 0; i < video.width; ++ i) {
		if ((i + cam.x) % (unitPx * 4) == 0) {
			Backend_VLine(i, 0, 1, video.height, (Colour) {0x80, 0x80, 0x80});
		}
		else if ((i + cam.x) % unitPx == 0) {
			Backend_VLine(i, 0, 1, video.height, (Colour) {0x40, 0x40, 0x40});
		}
	}

	for (size_t i = 0; i < sectorsLen; ++ i) {
		for (size_t j = 0; j < sectors[i].pointsLen; ++ j) {
			EPoint* point = &sectors[i].points[j];

			Rect rect = (Rect) {
				.x = ((int) ((point->pos.x - mCamera.x) * unitPx)) - 2,
				.y = ((int) ((point->pos.y - mCamera.y) * unitPx)) - 2,
				.w = 4, .h = 4
			};
			Backend_RenderRect(rect, (Colour) {0xFF, 0xFF, 0xFF});

			size_t next = (j == sectors[i].pointsLen - 1)?
				0 : j + 1;
			point = &sectors[i].points[next];

			Vec2 b = (Vec2) {
				.x = ((int) ((point->pos.x - mCamera.x) * unitPx)),
				.y = ((int) ((point->pos.y - mCamera.y) * unitPx))
			};

			Backend_RenderLine(
				(Vec2) {rect.x + 2, rect.y + 2}, b, (Colour) {0x80, 0x80, 0x80}
			);
		}
	}

	Rect cursor = (Rect) {
		.x = input.mousePos.x - 5,
		.y = input.mousePos.y - 5,
		.w = 10, .h = 10
	};
	if (editorMode == ME_MODE_EDIT) {
		Backend_RenderRectOL(cursor, (Colour) {0xFF, 0xFF, 0xFF});
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
