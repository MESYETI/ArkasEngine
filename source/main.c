#include "app.h"

int main(void) {
	App_Init();
	while (app.running) {
		App_Update();
	}
	App_Free();
}
