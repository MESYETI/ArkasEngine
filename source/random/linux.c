#include <string.h>
#include "../util.h"
#include "../random.h"

#ifdef AE_RANDOM_LINUX
#include <errno.h>
#include <sys/random.h>

void Random_Init(void) {
	
}

uint64_t Random_Gen(void) {
	uint8_t res[8];

	if (getrandom(res, 8, GRND_RANDOM) < 0) {
		Error("getrandom failed: %s", strerror(errno));
	}

	return *((uint64_t*) res);
}

#endif
