#ifndef AE_UTIL_H
#define AE_UTIL_H

#include <math.h>
#include <stdio.h>
#include "types.h"

#define FUNCTION_POINTER(TYPE, NAME, ...) TYPE (*NAME)(__VA_ARGS__)
#define MIN(A, B) (((A) < (B))? (A) : (B))
#define MAX(A, B) (((A) > (B))? (A) : (B))

bool   DirExists(const char* path);
float  RadToDeg(float rad);
float  DegToRad(float deg);
float  Lerp(float a, float b, float t);
float  CosDeg(float deg);
float  SinDeg(float deg);
float  Distance(FVec2 a, FVec2 b);
char*  NewString(const char* src);
char*  ConcatString(const char* first, const char* second);
size_t StrArrayLength(char** array);
char** AppendStrArray(char** array, char* string);
void   FreeStrArray(char** array);
void   Log(const char* format, ...);
void   Error(const char* format, ...);

#endif
