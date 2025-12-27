#ifndef AE_SCENE_H
#define AE_SCENE_H

#include "ui.h"
#include "util.h"
#include "video.h"
#include "common.h"

enum {
	SCENE_TYPE_GAME = 0,
	SCENE_TYPE_OTHER
};

typedef struct Scene Scene;

struct Scene {
	int         type;
	void*       data;
	const char* name;
	UI_Manager  ui;

	FUNCTION_POINTER(void, init, Scene*);
	FUNCTION_POINTER(void, free, Scene*);
	FUNCTION_POINTER(bool, handleEvent, Scene*, Event* e);
	FUNCTION_POINTER(void, update, Scene*, bool top);
	FUNCTION_POINTER(void, render, Scene*);
};

typedef struct {
	Scene  scenes[8];
	size_t activeScenes;
} SceneManager;

void SceneManager_Init(void);
void SceneManager_Free(void);
void SceneManager_AddScene(Scene scene);
void SceneManager_PopScene(void);
void SceneManager_InitActive(void);
void SceneManager_FreeActive(void);
void SceneManager_UpdateActive(void);
void SceneManager_HandleEvent(Event* e);
void SceneManager_Update(void);
void SceneManager_Render(void);

#endif
