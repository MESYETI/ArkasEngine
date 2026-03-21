#include <string.h>
#include "mem.h"
#include "variables.h"

Variable* vars = NULL;
size_t    varsAmount = 0;

void Variables_Free(void) {
	if (vars) {
		free(vars);
	}
}

void Variables_Add(int type, const char* name, void* ptr, size_t size) {
	++ varsAmount;
	vars = SafeRealloc(vars, varsAmount * sizeof(Variable));

	vars[varsAmount - 1] = (Variable) {type, name, ptr, size};
}

Variable* Variables_Get(const char* name) {
	for (size_t i = 0; i < varsAmount; ++ i) {
		if (strcmp(vars[i].name, name) == 0) {
			return &vars[i];
		}
	}

	return NULL;
}
