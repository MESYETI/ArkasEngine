#ifndef AE_GAME_H
#define AE_GAME_H

#include "event.h"
#include "input.h"
#include "video.h"
#include "common.h"

typedef struct {
	float sensitivity;
	float musicVolume;

	Input_BindID forward;
	Input_BindID left;
	Input_BindID backward;
	Input_BindID right;
	Input_BindID jump;
} GameBaseConfig;

extern GameBaseConfig gameBaseConfig;

void Game_Init(void);
void Game_Free(void);
void Game_Update(bool top);
void Game_HandleEvent(Event* e);
void Game_Render(void);

#endif
