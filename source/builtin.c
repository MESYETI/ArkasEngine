#include <assert.h>
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

static bool DriveFileExists(ResourceDrive* drive, const char* path) {
	(void) drive;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		if (strcmp(files[i].name, path) == 0) return true;
	}

	return false;
}

static void DrivePrintList(ResourceDrive* drive, const char* folder) {
	(void) drive;
	(void) folder;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		Log("  [ ] %s", files[i].name);
	}
}

static ResourceFile* DriveList(ResourceDrive* drive, const char* folder, size_t* sz) {
	*sz = sizeof(files) / sizeof(File);

	ResourceFile* ret = SafeMalloc(*sz * sizeof(ResourceFile));

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

static void* DriveReadFile(ResourceDrive* drive, const char* path, size_t* size) {
	(void) drive;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		if (strcmp(files[i].name, path) == 0) {
			*size = files[i].len;

			void* ret = SafeMalloc(files[i].len);
			memcpy(ret, files[i].data, files[i].len);
			return ret;
		}
	}

	assert(0);
}

ResourceDrive* BuiltIn_GetDrive(void) {
	ResourceDrive* ret = SafeMalloc(sizeof(ResourceDrive));
	// expect caller to write to name

	ret->free       = NULL;
	ret->fileExists = &DriveFileExists;
	ret->list       = &DriveList;
	ret->printList  = &DrivePrintList;
	ret->readFile   = &DriveReadFile;
	return ret;
}
