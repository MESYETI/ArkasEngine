#include "mem.h"
#include "ramDrive.h"

typedef struct File File;

struct File {
	bool folder;

	char     name[32];
	uint8_t* contents;
	size_t   size; // if file, size of contents. if folder, number of children
	File*    children;
};

typedef struct {
	ResourceDrive parent;
	File          fs;
} RamDrive;

static void DeleteChildren(File* entry) {
	if (!entry->folder) return;

	for (size_t i = 0; i < entry->size; ++ i) {
		if (entry->children[i].folder) {
			DeleteChildren(&entry->children[i]);
		}
		else {
			free(entry->children[i].contents);
		}
	}

	free(entry->children);
	entry->children = NULL;
	entry->size     = 0;
}

static File* GetFileInDir(File* folder, const char* name, size_t len) {
	for (size_t i = 0; i < folder->size; ++ i) {
		if (
			(strlen(folder->children[i].name) == len) &&
			(strncmp(name, folder->children[i].name, len) == 0)
		) {
			return &folder->children[i];
		}
	}

	return NULL;
}

static File* GetFile(File* fs, const char* path) {
	if (path[0] == 0) {
		return fs;
	}

	File* dir = fs;

	const char* pathIt = path;
	while (true) {
		char* next = strchr(pathIt, '/');

		if (next) {
			File* entry = GetFileInDir(dir, pathIt, next - pathIt);

			if (entry && entry->folder) {
				dir = entry;
			}
			else {
				return NULL;
			}
		}
		else {
			return GetFileInDir(dir, pathIt, strlen(pathIt));
		}

		pathIt = next + 1;
	}
}

static void DriveFree(ResourceDrive* p_drive) {
	RamDrive* drive = (RamDrive*) p_drive;

	DeleteChildren(&drive->fs);
}

static bool DriveFileExists(ResourceDrive* p_drive, const char* path) {
	RamDrive* drive = (RamDrive*) p_drive;

	return GetFile(&drive->fs, path);
}

static void DrivePrintList(ResourceDrive* p_drive, const char* folder) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     entry = GetFile(&drive->fs, folder);

	if (!entry) {
		Log("Directory '%s' does not exist", folder);
		return;
	}
	else if (!entry->folder) {
		Log("Path '%s' leads to a file, not a directory", folder);
		return;
	}

	Log("Contents of: %s", entry->name);

	for (size_t i = 0; i < entry->size; ++ i) {
		Log(
			"  [%c] %s",
			entry->children[i].folder? 'D' : ' ', entry->children[i].name
		);
	}
}

static ResourceFile* DriveList(ResourceDrive* p_drive, const char* folder, size_t* sz) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     entry = GetFile(&drive->fs, folder);

	if (!entry) {
		Log("Directory '%s' does not exist", folder);
		return NULL;
	}
	else if (!entry->folder) {
		Log("Path '%s' leads to a file, not a directory", folder);
		return NULL;
	}

	*sz = entry->size;

	ResourceFile* ret = SafeMalloc(*sz * sizeof(ResourceFile));

	for (size_t i = 0; i < entry->size; ++ i) {
		ResourceFile* file = &ret[i];
		file->fullPath = SafeMalloc(
			strlen(p_drive->name) + strlen(folder) +
			strlen(entry->children[i].name) + 4
		);

		strcpy(file->fullPath, ":");
		strcat(file->fullPath, p_drive->name);
		strcat(file->fullPath, folder);
		strcat(file->fullPath, "/");
		strcat(file->fullPath, entry->children[i].name);

		file->dir = entry->children[i].folder;
	}

	return ret;
}

static Stream DriveOpen(ResourceDrive* p_drive, const char* path, bool* success) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     file  = GetFile(&drive->fs, path);

	*success = true;

	if (!file) {
		Log("File '%s' does not exist", path);
		*success = false;
		return Stream_Blank();
	}
	else if (file->folder) {
		Log("Path '%s' leads to a directory, not a file", path);
		*success = false;
		return Stream_Blank();
	}

	return Stream_Memory(file->contents, file->size, false);
}

static void* DriveReadFile(ResourceDrive* p_drive, const char* path, size_t* size) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     entry = GetFile(&drive->fs, path);

	if (!entry) {
		Log("File '%s' does not exist", path);
		return NULL;
	}
	else if (entry->folder) {
		Log("Path '%s' leads to a directory, not a file", path);
		return NULL;
	}

	*size = entry->size;
	return entry->contents;
}

static bool DriveMakeDir(ResourceDrive* p_drive, const char* path) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     entry;

	char* slash = strrchr(path, '/');
	if (slash) {
		if (slash[1] == 0) {
			Log("Empty folder name");
			return false;
		}
	
		*slash = 0;
		entry = GetFile(&drive->fs, path);
		*slash = '/';
	}
	else {
		if (path[0] == 0) {
			Log("Empty folder name");
			return false;
		}

		entry = &drive->fs;
	}

	if (!entry->folder) {
		Log("Invalid path");
		return false;
	}

	++ entry->size;
	entry->children = SafeRealloc(entry->children, entry->size * sizeof(File));

	entry->children[entry->size - 1] = (File) {
		.folder   = true,
		.name     = "",
		.contents = NULL,
		.size     = 0,
		.children = NULL
	};

	if (slash) {
		++ slash;
	}
	else {
		slash = (char*) path;
	}

	strncpy(entry->children[entry->size - 1].name, slash, sizeof(entry->name));
	return true;
}

static bool DriveWriteFile(ResourceDrive* p_drive, const char* path, void* contents, size_t size) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     entry;

	char* slash = strrchr(path, '/');
	if (slash) {
		if (slash[1] == 0) {
			Log("Empty folder name");
			return false;
		}
	
		*slash = 0;
		entry = GetFile(&drive->fs, path);
		*slash = '/';
	}
	else {
		if (path[0] == 0) {
			Log("Empty folder name");
			return false;
		}

		entry = &drive->fs;
	}

	if (!entry->folder) {
		Log("Invalid path");
		return false;
	}

	++ entry->size;
	entry->children = SafeRealloc(entry->children, entry->size * sizeof(File));

	entry->children[entry->size - 1] = (File) {
		.folder   = false,
		.name     = "",
		.contents = contents,
		.size     = size,
		.children = NULL
	};
	strncpy(entry->children[entry->size - 1].name, &slash[1], sizeof(entry->name));
	return true;
}

static bool DriveDelete(ResourceDrive* p_drive, const char* path) {
	RamDrive* drive = (RamDrive*) p_drive;
	File*     parent;

	char* slash = strrchr(path, '/');
	if (slash) {
		if (slash[1] == 0) {
			Log("Empty folder name");
			return false;
		}
	
		*slash = 0;
		parent = GetFile(&drive->fs, path);
		*slash = '/';
	}
	else {
		if (path[0] == 0) {
			Log("Empty folder name");
			return false;
		}

		parent = &drive->fs;
	}

	File* entry = GetFile(parent, slash? &slash[1] : path);

	DeleteChildren(entry);

	// now delete the entry
	if (!entry->folder) {
		free(entry->contents);
	}

	size_t index = entry - parent->children;
	memmove(
		&parent->children[index], &parent->children[index + 1],
		(parent->size - index) * sizeof(File)
	);
	-- parent->size;

	return true;
}

ResourceDrive* NewRamDrive(void) {
	RamDrive* ret = SafeMalloc(sizeof(RamDrive));

	ret->parent.free       = &DriveFree;
	ret->parent.fileExists = &DriveFileExists;
	ret->parent.printList  = &DrivePrintList;
	ret->parent.list       = &DriveList;
	ret->parent.open       = &DriveOpen;
	ret->parent.readFile   = &DriveReadFile;
	ret->parent.makeDir    = &DriveMakeDir;
	ret->parent.writeFile  = &DriveWriteFile;
	ret->parent.delete     = &DriveDelete;

	ret->fs = (File) {
		.folder   = true,
		.name     = "/",
		.contents = NULL,
		.size     = 0,
		.children = NULL
	};

	return (ResourceDrive*) ret;
}
