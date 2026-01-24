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

static void DrivePrintList(ResourceDrive* p_drive, const char* folder) {
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

	closedir(dir);
}

static ResourceFile* DriveList(ResourceDrive* p_drive, const char* folder, size_t* sz) {
	FolderDrive* drive = (FolderDrive*) p_drive;

	if (strstr(folder, "..")) {
		Error("Insecure path: %s", folder);
		return NULL;
	}

	char pathAdd[4096];
	strncpy(pathAdd, drive->path, 4096);
	strncat(pathAdd, "/", 4096 - strlen(pathAdd));
	strncat(pathAdd, folder, 4096 - strlen(pathAdd));

	if (!DirExists(pathAdd)) {
		Log("Folder '%s' does not exist", folder);
		return NULL;
	}

	DIR*          dir = opendir(pathAdd);
	ResourceFile* ret = NULL;
	size_t        len = 0;

	char pathStart[4096];
	strcpy(pathStart,  ":");
	strncat(pathStart, p_drive->name, 4095);
	strncat(pathStart, folder, 4095 - strlen(pathStart));

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (
			(strcmp(entry->d_name, ".") == 0) ||
			(strcmp(entry->d_name, "..") == 0)
		) {
			continue;
		}

		++ len;

		ret = SafeRealloc(ret, len * sizeof(ResourceFile));

		ResourceFile* file = &ret[len - 1];

		file->fullPath = SafeMalloc(strlen(pathStart) + strlen(entry->d_name) + 2);
		strcpy(file->fullPath, pathStart);
		strcat(file->fullPath, "/");
		strcat(file->fullPath, entry->d_name);

		// path on disk
		char entryPath[4096];
		strncpy(entryPath, pathAdd, 4096);
		strncat(pathAdd, "/", 4096 - strlen(pathAdd));
		strncat(entryPath, entry->d_name, 4096 - strlen(entryPath));

		file->dir = IsDir(entryPath);
	}

	*sz = len;
	closedir(dir);
	return ret;
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
	ret->parent.printList  = &DrivePrintList;
	ret->parent.list       = &DriveList;
	ret->parent.readFile   = &DriveReadFile;
	ret->path              = path;

	return (ResourceDrive*) ret;
}
