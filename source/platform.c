#include "fs.h"
#include "common.h"
#include "platform.h"

void Platform_Init(void) {
	#ifdef AE_PLATFORM_PSVITA
		MakeDir("ux0:data", true);
		MakeDir("ux0:data/ArkasEngine", true);
	#endif
}
