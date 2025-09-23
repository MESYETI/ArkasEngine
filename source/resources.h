#ifndef AE_RESOURCES_H
#define AE_RESOURCES_H

#include "util.h"
#include "common.h"

typedef struct ResourceDrive ResourceDrive;

struct ResourceDrive {
	char* name;

	FUNCTION_POINTER(void,  free, ResourceDrive*);
	FUNCTION_POINTER(bool,  fileExists, ResourceDrive*, const char* path);
	FUNCTION_POINTER(void,  list, ResourceDrive*, const char* folder);
	FUNCTION_POINTER(void*, readFile, ResourceDrive*, const char* path, size_t* size);
};

typedef struct {
	ResourceDrive** drives;
	size_t          drivesNum;
} ResourceManager;

extern ResourceManager resources;

void ResourceManager_Init(void);
void ResourceManager_Free(void);
bool ResourceManager_FileExists(const char* path);
void ResourceManager_List(const char* path);

#endif
