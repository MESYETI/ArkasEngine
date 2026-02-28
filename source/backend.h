#ifndef AE_BACKEND_H
#define AE_BACKEND_H

#include "types.h"
#include "model.h"

// defined:
//   (struct) Texture
#if defined(AE_BACKEND_GL_LEGACY)
	#include "backends/glLegacy.h"
#else
	#error "No backend"
#endif

typedef struct {
	Rect rect;
	bool enabled;
} BackendViewport;

typedef struct {
	bool vsync;
	char name[20];
} BackendOptions;

extern BackendOptions backendOptions;

typedef struct {
	bool   doTint;
	Colour tint;
} TextureRenderOpt;

// implemented per backend
void     Backend_Init(bool beforeWindow);
void     Backend_Free(void);
Texture* Backend_LoadTexture(uint8_t* data, int width, int height, int ch);
void     Backend_FreeTexture(Texture* texture);
Vec2     Backend_GetTextureSize(Texture* texture);
void     Backend_RenderScene(void);
void     Backend_OnWindowResize(void);
void     Backend_RenderModel(Model* model, ModelRenderOpt* opt);

void Backend_DrawTexture(
	Texture* texture, TextureRenderOpt* p_opt, Rect* p_src, Rect* p_dest
);

void Backend_Begin(void);
void Backend_Begin2D(void);
void Backend_Clear(uint8_t r, uint8_t g, uint8_t b);
void Backend_SetViewport(int x, int y, int w, int h);
void Backend_EnableViewport(bool enable);
void Backend_RenderRect(Rect rect, Colour colour);
void Backend_RenderLine(Vec2 a, Vec2 b, Colour colour);
void Backend_InitSkybox(void);
void Backend_FinishRender(void);

BackendViewport Backend_SaveViewport(void);
void            Backend_RestoreViewport(BackendViewport viewport);

// portable
void Backend_HLine(int x, int y, int thick, int len, Colour colour);
void Backend_VLine(int x, int y, int thick, int len, Colour colour);
void Backend_RenderRectOL(Rect rect, Colour colour);

#endif
