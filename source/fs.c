#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include "fs.h"
#include "mem.h"

#ifdef PLATFORM_WINDOWS
	#include <windows.h>
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

void* ReadFile_(const char* path, size_t* size) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void* res = SafeMalloc(*size + 1);
	fread(res, 1, *size, file);
	fclose(file);

	((uint8_t*) res)[*size] = 0;
	return res;
}

void WriteFile_(const char* path, const char* contents) {
	FILE* file = fopen(path, "w");

	if (file == NULL) {
		assert(0);
	}

	size_t len = strlen(contents);
	assert(fwrite(contents, 1, len, file) == len);
	fclose(file);
}

bool IsDir(const char* path) {
	#ifdef PLATFORM_WINDOWS
		return (GetFileAttributesA(path) & 0x00000010)? true : false;
	#else
		struct stat statVal;

		if (stat(path, &statVal) != 0) {
			return false;
		}

		return (statVal.st_mode & 0040000)? true : false;
	#endif
}
