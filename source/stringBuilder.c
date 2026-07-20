#include "mem.h"
#include "stringBuilder.h"

StrBuild StrBuild_New(void) {
	StrBuild ret;
	ret.str    = SafeMalloc(8);
	ret.str[0] = 0;
	ret.cap    = 8;
	ret.len    = 0;
	return ret;
}

void StrBuild_Add(StrBuild* this, const char* str) {
	(void) this;
	(void) str; // TODO
}
