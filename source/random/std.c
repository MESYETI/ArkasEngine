#include "../util.h"
#include "../platform.h"

#ifdef AE_RANDOM_STD

void Random_Init(void) {
	srand(Platform_GetTime());
}

uint64_t Random_Gen(void) {
	uint64_t low  = (uint64_t) rand();
	uint64_t high = (uint64_t) rand();

	return low | (high << 32);
}

#endif
