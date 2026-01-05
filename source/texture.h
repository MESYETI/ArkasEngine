#ifndef AE_TEXTURE_H
#define AE_TEXTURE_H

#include "backend.h"

Texture* Texture_LoadMem(uint8_t* img, size_t len);
Texture* Texture_LoadFile(const char* path);

#endif
