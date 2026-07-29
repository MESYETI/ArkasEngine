#include "mem.h"
#include "engine.h"
#include "entity.h"

EntityPool entityPool;

void EntityPool_Init(void) {
	entityPool.pool = SafeMalloc(64 * sizeof(Entity));
	entityPool.size = 64;
}

void EntityPool_Free(void) {
	free(entityPool.pool);
	entityPool.pool = NULL;
	entityPool.size = 0;
}

size_t EntityPool_New(void) {
	for (size_t i = 0; i < entityPool.size; ++ i) {
		if (!entityPool.pool[i].used) {
			return i;
		}
	}

	size_t ret       = entityPool.size;
	entityPool.size *= 2;
	entityPool.pool  = SafeRealloc(entityPool.pool, entityPool.size * sizeof(Entity));

	return ret;
}

Entity* EntityPool_Get(size_t idx) {
	if (idx >= entityPool.size) return NULL;

	return &entityPool.pool[idx];
}

void EntityPool_FreeEntity(size_t idx) {
	Entity* entity = EntityPool_Get(idx);

	if (!entity) {
		Error("Invalid entity %d", (int) idx);
	}
}

typedef struct {
	bool        movable;
	EntitySound sound;
} PropEntity;

static void PropFree(Entity* p_ent) {
	PropEntity* ent = (PropEntity*) p_ent->data;

	// TODO: free emitter
	ent->sound.enabled = false;
}

static void* PropGetComponent(Entity* p_ent, int id) {
	PropEntity* ent = (PropEntity*) p_ent->data;

	switch (id) {
		case AE_COMPONENT_MOVABLE: return (void*) &ent->movable;
		case AE_COMPONENT_SOUND:   return (void*) &ent->sound;
		default:                   return NULL;
	}
}

static void PropRender(Entity* ent, FVec2 portalOff) {
	ModelRenderOpt opt = {
		.scale = ent->modelScale, .pos = ent->pos, .rot = ent->dir.yaw
	};
	opt.pos.x += portalOff.x;
	opt.pos.z += portalOff.y;

	Backend_RenderModel(&ent->model->v.model, &opt);
}

size_t PropEntity_New(Sector* sect, FVec3 pos, Direction dir, Resource* model, bool movable) {
	size_t  ret = EntityPool_New();
	Entity* ent = EntityPool_Get(ret);

	PropEntity* data = NEW(PropEntity);

	ent->used         = true;
	ent->data         = NEW(PropEntity);
	ent->type         = AE_ENTITY_PROP;
	ent->pos          = pos;
	ent->vel          = (FVec3) {0.0f, 0.0f, 0.0f};
	ent->grounded     = false;
	ent->sector       = sect;
	ent->dir          = dir;
	ent->model        = model;
	ent->modelScale   = 1.0f;
	ent->modelVisible = true;
	ent->nextSect     = SECTOR_NO_ENTITIES;
	ent->prevSect     = SECTOR_NO_ENTITIES;
	ent->free         = &PropFree;
	ent->getComponent = &PropGetComponent;
	ent->update       = NULL;
	ent->render       = &PropRender;

	data->movable       = movable;
	data->sound.enabled = false;

	return ret;
}

void PropEntity_AddSound(Entity* entity, AudioEmitter emitter) {
	PropEntity* ent    = (PropEntity*) entity;
	ent->sound.enabled = true;
	ent->sound.emitter = emitter;
}
