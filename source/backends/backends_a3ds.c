#include "../util.h"
#include "../backend.h"

#ifdef AE_BACKEND_CITRO3D
#include <citro2d.h>
#include "citro3d.h"

typedef struct {
	Texture textures[64];

	C3D_RenderTarget* target;
} State;

static State state;

static uint32_t CompToABGR(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	// return
	// 	((uint32_t) r) | (((uint32_t) g) << 8) | (((uint32_t) b) << 16) |
	// 	(((uint32_t) a) << 24);
	return
		((uint32_t) a) | (((uint32_t) b) << 8) | (((uint32_t) g) << 16) |
		(((uint32_t) r) << 24);
}

static uint32_t ColourToABGR(Colour c) {
	return
		(((uint32_t) c.a) << 24) | (((uint32_t) c.b) << 16) |
		(((uint32_t) c.g) << 8) | ((uint32_t) c.r);
}

void Backend_Init(bool beforeWindow) {

}

void Backend_Free(void) {
	
}

void Backend_SetTarget(Window* window) {
	state.target = window->target;
}

static Texture* AllocTexture(void) {
	for (size_t i = 0; i < sizeof(state.textures) / sizeof(Texture); ++ i) {
		if (!state.textures[i].used) {
			return &state.textures[i];
		}
	}

	return NULL;
}

Texture* Backend_LoadTexture(uint8_t* data, int w, int h, int aW, int aH, int ch) {
	Texture* ret = AllocTexture();

	if (ret == NULL) {
		return ret;
	}

	ret->tex.width  = aW;
	ret->tex.height = aH;

	if (!C3D_TexInit(&ret->tex, aW, aH, GPU_RGBA8)) {
		Log("Failed to loda texture");
		return NULL;
	}

	C3D_TexSetFilter(&ret->tex, GPU_NEAREST, GPU_NEAREST);

	for (uint32_t x = 0; x < w; ++ x) {
		for (uint32_t y = 0; y < h; ++ y) {
			size_t idx = ((y * aW) + x) * ch;
			Colour colour;

			colour.r = data[idx];
			colour.g = data[idx + 1];
			colour.b = data[idx + 2];
			colour.a = ch == 4? data[idx + 3] : 255;

			// what the fuck?
			uint32_t destOff =
				((((y >> 3) * (ret->tex.width >> 3) + (x >> 3)) << 6) +
				((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) |
				((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)));

			((uint32_t*) ret->tex.data)[destOff] = ColourToABGR(colour);
		}
	}

	ret->used   = true;
	ret->width  = w;
	ret->height = h;
	return ret;
}

void Backend_FreeTexture(Texture* texture) {
	C3D_TexDelete(&texture->tex);
	texture->used = false;
}

Vec2 Backend_GetTextureSize(Texture* texture) {
	return (Vec2) {texture->width, texture->height};
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

	Tex3DS_SubTexture subTex;

	subTex.top    = 1.0f - ((float) src.y) / ((float) texture->tex.height);
	subTex.left   = ((float) src.x) / ((float) texture->tex.width);
	subTex.right  = ((float) (src.x + src.w)) / ((float) texture->tex.width);
	subTex.bottom = 1.0f - ((float) (src.y + src.h)) / ((float) texture->tex.height);
	subTex.width  = src.w;
	subTex.height = src.h;

	C2D_Image img;
	img.tex    = &texture->tex;
	img.subtex = &subTex;

	// TODO: support tint
	assert(C2D_DrawImageAt(
		img, (float) dest.x, (float) dest.y, 0.0f, NULL,
		((float) dest.w) / ((float) src.w), ((float) dest.h) / ((float) src.h)
	));
}

void Backend_Begin(void) {
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C3D_FrameDrawOn(state.target);
}

void Backend_Begin2D(void) {
	C2D_Prepare();
	C2D_SceneBegin(state.target);
}

void Backend_Clear(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t colour = CompToABGR(r, g, b, 255);
	C3D_RenderTargetClear(state.target, C3D_CLEAR_ALL, colour, 0);
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
	uint32_t c = C2D_Color32(colour.r, colour.g, colour.b, colour.a);
	C2D_DrawRectangle(
		(float) rect.x, (float) rect.y, 0.0f, (float) rect.w, (float) rect.h,
		c, c, c, c
	);
}

void Backend_RenderLine(Vec2 a, Vec2 b, Colour colour) {
	uint32_t c = C2D_Color32(colour.r, colour.g, colour.b, colour.a);
	C2D_DrawLine((float) a.x, (float) a.y, c, (float) b.x, (float) b.y, c, 1.0f, 0.0f);
}

void Backend_EnableAlpha(bool enable) {
	(void) enable;
}

void Backend_InitSkybox(void) {
	
}

void Backend_FinishRender(void) {
	C2D_Flush();
	C3D_FrameEnd(0);
}

BackendViewport Backend_SaveViewport(void) {
	return (BackendViewport) {};
}

void Backend_RestoreViewport(BackendViewport viewport) {
	(void) viewport;
}

#endif
