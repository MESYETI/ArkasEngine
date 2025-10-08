#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include "fs.h"

#ifdef PLATFORM_WINDOWS
	bool CreateDirectoryA(const char* path, void* security);
#else
	#include <sys/stat.h>
#endif

bool DirExists(const char* path) {
	DIR* dir = opendir(path);

	if (dir) {
		closedir(dir);
		return true;
	}
	else {
		return false;
	}
}

void MakeDir(const char* path, bool p) {
	if (DirExists(path)) {
		if (p) return;
		else   assert(0);
	}

	#ifdef PLATFORM_WINDOWS
		CreateDirectoryA(path, NULL);
	#else
		mkdir(path, 0700);
	#endif
}

bool FileExists(const char* path) {
	FILE* file = fopen(path, "r");

	if (file) {
		fclose(file);
		return true;
	}

	return false;
}

void WriteFile(const char* path, const char* contents) {
	FILE* file = fopen(path, "w");

	if (file == NULL) {
		assert(0);
	}

	size_t len = strlen(contents);
	assert(fwrite(contents, 1, len, file) == len);
}
