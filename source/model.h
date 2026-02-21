#ifndef AE_MODEL_H
#define AE_MODEL_H

#include "types.h"
#include "video.h"
#include "stream.h"

typedef struct {
	uint32_t indices[3];
	Colour   colour;
} ModelFace;

typedef struct {
	FVec3*     vertices;
	ModelFace* faces;
	uint32_t   verticesNum;
	uint32_t   facesNum;
} Model;

typedef struct {
	float scale;
	FVec3 pos;
} ModelRenderOpt;

void Model_Load(Model* model, Stream* file, const char* path);
void Model_Free(Model* model);

#endif
