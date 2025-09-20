#include <errno.h>
#include <assert.h>
#include <dirent.h>
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
