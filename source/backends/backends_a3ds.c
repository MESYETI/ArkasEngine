#include "../util.h"
#include "../camera.h"
#include "../backend.h"

#ifdef AE_BACKEND_CITRO3D
#include <citro2d.h>
#include "citro3d.h"
#include "vshader_shbin.h"

typedef struct {
	Texture         textures[64];
	int             uniProjection;
	int             uniView;
	C3D_Mtx         projMatrix;
	C3D_Mtx         viewMatrix;
	shaderProgram_s program;
	DVLB_s*         dvlb;
	bool            done2D;

	C3D_RenderTarget* target;
	int               targetW;
	int               targetH;
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

static void CalcProjMatrix(void) {
	// TODO: iod = slider/3
	Mtx_PerspStereoTilt(
		&state.projMatrix, C3D_AngleFromDegrees(70.0f),
		((float) video.windows[0].width) / ((float) video.windows[0].height),
		/* near */ 0.1f, /* far */ 1000.0f, /* iod */ 0.0f,
		/* screen */ 2.0f, false
	);
}

static void CalcViewMatrix(void) {
	Mtx_Identity(&state.viewMatrix);
	// Mtx_Translate(&state.viewMatrix, -camera.pos.x, -camera.pos.y, -camera.pos.z, true);
	// Mtx_RotateX(&state.viewMatrix, C3D_AngleFromDegrees(camera.pitch), false);
	// Mtx_RotateY(&state.viewMatrix, C3D_AngleFromDegrees(camera.yaw), false);
	// Mtx_RotateZ(&state.viewMatrix, C3D_AngleFromDegrees(camera.roll), false);
	Mtx_Scale(&state.viewMatrix, 1.0f, 1.0f, 1.0f);
}

void Backend_Init(bool beforeWindow) {
	if (beforeWindow) return;

	state.dvlb = DVLB_ParseFile((uint32_t*) vshader_shbin, vshader_shbin_size);

	shaderProgramInit(&state.program);
	shaderProgramSetVsh(&state.program, &state.dvlb->DVLE[0]);
	C3D_BindProgram(&state.program);

	state.uniProjection = shaderInstanceGetUniformLocation(
		state.program.vertexShader, "projection"
	);
	state.uniView = shaderInstanceGetUniformLocation(
		state.program.vertexShader, "modelView"
	);

	C3D_AttrInfo* attr = C3D_GetAttrInfo();
	AttrInfo_Init(attr);
	AttrInfo_AddLoader(attr, 0, GPU_FLOAT, 3); // v0 = position
	AttrInfo_AddLoader(attr, 1, GPU_FLOAT, 3); // v1 = colour

	CalcProjMatrix();

	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

	C3D_CullFace(GPU_CULL_NONE);
}

void Backend_Free(void) {
	shaderProgramFree(&state.program);
	DVLB_Free(state.dvlb);
}

void Backend_SetTarget(Window* window) {
	state.target  = window->target;
	state.targetW = window->width;
	state.targetH = window->height;
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
		Log("Failed to load texture");
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
	puts("hi im drawing triangles");
	C3D_ImmDrawBegin(GPU_TRIANGLES);
		C3D_ImmSendAttrib(200.0f, 200.0f, 0.5f, 0.0f); // v0=position
		C3D_ImmSendAttrib(1.0f, 0.0f, 0.0f, 1.0f);     // v1=color

		C3D_ImmSendAttrib(100.0f, 40.0f, 0.5f, 0.0f);
		C3D_ImmSendAttrib(0.0f, 1.0f, 0.0f, 1.0f);

		C3D_ImmSendAttrib(300.0f, 40.0f, 0.5f, 0.0f);
		C3D_ImmSendAttrib(0.0f, 0.0f, 1.0f, 1.0f);
	C3D_ImmDrawEnd();
}

void Backend_OnMapLoad(void) {

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
		dest = (Rect) {0, 0, state.targetW, state.targetH};
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
	CalcProjMatrix();
	CalcViewMatrix();

	// update uniforms
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, state.uniProjection, &state.projMatrix);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, state.uniView,       &state.viewMatrix);

	state.done2D = false;
}

void Backend_Begin2D(void) {
	C2D_Prepare();
	C2D_SceneBegin(state.target);

	state.done2D = true;
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
	if (state.done2D) {
		C2D_Flush();
	}
	C3D_FrameEnd(0);
}

BackendViewport Backend_SaveViewport(void) {
	return (BackendViewport) {};
}

void Backend_RestoreViewport(BackendViewport viewport) {
	(void) viewport;
}

#endif
