#include "stub.h"
#include "../audio.h"

#ifdef AE_AUDIO_STUB

void Audio_Init(void) {

}

void Audio_Free(void) {

}

void Audio_StartAudio(void) {

}

void Audio_StopAudio(void) {

}

void Audio_Update(void) {

}

bool Audio_Play2DSound(
	AudioEmitter emitter, Resource* resource, int prio, unsigned int flags,
	unsigned int fxMask, const struct audiofx* fx
) {
	(void) emitter;
	(void) resource;
	(void) prio;
	(void) flags;
	(void) fxMask;
	(void) fx;
	return true;
}

bool Audio_Play3DSound(
	AudioEmitter emitter, Resource* resource, int prio, unsigned int flags,
	unsigned int fxMask, const struct audiofx* fx
) {
	(void) emitter;
	(void) resource;
	(void) prio;
	(void) flags;
	(void) fxMask;
	(void) fx;
	return true;
}

bool Audio_MusicPlaying(void) {
	return false;
}

bool Audio_PlayMusic(const char* path, bool loop) {
	(void) path;
	(void) loop;
	return true;
}

void Audio_StopMusic(void) {

}

void Audio_DefaultState(void) {

}

void Audio_SetPlayerState(void) {

}

#endif
