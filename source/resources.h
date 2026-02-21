#ifndef AE_RESOURCES_H
#define AE_RESOURCES_H

#include "text.h"
#include "util.h"
#include "common.h"
#include "stream.h"
#include "backend.h"

typedef struct ResourceDrive ResourceDrive;

typedef struct {
	char* fullPath;
	bool  dir;
} ResourceFile;

struct ResourceDrive {
	char* name;

	FUNCTION_POINTER(void,          free, ResourceDrive*);
	FUNCTION_POINTER(bool,          fileExists, ResourceDrive*, const char* path);
	FUNCTION_POINTER(void,          printList, ResourceDrive*, const char* folder);
	FUNCTION_POINTER(ResourceFile*, list, ResourceDrive*, const char* folder, size_t* sz);
	FUNCTION_POINTER(Stream,        open, ResourceDrive*, const char* path, bool* success);

	FUNCTION_POINTER(void*, readFile, ResourceDrive*, const char* path, size_t* size);

};

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
	ResourceDrive** drives;
	size_t          drivesNum;

	Resource* resources;
	size_t    capacity;
} ResourceManager;

extern ResourceManager resources;

void          Resources_Init(void);
void          Resources_Free(void);
bool          Resources_FileExists(const char* path);
ResourceFile* Resources_List(const char* path, size_t* sz);
void          Resources_FreeFileList(ResourceFile* list, size_t sz);
void          Resources_PrintList(const char* path);
void*         Resources_ReadFile(const char* path, size_t* size);
Resource*     Resources_GetRes(const char* path, uint32_t opt);
void          Resources_FreeRes(Resource* resource);

#endif
