#ifndef AE_AUDIO_H
#define AE_AUDIO_H

#include "common.h"

#ifdef AE_AUDIO_PSRC
	typedef AudioEmitter uint32_t;
#else
	#error
#endif

typedef struct {
	short* data;
	size_t len;
	int    channels;
	int    sampleRate;
} AudioResource;

void Audio_Init(void);
void Audio_Free(void);

#endif
