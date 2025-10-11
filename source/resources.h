#ifndef AE_RESOURCES_H
#define AE_RESOURCES_H

#include "util.h"
#include "common.h"
#include "backend.h"

typedef struct ResourceDrive ResourceDrive;

struct ResourceDrive {
	char* name;

	FUNCTION_POINTER(void,  free, ResourceDrive*);
	FUNCTION_POINTER(bool,  fileExists, ResourceDrive*, const char* path);
	FUNCTION_POINTER(void,  list, ResourceDrive*, const char* folder);
	FUNCTION_POINTER(void*, readFile, ResourceDrive*, const char* path, size_t* size);
};

enum {
	RESOURCE_TYPE_TEXTURE = 0,
	RESOURCE_TYPE_AUDIO
};

typedef union {
	Texture*      texture;
	AudioResource audio;
} ResourceContents; // C99 moment

typedef struct {
	bool   active;
	int    type;
	char*  name;
	size_t usedBy;

	ResourceContents v;
} Resource;

typedef struct {
	ResourceDrive** drives;
	size_t          drivesNum;

	Resource* resources;
	size_t    capacity;
} ResourceManager;

extern ResourceManager resources;

void      Resources_Init(void);
void      Resources_Free(void);
bool      Resources_FileExists(const char* path);
void      Resources_List(const char* path);
void*     Resources_ReadFile(const char* path, size_t* size);
Resource* Resources_GetRes(const char* path);
void      Resources_FreeRes(Resource* resource);

#endif
