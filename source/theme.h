#ifndef AE_THEME_H
#define AE_THEME_H

#include "video.h"

typedef struct {
	Colour bg;
	Colour text;
	Colour button;
} Theme;

extern Theme theme;

void Theme_Init(void);

#endif
