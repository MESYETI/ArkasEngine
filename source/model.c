#include <stdio.h>
#include "util.h"
#include "mem.h"
#include "model.h"
#include "video.h"
#include "stream.h"

#define EOF_ERROR(p) Error("Hit end of file '%s' too early on line %u", (p), __LINE__)

void Model_Load(Model* model, const char* path) {
	FILE* fd = fopen(path, "rb");

	if (fd == NULL) {
		perror(path);
		Error("Failed to open '%s'", path);
	}

	Stream file = Stream_File(fd, true);

	char magic[3];

	// if (fread(magic, 1, 3, file) != 3) EOF_ERROR(path);
	if (Stream_Read(&file, 3, magic) != 3) EOF_ERROR(path);
	if ((magic[0] != 'Z') || (magic[1] != 'K') || (magic[2] != 'M')) {
		Error("File '%s' is not a ZKM model", path);
	}

	// int ver = fgetc(file);
	uint8_t ver;
	if (Stream_Read(&file, 1, &ver) != 1) EOF_ERROR(path);

	if (ver != 0) {
		Error("Model '%s' is out of date or too new", path);
	}

	if (Stream_Read(&file, 4, &model->verticesNum) != 4) EOF_ERROR(path);
	if (Stream_Read(&file, 4, &model->facesNum) != 4) EOF_ERROR(path);
	// TODO: swap these ^ on big endian

	#ifndef NDEBUG
	printf("%u vertices, %u faces\n", model->verticesNum, model->facesNum);
	#endif

	model->vertices = SafeMalloc(model->verticesNum * sizeof(*model->vertices));
	model->faces    = SafeMalloc(model->facesNum    * sizeof(*model->faces));

	for (uint32_t i = 0; i < model->verticesNum; ++i) {
		model->vertices[i].x = Stream_ReadFloat(&file);
		model->vertices[i].y = Stream_ReadFloat(&file);
		model->vertices[i].z = Stream_ReadFloat(&file);
	}

	for (uint32_t i = 0; i < model->facesNum; ++i) {
		assert((model->faces[i].indices[0] = Stream_Read32(&file)) < model->verticesNum);
		assert((model->faces[i].indices[1] = Stream_Read32(&file)) < model->verticesNum);
		assert((model->faces[i].indices[2] = Stream_Read32(&file)) < model->verticesNum);
		model->faces[i].colour.r           = Stream_Read8(&file);
		model->faces[i].colour.g           = Stream_Read8(&file);
		model->faces[i].colour.b           = Stream_Read8(&file);
	}

    Stream_Close(&file);
}

void Model_Free(Model* model) {
	free(model->vertices);
	free(model->faces);
}
