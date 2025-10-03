#include <stdio.h>
#include "mem.h"
#include "common.h"

void* SafeMalloc(size_t size) {
	void* ret = malloc(size);

	if ((ret == NULL) && (size != 0)) {
		fprintf(stderr, "Malloc returned NULL\n");
		exit(1);
	}

	return ret;
}

void* SafeRealloc(void* ptr, size_t size) {
	void* ret = realloc(ptr, size);

	if ((ret == NULL) && (size > 0)) {
		fprintf(stderr, "Malloc returned NULL\n");
		exit(1);
	}

	return ret;
}
