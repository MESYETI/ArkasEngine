#ifndef AE_FS_H
#define AE_FS_H

#include "common.h"

#ifdef PLATFORM_PSVITA
	#define AE_LOCATION "ux0:data/ArkasEngine/"
#else
	#define AE_LOCATION "./"
#endif

bool  DirExists(const char* path);
bool  MakeDir(const char* path, bool p); // p = ignore if already exists
bool  FileExists(const char* path);
void* ReadFile_(const char* path, size_t* size);
bool  WriteFile_(const char* path, const char* contents);
bool  WriteFileBin(const char* path, void* contents, size_t len);
bool  IsDir(const char* path);

#endif
