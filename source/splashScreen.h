#ifndef AE_SPLASH_SCREEN_H
#define AE_SPLASH_SCREEN_H

#include "scene.h"

typedef void (*SplashScreenCallback)(void);

Scene NewSplashScreen(const char* path, SplashScreenCallback p_callback, float p_time);

#endif
