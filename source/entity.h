#ifndef AE_ENTITY_H
#define AE_ENTITY_H

#include "map.h"
#include "audio.h"
#include "types.h"
#include "resources.h"

typedef struct Entity {
	uint32_t   type;
	FVec3      pos;
	Sector*    sector;
	Direction  dir;
	Resource*  model;
	float      modelScale;

	void  (*free)(struct Entity* ent);
	void* (*getComponent)(struct Entity* ent, int id);
	void  (*update)(struct Entity* ent);
	void  (*render)(struct Entity* ent, FVec2 portalOff);
} Entity;

// built in entity types
enum {
	AE_ENTITY_PROP = 0,
	AE_ENTITY_LIGHT
};
// games should start their entity types at 0x100

// built in entity components
enum {
	AE_COMPONENT_MOVABLE = 0,
	AE_COMPONENT_SOUND,
	AE_COMPONENT_FOLLOW
};

typedef struct {
	bool     enabled;
	uint32_t emitter;
} EntitySound;

typedef struct {
	Entity* entity;
	FVec3   update;
} EntityFollow;

// built in entities
Entity* PropEntity_New(
	Sector* sect, FVec3 pos, Direction dir, Resource* model, bool movable
);
void PropEntity_AddSound(Entity* entity, AudioEmitter emitter);

#endif
