#ifndef AE_BACKEND_AE3DS_H
#define AE_BACKEND_AE3DS_H

#include <3ds.h>
#include <citro3d.h>

typedef struct {
	bool    used;
	int     width;
	int     height;
	C3D_Tex tex;
} Texture;

#endif
