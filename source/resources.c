#include <dirent.h>
#include <string.h>
#include "ark.h"
#include "mem.h"
#include "stb.h"
#include "util.h"
#include "builtin.h"
#include "resources.h"
#include "folderDrive.h"

ResourceManager resources;

void Resources_Init(void) {
	DIR* dir = opendir("game");

	if (dir == NULL) {
		Error("Failed to open directory 'game'");
	}

	resources.drives          = SafeMalloc(sizeof(ResourceDrive*) * 2);
	resources.drivesNum       = 2;
	resources.drives[0]       = BuiltIn_GetDrive();
	resources.drives[0]->name = NewString("builtin");
	resources.drives[1]       = NewFolderDrive("game/extra");
	resources.drives[1]->name = NewString("extra");

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

	closedir(dir);

	Log("%d resource drives mounted", resources.drivesNum);

	// init resource pool
	resources.resources = SafeMalloc(64 * sizeof(resources));
	resources.capacity  = 64;

	for (size_t i = 0; i < resources.capacity; ++ i) {
		resources.resources[i].active = false;
	}

	Log("Resource pool initialised at %d bytes", (int) (64 * sizeof(resources)));
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

bool Resources_FileExists(const char* path) {
	ResourceDrive* drive = GetDrive(path);

	if (!drive) {
		Log("Resources_FileExists: Automatic drive selection not implemented");
		return false;
	}

	const char* drivePath = strchr(path, '/');

	if (!drivePath) {
		Log("Invalid file path: '%s'", path);
		return false;
	}

	return drive->fileExists(drive, drivePath + 1);
}

void Resources_List(const char* path) {
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

void* Resources_ReadFile(const char* path, size_t* size) {
	ResourceDrive* drive = GetDrive(path);

	if (!drive) {
		Log("Invalid drive");
		return NULL;
	}

	const char* drivePath = strchr(path, '/');

	if (drivePath == NULL) {
		drivePath = "";
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

Resource* Resources_GetRes(const char* path) {
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

	if (strcmp(ext, ".png") == 0) {
		ret->type = RESOURCE_TYPE_TEXTURE;

		size_t   size;
		uint8_t* data = (uint8_t*) Resources_ReadFile(path, &size);

		if (!data) {
			Log("Failed to read path '%s'", path);
			ret->active = false;
			free(ret->name);
			return NULL;
		}

		ret->v.texture = Backend_LoadMemTexture(data, size);
		free(data);

		if (!ret->v.texture) {
			Log("Failed to load resource '%s'", path);
			ret->active = false;
			free(ret->name);
			return NULL;
		}
	}
	else if (strcmp(ext, ".ogg") == 0) {
		ret->type = RESOURCE_TYPE_AUDIO;

		size_t   size;
		uint8_t* data = (uint8_t*) Resources_ReadFile(path, &size);

		int res = stb_vorbis_decode_memory(
			data, (int) size, &ret->audio.channels, &ret->audio.sampleRate,
			&ret->audio.data
		);
	
		if (res == -1) {
			Log("Failed to load resource '%s'", path);
			ret->active = false;
			free(ret->name);
			return NULL;
		}

		
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
			default: assert(0);
		}

		free(resource->name);
		resource->active = false;
	}
}
