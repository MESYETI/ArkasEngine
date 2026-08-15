#include <dirent.h>
#include <string.h>
#include "fs.h"
#include "ark.h"
#include "mem.h"
#include "vfs.h"
#include "builtin.h"
#include "folderDrive.h"

VFS vfs;

void VFS_Init(void) {
	DIR* dir = opendir(AE_LOCATION "game");

	if (dir == NULL) {
		Error("Failed to open directory 'game'");
	}

	vfs.drives          = SafeMalloc(sizeof(VFS_Drive*) * 5);
	vfs.drivesNum       = 5;
	vfs.drives[0]       = BuiltIn_GetDrive();
	vfs.drives[0]->name = NewString("builtin");
	vfs.drives[1]       = NewFolderDrive(AE_LOCATION "game/extra");
	vfs.drives[1]->name = NewString("extra");
	vfs.drives[2]       = NewFolderDrive(AE_LOCATION "maps");
	vfs.drives[2]->name = NewString("maps");
	vfs.drives[3]       = NewFolderDrive(AE_LOCATION "screenshots");
	vfs.drives[3]->name = NewString("screenshots");
	vfs.drives[4]       = NewFolderDrive(AE_LOCATION "game/projects");
	vfs.drives[4]->name = NewString("projects");

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		char* path = strrchr(entry->d_name, '.');

		if (path == NULL) continue;
		if (strcmp(path, ".ark") != 0) {
			continue;
		}

		char* concatPath = ConcatString(AE_LOCATION "game/", entry->d_name);
		FILE* file       = fopen(concatPath, "rb");

		if (!file) {
			Log("Failed to load '%s'", concatPath);
			continue;
		}

		VFS_Drive* drive = Ark_CreateDrive(Stream_ToHeap(Stream_File(file, true)), true);
		free(concatPath);

		if (drive == NULL) {
			Log("Failed to load archive '%s'", entry->d_name);
			continue;
		}
		else {
			vfs.drives = SafeRealloc(
				vfs.drives, (vfs.drivesNum + 1) * sizeof(VFS_Drive*)
			);
			vfs.drives[vfs.drivesNum] = drive;
			++ vfs.drivesNum;
		}

		drive->name = NewString(entry->d_name);
		*strrchr(drive->name, '.') = 0;
		drive->name = SafeRealloc(drive->name, strlen(drive->name) + 1);
	}

	closedir(dir);

	Log("%d VFS drives mounted", vfs.drivesNum);
}

void VFS_Free(void) {
	for (size_t i = 0; i < vfs.drivesNum; ++ i) {
		if (vfs.drives[i]->free) {
			vfs.drives[i]->free(vfs.drives[i]);
		}

		free(vfs.drives[i]->name);
		free(vfs.drives[i]);
	}
	free(vfs.drives);

	vfs.drives    = NULL;
	vfs.drivesNum = 0;
}

static VFS_Drive* GetDrive(const char* path) {
	const char* name = path;
	size_t      nameLen;

	const char* colon = strchr(path, ':');

	if (colon) {
		nameLen = colon - name;
	}
	else {
		return NULL;
	}

	// find drive
	for (size_t i = 0; i < vfs.drivesNum; ++ i) {
		if (
			(strlen(vfs.drives[i]->name) == nameLen) &&
			(strncmp(vfs.drives[i]->name, name, nameLen) == 0)
		) {
			return vfs.drives[i];
		}
	}

	return NULL;
}

bool VFS_DriveExists(const char* name) {
	return GetDrive(name)? true : false;
}

bool VFS_AddDrive(VFS_Drive* drive, const char* name) {
	if (VFS_DriveExists(name)) {
		Log("Error: Drive '%s' already exists", name);
		return false;
	}

	++ vfs.drivesNum;
	vfs.drives = SafeRealloc(vfs.drives, vfs.drivesNum * sizeof(void*));

	VFS_Drive** newDrive = &vfs.drives[vfs.drivesNum - 1];

	*newDrive         = drive;
	(*newDrive)->name = NewString(name);
	Log("New drive: %s:", name);
	return true;
}

bool VFS_DeleteDrive(const char* name) {
	for (size_t i = 0; i < vfs.drivesNum; ++ i) {
		VFS_Drive* drive = vfs.drives[i];

		if (strcmp(drive->name, name) == 0) {
			if (drive->free) {
				drive->free(drive);
			}
			free(drive);

			memmove(
				&vfs.drives[i], &vfs.drives[i + 1], (vfs.drivesNum - 1) * sizeof(void*)
			);
			-- vfs.drivesNum;
			return true;
		}
	}

	return false;
}

bool VFS_FileExists(const char* path) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Resources_FileExists: Automatic drive selection not implemented");
		return false;
	}

	const char* drivePath = strchr(path, ':');

	if (!drivePath) {
		Log("Invalid file path: '%s'", path);
		return false;
	}

	return drive->fileExists(drive, drivePath + 1);
}

VFS_File* VFS_List(const char* path, size_t* sz) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive: %s", path);
		return NULL;
	}

	const char* drivePath = strchr(path, ':');

	if (drivePath == NULL) {
		Log("Invalid file path: '%s'", path);
		return NULL;
	}
	else {
		++ drivePath;
	}

	return drive->list(drive, drivePath, sz);
}

void VFS_FreeFileList(VFS_File* list, size_t size) {
	for (size_t i = 0; i < size; ++ i) {
		free(list[i].fullPath);
	}

	free(list);
}

void VFS_PrintList(const char* path) {
	if (!path) {
		Log("Mounted resource drives:");

		for (size_t i = 0; i < vfs.drivesNum; ++ i) {
			Log("  %s:", vfs.drives[i]->name);
		}
	}
	else {
		VFS_Drive* drive = GetDrive(path);

		if (!drive) {
			Log("Invalid drive");
			return;
		}

		const char* drivePath = strchr(path, ':');

		if (drivePath == NULL) {
			Log("Invalid file path: '%s'", path);
			return;
		}
		else {
			++ drivePath;
		}

		drive->printList(drive, drivePath);
	}
}

void* VFS_ReadFile(const char* path, size_t* size) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive");
		return NULL;
	}

	const char* drivePath = strchr(path, ':');

	if (drivePath == NULL) {
		Log("Invalid file path: '%s'", path);
		return NULL;
	}
	else {
		++ drivePath;
	}

	return drive->readFile(drive, drivePath, size);
}

bool VFS_MakeDir(const char* path) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive");
		return NULL;
	}

	if (!drive->makeDir) {
		Log("Operation not available");
	}

	const char* drivePath = strchr(path, ':');

	if (drivePath == NULL) {
		Log("Invalid file path: '%s'", path);
		return NULL;
	}
	else {
		++ drivePath;
	}

	return drive->makeDir(drive, drivePath);
}

bool VFS_Delete(const char* path) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive");
		return false;
	}

	if (!drive->delete) {
		Log("Operation not available");
		return false;
	}

	const char* drivePath = strchr(path, ':');

	if (drivePath == NULL) {
		Log("Invalid file path: '%s'", path);
		return false;
	}
	else {
		++ drivePath;
	}

	return drive->delete(drive, drivePath);
}

bool VFS_WriteFile(const char* path, void* contents, size_t size) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive");
		return false;
	}

	if (!drive->writeFile) {
		Log("Operation not available");
		return false;
	}

	const char* drivePath = strchr(path, ':');

	if (drivePath == NULL) {
		Log("Invalid file path: '%s'", path);
		return false;
	}
	else {
		++ drivePath;
	}

	return drive->writeFile(drive, drivePath, contents, size);
}

Stream VFS_Open(const char* path, bool* success, bool write) {
	VFS_Drive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive");
		*success = false;

		return Stream_Blank();
	}

	const char* drivePath = strchr(path, ':');

	if (drivePath == NULL) {
		Log("Invalid file path: '%s'", path);
		*success = false;
		return Stream_Blank();
	}
	else {
		++ drivePath;
	}

	return drive->open(drive, drivePath, success, write);
}
