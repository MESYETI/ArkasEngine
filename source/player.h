#ifndef AE_PLAYER_H
#define AE_PLAYER_H

#include "map.h"
#include "types.h"

typedef struct {
	// config, might be moved into components at some point
	float maxSpeed;
	float gravity;
	float groundFriction;
	float airFriction;
	float speed;
	float airSpeed;
	float jumpSpeed;
	bool  skipFriction;

	// state
	FVec3 acc;

	// callbacks
	void (*createPlayerEntity)(FVec3 pos, Direction dir, const char* name);
} Player;

// player is 0.6 units tall

extern Player player;

void Player_Init(void);
void Player_FPCamera(void);
void Player_Physics(void);

#endif
