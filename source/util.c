#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "util.h"
#include "safe.h"
#include "video.h"
#include "common.h"
#include "console.h"

float RadToDeg(float rad) {
	return rad * 180 / PI;
}

float DegToRad(float deg) {
	return deg * (PI / 180);
}

float Lerp(float a, float b, float t) {
	return a * (1.0 - t) + (b * t);
}

float CosDeg(float deg) {
	return cos(DegToRad(deg));
}

float SinDeg(float deg) {
	return sin(DegToRad(deg));
}

float Distance(FVec2 a, FVec2 b) {
	float x = (b.x - a.x) * (b.x - a.x);
	float y = (b.y - a.y) * (b.y - a.y);
	return sqrtf(x + y);
}

char* NewString(const char* src) {
	char* ret = SafeMalloc(strlen(src) + 1);
	strcpy(ret, src);
	return ret;
}

char* ConcatString(const char* first, const char* second) {
	char* ret = SafeMalloc(strlen(first) + strlen(second) + 1);
	strcpy(ret, first);
	strcat(ret, second);
	return ret;
}

size_t StrArrayLength(char** array) {
	size_t ret = 0;

	while (*array != NULL) {
		++ ret;
		++ array;
	}

	return ret;
}

char** AppendStrArray(char** array, char* string) {
	size_t len = StrArrayLength(array);

	array          = SafeRealloc(array, (len + 2) * sizeof(char*));
	array[len]     = string;
	array[len + 1] = NULL;
	return array;
}

void FreeStrArray(char** array) {
	for (size_t i = 0; array[i] != NULL; ++ i) {
		free(array[i]);
	}

	free(array);
}

// most of this is taken from vsprintf(3)
#define FMT(RET, FORMAT) do { \
	int     n    = 0; \
	size_t  size = 0; \
	va_list ap; \
\
	va_start(ap, FORMAT); \
	n = vsnprintf(RET, size, FORMAT, ap); \
	va_end(ap); \
\
	assert(n >= 0); \
\
	size = n + 1; \
	RET = (char*) SafeMalloc(size); \
	if (RET == NULL) { \
		return; \
	} \
\
	va_start(ap, FORMAT); \
	n = vsnprintf(RET, size, FORMAT, ap); \
	va_end(ap); \
\
	assert(n >= 0); \
} while (0);

void Log(const char* format, ...) {
	char* ret = NULL;

	FMT(ret, format);

	time_t     rawTime;
	struct tm* tm;
	
	time(&rawTime);
	tm = localtime(&rawTime);
	
	printf("[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, ret);

	Console_WriteLine(ret);
	free(ret);
}

void Error(const char* format, ...) {
	char* ret = NULL;

	FMT(ret, format);

	time_t     rawTime;
	struct tm* tm;
	
	time(&rawTime);
	tm = localtime(&rawTime);
	
	printf("[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, ret);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", ret, NULL);
	free(ret);
	exit(1);
}
