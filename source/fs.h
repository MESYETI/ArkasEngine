#ifndef AE_FS_H
#define AE_FS_H

#include "common.h"

bool DirExists(const char* path);
void MakeDir(const char* path, bool p); // p = ignore if already exists
bool FileExists(const char* path);
void WriteFile(const char* path, const char* contents);

#endif
