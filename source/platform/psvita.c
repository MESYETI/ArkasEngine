#include "../fs.h"
#include "../platform.h"

#ifdef PLATFORM_PSVITA
void Platform_Init(void) {
	MakeDir("ux0:data", true);
	MakeDir("ux0:data/ArkasEngine", true);
}
#endif
