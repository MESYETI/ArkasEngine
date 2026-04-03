#include "fs.h"
#include "event.h"
#include "engine.h"
#include "console.h"
#include "mapEditor.h"

#ifdef AE_STANDALONE
int main(int argc, const char** argv) {
	Engine_Init("Arkas Engine (standalone)", argc, argv);
	engine.console = false;

	if (Console_RunFile(AE_LOCATION "game.cmd")) {
		Log("Ran game.cmd");
	}

	while (engine.running) {
		Engine_Update();
	}
	Engine_Free();
}
#endif
