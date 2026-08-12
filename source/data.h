#ifndef AE_DATA_H
#define AE_DATA_H

#include "common.h"

uint16_t Data_Read16(uint8_t* data);
uint32_t Data_Read32(uint8_t* data);
float    Data_ReadFloat(uint8_t* data);
void     Data_Write16(uint8_t* data, uint16_t value);
void     Data_Write32(uint8_t* data, uint32_t value);
void     Data_WriteFloat(uint8_t* data, float value);
uint16_t Data_Swap16(uint16_t value);
uint32_t Data_Swap32(uint32_t value);
float    Data_SwapFloat(float value);

#ifdef AE_BIG_ENDIAN
	#define AE_SWAP_16(V)    Data_Swap16(V)
	#define AE_SWAP_32(V)    Data_Swap32(V)
	#define AE_SWAP_FLOAT(V) Data_SwapFloat(V)
#else
	#define AE_SWAP_16(V)    (V)
	#define AE_SWAP_32(V)    (V)
	#define AE_SWAP_FLOAT(V) (V)
#endif

#endif
