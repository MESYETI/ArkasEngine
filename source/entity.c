#include "mem.h"
#include "data.h"
#include "engine.h"
#include "entity.h"

EntitySystem entities;

static bool CompMovableRead(Entity* ent, uint8_t* data) {
	bool* movable = (bool*) ent->getComponent(ent, AE_COMPONENT_MOVABLE);

	*movable = *data != 0;
	return true;
}

static void CompMovableWrite(Entity* ent, uint8_t* out) {
	bool* movable = (bool*) ent->getComponent(ent, AE_COMPONENT_MOVABLE);

	*out = *movable? 1 : 0;
}

static bool CompNameRead(Entity* ent, uint8_t* data) {
	EntityName* name = (EntityName*) ent->getComponent(ent, AE_COMPONENT_NAME);

	memcpy(name->name, data, 64);
	name->name[64] = 0;
	return true;
}

static void CompNameWrite(Entity* ent, uint8_t* out) {
	EntityName* name = (EntityName*) ent->getComponent(ent, AE_COMPONENT_NAME);

	memcpy(out, name->name, 64);
	out[64] = name->render? 1 : 0;
}

static void CompModelFree(Entity* ent) {
	EntityModel* model = (EntityModel*) ent->getComponent(ent, AE_COMPONENT_MODEL);

	if (model->model) {
		Resources_FreeRes(model->model);
	}
}

static bool CompModelRead(Entity* ent, uint8_t* data) {
	EntityModel* model = (EntityModel*) ent->getComponent(ent, AE_COMPONENT_MODEL);

	char path[257];
	memcpy(path, data, 256);
	path[256] = 0;

	model->model = Resources_GetRes(path, 0);

	if (model->model->type != RESOURCE_TYPE_MODEL) {
		Log("Model resource is not a model");

		Resources_FreeRes(model->model);
		return false;
	}

	model->modelScale   = Data_ReadFloat(&data[256]);
	model->modelVisible = data[260] != 0;
	return true;
}

static void CompModelWrite(Entity* ent, uint8_t* out) {
	EntityModel* model = (EntityModel*) ent->getComponent(ent, AE_COMPONENT_MODEL);

	strncpy((char*) out, model->model->name, 256);

	Data_WriteFloat(&out[256], model->modelScale);
	out[260] = model->modelVisible;
}

static void CompModelRender(Entity* ent, FVec2 portalOff) {
	EntityModel* model = (EntityModel*) ent->getComponent(ent, AE_COMPONENT_MODEL);

	ModelRenderOpt opt = {
		.scale = model->modelScale, .pos = ent->pos, .rot = ent->dir.yaw
	};
	opt.pos.x += portalOff.x;
	opt.pos.z += portalOff.y;

	Backend_RenderModel(&model->model->v.model, &opt);
}

typedef struct {
	bool        movable;
	EntityModel model;
} PropEntity;

static void* PropGetComponent(Entity* p_ent, int id) {
	PropEntity* ent = (PropEntity*) p_ent->data;

	switch (id) {
		case AE_COMPONENT_MOVABLE: return (void*) &ent->movable;
		case AE_COMPONENT_MODEL:   return (void*) &ent->model;
		default:                   return NULL;
	}
}

typedef struct {
	EntityName  name;
	EntityModel model;
} PlayerEntity;

static void* PlayerGetComponent(Entity* p_ent, int id) {
	PlayerEntity* ent = (PlayerEntity*) p_ent->data;

	switch (id) {
		case AE_COMPONENT_NAME:  return (void*) &ent->name;
		case AE_COMPONENT_MODEL: return (void*) &ent->model;
		default:                 return NULL;
	}
}

#define B(N) entities.builtInComp[N] = (EntityComponent)

void Entities_Init(void) {
	entities.gameComp       = NULL;
	entities.gameCompLength = 0;

	B(AE_COMPONENT_MOVABLE) {
		1, NULL, &CompMovableRead, &CompMovableWrite, NULL, NULL
	};

	B(AE_COMPONENT_NAME) {
		64 + 1, NULL, &CompNameRead, &CompNameWrite, NULL, NULL
	};

	B(AE_COMPONENT_MODEL) {
		256 + 4 + 1, &CompModelFree, &CompModelRead, &CompModelWrite, NULL, &CompModelRender
	};

	entities.gameDef       = NULL;
	entities.gameDefLength = 0;

	static const int propComp[] = {
		AE_COMPONENT_MOVABLE, AE_COMPONENT_MODEL
	};
	entities.builtInDef[AE_ENTITY_PROP]	= (EntityDef) {
		propComp, 2, sizeof(PropEntity), &PropGetComponent
	};

	static const int playerComp[] = {
		AE_COMPONENT_NAME, AE_COMPONENT_MODEL
	};
	entities.builtInDef[AE_ENTITY_PLAYER] = (EntityDef) {
		playerComp, 2, sizeof(PlayerEntity), &PlayerGetComponent
	};
}

#undef B

void Entities_InitPool(void) {
	entities.pool = SafeMalloc(64 * sizeof(Entity));
	entities.size = 64;

	for (size_t i = 0; i < entities.size; ++ i) {
		entities.pool[i].used = false;
	}
}

void Entities_FreePool(void) {
	for (size_t i = 0; i < entities.size; ++ i) {
		if (entities.pool[i].used) {
			Entities_FreeEntity(i);
		}
	}

	free(entities.pool);
	entities.pool = NULL;
	entities.size = 0;
}

size_t Entities_New(void) {
	for (size_t i = 0; i < entities.size; ++ i) {
		if (!entities.pool[i].used) {
			return i;
		}
	}

	size_t ret       = entities.size;
	entities.size *= 2;
	entities.pool  = SafeRealloc(entities.pool, entities.size * sizeof(Entity));

	for (size_t i = ret; i < entities.size; ++ i) {
		entities.pool[i].used = false;
		entities.pool[i].id   = (uint32_t) i;
	}

	return ret;
}

Entity* Entities_Get(size_t idx) {
	if (idx >= entities.size) return NULL;

	return &entities.pool[idx];
}

void Entities_FreeEntity(size_t idx) {
	Entity* entity = Entities_Get(idx);

	if (!entity) {
		Error("Invalid entity %d", (int) idx);
	}

	entity->used = false;

	EntityDef* def = Entities_GetDef(entity->type);

	if (!def) {
		Error("Broken entity in pool");
	}

	for (size_t i = 0; i < def->len; ++ i) {
		EntityComponent* comp = Entities_GetComponent(def->components[i]);

		if (!comp) {
			Error("Failed to get entity component %d", def->components[i]);
		}
		if (!comp->update) {
			continue;
		}

		comp->free(entity);
	}

	free(entity->data);
}

EntityComponent* Entities_GetComponent(int comp) {
	if (comp < 0) {
		return NULL;
	}

	if (comp < 0x100) {
		if (comp >= AE_COMPONENT_AMOUNT) {
			return NULL;
		}

		return &entities.builtInComp[comp];
	}
	else {
		if (comp >= (int) entities.gameCompLength) {
			return NULL;
		}

		return &entities.gameComp[comp - 0x100];
	}
}

EntityDef* Entities_GetDef(uint32_t type) {
	if (type < 0x100) {
		if (type >= AE_ENTITY_AMOUNT) {
			return NULL;
		}

		return &entities.builtInDef[type];
	}
	else {
		if (type >= entities.gameDefLength) {
			return NULL;
		}

		return &entities.gameDef[type - 0x100];
	}
}

size_t Entities_CalcSerialSize(EntityDef* def) {
	size_t ret = 0;

	for (size_t i = 0; i < def->len; ++ i) {
		EntityComponent* comp = Entities_GetComponent(def->components[i]);

		if (!comp) {
			Error("Broken entity definition");
		}

		ret += comp->serialSize;
	}

	return ret;
}

bool Entities_Deserialise(uint8_t* data, size_t size, Entity* out) {
	if (size < AE_ENTITY_HEADER_SIZE) {
		Log("Cannot fit entity header");
		return false;
	}

	uint32_t   id  = Data_Read32(data);
	EntityDef* def = Entities_GetDef(id);

	if (!def) {
		Log("Broken entity");
		return false;
	}

	if (size < Entities_CalcSerialSize(def) + AE_ENTITY_HEADER_SIZE) {
		Log("Not enough room to deserialise entity");
		return false;
	}

	uint32_t sectorID = Data_Read32(&data[16]);

	if (sectorID >= map.sectorsLen) {
		Log("Sector ID out of bounds");
		return false;
	}

	out->data      = SafeMalloc(def->dataSize);
	out->id        = id;
	out->pos.x     = Data_ReadFloat(&data[4]);
	out->pos.y     = Data_ReadFloat(&data[8]);
	out->pos.z     = Data_ReadFloat(&data[12]);
	out->sector    = &map.sectors[sectorID];
	out->dir.pitch = Data_ReadFloat(&data[20]);
	out->dir.yaw   = Data_ReadFloat(&data[24]);
	out->dir.roll  = Data_ReadFloat(&data[28]);

	size_t offset = 32;

	for (size_t i = 0; i < def->len; ++ i) {
		EntityComponent* comp = Entities_GetComponent(def->components[i]);

		comp->read(out, &data[offset]);
		offset += comp->serialSize;
	}

	return true;
}

bool Entities_Serialise(Entity* ent, uint8_t* dest, size_t size) {
	if (size < AE_ENTITY_HEADER_SIZE) {
		Log("Cannot fit entity header");
		return false;
	}

	EntityDef* def = Entities_GetDef(ent->type);

	if (!def) {
		Log("Broken entity");
		return false;
	}

	if (size < Entities_CalcSerialSize(def) + AE_ENTITY_HEADER_SIZE) {
		Log("Not enough room to serialise entity");
		return false;
	}

	Data_Write32(dest,         ent->id);
	Data_WriteFloat(&dest[4],  ent->pos.x);
	Data_WriteFloat(&dest[8],  ent->pos.y);
	Data_WriteFloat(&dest[12], ent->pos.z);
	Data_Write32(&dest[16],    (uint32_t) (ent->sector - map.sectors));
	Data_WriteFloat(&dest[20], ent->dir.pitch);
	Data_WriteFloat(&dest[24], ent->dir.yaw);
	Data_WriteFloat(&dest[28], ent->dir.roll);

	size_t offset = 32;

	for (size_t i = 0; i < def->len; ++ i) {
		EntityComponent* comp = Entities_GetComponent(def->components[i]);

		comp->write(ent, &dest[offset]);
		offset += comp->serialSize;
	}

	return true;
}

void Entities_Update(void) {
	for (size_t i = 0; i < entities.size; ++ i) {
		if (!entities.pool[i].used) continue;

		Entity*    ent = &entities.pool[i];
		EntityDef* def = Entities_GetDef(ent->type);

		if (!def) {
			Error("Broken entity in pool");
		}

		for (size_t i = 0; i < def->len; ++ i) {
			EntityComponent* comp = Entities_GetComponent(def->components[i]);

			if (!comp) {
				Error("Failed to get entity component %d", def->components[i]);
			}
			if (!comp->update) {
				continue;
			}

			comp->update(ent);
		}
	}
}

void Entity_Render(Entity* ent, FVec2 portalOff) {
	EntityDef* def = Entities_GetDef(ent->type);

	if (!def) {
		Error("Broken entity in pool");
	}

	for (size_t i = 0; i < def->len; ++ i) {
		EntityComponent* comp = Entities_GetComponent(def->components[i]);

		if (!comp) {
			Error("Failed to get entity component %d", def->components[i]);
		}
		if (!comp->render) {
			continue;
		}

		comp->render(ent, portalOff);
	}
}

#ifndef AE_NO_BASIC_PROP_ENT
	size_t PropEntity_New(Sector* sect, FVec3 pos, Direction dir, Resource* model, bool movable) {
		size_t  ret = Entities_New();
		Entity* ent = Entities_Get(ret);

		PropEntity* data = NEW(PropEntity);

		ent->used          = true;
		ent->data          = data;
		ent->type          = AE_ENTITY_PROP;
		ent->pos           = pos;
		ent->vel           = (FVec3) {0.0f, 0.0f, 0.0f};
		ent->grounded      = false;
		ent->sector        = sect;
		ent->dir           = dir;
		ent->nextSect      = SECTOR_NO_ENTITIES;
		ent->prevSect      = SECTOR_NO_ENTITIES;
		ent->getComponent  = &PropGetComponent;

		data->movable            = movable;
		data->model.model        = model;
		data->model.modelScale   = 1.0f;
		data->model.modelVisible = true;

		return ret;
	}
#endif

#ifndef AE_NO_BASIC_PLAYER_ENT
	size_t PlayerEntity_New(Sector* sect, FVec3 pos, Direction dir, Resource* model, const char* name) {
		size_t  ret = Entities_New();
		Entity* ent = Entities_Get(ret);
		
		PlayerEntity* data = NEW(PlayerEntity);

		ent->used          = true;
		ent->data          = data;
		ent->type          = AE_ENTITY_PLAYER;
		ent->pos           = pos;
		ent->vel           = (FVec3) {0.0f, 0.0f, 0.0f};
		ent->grounded      = false;
		ent->sector        = sect;
		ent->dir           = dir;
		ent->nextSect      = SECTOR_NO_ENTITIES;
		ent->prevSect      = SECTOR_NO_ENTITIES;
		ent->getComponent  = &PlayerGetComponent;

		strncpy(data->name.name, name, sizeof(data->name.name) - 1);
		data->model.model        = model;
		data->model.modelScale   = 1.0f;
		data->model.modelVisible = true;

		return ret;
	}
#endif
