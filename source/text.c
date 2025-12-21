#include "text.h"
#include "util.h"
#include "resources.h"

Font Text_LoadFont(const char* path, bool* success) {
	Font ret = {0};
	*success = true;
	/*ret.texture = Backend_LoadTexture(path);*/

	size_t   size;
	uint8_t* data = (uint8_t*) Resources_ReadFile(path, &size);

	if (!data) {
		Log("Failed to read path '%s'", path);
		*success = false;
		return ret;
	}

	ret.texture = Backend_LoadMemTexture(data, size);
	free(data);

	if (!ret.texture) {
		Log("Failed to load file '%s'", path);
		*success = false;
		return ret;
	}

	Vec2 fontSize  = Backend_GetTextureSize(ret.texture);
	ret.charWidth  = fontSize.x / 16;
	ret.charHeight = fontSize.y / 16;
	return ret;
}

void Text_FreeFont(Font* font) {
	Backend_FreeTexture(font->texture);
}

void Text_Render(Font* font, const char* string, int x, int y) {
	Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = font->charWidth;
	dest.h = font->charHeight;

	Rect src;
	src.w = font->charWidth;
	src.h = font->charHeight;
	// Backend_DrawTexture(font->texture, NULL, NULL, NULL);

	for (size_t i = 0; i < strlen(string); ++ i) {
		src.x = ((int) (string[i] % 16)) * font->charWidth;
		src.y = ((int) (string[i] / 16)) * font->charHeight;

		Backend_DrawTexture(font->texture, NULL, &src, &dest);
		dest.x += font->charWidth;
	}
}
