#include "engine.h"

#ifdef AE_STANDALONE
int main(int argc, const char** argv) {
	Engine_Init("Arkas Engine (standalone)", argc, argv);
	engine.console = true;

	while (engine.running) {
		Engine_Update();
	}
	Engine_Free();
}
#endif
