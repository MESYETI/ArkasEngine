#ifndef AE_FILE_H
#define AE_FILE_H

#include <stdio.h>
#include "common.h"

uint8_t  File_ReadByte(FILE* file);
uint32_t File_Read32Bit(FILE* file);
float    File_ReadFloat(FILE* file);
char*    File_ReadString(FILE* file);
void     File_WriteByte(FILE* file, uint8_t byte);
void     File_Write32Bit(FILE* file, uint32_t value);
void     File_WriteFloat(FILE* file, float value);

#endif
