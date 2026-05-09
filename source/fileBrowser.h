#ifndef AE_FILE_BROWSER_H
#define AE_FILE_BROWSER_H

#include "scene.h"

enum {
	FILE_BROWSE_OPEN = 0,
	FILE_BROWSE_SAVE
};

// NULL if cancelled, non-NULL if path chosen
typedef void (*FileBrowserCallback)(const char* path);

Scene FileBrowserScene(int p_mode, FileBrowserCallback p_callback);

#endif
