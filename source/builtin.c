#include <assert.h>
#include <string.h>
#include "mem.h"
#include "builtin.h"

typedef struct {
	const char*    name;
	const uint8_t* data;
	const size_t   len;
} File;

#include "../builtin/font.c"
#include "../builtin/no_texture.c"

static const File files[2] = {
	{"font.png",       font,      sizeof(font)},
	{"no_texture.png", noTexture, sizeof(noTexture)}
};

static bool DriveFileExists(VFS_Drive* drive, const char* path) {
	(void) drive;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		if (strcmp(files[i].name, path) == 0) return true;
	}

	return false;
}

static void DrivePrintList(VFS_Drive* drive, const char* folder) {
	(void) drive;
	(void) folder;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		Log("  [ ] %s", files[i].name);
	}
}

static VFS_File* DriveList(VFS_Drive* drive, const char* folder, size_t* sz) {
	*sz = sizeof(files) / sizeof(File);

	VFS_File* ret = SafeMalloc(*sz * sizeof(VFS_File));

	for (size_t i = 0; i < *sz; ++ i) {
		ret[i].fullPath = SafeMalloc(
			strlen(drive->name) + strlen(folder) + strlen(files[i].name) + 4
		);

		strcpy(ret[i].fullPath, ":");
		strcat(ret[i].fullPath, drive->name);
		strcat(ret[i].fullPath, folder);
		strcat(ret[i].fullPath, "/");
		strcat(ret[i].fullPath, files[i].name);

		ret[i].dir = false;
	}

	return ret;
}

static Stream DriveOpen(VFS_Drive* drive, const char* path, bool* success, bool write) {
	(void) drive;

	if (write) {
		*success = false;
		return Stream_Blank();
	}

	*success = false;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		if (strcmp(files[i].name, path) == 0) {
			*success = true;
			return Stream_Memory((void*) files[i].data, files[i].len, false);
		}
	}

	return Stream_Blank();
}

static void* DriveReadFile(VFS_Drive* drive, const char* path, size_t* size) {
	(void) drive;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		if (strcmp(files[i].name, path) == 0) {
			*size = files[i].len;

			void* ret = SafeMalloc(files[i].len);
			memcpy(ret, files[i].data, files[i].len);
			return ret;
		}
	}

	return NULL;
}

VFS_Drive* BuiltIn_GetDrive(void) {
	VFS_Drive* ret = SafeMalloc(sizeof(VFS_Drive));
	// expect caller to write to name

	ret->free       = NULL;
	ret->fileExists = &DriveFileExists;
	ret->printList  = &DrivePrintList;
	ret->list       = &DriveList;
	ret->open       = &DriveOpen;
	ret->readFile   = &DriveReadFile;
	ret->makeDir    = NULL;
	ret->writeFile  = NULL;
	ret->delete     = NULL;
	return ret;
}
