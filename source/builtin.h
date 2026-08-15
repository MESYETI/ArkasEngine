#ifndef AE_BUILTIN_H
#define AE_BUILTIN_H

// builtin: is a folderless drive containing files inside the engine executable

#include "vfs.h"

VFS_Drive* BuiltIn_GetDrive(void);

#endif
