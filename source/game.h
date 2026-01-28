#ifndef AE_GAME_H
#define AE_GAME_H

#include "event.h"
#include "input.h"
#include "video.h"
#include "common.h"

typedef struct {
	float sensitivity;
	float musicVolume;
	bool  skyboxFiltering;

	Input_BindID forward;
	Input_BindID left;
	Input_BindID backward;
	Input_BindID right;
	Input_BindID jump;
} GameBaseConfig;

extern GameBaseConfig gameBaseConfig;

void GameBase_Init(void);
void GameBase_Free(void);
void GameBase_Update(bool top);
void GameBase_HandleEvent(Event* e);
void GameBase_Render(void);

#endif
