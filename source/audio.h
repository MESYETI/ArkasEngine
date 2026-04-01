#ifndef AE_AUDIO_H
#define AE_AUDIO_H
#include "common.h"
#include "resources.h"

#ifdef AE_AUDIO_PSRC
	#include "audio/psrc.h"
#elif defined(AE_AUDIO_STUB)
	#include "audio/stub.h"

	struct audiofx {
		int unused;
	};
#endif

void Audio_Init(void);
void Audio_Free(void);
void Audio_StartAudio(void);
void Audio_StopAudio(void);
void Audio_Update(void);
bool Audio_Play2DSound(
	AudioEmitter, Resource*, int prio, unsigned int flags, unsigned int fxMask,
	const struct audiofx* fx
);
bool Audio_Play3DSound(
	AudioEmitter, Resource*, int prio, unsigned int flags, unsigned int fxMask,
	const struct audiofx* fx
);
bool Audio_MusicPlaying(void);
bool Audio_PlayMusic(const char* path, bool loop);
void Audio_StopMusic(void);
void Audio_DefaultState(void);
void Audio_SetPlayerState(void);

#endif
