#ifndef AE_ART_H
#define AE_ART_H

#include "common.h"
#include "stream.h"

typedef enum {
	AE_ART_TYPE_RGB    = 0x03,
	AE_ART_TYPE_RGBA   = 0x04,
	AE_ART_TYPE_G      = 0x01,
	AE_ART_TYPE_GA     = 0x02,
	AE_ART_TYPE_P_RGB  = 0x11,
	AE_ART_TYPE_P_RGBA = 0x21
} ArtType;

uint8_t* ART_Read(Stream* stream, int* width, int* height, int* ch);

#endif
