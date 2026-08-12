#include "data.h"

uint16_t Data_Read16(uint8_t* data) {
	return
		((uint16_t) data[0]) |
		(((uint16_t) data[1]) << 8);
}

uint32_t Data_Read32(uint8_t* data) {
	return
		((uint32_t) data[0]) |
		(((uint32_t) data[1]) << 8) |
		(((uint32_t) data[2]) << 16) |
		(((uint32_t) data[3]) << 24);
}

float Data_ReadFloat(uint8_t* data) {
	uint32_t bytes = Data_Read32(data);

	return *((float*) &bytes);
}

void Data_Write16(uint8_t* data, uint16_t value) {
	data[0] = (uint8_t) (value & 0xFF);
	data[1] = (uint8_t) ((value & 0xFF00) >> 8);
}

void Data_Write32(uint8_t* data, uint32_t value) {
	data[0] = (uint8_t) (value & 0xFF);
	data[1] = (uint8_t) ((value & 0xFF00) >> 8);
	data[2] = (uint8_t) ((value & 0xFF0000) >> 16);
	data[3] = (uint8_t) ((value & 0xFF000000) >> 24);
}

void Data_WriteFloat(uint8_t* data, float value) {
	uint32_t bytes = *((uint32_t*) &value);

	Data_Write32(data, bytes);
}

uint16_t Data_Swap16(uint16_t value) {
	return ((value & 0xFF00) >> 8) | ((value & 0xFF) << 8);
}

uint32_t Data_Swap32(uint32_t value) {
	return
		((value & 0xFF000000) >> 24) |
		((value & 0x00FF0000) >> 8)  |
		((value & 0x0000FF00) << 8)  |
		((value & 0x000000FF) << 24);
}

float Data_SwapFloat(float value) {
	uint32_t res = Data_Swap32(*((uint32_t*) &value));
	return *((float*) &res);
}
