#ifndef AE_ERROR_H
#define AE_ERROR_H

#include "common.h"

typedef struct {
	bool        success;
	const char* reason;
} ErrorRet;

ErrorRet Error_Success(void);
ErrorRet Error_Fail(const char* reason);

#endif
