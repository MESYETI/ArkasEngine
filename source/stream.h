#ifndef AE_STREAM_H
#define AE_STREAM_H

#include "common.h"

typedef struct Stream Stream;

struct Stream {
	void* data;

	void   (*close)(Stream* stream);
	size_t (*write)(Stream* stream, size_t size, void* data);
	size_t (*read)(Stream* stream, size_t size, void* dest);
	size_t (*peek)(Stream* stream);
	bool   (*seek)(Stream* stream, size_t where);
	size_t (*size)(Stream* stream);
};

// stream functions
void   Stream_Close(Stream* stream);
size_t Stream_Write(Stream* stream, size_t size, void* data);
size_t Stream_Read(Stream* stream, size_t size, void* dest);
size_t Stream_Peek(Stream* stream);
bool   Stream_Seek(Stream* stream, size_t where);
size_t Stream_Size(Stream* stream);

// different types of stream
Stream Stream_File(FILE* file, bool close);
Stream Stream_Memory(void* addr, size_t size, bool free);

// stream utils
uint8_t  Stream_Read8(Stream* stream);
uint16_t Stream_Read16(Stream* stream);
uint32_t Stream_Read32(Stream* stream);
float    Stream_ReadFloat(Stream* stream);
char*    Stream_ReadString(Stream* stream);
char*    Stream_ReadNTString(Stream* stream);
void     Stream_Write8(Stream* stream, uint8_t byte);
void     Stream_Write16(Stream* stream, uint16_t value);
void     Stream_Write32(Stream* stream, uint32_t value);
void     Stream_WriteFloat(Stream* stream, float value);
void     Stream_WriteString(Stream* stream, const char* string);

#endif
