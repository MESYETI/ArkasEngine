#ifndef AE_SUB_DRIVE_H
#define AE_SUB_DRIVE_H

#include "resources.h"

ResourceDrive* SubDrive_New(ResourceDrive** drives, size_t num, bool readOnly);
void           SubDrive_Add(ResourceDrive* subDrive, ResourceDrive* drive);

#endif
