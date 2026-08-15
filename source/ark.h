#ifndef AE_ARK_H
#define AE_ARK_H

#include <stdio.h>
#include "vfs.h"
#include "error.h"
#include "stream.h"
#include "common.h"

typedef struct ArkEntry ArkEntry;

struct ArkEntry {
	bool   folder;
	size_t size;
	char*  name;
	size_t contentsOffset;

	ArkEntry* folderContents;
	size_t    folderSize;
};

typedef struct {
	Stream*  file;
	uint16_t ver;
	char*    strings;
	size_t   stringsLen;
	ArkEntry root;
	bool     free;
} ArchiveReader;

ErrorRet   Ark_InitReader(ArchiveReader* reader, Stream* stream, bool free);
void       Ark_FreeReader(ArchiveReader* reader);
ErrorRet   Ark_Read(ArchiveReader* reader);
void*      Ark_ReadFile(ArchiveReader* reader, ArkEntry* entry);
VFS_Drive* Ark_CreateDrive(Stream* stream, bool free);

#endif
