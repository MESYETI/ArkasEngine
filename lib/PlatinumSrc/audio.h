#ifndef AE_PSRC_AUDIO_H
#define AE_PSRC_AUDIO_H

#include "vlb.h"

#include <SDL2/SDL.h>

#include <stdint.h>
#include <stdbool.h>

// The type for the callback that is called when the mixer needs more samples
//   ctx    - Passed in to play(2D|3D)Sound
//   loop   - The current loop index (should be assumed to be arbitrary)
//   pos    - The position of the play head (should be assumed to be arbitrary)
//   start  - Where you output the play head position of the first sample in the buffer (should be <= pos)
//   end    - Where you output the play head position of the last sample in the buffer (should be >= pos)
// Returns a pointer to a buffer of interleaved int16_t samples on success, or NULL on failure
// When freeing the sound, the callback will be called with bufptr being NULL
// See lines 10-64 of audio.c for examples
typedef int16_t* (*audiocb)(void* ctx, long loop, long pos, long* start, long* end);

// Audio priorities
#define AUDIOPRIO_DEFAULT (-128)
#define AUDIOPRIO_INVALID (-128) /* Used for invalid handle detection, do not use */
#define AUDIOPRIO_MIN (-127)
#define AUDIOPRIO_NORMAL (0)
#define AUDIOPRIO_MAX (127)

// Audio effect params
#define AUDIOFXMASK_TOFF (1U << 0)
#define AUDIOFXMASK_SPEED (1U << 1)
#define AUDIOFXMASK_VOL_L (1U << 2)
#define AUDIOFXMASK_VOL_R (1U << 3)
#define AUDIOFXMASK_VOL (AUDIOFXMASK_VOL_L | AUDIOFXMASK_VOL_R)
#define AUDIOFXMASK_LPFILT_L (1U << 4)
#define AUDIOFXMASK_LPFILT_R (1U << 5)
#define AUDIOFXMASK_LPFILT (AUDIOFXMASK_LPFILT_L | AUDIOFXMASK_LPFILT_R)
#define AUDIOFXMASK_HPFILT_L (1U << 6)
#define AUDIOFXMASK_HPFILT_R (1U << 7)
#define AUDIOFXMASK_HPFILT (AUDIOFXMASK_HPFILT_L | AUDIOFXMASK_HPFILT_R)
#define AUDIOFXMASK_ALL (-1U)
struct audiofx {
	int64_t toff;    // Time offset (in microseconds) (default is 0)
	float speed;     // Playback speed (default is 1.0)
	float vol[2];    // Volume (default is {1.0, 1.0})
	float lpfilt[2]; // L/R low-pass filter amounts (0.0 = off, 1.0 = max) (default is {0.0, 0.0})
	float hpfilt[2]; // L/R high-pass filter amounts (0.0 = off, 1.0 = max) (default is {0.0, 0.0})
};

// 3D-only effect params
#define AUDIO3DFXMASK_POS (1U << 0)
#define AUDIO3DFXMASK_RANGE (1U << 1)
#define AUDIO3DFXMASK_RADIUS (1U << 2)
#define AUDIO3DFXMASK_VOLDAMP (1U << 3)
#define AUDIO3DFXMASK_FREQDAMP (1U << 4)
#define AUDIO3DFXMASK_NODOPPLER (1U << 5)
#define AUDIO3DFXMASK_RELPOS (1U << 6)
#define AUDIO3DFXMASK_RELROT (1U << 7)
#define AUDIO3DFXMASK_ALL (-1U)
struct audio3dfx {
	float pos[3];          // Position (default is {0.0, 0.0, 0.0})
	float range;           // Range (default is 25.0)
	float radius[3];       // Radius (default is {0.0, 0.0, 0.0})
	float voldamp;         // Volume dampening (for walls; inversely related to volume) (default is 0.0)
	float freqdamp;        // Frequency dampening (also for walls; inversely related to the low-pass filter) (default is 0.0)
	uint8_t nodoppler : 1; // Disable doppler effect
	uint8_t relpos : 1;    // Position is relative to the camera (the camera pos is not subtracted from the sound pos)
	uint8_t relrot : 1;    // The position of the sound is not rotated around the camera (only makes sense if relpos is also set)
};

// Effects output
struct audiocalcfx {
	long posoff; // position offset in output freq samples
	int speedmul; // speed mult in units of 256
	int volmul[2]; // volume mult in units of 32768
	unsigned lpfiltmul[2]; // from 0 to output freq
	unsigned hpfiltmul[2]; // from 0 to output freq
};

// 3D audio emitter
#define AUDIOEMITTER3DFLAG_PAUSED (1U << 0) /* Paused */
#define AUDIOEMITTER3DFLAG_NOENV (1U << 1)  /* Do not apply audio environment effects to this emitter */
struct audioemitter3d {
	//uint32_t player;
	struct audiofx fx;
	struct audio3dfx fx3d;
	struct {
		long posoff;
		float volmul[2];
		float lpfiltmul[2];
		float hpfiltmul[2];
	} fx3dout;
	unsigned cursounds;
	unsigned maxsounds;
	int8_t prio;
	uint8_t flags;
	uint8_t fxch;
	uint8_t fxchimm;
	float dist;
};

// 2D audio emitter
#define AUDIOEMITTER2DFLAG_PAUSED (1U << 0)   /* Paused */
#define AUDIOEMITTER2DFLAG_APPLYENV (1U << 1) /* Apply audio environment effects to this emitter */
struct audioemitter2d {
	//uint32_t player;
	struct audiofx fx;
	unsigned cursounds;
	unsigned maxsounds;
	int8_t prio;
	uint8_t flags;
	uint8_t fxch;
	uint8_t fxchimm;
};

// Sound refill callback params
struct audiosoundcb {
	audiocb cb;    // Callback
	void* ctx;     // Callback 'ctx' param
	long len;      // Sound length in samples
	unsigned freq; // Sound frequency in Hz
	unsigned ch;   // Sound channels (must be 1 or 2 for now)
};

// Sound
#define SOUNDFLAG_LOOP (1U << 0)       /* Loop the sound */
#define SOUNDFLAG_WRAP (1U << 1)       /* Wrap the sound (use (LOOP | WRAP) for ambient/environment sounds */
#define SOUNDIFLAG_USESCB (1U << 0)    /*  */
#define SOUNDIFLAG_NEEDSINIT (1U << 1) /* Internal, do not use */
struct audiosound {
	uint32_t emitter;
	long loop;
	long pos;
	long frac;
	int8_t prio;
	uint8_t flags;
	uint8_t iflags;
	uint8_t fxch;
	uint8_t fxi;
	struct audiofx fx;
	struct audiocalcfx calcfx[2];
	int lplastout[2];
	int hplastout[2];
	int hplastin[2];
	struct audiosoundcb cb;
};

// Audio environment params
#define AUDIOENVMASK_PANNING (1U << 0)
#define AUDIOENVMASK_LPFILT (1U << 1)
#define AUDIOENVMASK_HPFILT (1U << 2)
#define AUDIOENVMASK_REVERB_DELAY (1U << 3)
#define AUDIOENVMASK_REVERB_MIX (1U << 4)
#define AUDIOENVMASK_REVERB_FEEDBACK (1U << 5)
#define AUDIOENVMASK_REVERB_MERGE (1U << 6)
#define AUDIOENVMASK_REVERB_LPFILT (1U << 7)
#define AUDIOENVMASK_REVERB_HPFILT (1U << 8)
#define AUDIOENVMASK_REVERB (AUDIOENVMASK_REVERB_DELAY | AUDIOENVMASK_REVERB_FEEDBACK | AUDIOENVMASK_REVERB_MIX | \
							AUDIOENVMASK_REVERB_MERGE | AUDIOENVMASK_REVERB_LPFILT | AUDIOENVMASK_REVERB_HPFILT)
#define AUDIOENVMASK_ALL (-1U)
struct audioenv {
	float panning;      // Side-to-side panning (-1.0 = left, 1.0 = right) (default is 0.0)
	float lpfilt;       // Low-pass filter amount (0.0 = off, 1.0 = max) (default is 0.0)
	float hpfilt;       // High-pass filter amount (0.0 = off, 1.0 = max) (default is 0.0)
	struct {            // Reverb
		float delay;    //   Reverb length (in seconds) (0.0 = off) (default is 0.0)
		float mix;      //   How much to mix the reverb output with the output stream (default is 0.0)
		float feedback; //   How much to mix the reverb output with the input stream (default is 0.0)
		float merge;    //   How much to merge the left and right channels of the input stream together (default is 0.0)
		float lpfilt;   //   Low-pass filter amount (default is 0.0)
		float hpfilt;   //   High-pass filter amount (default is 0.0)
	} reverb;           // ---
};

// Reverb state
struct audioreverbstate {
	int16_t* buf[2];
	unsigned len;
	unsigned size;
	unsigned head;
	unsigned tail;
	int lplastout[2];
	int hplastout[2];
	int hplastin[2];
	uint8_t parami;
	int mix[2];
	int feedback[2];
	int merge[2];
	unsigned lpfilt[2];
	unsigned hpfilt[2];
	unsigned filtdiv;
};

// Audio environment state
struct audioenvstate {
	uint16_t envch;
	uint16_t envchimm;
	float panning;
	struct {
		float amount;
		uint8_t muli;
		unsigned mul[2];
		int lastout[2];
	} lpfilt;
	struct {
		float amount;
		uint8_t muli;
		unsigned mul[2];
		int lastout[2];
		int lastin[2];
	} hpfilt;
	struct {
		float delay;
		float mix;
		float feedback;
		float merge;
		float lpfilt;
		float hpfilt;
		struct audioreverbstate state;
	} reverb;
};

// Player data
struct audioplayerdata {
	bool valid;               // Validity tracker (do not modify)
	float pos[3];             // Camera position (KEEP THIS UPDATED with the new camera position)
	float rotsin[3];          // Camera negative rotation sines (KEEP THIS UPDATED with the sin()s of the negative camera angles)
	float rotcos[3];          // Camera negative rotation cosines (KEEP THIS UPDATED with the cos()s of the negative camera angles)
	struct audioenvstate env; // Audio environment state (do not modify)
};

// Audio state
extern struct audiostate {
	volatile bool valid;
	bool usecallback;
	SDL_AudioDeviceID output;
	unsigned vol;
	unsigned freq;
	unsigned channels;
	float soundspeedmul;
	unsigned buflen;
	int* fxbuf[2]; // l/r
	int* envbuf[2]; // l/r
	int* mixbuf[2]; // l/r
	unsigned outsize;
	unsigned outqueue;
	int16_t* outbuf[2]; // old/new, interleaved
	unsigned outbufi;
	unsigned mixoutbufi;
	unsigned max3dsounds;
	unsigned voices3d;
	struct {
		struct audioemitter3d* data;
		uint32_t len;
		size_t size;
	} emitters3d;
	struct VLB(struct audiosound) sounds3d;
	struct VLB(size_t) sounds3dorder;
	unsigned max2dsounds;
	unsigned voices2d;
	struct {
		struct audioemitter2d* data;
		uint32_t len;
		size_t size;
	} emitters2d;
	struct VLB(struct audiosound) sounds2d;
	struct VLB(size_t) sounds2dorder;
	// tomfoolery to allow for easily adding back splitscreen support
	//struct {
	//    struct audioplayerdata* data;
	//    uint32_t len;
	//    size_t size;
	//} playerdata;
	struct {
		struct audioplayerdata data[1];
		uint32_t len;
	} playerdata;
} audiostate;



// Initialize audio state (call once at startup)
// Returns true if successful, false if not
bool initAudio(void);

// Start processing audio (call before starting the game)
// Returns true if successful, false if not
bool startAudio(void);

// Process the audio state (call after processing input and updating the camera, and before rendering)
// !!! REMEMBER TO ENSURE 'audiostate.player' IS UPDATED BEFORE CALLING (see 'struct audioplayerdata' above) !!!
//   delta - Delta time in seconds
void updateAudio(float delta);

// Stop processing audio
void stopAudio(void);

// Stop and then restart audio processing
// Returns true if successful, false if not
bool restartAudio(void);

// Denitialize audio state (call once before exit)
//   quick - Set this to true if you are exiting the program entirely and proper deinit doesn't matter
void quitAudio(bool quick);



// Create a new 3D audio emitter
//   prio      - AUDIOPRIO_... (use AUDIOPRIO_DEFAULT if unsure or don't care)
//   maxsounds - Max amount of sounds that can play under this emitter (pass -1 for "unlimited")
//   flags     - AUDIOEMITTER3DFLAG_... flags
//   fxmask    - AUDIOFXMASK_... flags specifying which params to copy from 'fx' (should be 0 if 'fx' is NULL)
//   fx        - A pointer to a 'struct audiofx' to copy effect params from, or NULL
//   fx3dmask  - AUDIO3DFXMASK_... flags specifying which params to copy from 'fx3d' (should be 0 if 'fx3d' is NULL)
//   fx3d      - A pointer to a 'struct audio3dfx' to copy 3D effect params from, or NULL
// Returns an ID on success or -1 on failure
uint32_t new3DAudioEmitter(int8_t prio, unsigned maxsounds, unsigned flags, unsigned fxmask, const struct audiofx* fx, unsigned fx3dmask, const struct audio3dfx* fx3d);

// Edit a 3D audio emitter
//   e         - Emitter ID
//   fenable   - Flags to set
//   fdisable  - Flags to unset
//   fxmask    - AUDIOFXMASK_... flags specifying which params to update from 'fx' (should be 0 if 'fx' is NULL)
//   fx        - A pointer to a 'struct audiofx' to update effect params from, or NULL
//   fx3dmask  - AUDIO3DFXMASK_... flags specifying which params to update from 'fx3d' (should be 0 if 'fx3d' is NULL)
//   fx3d      - A pointer to a 'struct audio3dfx' to update 3D effect params from, or NULL
//   immfxmask - AUDIOFXMASK_... flags specifying which effect params to immediately update (which params to not interpolate) (indirectly affects the 3D effects)
void edit3DAudioEmitter(uint32_t e, unsigned fenable, unsigned fdisable, unsigned fxmask, const struct audiofx* fx, unsigned fx3dmask, const struct audio3dfx* fx3d, unsigned immfxmask);

// Stop all sounds playing under the provided 3D emitter
//   e - Emitter ID
void stop3DAudioEmitter(uint32_t e);

// Delete the provided 3D emitter (also stops the sounds)
//   e - Emitter ID
void delete3DAudioEmitter(uint32_t e);

// Play a 3D sound
//   e      - Emitter ID
//   cb     - Refill callback
//   prio   - Priority override (AUDIOPRIO_DEFAULT to use the priority provided in the emitter)
//   flags  - SOUNDFLAG_... flags
//   fxmask - AUDIOFXMASK_... flags specifying which params to copy from 'fx' (should be 0 if 'fx' is NULL)
//   fx     - A pointer to a 'struct audiofx' to copy effect params from, or NULL
// Returns true if successful, false if not
bool play3DSound(uint32_t e, struct audiosoundcb* cb, int8_t prio, uint8_t flags, unsigned fxmask, const struct audiofx* fx);



// Create a new 2D audio emitter
//   prio      - AUDIOPRIO_... (use AUDIOPRIO_DEFAULT if unsure or don't care)
//   maxsounds - Max amount of sounds that can play under this emitter (pass -1 for "unlimited")
//   flags     - AUDIOEMITTER2DFLAG_... flags
//   fxmask    - AUDIOFXMASK_... flags specifying which params to copy from 'fx' (should be 0 if 'fx' is NULL)
//   fx        - A pointer to a 'struct audiofx' to copy effect params from, or NULL
// Returns an ID on success or -1 on failure
uint32_t new2DAudioEmitter(int8_t prio, unsigned maxsounds, unsigned flags, unsigned fxmask, const struct audiofx* fx);

// Edit a 2D audio emitter
//   e         - Emitter ID
//   fenable   - Flags to set
//   fdisable  - Flags to unset
//   fxmask    - AUDIOFXMASK_... flags specifying which params to update from 'fx' (should be 0 if 'fx' is NULL)
//   fx        - A pointer to a 'struct audiofx' to update effect params from, or NULL
//   immfxmask - AUDIOFXMASK_... flags specifying which effect params to immediately update (which params to not interpolate)
void edit2DAudioEmitter(uint32_t e, unsigned fenable, unsigned fdisable, unsigned fxmask, const struct audiofx* fx, unsigned immfxmask);

// Stop all sounds playing under the provided 2D emitter
//   e - Emitter ID
void stop2DAudioEmitter(uint32_t e);

// Delete the provided 2D emitter (also stops the sounds)
//   e - Emitter ID
void delete2DAudioEmitter(uint32_t e);

// Play a 2D sound
//   e      - Emitter ID
//   cb     - Refill callback
//   prio   - Priority override (AUDIOPRIO_DEFAULT to use the priority provided in the emitter)
//   flags  - SOUNDFLAG_... flags
//   fxmask - AUDIOFXMASK_... flags specifying which params to copy from 'fx' (should be 0 if 'fx' is NULL)
//   fx     - A pointer to a 'struct audiofx' to copy effect params from, or NULL
// Returns true if successful, false if not
bool play2DSound(uint32_t e, struct audiosoundcb* cb, int8_t prio, uint8_t flags, unsigned fxmask, const struct audiofx* fx);



// Set audio environment params
//   mask    - 
//   env     - 
//   immmask - 
void setAudioEnv(unsigned mask, struct audioenv* env, unsigned immmask);

#endif
