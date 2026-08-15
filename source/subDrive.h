#ifndef AE_SUB_DRIVE_H
#define AE_SUB_DRIVE_H

#include "vfs.h"

VFS_Drive* SubDrive_New(VFS_Drive** drives, size_t num, bool readOnly);
void       SubDrive_Add(VFS_Drive* subDrive, VFS_Drive* drive);

#endif
