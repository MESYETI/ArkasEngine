#ifndef AE_UTIL_H
#define AE_UTIL_H

#include <math.h>
#include <stdio.h>
#include "types.h"

#define FUNCTION_POINTER(TYPE, NAME, ...) TYPE (*NAME)(__VA_ARGS__)
#define MIN(A, B) (((A) < (B))? (A) : (B))
#define MAX(A, B) (((A) > (B))? (A) : (B))

#define SWAP(TYPE, A, B) \
	do { \
		TYPE* macro##a   = &(A); \
		TYPE* macro##b   = &(B); \
		TYPE  macro##tmp = *macro##a; \
		*macro##a = *macro##b; \
		*macro##b = macro##tmp; \
	} while (0);

bool   FileExists(const char* path);
float  RadToDeg(float rad);
float  DegToRad(float deg);
float  Lerp(float a, float b, float t);
float  CosDeg(float deg);
float  SinDeg(float deg);
bool   FloatEqual(float a, float b, float margin);
float  Distance(FVec2 a, FVec2 b);
float  PointLineSide(FVec2 p, FVec2 a, FVec2 b);
FVec2  LineIntersect(FVec2 a1, FVec2 a2, FVec2 b1, FVec2 b2);
bool   PointInLine(FVec2 p, FVec2 a, FVec2 b);
char*  NewString(const char* src);
char*  ConcatString(const char* first, const char* second);
size_t StrArrayLength(char** array);
char** AppendStrArray(char** array, char* string);
bool   StrArrayContains(char** array, char* string);
size_t StrArrayFind(char** array, char* string);
void   FreeStrArray(char** array);
void   Log(const char* format, ...);
void   Error(const char* format, ...);

const char* BaseName(const char* path);

#endif
