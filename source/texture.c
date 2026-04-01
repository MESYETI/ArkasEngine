#include "art.h"
#include "stb.h"
#include "util.h"
#include "texture.h"
#include "resources.h"

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
		Log("Texture_LoadMem: data = NULL");
		return NULL;
	}

	int newWidth  = width;
	int newHeight = height;

	if (width & (width - 1)) {
		// i don't know what's going on either
		-- newWidth;
		newWidth |= newWidth >> 1;
		newWidth |= newWidth >> 2;
		newWidth |= newWidth >> 4;
		newWidth |= newWidth >> 8;
		newWidth |= newWidth >> 16;
		++ newWidth;
	}

	if (height & (height - 1)) {
		-- newHeight;
		newHeight |= newHeight >> 1;
		newHeight |= newHeight >> 2;
		newHeight |= newHeight >> 4;
		newHeight |= newHeight >> 8;
		newHeight |= newHeight >> 16;
		++ newHeight;
	}

	uint8_t* data2 = malloc(newWidth * newHeight * ch);
	if (data2 == NULL) {
		free(data);
		Log("Failed to allocate texture");
		return NULL;
	}

	//stbir_resize_uint8_linear(data, width, height, 0, data2, newWidth, newHeight, 0, ch);

	memset(data2, 0, newWidth * newHeight * ch);

	for (int y = 0; y < height; ++ y) {
		memcpy(&data2[(y * newWidth * ch)], &data[(y * width * ch)], width * ch);
	}

	free(data);
	return Backend_LoadTexture(data2, width, height, newWidth, newHeight, ch);
}

Texture* Texture_LoadFile(const char* path) {
	size_t   size;
	uint8_t* data = Resources_ReadFile(path, &size);

	if (data == NULL) {
		Log("Failed to read file: %s", path);
		return NULL;
	}

	Texture* ret = Texture_LoadMem(data, size);
	free(data);
	return ret;
}
