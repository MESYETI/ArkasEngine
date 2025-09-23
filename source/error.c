#include "error.h"

ErrorRet Error_Success(void) {
	return (ErrorRet) {true, NULL};
}

ErrorRet Error_Fail(const char* reason) {
	return (ErrorRet) {false, reason};
}
