#include "mem.h"
#include "entity.h"

typedef struct {
	Entity      ent;
	bool        movable;
	EntitySound sound;
} PropEntity;

static void PropFree(Entity* p_ent) {
	PropEntity* ent = (PropEntity*) p_ent;

	// TODO: free emitter
	ent->sound.enabled = false;
}

static void* PropGetComponent(Entity* p_ent, int id) {
	PropEntity* ent = (PropEntity*) p_ent;

	switch (id) {
		case AE_COMPONENT_MOVABLE: return (void*) &ent->movable;
		case AE_COMPONENT_SOUND:   return (void*) &ent->sound;
		default:                   return NULL;
	}
}

static void PropRender(Entity* ent, FVec2 portalOff) {
	ModelRenderOpt opt = {
		.scale = ent->modelScale, .pos = ent->pos
	};
	opt.pos.x += portalOff.x;
	opt.pos.z += portalOff.y;

	Backend_RenderModel(&ent->model->v.model, &opt);
}

Entity* PropEntity_New(
	Sector* sect, FVec3 pos, Direction dir, Resource* model, bool movable
) {
	PropEntity* res = NEW(PropEntity);

	res->ent.type         = AE_ENTITY_PROP;
	res->ent.pos          = pos;
	res->ent.sector       = sect;
	res->ent.dir          = dir;
	res->ent.model        = model;
	res->ent.modelScale   = 1.0f;
	res->ent.free         = &PropFree;
	res->ent.getComponent = &PropGetComponent;
	res->ent.update       = NULL;
	res->ent.render       = &PropRender;

	res->movable       = movable;
	res->sound.enabled = false;

	return (Entity*) res;
}

void PropEntity_AddSound(Entity* entity, AudioEmitter emitter) {
	PropEntity* ent    = (PropEntity*) entity;
	ent->sound.enabled = true;
	ent->sound.emitter = emitter;
}
