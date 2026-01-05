#include "art.h"
#include "stb.h"
#include "texture.h"

Texture* Texture_LoadMem(uint8_t* img, size_t len) {
	int      width, height, ch;
	uint8_t* data;

	if (memcmp(img, "ART", 3) == 0) {
		Stream stream = Stream_Memory(img, len, false);

		data = ART_Read(&stream, &width, &height, &ch);
	}
	else {
		data = stbi_load_from_memory(img, (int) len, &width, &height, &ch, 0);

		if (data == NULL) {
			Log("STB error: %s", stbi_failure_reason());
		}
	}

	if (data == NULL) {
		return NULL;
	}

	return Backend_LoadTexture(data, width, height, ch);
}

Texture* Texture_LoadFile(const char* path) {
	size_t   size;
	uint8_t* data = Resources_ReadFile(path, &size);

	if (data == NULL) {
		return NULL;
	}

	return Texture_LoadMem(data, size);
}
