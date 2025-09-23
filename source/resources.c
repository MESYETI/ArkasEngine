#include <dirent.h>
#include <string.h>
#include "ark.h"
#include "util.h"
#include "safe.h"
#include "resources.h"

ResourceManager resources;

void ResourceManager_Init(void) {
	DIR* dir = opendir("game");

	if (dir == NULL) {
		Error("Failed to open directory 'game'");
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		char* path = strrchr(entry->d_name, '.');

		if (path == NULL) continue;
		if (strcmp(path, ".ark") != 0) {
			continue;
		}

		char* concatPath = ConcatString("game/", entry->d_name);
		ResourceDrive* drive = Ark_CreateResourceDrive(concatPath);
		free(concatPath);

		if (drive == NULL) {
			Log("Failed to load archive '%s'", entry->d_name);
			continue;
		}
		else {
			resources.drives = SafeRealloc(
				resources.drives, (resources.drivesNum + 1) * sizeof(ResourceDrive*)
			);
			resources.drives[resources.drivesNum] = drive;
			++ resources.drivesNum;
		}

		drive->name = NewString(entry->d_name);
		*strrchr(drive->name, '.') = 0;
		drive->name = SafeRealloc(drive->name, strlen(drive->name) + 1);
	}

	Log("%d resource drives mounted", resources.drivesNum);
}

void ResourceManager_Free(void) {
	for (size_t i = 0; i < resources.drivesNum; ++ i) {
		resources.drives[i]->free(resources.drives[i]);
		free(resources.drives[i]);
	}
	free(resources.drives);

	resources.drives    = NULL;
	resources.drivesNum = 0;
}

static ResourceDrive* GetDrive(const char* path) {
	if (path[0] != ':') return NULL;

	const char* name = &path[1];
	size_t      nameLen;

	const char* slash = strchr(path, '/');

	if (slash) {
		nameLen = slash - name;
	}
	else {
		nameLen = strlen(name);
	}

	// find drive
	for (size_t i = 0; i < resources.drivesNum; ++ i) {
		if (
			(strlen(resources.drives[i]->name) == nameLen) &&
			(strncmp(resources.drives[i]->name, name, nameLen) == 0)
		) {
			return resources.drives[i];
		}
	}

	return NULL;
}

bool ResourceManager_FileExists(const char* path) {
	ResourceDrive* drive = GetDrive(path);

	if (!drive) {
		Log("ResourceManager_FileExists: Automatic drive selection not implemented");
		return false;
	}

	const char* drivePath = strchr(path, '/');

	if (!drivePath) {
		Log("Invalid file path: '%s'", path);
		return false;
	}

	return drive->fileExists(drive, drivePath + 1);
}

void ResourceManager_List(const char* path) {
	if (!path) {
		Log("Mounted resource drives:");

		for (size_t i = 0; i < resources.drivesNum; ++ i) {
			Log("  :%s", resources.drives[i]->name);
		}
	}
	else {
		ResourceDrive* drive = GetDrive(path);

		if (!drive) {
			Log("Invalid drive");
			return;
		}

		const char* drivePath = strchr(path, '/');

		if (drivePath == NULL) {
			drivePath = "";
		}
		else {
			++ drivePath;
		}

		drive->list(drive, drivePath);
	}
}
