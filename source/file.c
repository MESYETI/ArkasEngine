#include <assert.h>
#include "file.h"
#include "safe.h"

uint8_t File_Read8(FILE* file) {
	int ret = fgetc(file);
	assert(ret != EOF);
	return ret;
}

uint16_t File_Read16(FILE* file) {
	uint16_t ret;
	uint8_t  bytes[4];

	assert(fread(&bytes, 2, 1, file) == 1);

	ret  = bytes[0];
	ret |= ((uint16_t) bytes[1]) << 8;

	return ret;
}

uint32_t File_Read32(FILE* file) {
	uint32_t ret;
	uint8_t  bytes[4];

	assert(fread(&bytes, 4, 1, file) == 1);

	ret  = bytes[0];
	ret |= ((uint32_t) bytes[1]) << 8;
	ret |= ((uint32_t) bytes[2]) << 16;
	ret |= ((uint32_t) bytes[3]) << 24;

	return ret;
}

float File_ReadFloat(FILE* file) {
	uint32_t ret;
	uint8_t  bytes[4];

	assert(fread(&bytes, 4, 1, file) == 1);

	ret  = bytes[0];
	ret |= ((uint32_t) bytes[1]) << 8;
	ret |= ((uint32_t) bytes[2]) << 16;
	ret |= ((uint32_t) bytes[3]) << 24;

	return *(float*) &ret;
}

char* File_ReadString(FILE* file) {
	uint32_t length = File_Read32(file);
	char*    ret    = SafeMalloc(length);

	assert(fread(ret, 1, length, file) == length);
	return ret;
}

char* File_ReadNTString(FILE* file) {
	long here = ftell(file);
	assert(here >= 0);

	size_t length = 0;
	while (true) {
		char ch;
		assert(fread(&ch, 1, 1, file) == 1);

		if (ch == 0) break;
		++ length;
	}

	fseek(file, here, SEEK_SET);
	char* ret = SafeMalloc(length + 1);
	assert(fread(ret, 1, length, file) == length);
	ret[length] = 0;

	return ret;
}

void File_Write8(FILE* file, uint8_t byte) {
	fwrite(&byte, 1, 1, file);
}

void File_Write16(FILE* file, uint16_t value) {
	uint8_t bytes[4];
	bytes[0] = (uint8_t) (value & 0xFF);
	bytes[1] = (uint8_t) ((value & 0xFF00) >> 8);

	fwrite(&bytes, 1, 2, file);
}

void File_Write32(FILE* file, uint32_t value) {
	uint8_t bytes[4];
	bytes[0] = (uint8_t) (value & 0xFF);
	bytes[1] = (uint8_t) ((value & 0xFF00)     >> 8);
	bytes[2] = (uint8_t) ((value & 0xFF0000)   >> 16);
	bytes[3] = (uint8_t) ((value & 0xFF000000) >> 24);

	fwrite(&bytes, 1, 4, file);
}

void File_WriteFloat(FILE* file, float value) {
	File_Write32(file, *((uint32_t*) &value));
}
