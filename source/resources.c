#include <dirent.h>
#include <string.h>
#include "ark.h"
#include "mem.h"
#include "stb.h"
#include "util.h"
#include "builtin.h"
#include "texture.h"
#include "resources.h"
#include "folderDrive.h"

ResourceManager resources;

void Resources_Init(void) {
	DIR* dir = opendir("game");

	if (dir == NULL) {
		Error("Failed to open directory 'game'");
	}

	resources.drives          = SafeMalloc(sizeof(ResourceDrive*) * 4);
	resources.drivesNum       = 4;
	resources.drives[0]       = BuiltIn_GetDrive();
	resources.drives[0]->name = NewString("builtin");
	resources.drives[1]       = NewFolderDrive("game/extra");
	resources.drives[1]->name = NewString("extra");
	resources.drives[2]       = NewFolderDrive("maps");
	resources.drives[2]->name = NewString("maps");
	resources.drives[3]       = NewFolderDrive("screenshots");
	resources.drives[3]->name = NewString("screenshots");

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		char* path = strrchr(entry->d_name, '.');

		if (path == NULL) continue;
		if (strcmp(path, ".ark") != 0) {
			continue;
		}

		char* concatPath = ConcatString("game/", entry->d_name);
		FILE* file       = fopen(concatPath, "rb");

		ResourceDrive* drive = Ark_CreateResourceDrive(
			Stream_ToHeap(Stream_File(file, true)), true
		);
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

	closedir(dir);

	Log("%d resource drives mounted", resources.drivesNum);

	// init resource pool
	resources.resources = SafeMalloc(64 * sizeof(*resources.resources));
	resources.capacity  = 64;

	for (size_t i = 0; i < resources.capacity; ++ i) {
		resources.resources[i].active = false;
	}

	Log("Resource pool initialised at %d bytes", (int) (64 * sizeof(*resources.resources)));
}

void Resources_Free(void) {
	for (size_t i = 0; i < resources.drivesNum; ++ i) {
		if (resources.drives[i]->free) {
			resources.drives[i]->free(resources.drives[i]);
		}

		free(resources.drives[i]->name);
		free(resources.drives[i]);
	}
	free(resources.drives);

	resources.drives    = NULL;
	resources.drivesNum = 0;

	free(resources.resources);
	resources.capacity = false;
}

static ResourceDrive* GetDrive(const char* path) {
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

bool Resources_FileExists(const char* path) {
	ResourceDrive* drive = GetDrive(path);

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

ResourceFile* Resources_List(const char* path, size_t* sz) {
	ResourceDrive* drive = GetDrive(path);

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

void Resources_FreeFileList(ResourceFile* list, size_t size) {
	for (size_t i = 0; i < size; ++ i) {
		free(list[i].fullPath);
	}

	free(list);
}

void Resources_PrintList(const char* path) {
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

void* Resources_ReadFile(const char* path, size_t* size) {
	ResourceDrive* drive = GetDrive(path);

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

static Resource* AllocResource(void) {
	for (size_t i = 0; i < resources.capacity; ++ i) {
		if (!resources.resources[i].active) {
			resources.resources[i].active = true;
			resources.resources[i].usedBy = 1;
			return &resources.resources[i];
		}
	}

	return NULL;
}

Resource* Resources_GetRes(const char* path, uint32_t opt) {
	for (size_t i = 0; i < resources.capacity; ++ i) {
		if (
			resources.resources[i].active &&
			(strcmp(path, resources.resources[i].name) == 0)
		) {
			++ resources.resources[i].usedBy;
			return &resources.resources[i];
		}
	}

	Resource* ret = AllocResource();

	if (ret == NULL) {
		resources.resources = SafeRealloc(resources.resources, resources.capacity * 2);

		for (size_t i = resources.capacity; i < resources.capacity * 2; ++ i) {
			resources.resources[i].active = false;
		}

		resources.capacity *= 2;

		ret = AllocResource();
		assert(ret);
	}

	ret->name = NewString(path);

	char* ext = strrchr(path, '.');

	if (!ext) {
		Log("No extension on resource '%s'", path);
		ret->active = false;
		free(ret->name);
		return NULL;
	}

	if ((strcmp(ext, ".png") == 0) || (strcmp(ext, ".art") == 0)) {
		if (opt & RESOURCE_IS_FONT) {
			bool success;

			ret->type   = RESOURCE_TYPE_FONT;
			ret->v.font = Text_LoadFont(path, &success);

			if (!success) {
				Log("Failed to read font");
				ret->active = false;
				free(ret->name);
				return NULL;
			}
		}
		else {
			ret->type      = RESOURCE_TYPE_TEXTURE;
			ret->v.texture = Texture_LoadFile(path);

			if (!ret->v.texture) {
				Log("Failed to load resource '%s'", path);
				ret->active = false;
				free(ret->name);
				return NULL;
			}
		}
	}
	else if (strcmp(ext, ".ogg") == 0) {
		// TODO: don't allow loading ogg resources when it's received via
		// the internet
		ret->type = RESOURCE_TYPE_AUDIO;

		size_t   size;
		uint8_t* data = (uint8_t*) Resources_ReadFile(path, &size);

		int res = stb_vorbis_decode_memory(
			data, (int) size, &ret->v.audio.channels, &ret->v.audio.sampleRate,
			&ret->v.audio.data
		);
		free(data);

		if (res == -1) {
			Log("Failed to load resource '%s'", path);
			ret->active = false;
			free(ret->name);
			return NULL;
		}

		ret->v.audio.len = res;
	}
	else if (strcmp(ext, ".zkm") == 0) {
		ret->type = RESOURCE_TYPE_MODEL;
		
		size_t   size;
		uint8_t* data   = (uint8_t*) Resources_ReadFile(path, &size);
		Stream   stream = Stream_Memory(data, size, true);

		Model_Load(&ret->v.model, &stream, path);
	}
	else {
		Log("Unknown resource type '%s'", ext);
		ret->active = false;
		free(ret->name);
		return NULL;
	}

	return ret;
}

void Resources_FreeRes(Resource* resource) {
	-- resource->usedBy;

	if (resource->usedBy == 0) {
		switch (resource->type) {
			case RESOURCE_TYPE_TEXTURE: {
				Backend_FreeTexture(resource->v.texture);
				break;
			}
			case RESOURCE_TYPE_AUDIO: {
				free(resource->v.audio.data);
				break;
			}
			default: assert(0);
		}

		free(resource->name);
		resource->active = false;
	}
}
