#include "ui.h"
#include "engine.h"
#include "mem.h"
#include "map.h"
#include "video.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "backend.h"
#include "mapProject.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "ui/dynLabel.h"
#include "ui/dropDown.h"
#include "testScene.h"

#ifdef PLATFORM_3DS
	#define UI_WIN 1
#else
	#define UI_WIN 0
#endif

enum {
	ME_MODE_EDIT = 0,
	ME_MODE_SECTOR
};

static MProject project;

static int          editorMode;
static MProjSector* thisSect;

static UI_Container* topCont;
static UI_Container* bottomCont;

static FVec2 mCamera;
static bool  align = true;

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

static void ToggleAlignButton(uint8_t button) {
	if (button != 0) return;

	align = !align;
}

static void NewSectorButton(uint8_t button) {
	if (button != 0) return;

	editorMode = ME_MODE_SECTOR;

	thisSect = MapProj_NewSector(&project);
}

static void FinishSectorButton(uint8_t button) {
	if (button != 0) return;

	thisSect = NULL;
	editorMode = ME_MODE_EDIT;
}

static void PlayButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	MapProj_Export(&project);

	SceneManager_Free();
	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL,
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

	MapProj_Init(&project);

	scene->ui = UI_ManagerInit(4);

	topCont = UI_ManagerAddContainer(scene->ui, video.windows[UI_WIN].width, NULL);
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

	static UI_DropDownButton editButtons[] = {
		{"Toggle align", &ToggleAlignButton}
	};

	static UI_DropDownButton sectorButtons[] = {
		{"New", &NewSectorButton},
		{"Finish", &FinishSectorButton}
	};

	UI_RowAddElement(row, UI_NewLabel(&engine.font, "Arkas Map Editor", 0));
	UI_RowAddElement(row, UI_NewDropDown("File", fileButtons, 4, false));
	UI_RowAddElement(row, UI_NewDropDown("Edit", editButtons, 1, false));
	UI_RowAddElement(row, UI_NewDropDown("Sector", sectorButtons, 2, false));
	UI_RowAddElement(row, UI_NewButton("Portal", false, NULL));
	UI_RowUpdate(row);

	bottomCont = UI_ManagerAddContainer(scene->ui, video.windows[UI_WIN].width, NULL);
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
	UI_ManagerFree(scene->ui);
	MapProj_Free(&project);
}

static bool HandleEvent(Scene* scene, Event* e) {
	if (UI_ManagerHandleEvent(scene->ui, e)) return true;

	switch (e->type) {
		case AE_EVENT_MOUSE_BUTTON_DOWN: {
			switch (editorMode) {
				case ME_MODE_SECTOR: {
					if (e->mouseButton.button != 0) break;

					MProjPoint point;
					point.pos.x = (((float) input.mousePos.x) / 32.0) + mCamera.x;
					point.pos.y = (((float) input.mousePos.y) / 32.0) + mCamera.y;

					if (align) {
						point.pos.x = roundf(point.pos.x);
						point.pos.y = roundf(point.pos.y);
					}

					MapProj_AddPoint(thisSect, point);
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
					for (size_t i = 0; i < project.sectorsLen; ++ i) {
						for (size_t j = 0; j < project.sectors[i].pointsLen; ++ j) {
							MProjPoint* point = &project.sectors[i].points[j];

							Vec2 onScreen = (Vec2) {
								.x = ((int) ((point->pos.x - mCamera.x) * 32.0)),
								.y = ((int) ((point->pos.y - mCamera.y) * 32.0))
							};

							if (DistanceI(input.mousePos, onScreen) < 15.0) {
								point->pos.x =
									(((float) input.mousePos.x) / 32.0) + mCamera.x;
								point->pos.y =
									(((float) input.mousePos.y) / 32.0) + mCamera.y;

								if (align) {
									point->pos.x = roundf(point->pos.x);
									point->pos.y = roundf(point->pos.y);
								}
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
	Backend_Clear(0, 0, 0);

	const int unitPx = 32;

	Vec2 cam = {
		(int) (mCamera.x * unitPx),
		(int) (mCamera.y * unitPx)
	};

	for (int i = 0; i < video.windows[UI_WIN].height; ++ i) {
		int w = video.windows[UI_WIN].width;

		if ((i + cam.y) % (unitPx * 4) == 0) {
			Backend_HLine(0, i, 1, w, (Colour) {0x80, 0x80, 0x80, 0xFF});
		}
		else if ((i + cam.y) % unitPx == 0) {
			Backend_HLine(0, i, 1, w, (Colour) {0x40, 0x40, 0x40, 0xFF});
		}
	}

	for (int i = 0; i < video.windows[UI_WIN].width; ++ i) {
		if ((i + cam.x) % (unitPx * 4) == 0) {
			Backend_VLine(
				i, 0, 1, video.windows[UI_WIN].height,
				(Colour) {0x80, 0x80, 0x80, 0xFF}
			);
		}
		else if ((i + cam.x) % unitPx == 0) {
			Backend_VLine(
				i, 0, 1, video.windows[UI_WIN].height,
				(Colour) {0x40, 0x40, 0x40, 0xFF}
			);
		}
	}

	for (size_t i = 0; i < project.sectorsLen; ++ i) {
		for (size_t j = 0; j < project.sectors[i].pointsLen; ++ j) {
			MProjPoint* point = &project.sectors[i].points[j];

			Rect rect = (Rect) {
				.x = ((int) ((point->pos.x - mCamera.x) * unitPx)) - 2,
				.y = ((int) ((point->pos.y - mCamera.y) * unitPx)) - 2,
				.w = 4, .h = 4
			};
			Backend_RenderRect(rect, (Colour) {0xFF, 0xFF, 0xFF, 0xFF});

			size_t next = (j == project.sectors[i].pointsLen - 1)?
				0 : j + 1;
			point = &project.sectors[i].points[next];

			Vec2 b = (Vec2) {
				.x = ((int) ((point->pos.x - mCamera.x) * unitPx)),
				.y = ((int) ((point->pos.y - mCamera.y) * unitPx))
			};

			Backend_RenderLine(
				(Vec2) {rect.x + 2, rect.y + 2}, b, (Colour) {0x80, 0x80, 0x80, 0xFF}
			);
		}
	}

	Vec2 renderCursor = input.mousePos;
	if (align) {
		FVec2 point;
		point.x = (((float) input.mousePos.x) / 32.0) + mCamera.x;
		point.y = (((float) input.mousePos.y) / 32.0) + mCamera.y;

		FVec2 offset;
		offset.x = roundf(point.x) - point.x;
		offset.y = roundf(point.y) - point.y;

		renderCursor.x += (int) (offset.x * ((float) unitPx));
		renderCursor.y += (int) (offset.y * ((float) unitPx));
	}

	Rect cursor = (Rect) {
		.x = renderCursor.x - 5,
		.y = renderCursor.y - 5,
		.w = 10, .h = 10
	};
	Backend_RenderRectOL(cursor, (Colour) {0xFF, 0xFF, 0xFF, 0xFF});

	UI_ManagerRender(scene->ui);
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
