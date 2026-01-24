#ifndef AE_SKYBOX_H
#define AE_SKYBOX_H

#include "backend.h"

typedef struct {
	Texture* east;
	Texture* ground;
	Texture* north;
	Texture* sky;
	Texture* south;
	Texture* west;
	bool     active;
} Skybox;

extern Skybox skybox;

bool Skybox_Load(const char* path);

#endif
