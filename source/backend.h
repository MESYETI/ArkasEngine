#ifndef AE_BACKEND_H
#define AE_BACKEND_H

#include "types.h"
#include "model.h"

#ifdef AE_BACKEND_GL11
	#include "backends/gl11.h"
#else
	#error "No backend"
#endif

typedef struct {
	bool   doTint;
	Colour tint;
} TextureRenderOpt;

void     Backend_Init(bool beforeWindow);
void     Backend_Free(void);
Texture* Backend_LoadTexture(const char* path);
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
void Backend_FinishRender(void);

#endif
