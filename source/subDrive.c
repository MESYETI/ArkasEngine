#include "mem.h"
#include "subDrive.h"

typedef struct {
	ResourceDrive   parent;
	ResourceDrive** drives;
	size_t          num;
} SubDrive;

static ResourceDrive* GetSubDrive(SubDrive* drive, const char* path) {
	const char* name = path;
	size_t      nameLen;

	const char* slash = strchr(path, '/');

	if (slash) {
		nameLen = slash - name;
	}
	else {
		return NULL;
	}

	// find drive
	for (size_t i = 0; i < resources.drivesNum; ++ i) {
		if (
			(strlen(drive->drives[i]->name) == nameLen) &&
			(strncmp(drive->drives[i]->name, name, nameLen) == 0)
		) {
			return drive->drives[i];
		}
	}

	return NULL;
}

static bool DriveFileExists(ResourceDrive* p_drive, const char* path) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, path);

	if (!sub) {
		Log("Invalid file path: %s", path);
		return false;
	}

	const char* drivePath = strchr(path, '/') + 1;

	return sub->fileExists(sub, drivePath);
}

static void DrivePrintList(ResourceDrive* p_drive, const char* folder) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, folder);

	if (!sub) {
		Log("Invalid file path: %s", folder);
		return;
	}

	const char* drivePath = strchr(folder, '/') + 1;

	sub->printList(sub, drivePath);
}

static ResourceFile* DriveList(ResourceDrive* p_drive, const char* folder, size_t* sz) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, folder);

	if (!sub) {
		Log("Invalid file path: %s", folder);
		return NULL;
	}

	const char* drivePath = strchr(folder, '/') + 1;

	return sub->list(sub, drivePath, sz);
}

static Stream DriveOpen(ResourceDrive* p_drive, const char* path, bool* success) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, path);

	if (!sub) {
		Log("Invalid file path: %s", path);
		*success = false;

		return Stream_Blank();
	}

	const char* drivePath = strchr(path, '/') + 1;

	return sub->open(sub, drivePath, success);
}

static void* DriveReadFile(ResourceDrive* p_drive, const char* path, size_t* size) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, path);

	if (!sub) {
		Log("Invalid file path: %s", path);
		return NULL;
	}

	const char* drivePath = strchr(path, '/') + 1;

	return sub->readFile(sub, drivePath, size);
}

static bool DriveMakeDir(ResourceDrive* p_drive, const char* path) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, path);

	if (!sub) {
		Log("Invalid file path: %s", path);
		return false;
	}

	const char* drivePath = strchr(path, '/') + 1;

	return sub->makeDir(sub, drivePath);
}

static bool DriveWriteFile(
	ResourceDrive* p_drive, const char* path, void* contents, size_t size
) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, path);

	if (!sub) {
		Log("Invalid file path: %s", path);
		return false;
	}

	const char* drivePath = strchr(path, '/') + 1;

	return sub->writeFile(sub, drivePath, contents, size);
}

static bool DriveDelete(ResourceDrive* p_drive, const char* path) {
	SubDrive* drive = (SubDrive*) p_drive;

	ResourceDrive* sub = GetSubDrive(drive, path);

	if (!sub) {
		Log("Invalid file path: %s", path);
		return false;
	}

	const char* drivePath = strchr(path, '/') + 1;

	return sub->delete(sub, drivePath);
}

ResourceDrive* NewSubDrive(ResourceDrive** drives, size_t num, bool readOnly) {
	SubDrive* ret = SafeMalloc(sizeof(SubDrive));

	ret->parent.free       = NULL;
	ret->parent.fileExists = &DriveFileExists;
	ret->parent.printList  = &DrivePrintList;
	ret->parent.list       = &DriveList;
	ret->parent.open       = &DriveOpen;
	ret->parent.readFile   = &DriveReadFile;
	ret->parent.makeDir    = readOnly? NULL : &DriveMakeDir;
	ret->parent.writeFile  = readOnly? NULL : &DriveWriteFile;
	ret->parent.delete     = readOnly? NULL : &DriveDelete;
	ret->drives            = drives;
	ret->num               = num;

	return (ResourceDrive*) ret;
}

void SubDrive_Add(ResourceDrive* subDrive, ResourceDrive* drive) {
	SubDrive* sub = (SubDrive*) subDrive;

	++ sub->num;
	sub->drives = SafeRealloc(sub->drives, sub->num * sizeof(void*));

	sub->drives[sub->num - 1] = drive;
}
