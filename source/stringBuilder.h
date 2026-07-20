#ifndef AE_STRING_BUILDER_H
#define AE_STRING_BUILDER_H

#include "common.h"

typedef struct {
	char*  str;
	size_t cap;
	size_t len;
} StrBuild;

StrBuild StrBuild_New(void);
void     StrBuild_Add(StrBuild* this, const char* str);

#endif
