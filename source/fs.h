#ifndef AE_FS_H
#define AE_FS_H

#include "common.h"

#ifdef PLATFORM_PSVITA
	#define AE_LOCATION "ux0:data/ArkasEngine/"
#else
	#define AE_LOCATION "./"
#endif

bool  DirExists(const char* path);
void  MakeDir(const char* path, bool p); // p = ignore if already exists
bool  FileExists(const char* path);
void* ReadFile_(const char* path, size_t* size);
void  WriteFile_(const char* path, const char* contents);
bool  IsDir(const char* path);

#endif
