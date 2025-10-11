#include <PlatinumSrc/audio.h>
#include "fs.h"
#include "audio.h"

void Audio_Init(void) {
	initAudio();
	startAudio();

	if (FileExists("test.ogg")) {
		
	}
}

void Audio_Free(void) {
	stopAudio();
	quitAudio(true);
}
