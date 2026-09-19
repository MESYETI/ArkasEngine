#include "game.h"
#include "scene.h"
#include "engine.h"

static SceneManager sm;

void SceneManager_Init(void) {
	sm.activeScenes = 0;
}

void SceneManager_Free(void) {
	for (size_t i = 0; i < sm.activeScenes; ++ i) {
		if (sm.scenes[i].type == SCENE_TYPE_GAME) {
			GameBase_Free(&sm.scenes[i]);
		}

		if (sm.scenes[i].free == NULL) continue;

		sm.scenes[i].free(&sm.scenes[i]);
	}

	sm.activeScenes = 0;
}

void SceneManager_SchedulePop(void) {
	sm.pop = true;
}

void SceneManager_ScheduleAdd(Scene scene) {
	sm.addScene   = true;
	sm.addedScene = scene;
}

void SceneManager_AddScene(Scene scene) {
	if (sm.activeScenes == 8) {
		Error("Scene stack filled");
	}

	sm.scenes[sm.activeScenes] = scene;
	++ sm.activeScenes;

	SceneManager_InitActive();
}

void SceneManager_PopScene(void) {
	SceneManager_FreeActive();

	-- sm.activeScenes;
}

void SceneManager_InitActive(void) {
	if (sm.activeScenes == 0) return;

	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		GameBase_Init(&sm.scenes[sm.activeScenes - 1]);
	}

	if (sm.scenes[sm.activeScenes - 1].init == NULL) return;

	sm.scenes[sm.activeScenes - 1].init(&sm.scenes[sm.activeScenes - 1]);
}

void SceneManager_FreeActive(void) {
	if (sm.activeScenes == 0) return;

	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		GameBase_Free(&sm.scenes[sm.activeScenes - 1]);
	}

	if (sm.scenes[sm.activeScenes - 1].free == NULL) return;

	sm.scenes[sm.activeScenes - 1].free(&sm.scenes[sm.activeScenes - 1]);
}

void SceneManager_UpdateActive(void) {
	if (sm.activeScenes == 0) return;

	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		GameBase_Update(&sm.scenes[sm.activeScenes - 1], true);
	}

	if (sm.scenes[sm.activeScenes - 1].update == NULL) return;

	sm.scenes[sm.activeScenes - 1].update(&sm.scenes[sm.activeScenes - 1], true);
}

void SceneManager_HandleEvent(Event* e) {
	if (sm.activeScenes == 0) return;

	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		GameBase_HandleEvent(&sm.scenes[sm.activeScenes - 1], e);
	}

	for (size_t i = sm.activeScenes; i -- > 0;) {
		if (sm.scenes[i].handleEvent == NULL) continue;

		if (sm.scenes[i].handleEvent(&sm.scenes[i], e, i == sm.activeScenes - 1)) {
			return;
		}
	}
}

void SceneManager_Update(void) {
	if (sm.pop) {
		SceneManager_PopScene();
		sm.pop = false;
	}

	if (sm.addScene) {
		SceneManager_AddScene(sm.addedScene);
		sm.addScene = false;
	}

	for (size_t i = 0; i < sm.activeScenes; ++ i) {
		if (sm.scenes[i].type == SCENE_TYPE_GAME) {
			GameBase_Update(&sm.scenes[i], i == sm.activeScenes - 1);
		}

		if (sm.scenes[i].update == NULL) continue;
		sm.scenes[i].update(&sm.scenes[i], i == sm.activeScenes - 1);
	}
}

void SceneManager_Render(void) {
	for (size_t i = 0; i < sm.activeScenes; ++ i) {
		if (sm.scenes[i].type == SCENE_TYPE_GAME) {
			GameBase_Render(&sm.scenes[i]);
		}

		if (engine.console) {
			Window_SetRelativeMouseMode(false);
			Window_ShowCursor(true);
		}

		if (sm.scenes[i].render == NULL) continue;
		sm.scenes[i].render(&sm.scenes[i], i == sm.activeScenes - 1);
	}
}
