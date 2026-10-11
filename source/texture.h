#ifndef AE_TEXTURE_H
#define AE_TEXTURE_H

#include "backend.h"

Texture* Texture_LoadMem(uint8_t* img, size_t len, bool world);
Texture* Texture_LoadFile(const char* path, bool world);
uint8_t* Texture_Downscale(uint8_t* data, int w, int h, int comp, int by);

#endif
