#ifndef AE_ENTITY_H
#define AE_ENTITY_H

#include "types.h"
#include "resources.h"

typedef struct {
	FVec2     pos;
	Direction dir;
	Resource* model;
} Entity;

void Entity_Render(Entity* entity);

#endif
