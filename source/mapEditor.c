#include "ui.h"
#include "mem.h"
#include "map.h"
#include "video.h"
#include "input.h"
#include "engine.h"
#include "camera.h"
#include "player.h"
#include "backend.h"
#include "mapEditor.h"
#include "mapProject.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/spacer.h"
#include "ui/dynLabel.h"
#include "ui/dropDown.h"
#include "ui/textInput.h"
#include "mapEditor/sectorEdit.h"

MapEditor mapEditor = {
	.align      = true,
	.extensions = NULL,
	.extAmount  = 0
};

static const char* CoordLabel(void) {
	static char buf[64];

	FVec2 centerPos = {
		mapEditor.mCamera.x + (((float) video.windows[ME_UI_WIN].width)  / 32.0f / 2.0f),
		mapEditor.mCamera.y + (((float) video.windows[ME_UI_WIN].height) / 32.0f / 2.0f)
	};

	snprintf(buf, sizeof(buf), "X: %.2f, Y: %.2f", centerPos.x, centerPos.y);

	return (const char*) buf;
}

static const char* ModeLabel(void) {
	switch (mapEditor.editorMode) {
		case ME_MODE_EDIT:   return "Editing";
		case ME_MODE_SECTOR: return "New sector";
		default:             return "???";
	}
}

static void ToggleAlignButton(uint8_t button) {
	if (button != 0) return;

	mapEditor.align = !mapEditor.align;
}

static void NewSectorButton(uint8_t button) {
	if (button != 0) return;

	mapEditor.editorMode = ME_MODE_SECTOR;

	mapEditor.thisSect = MapProj_NewSector(&mapEditor.project);
}

static void FinishSectorButton(uint8_t button) {
	if (button != 0) return;

	mapEditor.thisSect   = NULL;
	mapEditor.editorMode = ME_MODE_EDIT;
}

static void AutoPortalButton(uint8_t button) {
	if (button != 0) return;

	mapEditor.thisSect   = NULL;
	mapEditor.editorMode = ME_MODE_AUTO_PORTAL;
}

static void PlayButton(UI_Button* this, uint8_t button) {
	(void) this;

	if (button != 0) return;

	MapProj_Export(&mapEditor.project);

	SceneManager_Free();
	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL,
		NULL, NULL
	});

	camera.sector = &map.sectors[0];
	player.sector = &map.sectors[0];
}

// static void RightContMinimise(UI_Button* this, uint8_t button) {
// 	if (button != 0) return;
// 
// 	rightCont->fixedHeight = rightCont->fixedHeight? 0 : 256;
// }

static void Unimplemented(uint8_t button) {
	(void) button;

	Log("Unimplemented");
}

static Vec2 BarResizer(UI_Container* cont) {
	return (Vec2) {video.windows[ME_UI_WIN].width, cont->h};
}

static void Init(Scene* scene) {
	mapEditor.editorMode = ME_MODE_EDIT;
	mapEditor.thisSect   = NULL;
	mapEditor.thisPoint  = NULL;

	MapProj_Init(&mapEditor.project);

	scene->ui = UI_ManagerInit(4);

	mapEditor.topCont = UI_ManagerAddContainer(
		scene->ui, video.windows[ME_UI_WIN].width, &BarResizer
	);

	UI_ContainerAlignLeft(mapEditor.topCont, 0);
	UI_ContainerAlignTop(mapEditor.topCont, 0);
	UI_ContainerSetPadding(mapEditor.topCont, 5, 5, 5, 5);

	UI_Row* row = UI_ContainerAddRow(mapEditor.topCont, 18);

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
		{"Finish", &FinishSectorButton},
		{"Auto Portal", &AutoPortalButton}
	};

	UI_RowAddElement(row, UI_NewLabel(&engine.font, "Arkas Map Editor", 0));
	UI_RowAddElement(row, UI_NewDropDown("File", fileButtons, 4, false));
	UI_RowAddElement(row, UI_NewDropDown("Edit", editButtons, 1, false));
	UI_RowAddElement(row, UI_NewDropDown("Sector", sectorButtons, 3, false));
	UI_RowAddElement(row, UI_NewButton("Portal", false, NULL));
	UI_RowUpdate(row);

	mapEditor.bottomCont = UI_ManagerAddContainer(
		scene->ui, video.windows[ME_UI_WIN].width, &BarResizer
	);

	UI_ContainerAlignLeft(mapEditor.bottomCont, 0);
	UI_ContainerAlignBottom(mapEditor.bottomCont, 0);
	UI_ContainerSetPadding(mapEditor.bottomCont, 5, 5, 5, 5);

	row = UI_ContainerAddRow(mapEditor.bottomCont, 18);

	UI_RowAddElement(row, UI_NewDynLabel(&engine.font, &CoordLabel, 0));
	UI_RowAddElement(row, UI_NewDynLabel(&engine.font, &ModeLabel, UI_LABEL_CENTERED));
	UI_RowAddElement(row, UI_NewButton("Play", false, &PlayButton));
	UI_RowUpdate(row);

	mapEditor.mCamera    = (FVec2) {0, 0};
	mapEditor.mCamera.x -= ((float) video.windows[ME_UI_WIN].width)  / 32.0f / 2.0f;
	mapEditor.mCamera.y -= ((float) video.windows[ME_UI_WIN].height) / 32.0f / 2.0f;

	for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
		MapEditorExt* ext = &mapEditor.extensions[i];

		if (ext->init) ext->init(scene->ui);
	}
}

static void Free(Scene* scene) {
	for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
		MapEditorExt* ext = &mapEditor.extensions[i];

		if (ext->free) ext->free();
	}

	UI_ManagerFree(scene->ui);
	MapProj_Free(&mapEditor.project);
}

static FVec2 CursorOnMap(void) {
	FVec2 ret = {
		(((float) input.mousePos.x) / 32.0) + mapEditor.mCamera.x,
		(((float) input.mousePos.y) / 32.0) + mapEditor.mCamera.y
	};

	if (mapEditor.align) {
		ret.x = roundf(ret.x);
		ret.y = roundf(ret.y);
	}

	return ret;
}

static bool HandleEvent(Scene* scene, Event* e, bool top) {
	if (!top) return false;

	if (UI_ManagerHandleEvent(scene->ui, e)) return true;

	switch (e->type) {
		case AE_EVENT_MOUSE_BUTTON_DOWN: {
			switch (mapEditor.editorMode) {
				case ME_MODE_EDIT: {
					for (size_t i = 0; i < mapEditor.project.sectorsLen; ++ i) {
						FVec2        point = CursorOnMap();
						MProjSector* sect  = &mapEditor.project.sectors[i];

						if (MapProj_PointInSector(sect, point)) {
							mapEditor.thisSect = sect;

							for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
								MapEditorExt* ext = &mapEditor.extensions[i];

								if (ext->onSectorSelect) {
									ext->onSectorSelect();
								}
							}

							break;
						}
					}
					break;
				}
				case ME_MODE_SECTOR: {
					if (e->mouseButton.button != 0) break;

					MProjPoint point;

					point.pos    = CursorOnMap();
					point.portal = false;

					MapProj_AddPoint(mapEditor.thisSect, point);
					break;
				}
				case ME_MODE_AUTO_PORTAL: {
					if (e->mouseButton.button != 0) break;

					MProjPoint point;
					point.pos = CursorOnMap();

					MProjSector* firstSector = NULL;
					MProjPoint*  firstPoint  = NULL;

					for (size_t i = 0; i < mapEditor.project.sectorsLen; ++ i) {
						MProjSector* sect = &mapEditor.project.sectors[i];

						for (size_t j = 0; j < sect->pointsLen; ++ j) {
							MProjPoint* mPoint = &sect->points[j];

							FVec2 a = mPoint->pos;
							FVec2 b = j == sect->pointsLen - 1?
								sect->points[0].pos : sect->points[j + 1].pos;

							if (
								(LinePointDistance(a, b, point.pos) < 10.0f / 32.0f) &&
								PointInLine(point.pos, a, b)
							) {
								if (firstPoint) {
									firstPoint->portal    = true;
									firstPoint->portalIdx = i;

									mPoint->portal    = true;
									mPoint->portalIdx =
										firstSector - mapEditor.project.sectors;
									goto endAutoPortal;
								}
								else {
									firstPoint  = &sect->points[j];
									firstSector = sect;
									break; // no portals within sectors
								}
							}
						}
					}

					endAutoPortal:
					break;
				}
				default: break;
			}
			break;
		}
		case AE_EVENT_MOUSE_MOVE: {
			if (input.mouseBtn[2]) {
				mapEditor.mCamera.x += ((float) -e->mouseMove.xRel) / 32.0;
				mapEditor.mCamera.y += ((float) -e->mouseMove.yRel) / 32.0;
				return true;
			}
			else if (input.mouseBtn[0]) {
				if (mapEditor.editorMode == ME_MODE_EDIT) {
					for (size_t i = 0; i < mapEditor.project.sectorsLen; ++ i) {
						for (size_t j = 0; j < mapEditor.project.sectors[i].pointsLen; ++ j) {
							MProjPoint* point = &mapEditor.project.sectors[i].points[j];

							Vec2 onScreen = (Vec2) {
								.x = ((int) ((point->pos.x - mapEditor.mCamera.x) * 32.0)),
								.y = ((int) ((point->pos.y - mapEditor.mCamera.y) * 32.0))
							};

							if (DistanceI(input.mousePos, onScreen) < 15.0) {
								point->pos          = CursorOnMap();

								MProjPoint* oldSelect = mapEditor.thisPoint;
								mapEditor.thisPoint = point;

								if (oldSelect != point) {
									for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
										MapEditorExt* ext = &mapEditor.extensions[i];

										if (ext->onPointSelect) {
											ext->onPointSelect();
										}
									}
								}
							}
						}
					}
				}
			}
			break;
		}
	}

	for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
		MapEditorExt* ext = &mapEditor.extensions[i];

		if (ext->handleEvent) {
			if (ext->handleEvent(e)) return true;
		}
	}

	return false;
}

static void Update(Scene* scene, bool top) {
	(void) scene;
	(void) top;

	for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
		MapEditorExt* ext = &mapEditor.extensions[i];

		if (ext->update) {
			ext->update();
		}
	}
}

static void Render(Scene* scene, bool top) {
	Backend_Begin2D();
	Backend_Clear(0, 0, 0);
	Backend_EnableAlpha(true);

	const int unitPx = 32;

	Vec2 cam = {
		(int) (mapEditor.mCamera.x * unitPx),
		(int) (mapEditor.mCamera.y * unitPx)
	};

	for (int i = 0; i < video.windows[ME_UI_WIN].height; ++ i) {
		int w = video.windows[ME_UI_WIN].width;

		if ((i + cam.y) % (unitPx * 4) == 0) {
			Backend_HLine(0, i, 1, w, (Colour) {0x80, 0x80, 0x80, 0xFF});
		}
		else if ((i + cam.y) % unitPx == 0) {
			Backend_HLine(0, i, 1, w, (Colour) {0x40, 0x40, 0x40, 0xFF});
		}
	}

	for (int i = 0; i < video.windows[ME_UI_WIN].width; ++ i) {
		if ((i + cam.x) % (unitPx * 4) == 0) {
			Backend_VLine(
				i, 0, 1, video.windows[ME_UI_WIN].height,
				(Colour) {0x80, 0x80, 0x80, 0xFF}
			);
		}
		else if ((i + cam.x) % unitPx == 0) {
			Backend_VLine(
				i, 0, 1, video.windows[ME_UI_WIN].height,
				(Colour) {0x40, 0x40, 0x40, 0xFF}
			);
		}
	}

	for (size_t i = 0; i < mapEditor.project.sectorsLen; ++ i) {
		MProjSector* sect = &mapEditor.project.sectors[i];

		for (size_t j = 0; j < sect->pointsLen; ++ j) {
			MProjPoint* point = &sect->points[j];

			Rect rect = (Rect) {
				.x = ((int) ((point->pos.x - mapEditor.mCamera.x) * unitPx)) - 2,
				.y = ((int) ((point->pos.y - mapEditor.mCamera.y) * unitPx)) - 2,
				.w = 4, .h = 4
			};
			Backend_RenderRect(rect, (Colour) {0xFF, 0xFF, 0xFF, 0xFF});

			size_t next = (j == sect->pointsLen - 1)?
				0 : j + 1;
			point = &sect->points[next];

			Vec2 a = (Vec2) {rect.x + 2, rect.y + 2};
			Vec2 b = (Vec2) {
				.x = ((int) ((point->pos.x - mapEditor.mCamera.x) * unitPx)),
				.y = ((int) ((point->pos.y - mapEditor.mCamera.y) * unitPx))
			};

			Colour wallColour = sect == mapEditor.thisSect?
				(Colour) {0xFF, 0x88, 0xFF, 255} : (Colour) {255, 255, 255, 255};

			wallColour = sect->points[j].portal?
				(Colour) {0xFF, 0x8C, 0x00, 255} : wallColour;

			FVec2 cursor = CursorOnMap();
			if (
				(LinePointDistance(
					sect->points[j].pos, sect->points[next].pos, cursor
				) < 10.0f / 32.0f) &&
				PointInLine(cursor, sect->points[j].pos, sect->points[next].pos)
			) {
				wallColour = (Colour) {0x88, 0x88, 0xFF, 0xFF};
			}

			Backend_RenderLine(a, b, wallColour);
		}
	}

	Vec2 renderCursor = input.mousePos;
	if (mapEditor.align) {
		FVec2 point;
		point.x = (((float) input.mousePos.x) / 32.0) + mapEditor.mCamera.x;
		point.y = (((float) input.mousePos.y) / 32.0) + mapEditor.mCamera.y;

		FVec2 offset;
		offset.x = roundf(point.x) - point.x;
		offset.y = roundf(point.y) - point.y;

		renderCursor.x += (int) (offset.x * ((float) unitPx));
		renderCursor.y += (int) (offset.y * ((float) unitPx));
	}

	// render camera lines
	Colour lineColour = {64, 64, 255, 128};
	Backend_HLine(
		0, video.windows[ME_UI_WIN].height / 2, 1, video.windows[ME_UI_WIN].width,
		lineColour
	);
	Backend_VLine(
		video.windows[ME_UI_WIN].width / 2, 0, 1, video.windows[ME_UI_WIN].height,
		lineColour
	);

	Rect cursor = (Rect) {
		.x = renderCursor.x - 5,
		.y = renderCursor.y - 5,
		.w = 10, .h = 10
	};
	if (top) {
		Backend_RenderRectOL(cursor, (Colour) {0xFF, 0xFF, 0xFF, 0xFF});
	}

	for (size_t i = 0; i < mapEditor.extAmount; ++ i) {
		MapEditorExt* ext = &mapEditor.extensions[i];

		if (ext->update) {
			ext->update();
		}
	}

	UI_ManagerRender(scene->ui);
}

void MapEditor_Init(void) {
	MapEditor_AddExt(MapEditorExt_SectorEdit());
}

Scene MapEditor_Scene(void) {
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

void MapEditor_AddExt(MapEditorExt ext) {
	++ mapEditor.extAmount;
	mapEditor.extensions = SafeRealloc(
		mapEditor.extensions, mapEditor.extAmount * sizeof(MapEditorExt)
	);

	mapEditor.extensions[mapEditor.extAmount - 1] = ext;
}
