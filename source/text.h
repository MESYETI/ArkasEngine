#ifndef AE_TEXT_H
#define AE_TEXT_H

#include "backend.h"

typedef struct {
	Texture* texture;
	int      charWidth;
	int      charHeight;
} Font;

Font Text_LoadFont(const char* path, bool* success);
void Text_FreeFont(Font* font);
void Text_Render(Font* font, const char* string, int x, int y);

#endif
