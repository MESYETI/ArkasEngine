#ifndef AE_BACKEND_H
#define AE_BACKEND_H

#include "types.h"
#include "model.h"

// defined:
//   (struct) Texture
#if defined(AE_BACKEND_GL11)
	#define AE_BACKEND_LEGACY_GL
	#include "backends/glLegacy.h"
#else
	#error "No backend"
#endif

typedef struct {
	bool   doTint;
	Colour tint;
} TextureRenderOpt;

// implemented per backend
void     Backend_Init(bool beforeWindow);
void     Backend_Free(void);
Texture* Backend_LoadTexture(uint8_t* img, int width, int height, int ch);
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
void Backend_Viewport(int x, int y, int w, int h);
void Backend_RenderRect(Rect rect, Colour colour);
void Backend_FinishRender(void);

// portable
void Backend_HLine(int x, int y, int thick, int len, Colour colour);
void Backend_VLine(int x, int y, int thick, int len, Colour colour);
void Backend_RenderRectOL(Rect rect, Colour colour);

#endif
