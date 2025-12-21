#include "app.h"
#include "map.h"
#include "mem.h"
#include "game.h"
#include "util.h"
#include "audio.h"
#include "scene.h"
#include "player.h"
#include "console.h"
#include "commands.h"
#include "resources.h"
#include "testScene.h"
#include "mapEditor.h"

#define ASSERT_ARGC(N) \
	(void) argv; \
	if (argc != (N)) { \
		Log("Command requires %d arguments", (N)); \
		return; \
	}

static void Command_Test(size_t argc, char** argv) {
	ASSERT_ARGC(0);

	Log("Starting map viewer");

	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", (UI_Manager) {0}, NULL, NULL, NULL,
		NULL, NULL
	});
	Map_LoadTest();
	app.console = false;
}

static void Command_ClearScenes(size_t argc, char** argv) {
	ASSERT_ARGC(0);

	SceneManager_Free();
	Log("Scenes cleared");
}

static void Command_Map(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	char* path1 = ConcatString("maps/", argv[0]);
	char* path2 = ConcatString(path1,   ".arm");
	free(path1);

	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", (UI_Manager) {0}, NULL, NULL, NULL,
		NULL, NULL
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
	VAR_FLOAT,
	VAR_BOOL
};

typedef struct {
	int         type;
	const char* name;
	void*       ptr;
} Variable;

static void Command_Set(size_t argc, char** argv) {
	static const Variable vars[] = {
		{VAR_FLOAT, "player.ground-friction", &player.groundFriction},
		{VAR_FLOAT, "player.gravity",         &player.gravity},
		{VAR_FLOAT, "player.speed",           &player.speed},
		{VAR_FLOAT, "player.air-speed",       &player.airSpeed},
		{VAR_FLOAT, "player.jump-speed",      &player.jumpSpeed},
		{VAR_FLOAT, "game.sensitivity",       &gameBaseConfig.sensitivity},
		{VAR_FLOAT, "game.music-volume",      &gameBaseConfig.musicVolume},
		{VAR_BOOL,  "echo",                   &console.echo}
	};

	if (argc == 0) {
		for (size_t i = 0; i < sizeof(vars) / sizeof(Variable); ++ i) {
			switch (vars[i].type) {
				case VAR_INT:   Log("int:   %s", vars[i].name); break;
				case VAR_FLOAT: Log("float: %s", vars[i].name); break;
				case VAR_BOOL:  Log("bool:  %s", vars[i].name); break;
				default:        Log("???:   %s", vars[i].name); break;
			}
		}
	}
	else if (argc == 1) {
		for (size_t i = 0; i < sizeof(vars) / sizeof(Variable); ++ i) {
			if (strcmp(vars[i].name, argv[0]) == 0) {
				switch (vars[i].type) {
					case VAR_INT: {
						Log("%s = %d", argv[0], *((int*) vars[i].ptr));
						break;
					}
					case VAR_FLOAT: {
						Log("%s = %g", argv[0], *((float*) vars[i].ptr));
						break;
					}
					case VAR_BOOL: {
						Log(
							"%s = %s", argv[0],
							(*((bool*) vars[i].ptr))? "true" : "false"
						);
						break;
					}
					default: assert(0);
				}

				return;
			}
		}

		Log("Variable '%s' does not exist", argv[0]);
	}
	else if (argc == 2) {
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
					case VAR_BOOL: {
						if (strcmp(argv[1], "true") == 0) {
							*((bool*) vars[i].ptr) = true;
						}
						else if (strcmp(argv[1], "false") == 0) {
							*((bool*) vars[i].ptr) = false;
						}
						else {
							Log("Invalid value");
							return;
						}

						break;
					}
					default: assert(0);
				}

				return;
			}
		}

		Log("Variable '%s' does not exist", argv[0]);
	}
	else {
		Log("Invalid command, available variants:");
		Log("  set            -- Show all variables");
		Log("  set NAME       -- Show value of variable NAME");
		Log("  set NAME VALUE -- Set variable NAME to VALUE");
	}
}

static void Command_Help(size_t argc, char** argv) {
	ASSERT_ARGC(0);

	for (size_t i = 0; i < console.cmdsLen; ++ i) {
		Log("%p %s", console.cmds[i].name, console.cmds[i].name);
	}
}

static void Command_Exit(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	app.running = false;
}

static void Command_Peak(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	Log("                                                                 #####");
	Log("                                                                #######");
	Log("                   #                                            ##O#O##");
	Log("  ######          ###                                           #VVVVV#");
	Log("    ##             #                                          ##  VVV  ##");
	Log("    ##         ###    ### ####   ###    ###  ##### #####     #          ##");
	Log("    ##        #  ##    ###    ##  ##     ##    ##   ##      #            ##");
	Log("    ##       #   ##    ##     ##  ##     ##      ###        #            ###");
	Log("    ##          ###    ##     ##  ##     ##      ###       QQ#           ##Q");
	Log("    ##       # ###     ##     ##  ##     ##     ## ##    QQQQQQ#       #QQQQQQ");
	Log("    ##      ## ### #   ##     ##  ###   ###    ##   ##   QQQQQQQ#     #QQQQQQQ");
	Log("  ############  ###   ####   ####   #### ### ##### #####   QQQQQ#######QQQQQ");
}

static void Command_Run(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	if (!Console_RunFile(argv[0])) {
		Log("Failed to run '%s'", argv[0]);
	}
}

static void Command_Comment(size_t argc, char** argv) {
	(void) argc;
	(void) argv;
}

static void Command_ParseTest(size_t argc, char** argv) {
	for (size_t i = 0; i < argc; ++ i) {
		Log("%s", argv[i]);
	}
}

static void Command_HexDump(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	size_t size;
	void*  contents = Resources_ReadFile(argv[0], &size);

	if (contents == NULL) {
		Log("Error reading file");
		return;
	}

	contents = SafeRealloc(contents, size + 1);
	((char*) contents)[size] = 0;

	for (size_t i = 0; i < size; ++ i) {
		Log("%.8X: %.2X", i, ((uint8_t*) contents)[i]);
	}
}

static void Command_Music(size_t argc, char** argv) {
	if (argc == 0) {
		if (!Audio_MusicPlaying()) {
			Log("Music is already stopped");
			return;
		}

		Audio_StopMusic();
	}
	else if (argc == 1) {
		if (!Audio_PlayMusic(argv[0])) {
			Log("Failed to play music");
		}
	}
	else {
		Log("Command requires 0 or 1 parameters\n");
	}
}

static void Command_TestScene(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	Log("Starting test scene");

	SceneManager_AddScene(TestScene());
	app.console = false;
}

static void Command_Editor(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	Log("Starting map editor");

	SceneManager_AddScene(MapEditorScene());
	app.console = false;
}

void Commands_Init(void) {
	Console_AddCommand((ConsoleCommand) {true,  "test-map",     &Command_Test});
	Console_AddCommand((ConsoleCommand) {true,  "clear-scenes", &Command_ClearScenes});
	Console_AddCommand((ConsoleCommand) {true,  "map",          &Command_Map});
	Console_AddCommand((ConsoleCommand) {true,  "dl-map",       &Command_DlMap});
	Console_AddCommand((ConsoleCommand) {true,  "ls",           &Command_Ls});
	Console_AddCommand((ConsoleCommand) {true,  "cat",          &Command_Cat});
	Console_AddCommand((ConsoleCommand) {true,  "echo",         &Command_Echo});
	Console_AddCommand((ConsoleCommand) {true,  "resources",    &Command_Resources});
	Console_AddCommand((ConsoleCommand) {true,  "set",          &Command_Set});
	Console_AddCommand((ConsoleCommand) {true,  "help",         &Command_Help});
	Console_AddCommand((ConsoleCommand) {true,  "exit",         &Command_Exit});
	Console_AddCommand((ConsoleCommand) {true,  "peak",         &Command_Peak});
	Console_AddCommand((ConsoleCommand) {true,  "run",          &Command_Run});
	Console_AddCommand((ConsoleCommand) {true,  "#",            &Command_Comment});
	Console_AddCommand((ConsoleCommand) {false, "parse-test",   &Command_ParseTest});
	Console_AddCommand((ConsoleCommand) {true,  "hex-dump",     &Command_HexDump});
	Console_AddCommand((ConsoleCommand) {true,  "music",        &Command_Music});
	Console_AddCommand((ConsoleCommand) {true,  "test-scene",   &Command_TestScene});
	Console_AddCommand((ConsoleCommand) {true,  "editor",       &Command_Editor});
}
