#include "app.h"
#include "player.h"
#include "camera.h"

Player player;

void Player_Init(void) {
	player.pos      = (FVec3) {0.0, -0.5, 0.0};
	player.vel      = (FVec3) {0.0,  0.0, 0.0};
	player.acc      = (FVec3) {0.0,  0.0, 0.0};
	player.yaw      = 0.0;
	player.pitch    = 0.0;
	player.grounded = true;
	player.maxSpeed = 2.0;
}

void Player_FPCamera(void) {
	camera.pos    = (FVec3) {player.pos.x, player.pos.y + 0.5, player.pos.z};
	camera.pitch  = player.pitch;

	// TODO: camera direction is handled by App so i commented this out until i change
	// that
	// camera.yaw    = player.yaw;
	// camera.sector = player.sector;
}

static void Zero(float* vel) {
	if ((*vel < 0.1) && (*vel > -0.1)) {
		*vel = 0.0;
	}
}

void Player_Physics(void) {
	player.vel.x += MAX(MIN(player.acc.x, player.maxSpeed), -player.maxSpeed);
	player.vel.y += MAX(MIN(player.acc.y, player.maxSpeed), -player.maxSpeed);
	player.vel.z += MAX(MIN(player.acc.z, player.maxSpeed), -player.maxSpeed);

	double friction  = 15 * app.delta;
	friction         = 1.0 / ((friction * app.delta) + 1);
	player.vel.x    *= friction;
	player.vel.z    *= friction;

	player.pos.x += player.vel.x * app.delta;
	player.pos.y += player.vel.y * app.delta;
	player.pos.z += player.vel.z * app.delta;

	Zero(&player.vel.x);
	Zero(&player.vel.z);

	if (player.pos.y < player.sector->floor) {
		player.pos.y = player.sector->floor;
	}

	if (player.pos.y > player.sector->ceiling - 0.6) {
		player.pos.y = player.sector->ceiling - 0.6;
	}
}
