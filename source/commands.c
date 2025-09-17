#include "app.h"
#include "util.h"
#include "scene.h"
#include "console.h"
#include "commands.h"

static void Command_HelloWorld(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	Log("Hello world");
}

static void Command_Test(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	Log("Starting map viewer");
	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL, NULL
	});
	app.console = false;
}

static void Command_ClearScenes(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	SceneManager_Free();
	Log("Scenes cleared");
}

void Commands_Init(void) {
	Console_AddCommand((ConsoleCommand) {"ae.hello_world",  &Command_HelloWorld});
	Console_AddCommand((ConsoleCommand) {"ae.test",         &Command_Test});
	Console_AddCommand((ConsoleCommand) {"ae.clear_scenes", &Command_ClearScenes});
}
