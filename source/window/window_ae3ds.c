#include "../util.h"
#include "../window.h"

#ifdef PLATFORM_3DS
#include <3ds.h>
#include <3ds/gpu/gx.h>
#include <citro2d.h>
#include <citro3d.h>
#include "../backend.h"

void Window_Init(void) {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_TOP, NULL);
}

void Window_Quit(void) {
	C2D_Fini();
	C3D_Fini();
	gfxExit();
}

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

Window Window_Create(const char* name, int width, int height) {
	(void) name;

	Window ret;
	ret.target = C3D_RenderTargetCreate(
		height, width, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8
	);

	if ((height == 240) && (width == 400)) {
		C3D_RenderTargetSetOutput(ret.target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	}
	else if ((height == 240) && (width == 320)) {
		C3D_RenderTargetSetOutput(ret.target, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	}
	else {
		Log("Invalid window");
		exit(1);
	}

	Backend_SetTarget(&ret);

	return ret;
}

void Window_Free(Window* window) {
	C3D_RenderTargetDetachOutput(window->target);
	C3D_RenderTargetDelete(window->target);
}

void Window_ShowCursor(bool show) {
	(void) show;
}

void Window_SetRelativeMouseMode(bool enable) {
	(void) enable;
}

void Window_MessageBox(int type, const char* title, const char* contents) {
	errorConf err;

	errorInit(&err, ERROR_TEXT, CFG_LANGUAGE_EN);
	errorText(&err, contents);
	errorDisp(&err);
}

#endif
