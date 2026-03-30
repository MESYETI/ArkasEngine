#include "../backend.h"

#ifdef AE_BACKEND_CITRO3D
#include "citro3d.h"

void Backend_Init(bool beforeWindow) {
	(void) beforeWindow;
}

void Backend_Free(void) {
	
}

Texture* Backend_LoadTexture(uint8_t* data, int width, int height, int ch) {
	(void) data;
	(void) width;
	(void) height;
	(void) ch;
	return NULL;
}

void Backend_FreeTexture(Texture* texture) {
	(void) texture;
}

Vec2 Backend_GetTextureSize(Texture* texture) {
	(void) texture;
	return (Vec2) {0, 0};
}

void Backend_RenderScene(void) {
	
}

void Backend_OnMapFree(void) {
	
}

void Backend_OnWindowResize(void) {
	
}

void Backend_RenderModel(Model* model, ModelRenderOpt* opt) {
	(void) model;
	(void) opt;
}

void Backend_DrawTexture(
	Texture* texture, TextureRenderOpt* p_opt, Rect* p_src, Rect* p_dest
) {
	(void) texture;

	TextureRenderOpt opt;
	if (p_opt == NULL) {
		opt.doTint = false;
	}
	else {
		opt = *p_opt;
	}

	Rect src;
	if (p_src == NULL) {
		src = (Rect) {0, 0, texture->width, texture->height};
	}
	else {
		src = *p_src;
	}

	Rect dest;
	if (p_dest == NULL) {
		dest = (Rect) {0, 0, video.width, video.height};
	}
	else {
		dest = *p_dest;
	}
}

void Backend_Begin(void) {
	
}

void Backend_Begin2D(void) {
	
}

void Backend_Clear(uint8_t r, uint8_t g, uint8_t b) {
	(void) r;
	(void) g;
	(void) b;
}

void Backend_SetViewport(int x, int y, int w, int h) {
	(void) x;
	(void) y;
	(void) w;
	(void) h;
}

void Backend_EnableViewport(bool enable) {
	(void) enable;
}

void Backend_RenderRect(Rect rect, Colour colour) {
	(void) rect;
	(void) colour;
}

void Backend_RenderLine(Vec2 a, Vec2 b, Colour colour) {
	(void) a;
	(void) b;
	(void) colour;
}

void Backend_EnableAlpha(bool enable) {
	(void) enable;
}

void Backend_InitSkybox(void) {
	
}

void Backend_FinishRender(void) {
	
}

BackendViewport Backend_SaveViewport(void) {
	return (BackendViewport) {};
}

void Backend_RestoreViewport(BackendViewport viewport) {
	(void) viewport;
}

#endif
