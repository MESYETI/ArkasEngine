#include <dirent.h>
#include "fs.h"
#include "mem.h"
#include "resources.h"

typedef struct {
	ResourceDrive parent;
	const char*   path;
} FolderDrive;

static bool DriveFileExists(ResourceDrive* p_drive, const char* path) {
	FolderDrive* drive = (FolderDrive*) p_drive;

	if (strstr(path, "..")) {
		Error("Insecure path: %s", path);
	}

	char pathAdd[4096];
	strncpy(pathAdd, drive->path, 4096);
	strncat(pathAdd, "/", 4096 - strlen(pathAdd));
	strncat(pathAdd, path, 4096 - strlen(pathAdd));

	return FileExists(pathAdd);
}

static void DriveList(ResourceDrive* p_drive, const char* folder) {
	FolderDrive* drive = (FolderDrive*) p_drive;

	if (strstr(folder, "..")) {
		Error("Insecure path: %s", folder);
	}

	char pathAdd[4096];
	strncpy(pathAdd, drive->path, 4096);
	strncat(pathAdd, "/", 4096 - strlen(pathAdd));
	strncat(pathAdd, folder, 4096 - strlen(pathAdd));

	if (!DirExists(pathAdd)) {
		Log("Folder '%s' does not exist", folder);
		return;
	}

	DIR* dir = opendir(pathAdd);

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
			continue;
		}

		char entryPath[4096];
		strncpy(entryPath, pathAdd, 4096);
		strncat(pathAdd, "/", 4096 - strlen(pathAdd));
		strncat(entryPath, entry->d_name, 4096 - strlen(entryPath));
		Log("  [%c] %s", IsDir(entryPath)? 'D' : ' ', entry->d_name);
	}
}

static void* DriveReadFile(ResourceDrive* p_drive, const char* path, size_t* size) {
	FolderDrive* drive = (FolderDrive*) p_drive;

	if (strstr(path, "..")) {
		Error("Insecure path: %s", path);
	}

	char pathAdd[4096];
	strncpy(pathAdd, drive->path, 4096);
	strncat(pathAdd, "/", 4096 - strlen(pathAdd));
	strncat(pathAdd, path, 4096 - strlen(pathAdd));

	return ReadFile_(pathAdd, size);
}

ResourceDrive* NewFolderDrive(const char* path) {
	FolderDrive* ret = SafeMalloc(sizeof(FolderDrive));

	ret->parent.free       = NULL;
	ret->parent.fileExists = &DriveFileExists;
	ret->parent.list       = &DriveList;
	ret->parent.readFile   = &DriveReadFile;
	ret->path              = path;

	return (ResourceDrive*) ret;
}
