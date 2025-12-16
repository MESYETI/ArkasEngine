#ifndef AE_AUDIO_H
#define AE_AUDIO_H

#include <PlatinumSrc/audio.h>
#include "common.h"
#include "resources.h"

#ifdef AE_AUDIO_PSRC
	typedef uint32_t AudioEmitter;
#else
	#error
#endif

void Audio_Init(void);
void Audio_Free(void);
void Audio_Update(void);
bool Audio_Play2DSound(
	AudioEmitter, Resource*, int prio, unsigned int flags, unsigned int fxMask,
	const struct audiofx* fx
);
bool Audio_Play3DSound(
	AudioEmitter, Resource*, int prio, unsigned int flags, unsigned int fxMask,
	const struct audiofx* fx
);

#endif
