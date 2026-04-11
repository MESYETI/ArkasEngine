#ifndef AE_MODEL_H
#define AE_MODEL_H

#include "types.h"
#include "video.h"
#include "stream.h"
#include "texture.h"

typedef struct {
	uint32_t indices[3];
	uint32_t uv[3];
	FVec3    normal[3];
	uint32_t texture;
} ModelFace;

typedef struct Model {
	FVec3*     vertices;
	ModelFace* faces;
	FVec2*     uv;
	Texture**  textures;
	uint32_t   verticesNum;
	uint32_t   facesNum;
	uint32_t   uvNum;
	uint32_t   texNum;
} Model;

typedef struct ModelRenderOpt {
	float scale;
	FVec3 pos;
	float rot;
} ModelRenderOpt;

void Model_Load(Model* model, Stream* file, const char* path);
void Model_Free(Model* model);

#endif
