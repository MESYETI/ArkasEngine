#include <string.h>
#include "ark.h"
#include "file.h"
#include "util.h"
#include "mem.h"
#include "resources.h"

ErrorRet Ark_InitReader(ArchiveReader* reader, const char* path) {
	reader->file = fopen(path, "rb");

	if (!reader->file) {
		Log("Failed to open archive '%s'", path);
		return Error_Fail("Failed to open archive");
	}

	return Error_Success();
}

static void FreeDir(ArkEntry* dir) {
	if (dir->folder) {
		for (size_t i = 0; i < dir->folderSize; ++ i) {
			FreeDir(&dir->folderContents[i]);
		}

		free(dir->folderContents);
	}
}

void Ark_FreeReader(ArchiveReader* reader) {
	if (reader->file) {
		fclose(reader->file);
	}

	FreeDir(&reader->root);
	free(reader->strings);
}

ArkEntry ReadEntry(ArchiveReader* reader, ErrorRet* error) {
	ArkEntry entry;

	entry.folder = File_Read8(reader->file) != 0;
	entry.size   = File_Read32(reader->file);

	uint32_t offset = File_Read32(reader->file);
	if (offset > reader->stringsLen) {
		Log("SECURITY ALERT: Out of bounds string table offset in archive");
		*error = Error_Fail("Out of bounds string table offset");
		return entry;
	}

	entry.name = &reader->strings[offset];

	long contentsOffset = ftell(reader->file);
	if (contentsOffset < 0) {
		assert(0);
	}

	entry.contentsOffset = (size_t) contentsOffset;

	if (entry.folder) {
		size_t length = File_Read32(reader->file);
		entry.folderContents = SafeMalloc(length * sizeof(ArkEntry));
		entry.folderSize     = length;

		for (size_t i = 0; i < length; ++ i) {
			ErrorRet error2;
			entry.folderContents[i] = ReadEntry(reader, &error2);

			if (!error2.success) {
				*error = error2;
				return entry;
			}
		}
	}
	else {
		fseek(reader->file, (long) entry.size, SEEK_CUR);
	}

	*error = Error_Success();
	return entry;
}

ErrorRet Ark_Read(ArchiveReader* reader) {
	reader->ver = File_Read16(reader->file);
	File_Read8(reader->file); // unused

	reader->stringsLen = File_Read32(reader->file);
	File_Read32(reader->file); // random number

	if (reader->stringsLen == 0xFFFFFFFF) {
		return Error_Fail("String table too big");
	}

	// read strings
	reader->strings = SafeMalloc(reader->stringsLen + 1);

	reader->strings[reader->stringsLen] = 0;
	
	size_t actualLen = fread(reader->strings, 1, reader->stringsLen, reader->file);
	if (actualLen != reader->stringsLen) {
		Log("Unexpected EOF in archive string table");
		return Error_Fail("Unexpected EOF");
	}

	// read file entries
	ErrorRet error;
	reader->root      = ReadEntry(reader, &error);
	reader->root.name = "";

	return error;
}

void* Ark_ReadFile(ArchiveReader* reader, ArkEntry* entry) {
	fseek(reader->file, (long) entry->contentsOffset, SEEK_SET);
	void* ret = SafeMalloc(entry->size);

	assert(fread(ret, 1, entry->size, reader->file) == entry->size);
	return ret;
}

typedef struct {
	ResourceDrive parent;
	ArchiveReader reader;
} ArkDrive;

static void FreeDrive(ResourceDrive* p_drive) {
	ArkDrive* drive = (ArkDrive*) p_drive;
	Ark_FreeReader(&drive->reader);
}

static ArkEntry* GetEntryInDir(ArkEntry* folder, const char* name, size_t len) {
	for (size_t i = 0; i < folder->folderSize; ++ i) {
		if (
			(strlen(folder->folderContents[i].name) == len) &&
			(strncmp(name, folder->folderContents[i].name, len) == 0)
		) {
			return &folder->folderContents[i];
		}
	}

	return NULL;
}

static ArkEntry* GetEntry(ArchiveReader* reader, const char* path) {
	if (path[0] == 0) {
		return &reader->root;
	}

	ArkEntry* dir = &reader->root;

	const char* pathIt = path;
	while (true) {
		char* next = strchr(pathIt, '/');

		if (next) {
			ArkEntry* entry = GetEntryInDir(dir, pathIt, next - pathIt);

			if (entry && entry->folder) {
				dir = entry;
			}
			else {
				return NULL;
			}
		}
		else {
			return GetEntryInDir(dir, pathIt, strlen(pathIt));
		}

		pathIt = next + 1;
	}
}

static bool DriveFileExists(ResourceDrive* p_drive, const char* path) {
	ArkDrive* drive = (ArkDrive*) p_drive;

	return GetEntry(&drive->reader, path);
}

static void DriveList(ResourceDrive* p_drive, const char* folder) {
	ArkDrive* drive = (ArkDrive*) p_drive;
	ArkEntry* entry = GetEntry(&drive->reader, folder);

	if (!entry) {
		Log("Directory '%s' does not exist", folder);
		return;
	}
	else if (!entry->folder) {
		Log("Path '%s' leads to a file, not a directory", folder);
		return;
	}

	Log("Contents of: %s", entry->name);

	for (size_t i = 0; i < entry->folderSize; ++ i) {
		Log(
			"  [%c] %s",
			entry->folderContents[i].folder? 'D' : ' ', entry->folderContents[i].name
		);
	}
}

static void* DriveReadFile(ResourceDrive* p_drive, const char* path, size_t* size) {
	ArkDrive* drive = (ArkDrive*) p_drive;
	ArkEntry* entry = GetEntry(&drive->reader, path);

	if (!entry) {
		Log("File '%s' does not exist", path);
		return NULL;
	}
	else if (entry->folder) {
		Log("Path '%s' leads to a directory, not a file", path);
		return NULL;
	}

	void* ret = Ark_ReadFile(&drive->reader, entry);
	*size     = entry->size;
	return ret;
}

ResourceDrive* Ark_CreateResourceDrive(const char* path) {
	ArkDrive* ret = SafeMalloc(sizeof(ArkDrive));
	// expect caller to write to name

	if (!Ark_InitReader(&ret->reader, path).success) {
		Log("Failed to initialise archive reader");
		return NULL;
	}

	ret->parent.free       = &FreeDrive;
	ret->parent.fileExists = &DriveFileExists;
	ret->parent.list       = &DriveList;
	ret->parent.readFile   = &DriveReadFile;

	if (Ark_Read(&ret->reader).success) {
		return (ResourceDrive*) ret;
	}
	else {
		Log("Failed to read archive");
		Ark_FreeReader(&ret->reader);
		return NULL;
	}
}
