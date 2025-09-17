#include "game.h"
#include "scene.h"

static SceneManager sm;

void SceneManager_Init(void) {
	sm.activeScenes = 0;
}

void SceneManager_Free(void) {
	for (size_t i = 0; i < sm.activeScenes; ++ i) {
		if (sm.scenes[i].type == SCENE_TYPE_GAME) {
			Game_Free();
		}

		if (sm.scenes[i].free == NULL) continue;

		sm.scenes[i].free(&sm.scenes[i]);
	}

	sm.activeScenes = 0;
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
	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		Game_Init();
	}

	if (sm.scenes[sm.activeScenes - 1].init == NULL) return;

	sm.scenes[sm.activeScenes - 1].init(&sm.scenes[sm.activeScenes - 1]);
}

void SceneManager_FreeActive(void) {
	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		Game_Free();
	}

	if (sm.scenes[sm.activeScenes - 1].free == NULL) return;

	sm.scenes[sm.activeScenes - 1].free(&sm.scenes[sm.activeScenes - 1]);
}

void SceneManager_UpdateActive(void) {
	if (sm.scenes[sm.activeScenes - 1].type == SCENE_TYPE_GAME) {
		Game_Update(true);
	}

	if (sm.scenes[sm.activeScenes - 1].update == NULL) return;

	sm.scenes[sm.activeScenes - 1].update(&sm.scenes[sm.activeScenes - 1], true);
}

void SceneManager_HandleEvent(SDL_Event* e) {
	for (size_t i = sm.activeScenes; i -- > 0;) {
		if (sm.scenes[i].handleEvent == NULL) continue;

		if (sm.scenes[i].handleEvent(&sm.scenes[i], e)) {
			return;
		}
	}
}

void SceneManager_Update(void) {
	for (size_t i = 0; i < sm.activeScenes; ++ i) {
		if (sm.scenes[i].type == SCENE_TYPE_GAME) {
			Game_Update(i == sm.activeScenes - 1);
		}

		if (sm.scenes[i].update == NULL) continue;
		sm.scenes[i].update(&sm.scenes[i], i == sm.activeScenes - 1);
	}
}

void SceneManager_Render(void) {
	for (size_t i = 0; i < sm.activeScenes; ++ i) {
		if (sm.scenes[i].type == SCENE_TYPE_GAME) {
			Game_Render();
		}

		if (sm.scenes[i].render == NULL) continue;
		sm.scenes[i].render(&sm.scenes[i]);
	}
}
