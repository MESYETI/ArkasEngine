#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "stream.h"

void Stream_Close(Stream* stream) {
	if (!stream->close) return;

	stream->close(stream);
}

size_t Stream_Write(Stream* stream, size_t size, void* data) {
	if (!stream->write) return 0;

	return stream->write(stream, size, data);
}

size_t Stream_Read(Stream* stream, size_t size, void* dest) {
	return stream->read(stream, size, dest);
}

size_t Stream_Peek(Stream* stream) {
	return stream->peek(stream);
}

bool Stream_Seek(Stream* stream, size_t where) {
	return stream->seek(stream, where);
}

size_t Stream_Size(Stream* stream) {
	return stream->size(stream);
}

bool Stream_Advance(Stream* stream, size_t bytes) {
	return Stream_Seek(stream, Stream_Peek(stream) + bytes);
}

/*
// TEMPLATE
static size_t Read(Stream* stream, size_t size, void* dest) {
	TYPE* data = (TYPE*) stream->data;
}
static size_t Peek(Stream* stream) {
	TYPE* data = (TYPE*) stream->data;
}

static bool Seek(Stream* stream, size_t where) {
	TYPE* data = (TYPE*) stream->data;
}

static size_t Size(Stream* stream) {
	TYPE* data = (TYPE*) stream->data;
}
*/

typedef struct {
	FILE* file;
} FileStream;

static void FileClose(Stream* stream) {
	FileStream* data = (FileStream*) stream->data;
	fclose(data->file);
}

static size_t FileWrite(Stream* p_stream, size_t size, void* data) {
	FileStream* stream = (FileStream*) p_stream->data;

	return fwrite(data, 1, size, stream->file);
}

static size_t FileRead(Stream* stream, size_t size, void* dest) {
	FileStream* data = (FileStream*) stream->data;

	return fread(dest, 1, size, data->file);
}

static size_t FilePeek(Stream* stream) {
	FileStream* data = (FileStream*) stream->data;

	return (size_t) ftell(data->file);
}

static bool FileSeek(Stream* stream, size_t where) {
	FileStream* data = (FileStream*) stream->data;

	return fseek(data->file, (long) where, SEEK_SET) == 0;
}

static size_t FileSize(Stream* stream) {
	FileStream* data = (FileStream*) stream->data;

	long oldPos = ftell(data->file);
	fseek(data->file, 0, SEEK_END);

	size_t ret = (size_t) ftell(data->file);
	fseek(data->file, oldPos, SEEK_SET);

	return ret;
}

Stream Stream_File(FILE* file, bool close) {
	FileStream* data = SafeMalloc(sizeof(data));

	data->file = file;

	return (Stream) {
		.data = data,
		.close = close? &FileClose : NULL,
		.write = &FileWrite,
		.read  = &FileRead,
		.peek  = &FilePeek,
		.seek  = &FileSeek,
		.size  = &FileSize
	};
}


typedef struct {
	uint8_t* data;
	size_t   offset;
	size_t   size;
} MemStream;

static void MemClose(Stream* stream) {
	MemStream* data = (MemStream*) stream->data;

	free(data->data);
}

static size_t MemRead(Stream* stream, size_t size, void* dest) {
	MemStream* data = (MemStream*) stream->data;

	if (data->offset + size > data->size) {
		size = data->size - data->offset;
	}
	if (data->offset >= data->size) {
		return 0;
	}

	memcpy(dest, &data->data[data->offset], size);

	data->offset += size;

	return size;
}

static size_t MemPeek(Stream* stream) {
	MemStream* data = (MemStream*) stream->data;

	return data->offset;
}

static bool MemSeek(Stream* stream, size_t where) {
	MemStream* data = (MemStream*) stream->data;

	data->offset = where;

	return data->offset <= data->size;
}

static size_t MemSize(Stream* stream) {
	MemStream* data = (MemStream*) stream->data;

	return data->size;
}

Stream Stream_Memory(void* addr, size_t size, bool free) {
	MemStream* data = SafeMalloc(sizeof(data));

	data->data   = addr;
	data->offset = 0;
	data->size   = size;

	return (Stream) {
		.data = data,
		.close = free? &MemClose : NULL,
		.write = NULL,
		.read  = &MemRead,
		.peek  = &MemPeek,
		.seek  = &MemSeek,
		.size  = &MemSize
	};
}

uint8_t Stream_Read8(Stream* stream) {
	uint8_t ret;

	assert(Stream_Read(stream, 1, &ret) == 1);

	return ret;
}

uint16_t Stream_Read16(Stream* stream) {
	uint16_t ret;
	uint8_t  bytes[2];

	assert(Stream_Read(stream, 2, &bytes) == 2);

	ret  = bytes[0];
	ret |= ((uint16_t) bytes[1]) << 8;

	return ret;
}

uint32_t Stream_Read32(Stream* stream) {
	uint32_t ret;
	uint8_t  bytes[4];

	assert(Stream_Read(stream, 4, &bytes) == 4);

	ret  = bytes[0];
	ret |= ((uint32_t) bytes[1]) << 8;
	ret |= ((uint32_t) bytes[2]) << 16;
	ret |= ((uint32_t) bytes[3]) << 24;

	return ret;
}

float Stream_ReadFloat(Stream* stream) {
	uint32_t ret;
	uint8_t  bytes[4];

	assert(Stream_Read(stream, 4, &bytes) == 4);

	ret  = bytes[0];
	ret |= ((uint32_t) bytes[1]) << 8;
	ret |= ((uint32_t) bytes[2]) << 16;
	ret |= ((uint32_t) bytes[3]) << 24;

	return *(float*) &ret;
}

char* Stream_ReadString(Stream* stream) {
	uint32_t length = Stream_Read32(stream);
	char*    ret    = SafeMalloc(length + 1);
	ret[length]     = 0;

	assert(Stream_Read(stream, length, ret) == length);
	return ret;
}

char* Stream_ReadNTString(Stream* stream) {
	long here = Stream_Peek(stream);
	assert(here >= 0);

	size_t length = 0;
	while (true) {
		char ch;
		assert(Stream_Read(stream, 1, &ch) == 1);

		if (ch == 0) break;
		++ length;
	}

	Stream_Seek(stream, here);
	char* ret = SafeMalloc(length + 1);
	assert(Stream_Read(stream, length, ret) == length);
	ret[length] = 0;

	return ret;
}

void Stream_Write8(Stream* stream, uint8_t byte) {
	Stream_Write(stream, 1, &byte);
}

void Stream_Write16(Stream* stream, uint16_t value) {
	uint8_t bytes[4];
	bytes[0] = (uint8_t) (value & 0xFF);
	bytes[1] = (uint8_t) ((value & 0xFF00) >> 8);

	Stream_Write(stream, 2, &bytes);
}

void Stream_Write32(Stream* stream, uint32_t value) {
	uint8_t bytes[4];
	bytes[0] = (uint8_t) (value & 0xFF);
	bytes[1] = (uint8_t) ((value & 0xFF00)     >> 8);
	bytes[2] = (uint8_t) ((value & 0xFF0000)   >> 16);
	bytes[3] = (uint8_t) ((value & 0xFF000000) >> 24);

	Stream_Write(stream, 4, &bytes);
}

void Stream_WriteFloat(Stream* stream, float value) {
	Stream_Write32(stream, *((uint32_t*) &value));
}

void Stream_WriteString(Stream* stream, const char* string) {
	size_t len = strlen(string);

	Stream_Write32(stream, (uint32_t) len);
	// assert(fwrite(string, 1, len, stream) == len);
	assert(Stream_Write(stream, len, (void*) string) == len);
}

