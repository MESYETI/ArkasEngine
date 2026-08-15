#ifndef AE_RESOURCES_H
#define AE_RESOURCES_H

#include "text.h"
#include "util.h"
#include "model.h"
#include "common.h"
#include "stream.h"
#include "backend.h"

enum {
	RESOURCE_TYPE_TEXTURE = 0,
	RESOURCE_TYPE_AUDIO,
	RESOURCE_TYPE_FONT,
	RESOURCE_TYPE_MODEL
};

enum {
	RESOURCE_IS_FONT = 1
};

typedef struct {
	short* data;
	size_t len;
	int    channels;
	int    sampleRate;
} AudioResource;

typedef union {
	Texture*      texture;
	AudioResource audio;
	Font          font;
	Model         model;
} ResourceContents; // C99 moment

typedef struct {
	bool   active;
	int    type;
	char*  name;
	size_t usedBy;

	ResourceContents v;
} Resource;

typedef struct {
	Resource* resources;
	size_t    capacity;
} ResourceManager;

extern ResourceManager resources;

void      Resources_Init(void);
void      Resources_Free(void);
Resource* Resources_GetRes(const char* path, uint32_t opt);
void      Resources_FreeRes(Resource* resource);

#endif
