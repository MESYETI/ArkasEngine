#include <dirent.h>
#include <string.h>
#include "fs.h"
#include "ark.h"
#include "mem.h"
#include "stb.h"
#include "vfs.h"
#include "util.h"
#include "builtin.h"
#include "texture.h"
#include "resources.h"
#include "folderDrive.h"

ResourceManager resources;

void Resources_Init(void) {
	// init resource pool
	resources.resources = SafeMalloc(256 * sizeof(*resources.resources));
	resources.capacity  = 256;

	for (size_t i = 0; i < resources.capacity; ++ i) {
		resources.resources[i].active = false;
	}

	Log("Resource pool initialised at %d bytes", (int) (64 * sizeof(*resources.resources)));
}

void Resources_Free(void) {
	free(resources.resources);
	resources.capacity = false;
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
// 		resources.resources = SafeRealloc(resources.resources, resources.capacity * 2);
// 
// 		for (size_t i = resources.capacity; i < resources.capacity * 2; ++ i) {
// 			resources.resources[i].active = false;
// 		}
// 
// 		resources.capacity *= 2;
// 
// 		ret = AllocResource();
// 		assert(ret);
		return NULL;
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
		uint8_t* data = (uint8_t*) VFS_ReadFile(path, &size);

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
		uint8_t* data   = (uint8_t*) VFS_ReadFile(path, &size);
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
	if (resource->usedBy <= 0) {
		Error("Freeing already freed resource");
	}

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
			case RESOURCE_TYPE_FONT: {
				Text_FreeFont(&resource->v.font);
				break;
			}
			case RESOURCE_TYPE_MODEL: {
				Model_Free(&resource->v.model);
				break;
			}
			default: assert(0);
		}

		free(resource->name);
		resource->active = false;
	}
}
