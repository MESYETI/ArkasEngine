#include <string.h>
#include "art.h"
#include "mem.h"
#include "util.h"

uint8_t* ART_Read(Stream* stream, int* width, int* height, int* ch) {
	char magic[4];
	if (Stream_Read(stream, 3, &magic) != 3) {
		Log("ART: EOF");
		return NULL;
	}

	magic[3] = 0;
	if (strcmp(magic, "ART") != 0) {
		Log("Not an ART file");
		return NULL;
	}

	ArtType type = (ArtType) Stream_Read8(stream);

	*width  = (int) Stream_Read16(stream);
	*height = (int) Stream_Read16(stream);

	if ((*width > 1024) || (*height > 1024)) {
		Log("Image too big");
		return NULL;
	}

	size_t   palSize;
	uint8_t* palette = NULL;
	if ((type == AE_ART_TYPE_P_RGB) || (type == AE_ART_TYPE_P_RGBA)) {
		size_t channels;

		if (type == AE_ART_TYPE_P_RGB)  channels = 3;
		if (type == AE_ART_TYPE_P_RGBA) channels = 4;

		palSize = ((size_t) Stream_Read8(stream)) + 1;
		palette = SafeMalloc(palSize * channels);

		if (Stream_Read(stream, palSize * channels, palette) != palSize * channels) {
			Log("Unexpected EOF in palette");
			return NULL;
		}
	}

	switch (type) {
		case AE_ART_TYPE_RGB:
		case AE_ART_TYPE_G:
		case AE_ART_TYPE_P_RGB: *ch = 3; break;
		case AE_ART_TYPE_RGBA:
		case AE_ART_TYPE_GA:
		case AE_ART_TYPE_P_RGBA: *ch = 4; break;
	}

	uint8_t* ret    = SafeMalloc(*width * *height * *ch);
	size_t   pixels = *width * *height;

	switch (type) {
		case AE_ART_TYPE_RGB:
		case AE_ART_TYPE_RGBA: {
			if (Stream_Read(stream, pixels, ret) != pixels) {
				Log("EOF");
				free(ret);
				return NULL;
			}
			break;
		}
		case AE_ART_TYPE_G:
		case AE_ART_TYPE_GA: {
			uint8_t* pixel = ret;

			for (size_t i = 0; i < pixels; ++ i, pixel += *ch) {
				uint8_t intensity = Stream_Read8(stream);

				pixel[0] = intensity;
				pixel[1] = intensity;
				pixel[2] = intensity;

				if (type == AE_ART_TYPE_GA) {
					pixel[3] = Stream_Read8(stream);
				}
			}
			break;
		}
		case AE_ART_TYPE_P_RGB:
		case AE_ART_TYPE_P_RGBA: {
			uint8_t* pixel = ret;

			for (size_t i = 0; i < pixels; ++ i, pixel += *ch) {
				uint8_t* palCol = &palette[Stream_Read8(stream)];

				if ((size_t) (palCol - palette) >= palSize) {
					Error("Dangerous ART file");
				}

				memcpy(pixel, palCol, *ch);
			}
			break;
		}
	}

	return ret;
}
