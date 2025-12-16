#include "app.h"
#include "audio.h"

void Audio_Init(void) {
	initAudio();
}

void Audio_Free(void) {
	quitAudio(true);
}

void Audio_Update(void) {
	updateAudio(app.delta);
}

static int16_t* Callback(
	Resource* resource, long loop, long pos, long* start, long* end
) {
	(void) loop;
	(void) pos;

	if (!end) { // 'start' and 'end' are NULL so deinit
		Resources_FreeRes(resource);
		return NULL;
	}

	*start = 0;                       // Set startpoint
	*end = resource->v.audio.len - 1; // Set endpoint
	return resource->v.audio.data;    // Set buffer pointer
}

bool Audio_Play2DSound(
	AudioEmitter emitter, Resource* resource, int prio, unsigned int flags,
	unsigned int fxMask, const struct audiofx* fx
) {
	struct audiosoundcb cb = {
		.cb = (audiocb)Callback,
		.ctx = resource,
		.len = resource->v.audio.len,
		.freq = resource->v.audio.sampleRate,
		.ch = resource->v.audio.channels
	};

	++ resource->usedBy;
	bool res = play2DSound(emitter, &cb, prio, flags, fxMask, fx);

	if (!res) Resources_FreeRes(resource);
	return res;
}

bool Audio_Play3DSound(
	AudioEmitter emitter, Resource* resource, int prio, unsigned int flags,
	unsigned int fxMask, const struct audiofx* fx
) {
	struct audiosoundcb cb = {
		.cb = (audiocb)Callback,
		.ctx = resource,
		.len = resource->v.audio.len,
		.freq = resource->v.audio.sampleRate,
		.ch = resource->v.audio.channels
	};

	++ resource->usedBy;
	bool res = play3DSound(emitter, &cb, prio, flags, fxMask, fx);

	if (!res) Resources_FreeRes(resource);
	return res;
}
