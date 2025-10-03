#ifndef AE_PLAYER_H
#define AE_PLAYER_H

#include "map.h"
#include "types.h"

typedef struct {
	FVec3   pos; // ground pos
	FVec3   vel;
	FVec3   acc;
	float   yaw;
	float   pitch;
	bool    grounded;
	bool    skipFriction;
	Sector* sector;

	// config
	float maxSpeed;
	float gravity;
	float groundFriction;
	float speed;
	float airSpeed;
	float jumpSpeed;
} Player;

// player is 0.6 units tall

extern Player player;

void Player_Init(void);
void Player_FPCamera(void);
void Player_Physics(void);

#endif
