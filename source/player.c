#include "engine.h"
#include "entity.h"
#include "player.h"
#include "camera.h"

Player player = {
	.createPlayerEntity = NULL
};

void Player_Init(void) {
	if (player.createPlayerEntity) {
		player.entityIdx = player.createPlayerEntity(
			(FVec3) {0.0f, -0.5f, 0.0f}, (Direction) {0.0f, 0.0f, 0.0f}, "Player"
		);
	}
	else {
		player.entityIdx = PlayerEntity_New(
			&map.sectors[0], (FVec3) {0.0f, -0.5f, 0.0f}, (Direction) {0.0f, 0.0f, 0.0f},
			NULL, "Player"
		);
	}

	player.maxSpeed       = 3.333f;
	player.skipFriction   = false;
	player.groundFriction = 30.0f;
	player.airFriction    = 0.4f;
	player.gravity        = 4.0f;
	player.speed          = 75.0f;
	player.airSpeed       = 1.0f;
	player.jumpSpeed      = 2.0f;

	player.acc = (FVec3) {0.0f, 0.0f, 0.0f};
}

void Player_FPCamera(void) {
	Entity* ent = Entities_Get(player.entityIdx);

	camera.pos    = (FVec3) {ent->pos.x, ent->pos.y + 0.5f, ent->pos.z};
	camera.pitch  = ent->dir.pitch;

	// TODO: camera direction is handled by App so i commented this out until i change
	// that
	camera.yaw    = ent->dir.yaw;
	camera.sector = ent->sector;
}

static void Zero(float* vel) {
	if ((*vel < 0.1) && (*vel > -0.1)) {
		*vel = 0.0;
	}
}

void Player_Physics(void) {
	Entity* ent = Entities_Get(player.entityIdx);

	if (!player.skipFriction) {
		ent->vel.x += player.acc.x;
		ent->vel.z += player.acc.z;
	}
	ent->vel.y += player.acc.y;

	float frictionValue;

	if (FloatEqual(ent->pos.y, ent->sector->floor, 0.005)) {
		frictionValue = player.groundFriction;
	}
	else {
		frictionValue = player.airFriction;
	}

	double friction = 1.0 / ((frictionValue * engine.delta) + 1);

	if (!player.skipFriction) {
		ent->vel.x *= friction;
		ent->vel.z *= friction;
	}
	ent->vel.y -= engine.delta * player.gravity;

	ent->pos.x += ent->vel.x * engine.delta;
	ent->pos.y += ent->vel.y * engine.delta;
	ent->pos.z += ent->vel.z * engine.delta;

	Zero(&ent->vel.x);
	Zero(&ent->vel.z);

	if (ent->pos.y < ent->sector->floor) {
		ent->pos.y = ent->sector->floor;
		ent->vel.y = 0.0;
	}

	if (ent->pos.y > ent->sector->ceiling - 0.6) {
		ent->pos.y = ent->sector->ceiling - 0.6;
		ent->vel.y = 0.0;
	}

	player.skipFriction = false;
}
