#include "../input.h"

#ifdef PLATFORM_3DS

bool Input_KeyPressed(Key key) {
	(void) key;
	return false;
}

bool Input_MouseButtonPressed(uint8_t button) {
	(void) button;
	return false;
}

#endif
