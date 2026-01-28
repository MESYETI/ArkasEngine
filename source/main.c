#include "engine.h"

#ifdef AE_STANDALONE
int main(void) {
	Engine_Init("Arkas Engine (standalone)");
	engine.console = true;

	while (engine.running) {
		Engine_Update();
	}
	Engine_Free();
}
#endif
