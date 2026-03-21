#ifndef AE_VARIABLES_H
#define AE_VARIABLES_H

#include "common.h"

enum {
	VAR_INT,
	VAR_FLOAT,
	VAR_BOOL,
	VAR_STR
};

typedef struct {
	int         type;
	const char* name;
	void*       ptr;
	size_t      size;
} Variable;

extern Variable* vars;
extern size_t    varsAmount;

void      Variables_Free(void);
void      Variables_Add(int type, const char* name, void* ptr, size_t size);
Variable* Variables_Get(const char* name);

#endif
