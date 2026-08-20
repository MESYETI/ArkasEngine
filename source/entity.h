#ifndef AE_ENTITY_H
#define AE_ENTITY_H

#include "map.h"
#include "audio.h"
#include "types.h"
#include "resources.h"

#define AE_ENTITY_HEADER_SIZE 32

typedef struct Entity Entity;

struct Entity {
	bool       used;
	uint32_t   id;
	void*      data;
	uint32_t   type;
	FVec3      pos;
	FVec3      vel;
	bool       grounded;
	Sector*    sector;
	Direction  dir;

	// link
	size_t nextSect;
	size_t prevSect;

	// overrides
	void* (*getComponent)(struct Entity* ent, int id);
};

typedef struct {
	int*   components;
	size_t len;
	size_t dataSize;

	void* (*getComponent)(Entity* ent, int id);
} EntityDef;

typedef struct {
	size_t serialSize;

	// read, write are required
	// update, render are not required
	bool (*read)(Entity* ent, uint8_t* data);
	void (*write)(Entity* ent, uint8_t* out);
	void (*update)(Entity* ent);
	void (*render)(Entity* ent, FVec2 portalOff);
} EntityComponent;

// TODO: make the built in entity types invalid if they are disabled

// built in entity types
enum {
	AE_ENTITY_PROP = 0,
	AE_ENTITY_PLAYER,

	AE_ENTITY_AMOUNT
};
// games should start their entity types at 0x100

// built in entity components
enum {
	AE_COMPONENT_MOVABLE = 0, // bool
	AE_COMPONENT_NAME,        // struct EntityName
	AE_COMPONENT_MODEL,       // struct EntityModel

	AE_COMPONENT_AMOUNT
};
// games should start their entity component types at 0x100

typedef struct {
	char name[65];
	bool render;
} EntityName;

typedef struct {
	Resource* model; // serialised as a char[256] path
	float     modelScale;
	bool      modelVisible;
} EntityModel;

typedef struct {
	Entity* pool;
	size_t  size;

	EntityComponent  builtInComp[AE_COMPONENT_AMOUNT];
	EntityComponent* gameComp;
	size_t           gameCompLength;

	EntityDef  builtInDef[AE_ENTITY_AMOUNT];
	EntityDef* gameDef;
	size_t     gameDefLength;
} EntitySystem;

// entity pool
extern EntitySystem entities;

void             Entities_Init(void);
void             Entities_InitPool(void);
void             Entities_FreePool(void);
size_t           Entities_New(void);
Entity*          Entities_Get(size_t idx);
void             Entities_FreeEntity(size_t idx);
EntityComponent* Entities_GetComponent(int comp);
EntityDef*       Entities_GetDef(uint32_t type);
size_t           Entities_CalcSerialSize(EntityDef* def);
bool             Entities_Deserialise(uint8_t* data, size_t size, Entity* out);
bool             Entities_Serialise(Entity* ent, uint8_t* dest, size_t size);
void             Entities_Update(void);

void Entity_Render(Entity* ent, FVec2 portalOff);

// built in entities
#ifndef AE_NO_BASIC_PROP_ENT
	size_t PropEntity_New(Sector* sect, FVec3 pos, Direction dir, Resource* model, bool movable);
#endif

#ifndef AE_NO_BASIC_PLAYER_ENT
	size_t PlayerEntity_New(Sector* sect, FVec3 pos, Direction dir, Resource* model, const char* name);
#endif

#endif
