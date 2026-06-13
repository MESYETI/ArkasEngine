#include "fs.h"
#include "model.h"
#include "event.h"
#include "engine.h"
#include "console.h"
#include "mapEditor.h"

#ifdef AE_STANDALONE
	int main(int argc, const char** argv) {
		Engine_Init("Arkas Engine (standalone)", argc, argv);
		engine.console = true;

		if (Console_RunFile(AE_LOCATION "game.cmd")) {
			Log("Ran game.cmd");
		}

		ModelRenderOpt opt;
		opt.scale = 0.4f;
		opt.pos   = (FVec3) {0.2f, -0.3f, 0.4f};
		opt.rot   = 0.0f;
		Backend_UseHoldModel("base:models/yeti/fly-swatter.zkm", opt);

		while (engine.running) {
			Engine_Update();
		}
		Engine_Free();
	}
#endif
