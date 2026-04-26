#include "map.h"
#include "mem.h"
#include "game.h"
#include "util.h"
#include "audio.h"
#include "scene.h"
#include "entity.h"
#include "client.h"
#include "config.h"
#include "engine.h"
#include "entity.h"
#include "server.h"
#include "skybox.h"
#include "player.h"
#include "console.h"
#include "commands.h"
#include "ramDrive.h"
#include "resources.h"
#include "variables.h"
#include "testScene.h"
#include "mapEditor.h"
#include "messageBox.h"
#include "fileBrowser.h"
#include "imageViewer.h"
#include "splashScreen.h"

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
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL,
		NULL, NULL
	});
	Map_LoadTest();
	engine.console = false;
}

static void Command_Test2(size_t argc, char** argv) {
	ASSERT_ARGC(0);

	Log("Starting map viewer");

	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL,
		NULL, NULL
	});
	Map_LoadTest2();
	engine.console = false;
}

static void Command_ClearScenes(size_t argc, char** argv) {
	ASSERT_ARGC(0);

	SceneManager_Free();
	Log("Scenes cleared");
}

static void Command_Map(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	if (server.running) {
		Server_SetMap(argv[0]);
	}
	else {
		char* path1 = ConcatString("maps:", argv[0]);
		char* path2 = ConcatString(path1,   ".arm");
		free(path1);

		SceneManager_AddScene((Scene) {
			SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL,
			NULL, NULL
		});

		size_t size;
		void*  res = Resources_ReadFile(path2, &size);

		if (!res) {
			Log("Failed to load map");
			SceneManager_PopScene();
			return;
		}

		Stream stream = Stream_Memory(res, size, true);

		if (!Map_LoadFile(&stream, path2)) {
			Log("Failed to load map");
			SceneManager_PopScene();
			return;
		}

		Stream_Close(&stream);

		free(path2);
		engine.console = false;
	}
}

static void Command_DlMap(size_t argc, char** argv) {
	if (argc > 1) {
		Log("dl-map must either have 1 parameter (map name) or none");
	}

	char* path1 = ConcatString("maps/", argc? argv[0] : map.name);
	char* path2 = ConcatString(path1,   ".arm");
	free(path1);

	FILE* file = fopen(path2, "wb+");
	free(path2);

	if (!file) {
		Log("Failed to save map");
		return;
	}

	Stream stream = Stream_File(file, true);
	if (!Map_SaveFile(&stream)) {
		Log("Failed to save map");
	}
}

static void Command_Ls(size_t argc, char** argv) {
	if (argc == 0) {
		Resources_PrintList(NULL);
	}
	else if (argc == 1) {
		Resources_PrintList(argv[0]);
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

static void Command_Set(size_t argc, char** argv) {
	if (argc == 0) {
		for (size_t i = 0; i < varsAmount; ++ i) {
			switch (vars[i].type) {
				case VAR_INT:   Log("int:    %s", vars[i].name); break;
				case VAR_FLOAT: Log("float:  %s", vars[i].name); break;
				case VAR_BOOL:  Log("bool:   %s", vars[i].name); break;
				case VAR_STR:   Log("string: %s[%d]", vars[i].name, vars[i].size); break;
				default:        Log("???:    %s", vars[i].name); break;
			}
		}
	}
	else if (argc == 1) {
		Variable* var = Variables_Get(argv[0]);

		if (!var) {
			Log("Variable '%s' does not exist", argv[0]);
			return;
		}

		switch (var->type) {
			case VAR_INT: {
				Log("%s = %d", argv[0], *((int*) var->ptr));
				break;
			}
			case VAR_FLOAT: {
				Log("%s = %g", argv[0], *((float*) var->ptr));
				break;
			}
			case VAR_BOOL: {
				Log(
					"%s = %s", argv[0],
					(*((bool*) var->ptr))? "true" : "false"
				);
				break;
			}
			case VAR_STR: {
				Log("%s = %s", argv[0], var->ptr);
				break;
			}
			default: assert(0);
		}
	}
	else if (argc == 2) {
		Variable* var = Variables_Get(argv[0]);

		if (!var) {
			Log("Variable '%s' does not exist", argv[0]);
			return;
		}

		switch (var->type) {
			case VAR_INT: {
				*((int*) var->ptr) = atoi(argv[1]);
				break;
			}
			case VAR_FLOAT: {
				*((float*) var->ptr) = (float) atof(argv[1]);
				break;
			}
			case VAR_BOOL: {
				if (strcmp(argv[1], "true") == 0) {
					*((bool*) var->ptr) = true;
				}
				else if (strcmp(argv[1], "false") == 0) {
					*((bool*) var->ptr) = false;
				}
				else {
					Log("Invalid value");
					return;
				}

				break;
			}
			case VAR_STR: {
				strncpy(var->ptr, argv[1], var->size);
				break;
			}
			default: assert(0);
		}
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
	engine.running = false;
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
		if (!Audio_PlayMusic(argv[0], true)) {
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
	engine.console = false;
}

static void Command_Editor(size_t argc, char** argv) {
	(void) argc;
	(void) argv;

	Log("Starting map editor");

	SceneManager_AddScene(MapEditor_Scene());
	engine.console = false;
}

typedef struct {
	const char*   name;
	Input_BindID* bind;
} Bind;

static void Command_Bind(size_t argc, char** argv) {
	static const Bind binds[] = {
		{"game.forward",  &gameBaseConfig.forward},
		{"game.left",     &gameBaseConfig.left},
		{"game.backward", &gameBaseConfig.backward},
		{"game.right",    &gameBaseConfig.right},
		{"game.jump",     &gameBaseConfig.jump}
	};

	if (argc == 0) {
		puts("Available binds");

		for (size_t i = 0; i < sizeof(binds) / sizeof(Bind); ++ i) {
			Log("- %s", binds[i].name);
		}

		return;
	}

	if (argc < 2) {
		Log("Command must have these parameters: ACTION [MOD1 MOD2 MOD3] KEY");
		return;
	}

	Key mod[3] = {0, 0, 0};

	if (argc > 2) mod[0] = Key_FromString(argv[1]);
	if (argc > 3) mod[1] = Key_FromString(argv[2]);
	if (argc > 4) mod[2] = Key_FromString(argv[3]);

	Key key = Key_FromString(argv[argc - 1]);

	const char*   bindName;
	Input_BindID* bind = NULL;
	for (size_t i = 0; i < sizeof(binds) / sizeof(Bind); ++ i) {
		if (strcmp(binds[i].name, argv[0]) == 0) {
			bindName = binds[i].name;
			bind     = binds[i].bind;
		}
	}

	if (!bind) {
		Log("Couldn't find action '%s'", argv[0]);
		return;
	}

	*bind = Input_AddKeyBind(mod, key);

	char bindStr[32];
	Input_PrintBind(bindStr, sizeof(bindStr), *bind);

	Log("Bound '%s' to %s", bindName, bindStr);
}

static void Command_ImageViewer(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	Log("Viewing image '%s'", argv[0]);

	Resource* res = Resources_GetRes(argv[0], 0);

	if (!res) {
		Log("Failed to load image viewer");
		return;
	}

	if (res->type != RESOURCE_TYPE_TEXTURE) {
		Log("Not an image");
		return;
	}

	SceneManager_AddScene(ImageViewerScene(argv[0]));

	Resources_FreeRes(res);
	engine.console = false;
}

static void Command_FPS(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	(void) argv;

	Log("FPS: %d", engine.fps);
}

static void Command_Browser(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	(void) argv;

	SceneManager_AddScene(FileBrowserScene());
	engine.console = false;
}

static void Command_Skybox(size_t argc, char** argv) {
	ASSERT_ARGC(1);
	Skybox_Load(argv[0]);
}

static void Command_StartServer(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	if (!Server_Start()) {
		Log("ERROR! Failed to start server");
	}
}

static void Command_StartLocalClient(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	if (!Client_StartLocal()) {
		Log("ERROR! Failed to start client");
	}
}

static void Command_StartNetClient(size_t argc, char** argv) {
	ASSERT_ARGC(2);

	if (!Client_StartINet(argv[0], (uint16_t) atoi(argv[1]))) {
		Log("ERROR! Failed to start client");
	}
}

static void Command_RamDrive(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	if (!Resources_AddDrive(NewRamDrive(), argv[0])) {
		Log("Failed to add RAM drive");
	}
}

static void Command_MakeDir(size_t argc, char** argv) {
	ASSERT_ARGC(1);
	if (!Resources_MakeDir(argv[0])) {
		Log("Failed to make directory");
	}
}

static void Command_Touch(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	if (!Resources_WriteFile(argv[0], NULL, 0)) {
		Log("Failed to create file");
	}
}

static void Command_Delete(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	if (!Resources_Delete(argv[0])) {
		Log("Failed to delete file/folder");
	}
}

static void Command_Unmount(size_t argc, char** argv) {
	ASSERT_ARGC(1);

	if (!Resources_DeleteDrive(argv[0])) {
		Log("Failed to unmount drive");
	}
}

static void Command_ViewMap(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	SceneManager_AddScene((Scene) {
		SCENE_TYPE_GAME, NULL, "Map Viewer", NULL, NULL, NULL, NULL,
		NULL, NULL
	});
	engine.console = false;
}

static void DefaultSplashCallback(void) {
	SceneManager_PopScene();
	engine.console = true;
}

static void Command_Splash(size_t argc, char** argv) {
	ASSERT_ARGC(1);
	SceneManager_AddScene(NewSplashScreen(argv[0], &DefaultSplashCallback, 5.0));
	engine.console = false;
}

static void Command_SaveConfig(size_t argc, char** argv) {
	ASSERT_ARGC(0);
	SaveConfig();
}

static void Command_SpawnProp(size_t argc, char** argv) {
	const char* path;
	float       scale = 1.0f;

	if (argc == 0) {
		path = "base:models/pq/generic_person.zkm";
	}
	else if (argc == 1) {
		path = argv[0];
	}
	else if (argc == 2) {
		path  = argv[0];
		scale = (float) atof(argv[1]);
	}
	else {
		Log("spawn-prop only accepts 0, 1, or 2 arguments");
		return;
	}

	Entity* entity = PropEntity_New(
		player.sector, player.pos, (Direction) {
			player.pitch, player.yaw, 0.0f
		}, Resources_GetRes(path, 0), false
	);
	entity->modelScale = scale;

	Map_AddEntity(entity);
}

static void Command_Message(size_t argc, char** argv) {
	ASSERT_ARGC(2);

	SceneManager_AddScene(NewMessageBoxScene(argv[0], argv[1]));
}

void Commands_Init(void) {
	Console_AddCommand(true,  "test-map",           &Command_Test);
	Console_AddCommand(true,  "test-map2",          &Command_Test2);
	Console_AddCommand(true,  "clear-scenes",       &Command_ClearScenes);
	Console_AddCommand(true,  "map",                &Command_Map);
	Console_AddCommand(true,  "dl-map",             &Command_DlMap);
	Console_AddCommand(true,  "ls",                 &Command_Ls);
	Console_AddCommand(true,  "cat",                &Command_Cat);
	Console_AddCommand(true,  "echo",               &Command_Echo);
	Console_AddCommand(true,  "resources",          &Command_Resources);
	Console_AddCommand(true,  "set",                &Command_Set);
	Console_AddCommand(true,  "help",               &Command_Help);
	Console_AddCommand(true,  "exit",               &Command_Exit);
	Console_AddCommand(true,  "peak",               &Command_Peak);
	Console_AddCommand(true,  "run",                &Command_Run);
	Console_AddCommand(true,  "#",                  &Command_Comment);
	Console_AddCommand(false, "parse-test",         &Command_ParseTest);
	Console_AddCommand(true,  "hex-dump",           &Command_HexDump);
	Console_AddCommand(true,  "music",              &Command_Music);
	Console_AddCommand(true,  "test-scene",         &Command_TestScene);
	Console_AddCommand(true,  "editor",             &Command_Editor);
	Console_AddCommand(true,  "bind",               &Command_Bind);
	Console_AddCommand(true,  "image-viewer",       &Command_ImageViewer);
	Console_AddCommand(true,  "fps",                &Command_FPS);
	Console_AddCommand(false, "browser",            &Command_Browser);
	Console_AddCommand(true,  "skybox",             &Command_Skybox);
	Console_AddCommand(true,  "start-server",       &Command_StartServer);
	Console_AddCommand(true,  "start-local-client", &Command_StartLocalClient);
	Console_AddCommand(true,  "start-net-client",   &Command_StartNetClient);
	Console_AddCommand(true,  "ram-drive",          &Command_RamDrive);
	Console_AddCommand(true,  "make-dir",           &Command_MakeDir);
	Console_AddCommand(true,  "touch",              &Command_Touch);
	Console_AddCommand(true,  "delete",             &Command_Delete);
	Console_AddCommand(true,  "unmount",            &Command_Unmount);
	Console_AddCommand(true,  "view-map",           &Command_ViewMap);
	Console_AddCommand(true,  "splash",             &Command_Splash);
	Console_AddCommand(true,  "save-config",        &Command_SaveConfig);
	Console_AddCommand(true,  "spawn-prop",         &Command_SpawnProp);
	Console_AddCommand(true,  "message",            &Command_Message);
}
