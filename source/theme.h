#ifndef AE_THEME_H
#define AE_THEME_H

#include "video.h"

typedef struct {
	Colour bg[4]; // 4 levels of bg
	Colour text;
} Theme;

extern Theme theme;

void Theme_Init(void);

#endif
