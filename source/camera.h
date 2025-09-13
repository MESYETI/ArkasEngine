#ifndef AE_CAMERA_H
#define AE_CAMERA_H

#include "types.h"

typedef struct {
	FVec3 pos;
	float pitch;
	float yaw;
	float roll;
} Camera;

extern Camera camera;

#endif
