#include "util.h"
#include "skybox.h"
#include "backend.h"
#include "texture.h"

Skybox skybox = {
	.east   = NULL,
	.ground = NULL,
	.north  = NULL,
	.sky    = NULL,
	.south  = NULL,
	.west   = NULL,
	.active = false
};

typedef struct {
	const char* name;
	Texture**   dest;
} SkyboxPart;

bool Skybox_Load(const char* path) {
	static const SkyboxPart parts[] = {
		{"east.png",   &skybox.east},
		{"ground.png", &skybox.ground},
		{"north.png",  &skybox.north},
		{"sky.png",    &skybox.sky},
		{"south.png",  &skybox.south},
		{"west.png",   &skybox.west}
	};

	for (size_t i = 0; i < 6; ++ i) {
		if (*parts[i].dest != NULL) {
			Backend_FreeTexture(*parts[i].dest);
		}

		*parts[i].dest = NULL;
	}
	skybox.active = false;

	for (size_t i = 0; i < 6; ++ i) {
		char thisPath[1024];
		thisPath[0] = 0;
		strncpy(thisPath, path, sizeof(thisPath));
		strncat(thisPath, "/", sizeof(thisPath) - strlen(thisPath) - 1);
		strncat(thisPath, parts[i].name, sizeof(thisPath) - strlen(thisPath) - 1);

		*parts[i].dest = Texture_LoadFile(thisPath);

		if (*parts[i].dest == NULL) {
			for (size_t j = 0; j < i; ++ j) {
				Backend_FreeTexture(*parts[i].dest);
				*parts[i].dest = NULL;
			}
			Log("Failed to load skybox texture: %s", thisPath);
			return false;
		}
	}

	skybox.active = true;
	Backend_InitSkybox();
	return true;
}
