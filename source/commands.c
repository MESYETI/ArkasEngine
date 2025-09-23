#include "app.h"
#include "map.h"
#include "util.h"
#include "scene.h"
#include "console.h"
#include "commands.h"
#include "resources.h"

#define ASSERT_ARGC(N) \
	if (argc != (N)) { \
		Log("Command requires %d arguments", (N)); \
		return; \
	}

static void Command_Test(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	(void) argv;

	Log("Starting map viewer");

	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL, NULL
	});
	Map_LoadTest();
	app.console = false;
}

static void Command_ClearScenes(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	(void) argv;

	SceneManager_Free();
	Log("Scenes cleared");
}

static void Command_Map(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	char* path1 = ConcatString("maps/", argv[0]);
	char* path2 = ConcatString(path1,   ".arm");
	free(path1);

	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL, NULL
	});

	if (!Map_LoadFile(path2)) {
		Log("Failed to load map");
		SceneManager_PopScene();
	}

	free(path2);
	app.console = false;
}

static void Command_DlMap(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	char* path1 = ConcatString("maps/", argv[0]);
	char* path2 = ConcatString(path1,   ".arm");
	free(path1);
	Map_SaveFile(path2);
	free(path2);
}

static void Command_Ls(size_t argc, char** argv) {
	if (argc == 0) {
		ResourceManager_List(NULL);
	}
	else if (argc == 1) {
		ResourceManager_List(argv[0]);
	}
	else {
		Log("ls must have either 0 or 1 parameters");
	}
}

void Commands_Init(void) {
	Console_AddCommand((ConsoleCommand) {"test-map",     &Command_Test});
	Console_AddCommand((ConsoleCommand) {"clear-scenes", &Command_ClearScenes});
	Console_AddCommand((ConsoleCommand) {"map",          &Command_Map});
	Console_AddCommand((ConsoleCommand) {"dl-map",       &Command_DlMap});
	Console_AddCommand((ConsoleCommand) {"ls",           &Command_Ls});
}
