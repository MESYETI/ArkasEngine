#include <stdio.h>
#include "file.h"
#include "util.h"
#include "safe.h"
#include "model.h"
#include "video.h"

#define EOF_ERROR(p) Error("Hit end of file '%s' too early on line %u", (p), __LINE__)

void Model_Load(Model* model, const char* path) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		perror(path);
		Error("Failed to open '%s'", path);
	}

	char magic[3];

	if (fread(magic, 1, 3, file) != 3) EOF_ERROR(path);
	if ((magic[0] != 'Z') || (magic[1] != 'K') || (magic[2] != 'M')) {
		Error("File '%s' is not a ZKM model", path);
	}

	int ver = fgetc(file);

	if (ver == EOF) EOF_ERROR(path);
	if (ver != 0) {
		Error("Model '%s' is out of date or too new", path);
	}

	if (fread(&model->verticesNum, 4, 1, file) != 1) EOF_ERROR(path);
	if (fread(&model->facesNum,    4, 1, file) != 1) EOF_ERROR(path);
	// TODO: swap these ^ on big endian

	#ifndef NDEBUG
	printf("%u vertices, %u faces\n", model->verticesNum, model->facesNum);
	#endif

	model->vertices = SafeMalloc(model->verticesNum * sizeof(*model->vertices));
	model->faces    = SafeMalloc(model->facesNum    * sizeof(*model->faces));

	for (uint32_t i = 0; i < model->verticesNum; ++i) {
		model->vertices[i].x = File_ReadFloat(file);
		model->vertices[i].y = File_ReadFloat(file);
		model->vertices[i].z = File_ReadFloat(file);
	}

	for (uint32_t i = 0; i < model->facesNum; ++i) {
		assert((model->faces[i].indices[0] = File_Read32(file)) < model->verticesNum);
		assert((model->faces[i].indices[1] = File_Read32(file)) < model->verticesNum);
		assert((model->faces[i].indices[2] = File_Read32(file)) < model->verticesNum);
		model->faces[i].colour.r           = File_Read8(file);
		model->faces[i].colour.g           = File_Read8(file);
		model->faces[i].colour.b           = File_Read8(file);
	}
}

void Model_Free(Model* model) {
	free(model->vertices);
	free(model->faces);
}

