#ifndef AE_VFS_H
#define AE_VFS_H

#include "util.h"
#include "stream.h"

typedef struct VFS_Drive VFS_Drive;

typedef struct {
	char* fullPath;
	bool  dir;
} VFS_File;

struct VFS_Drive {
	char* name;

	FUNCTION_POINTER(void,      free,       VFS_Drive*);
	FUNCTION_POINTER(bool,      fileExists, VFS_Drive*, const char* path);
	FUNCTION_POINTER(void,      printList,  VFS_Drive*, const char* folder);
	FUNCTION_POINTER(VFS_File*, list,       VFS_Drive*, const char* folder, size_t* sz);
	FUNCTION_POINTER(Stream,    open,       VFS_Drive*, const char* path, bool* success, bool write);
	FUNCTION_POINTER(void*,     readFile,   VFS_Drive*, const char* path, size_t* size);

	// write functions
	FUNCTION_POINTER(bool, makeDir, VFS_Drive*, const char* path);
	FUNCTION_POINTER(
		bool, writeFile, VFS_Drive*, const char* path, void* contents,
		size_t size
	);
	FUNCTION_POINTER(bool, delete, VFS_Drive*, const char* path);
};

typedef struct {
	VFS_Drive** drives;
	size_t      drivesNum;
} VFS;

extern VFS vfs;

void      VFS_Init(void);
void      VFS_Free(void);
bool      VFS_DriveExists(const char* name);
bool      VFS_AddDrive(VFS_Drive* drive, const char* name);
bool      VFS_DeleteDrive(const char* name);
bool      VFS_FileExists(const char* path);
VFS_File* VFS_List(const char* path, size_t* sz);
void      VFS_FreeFileList(VFS_File* list, size_t sz);
void      VFS_PrintList(const char* path);
void*     VFS_ReadFile(const char* path, size_t* size);
bool      VFS_MakeDir(const char* path);
bool      VFS_Delete(const char* path);
bool      VFS_WriteFile(const char* path, void* contents, size_t size);
Stream    VFS_Open(const char* path, bool* success, bool write);

#endif
