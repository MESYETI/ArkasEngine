#include "app.h"
#include "player.h"
#include "camera.h"

Player player;

void Player_Init(void) {
	player.pos            = (FVec3) {0.0, -0.5, 0.0};
	player.vel            = (FVec3) {0.0,  0.0, 0.0};
	player.acc            = (FVec3) {0.0,  0.0, 0.0};
	player.yaw            = 0.0;
	player.pitch          = 0.0;
	player.grounded       = true;
	player.maxSpeed       = 3.333;
	player.skipFriction   = false;
	player.groundFriction = 30;
	player.airFriction    = 0.1;
	player.gravity        = 4.0;
	player.speed          = 100.0;
	player.airSpeed       = 1.0;
	player.jumpSpeed      = 3.0;
}

void Player_FPCamera(void) {
	camera.pos    = (FVec3) {player.pos.x, player.pos.y + 0.5, player.pos.z};
	camera.pitch  = player.pitch;

	// TODO: camera direction is handled by App so i commented this out until i change
	// that
	camera.yaw    = player.yaw;
	camera.sector = player.sector;
}

static void Zero(float* vel) {
	if ((*vel < 0.1) && (*vel > -0.1)) {
		*vel = 0.0;
	}
}

void Player_Physics(void) {
	if (!player.skipFriction) {
		player.vel.x += player.acc.x;
		player.vel.z += player.acc.z;

// 		if (player.acc.x > 0) {
// 			if (player.vel.x + player.acc.x > player.maxSpeed) {
// 				player.vel.x = player.maxSpeed;
// 			}
// 			else {
// 				player.vel.x += player.acc.x;
// 			}
// 		}
// 		else if (player.acc.x < 0) {
// 			if (player.vel.x + player.acc.x < -player.maxSpeed) {
// 				player.vel.x = -player.maxSpeed;
// 			}
// 			else {
// 				player.vel.x += player.acc.x;
// 			}
// 		}
// 
// 		if (player.acc.z > 0) {
// 			if (player.vel.z + player.acc.z > player.maxSpeed) {
// 				player.vel.z = player.maxSpeed;
// 			}
// 			else {
// 				player.vel.z += player.acc.z;
// 			}
// 		}
// 		else if (player.acc.z < 0) {
// 			if (player.vel.z + player.acc.z < -player.maxSpeed) {
// 				player.vel.z = -player.maxSpeed;
// 			}
// 			else {
// 				player.vel.z += player.acc.z;
// 			}
// 		}
	}
	player.vel.y += player.acc.y;

	float frictionValue;

	if (FloatEqual(player.pos.y, player.sector->floor, 0.005)) {
		frictionValue = player.groundFriction;
	}
	else {
		frictionValue = player.airFriction;
	}

	double friction = 1.0 / ((frictionValue * app.delta) + 1);

	if (!player.skipFriction) {
		player.vel.x *= friction;
		player.vel.z *= friction;
	}
	player.vel.y -= app.delta * player.gravity;

	player.pos.x += player.vel.x * app.delta;
	player.pos.y += player.vel.y * app.delta;
	player.pos.z += player.vel.z * app.delta;

	Zero(&player.vel.x);
	Zero(&player.vel.z);

	if (player.pos.y < player.sector->floor) {
		player.pos.y = player.sector->floor;
		player.vel.y = 0.0;
	}

	if (player.pos.y > player.sector->ceiling - 0.6) {
		player.pos.y = player.sector->ceiling - 0.6;
		player.vel.y = 0.0;
	}

	player.skipFriction = false;
}
