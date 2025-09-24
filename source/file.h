#ifndef AE_FILE_H
#define AE_FILE_H

#include <stdio.h>
#include "common.h"

uint8_t  File_Read8(FILE* file);
uint16_t File_Read16(FILE* file);
uint32_t File_Read32(FILE* file);
float    File_ReadFloat(FILE* file);
char*    File_ReadString(FILE* file);
char*    File_ReadNTString(FILE* file);
void     File_Write8(FILE* file, uint8_t byte);
void     File_Write16(FILE* file, uint16_t value);
void     File_Write32(FILE* file, uint32_t value);
void     File_WriteFloat(FILE* file, float value);
void     File_WriteString(FILE* file, const char* string);

#endif
