#include "app.h"
#include "map.h"
#include "safe.h"
#include "util.h"
#include "scene.h"
#include "player.h"
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
		Resources_List(NULL);
	}
	else if (argc == 1) {
		Resources_List(argv[0]);
	}
	else {
		Log("ls must have either 0 or 1 parameters");
	}
}

static void Command_Cat(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	size_t size;
	void*  contents = Resources_ReadFile(argv[0], &size);

	if (contents == NULL) {
		Log("Error reading file");
		return;
	}

	contents = SafeRealloc(contents, size + 1);
	((char*) contents)[size] = 0;

	Log("%s", (char*) contents);
}

static void Command_Echo(size_t argc, char** argv) {
	for (size_t i = 1; i < argc; ++ i) {
		argv[i][-1] = ' ';
	}

	if (argc) {
		Log("%s", argv[0]);
	}
	else {
		Log("");
	}
}

static void Command_Resources(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	(void) argv;

	for (size_t i = 0; i < resources.capacity; ++ i) {
		if (resources.resources[i].active) {
			Log(
				"[%.4X] %s: used by %d resource instances",
				(int) i, resources.resources[i].name, resources.resources[i].usedBy
			);
		}
	}
}

enum {
	VAR_INT,
	VAR_FLOAT
};

typedef struct {
	int         type;
	const char* name;
	void*       ptr;
} Variable;

static void Command_Set(size_t argc, char** argv) {
	static const Variable vars[] = {
		{VAR_FLOAT, "ground-friction", &player.groundFriction},
		{VAR_FLOAT, "gravity",         &player.gravity},
		{VAR_FLOAT, "speed",           &player.speed},
		{VAR_FLOAT, "air-speed",       &player.airSpeed}
	};

	if (argc == 0) {
		for (size_t i = 0; i < sizeof(vars) / sizeof(Variable); ++ i) {
			switch (vars[i].type) {
				case VAR_INT:   Log("int:   %s", vars[i].name); break;
				case VAR_FLOAT: Log("float: %s", vars[i].name); break;
				default:        Log("???:   %s", vars[i].name); break;
			}
		}
	}
	else {
		ASSERT_ARGC(2);

		for (size_t i = 0; i < sizeof(vars) / sizeof(Variable); ++ i) {
			if (strcmp(vars[i].name, argv[0]) == 0) {
				switch (vars[i].type) {
					case VAR_INT: {
						*((int*) vars[i].ptr) = atoi(argv[1]);
						break;
					}
					case VAR_FLOAT: {
						*((float*) vars[i].ptr) = (float) atof(argv[1]);
						break;
					}
					default: assert(0);
				}
			}
		}
	}
}

void Commands_Init(void) {
	Console_AddCommand((ConsoleCommand) {"test-map",     &Command_Test});
	Console_AddCommand((ConsoleCommand) {"clear-scenes", &Command_ClearScenes});
	Console_AddCommand((ConsoleCommand) {"map",          &Command_Map});
	Console_AddCommand((ConsoleCommand) {"dl-map",       &Command_DlMap});
	Console_AddCommand((ConsoleCommand) {"ls",           &Command_Ls});
	Console_AddCommand((ConsoleCommand) {"cat",          &Command_Cat});
	Console_AddCommand((ConsoleCommand) {"echo",         &Command_Echo});
	Console_AddCommand((ConsoleCommand) {"resources",    &Command_Resources});
	Console_AddCommand((ConsoleCommand) {"set",          &Command_Set});
}
