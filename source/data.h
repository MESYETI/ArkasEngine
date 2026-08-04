#ifndef AE_DATA_H
#define AE_DATA_H

#include "common.h"

uint16_t Data_Read16(uint8_t* data);
uint32_t Data_Read32(uint8_t* data);
float    Data_ReadFloat(uint8_t* data);
void     Data_Write16(uint8_t* data, uint16_t value);
void     Data_Write32(uint8_t* data, uint32_t value);
void     Data_WriteFloat(uint8_t* data, float value);

#endif
