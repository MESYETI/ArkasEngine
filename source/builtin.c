#include <assert.h>
#include "mem.h"
#include "builtin.h"

typedef struct {
	const char* name;
	uint8_t*    data;
	size_t      len;
} File;

#include "../builtin/font.c"
#include "../builtin/no_texture.c"

static File files[2] = {
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

static void DriveList(ResourceDrive* drive, const char* folder) {
	(void) drive;
	(void) folder;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		Log("  [ ] %s", files[i].name);
	}
}

static void* DriveReadFile(ResourceDrive* drive, const char* path, size_t* size) {
	(void) drive;

	for (size_t i = 0; i < sizeof(files) / sizeof(File); ++ i) {
		if (strcmp(files[i].name, path) == 0) {
			*size = files[i].len;
			return (void*) files[i].data;
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
	ret->readFile   = &DriveReadFile;
	return ret;
}
