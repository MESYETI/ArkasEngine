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
	player.maxSpeed       = 2.0;
	player.skipFriction   = false;
	player.groundFriction = 30;
	player.gravity        = 4.0;
	player.speed          = 2.0;
	player.airSpeed       = 0.1;
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
		player.vel.x += MAX(MIN(player.acc.x, player.maxSpeed), -player.maxSpeed);
		player.vel.z += MAX(MIN(player.acc.z, player.maxSpeed), -player.maxSpeed);

// 		float maxX = MAX(player.maxSpeed - player.vel.x, 0);
// 		float maxZ = MAX(player.maxSpeed - player.vel.z, 0);
// 
// 		if (player.acc.x < 0) {
// 			player.vel.x += MAX(-maxX, player.acc.x);
// 		}
// 		else {
// 			player.vel.x += MIN(maxX,  player.acc.x);
// 		}
// 
// 		if (player.acc.z < 0) {
// 			player.vel.z += MAX(-maxZ, player.acc.z);
// 		}
// 		else {
// 			player.vel.z += MIN(maxZ,  player.acc.z);
// 		}
	}
	player.vel.y += player.acc.y;

	double friction = 1.0 / ((player.groundFriction * app.delta) + 1);

	if (FloatEqual(player.pos.y, player.sector->floor, 0.005) && !player.skipFriction) {
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
