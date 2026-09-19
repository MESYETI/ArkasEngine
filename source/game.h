#ifndef AE_GAME_H
#define AE_GAME_H

#include "event.h"
#include "input.h"
#include "video.h"
#include "scene.h"
#include "common.h"

typedef struct {
	UI_Element*   chatLog;
	UI_Container* chatCont;
	char          chatInput[128];
	UI_Element*   chatInputElem;
} GameBase;

extern GameBase gameBase;

typedef struct {
	float sensitivity;
	float musicVolume;
	bool  skyboxFiltering;
	int   debugInfoLevel;
	bool  noclip;

	Input_BindID forward;
	Input_BindID left;
	Input_BindID backward;
	Input_BindID right;
	Input_BindID jump;
	Input_BindID chat;
} GameBaseConfig;

extern GameBaseConfig gameBaseConfig;

void GameBase_Init(Scene* scene);
void GameBase_Free(Scene* scene);
void GameBase_Update(Scene* scene, bool top);
void GameBase_HandleEvent(Scene* scene, Event* e);
void GameBase_Render(Scene* scene);

#endif
