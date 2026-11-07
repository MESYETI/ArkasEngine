#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include "fs.h"
#include "log.h"
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

bool MakeDir(const char* path, bool p) {
	if (DirExists(path)) {
		if (p) return true;
		else   return false;
	}

	#ifdef PLATFORM_WINDOWS
		return CreateDirectoryA(path, NULL);
	#else
		return mkdir(path, 0700) == 0;
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

// TODO: check that callers are error checking this
// this function used to throw fatal errors
bool WriteFile_(const char* path, const char* contents) {
	FILE* file = fopen(path, "w");

	if (file == NULL) {
		Log("WriteFile_: Failed to open file");
		return false;
	}

	size_t len = strlen(contents);

	if (fwrite(contents, 1, len, file) != len) {
		Log("WriteFile_: Failed to write %lld bytes to file", (long long int) len);
		fclose(file);
		return false;
	}

	fclose(file);

	return true;
}

bool WriteFileBin(const char* path, void* contents, size_t len) {
	FILE* file = fopen(path, "wb");

	if (file == NULL) {
		Log("WriteFileBin: Failed to open file");
		return false;
	}

	if (fwrite(contents, 1, len, file) != len) {
		Log("WriteFileBin: Failed to write %lld bytes to file", (long long int) len);
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
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
