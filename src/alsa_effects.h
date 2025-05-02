/* alsa_effects.h */
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PCM_INPUT_DEVICE "hw:3,0"
#define PCM_OUTPUT_DEVICE "hw:2,0"
//#define SAMPLE_RATE 44100
#define MAX_INT16 32767
#define MIN_INT16 -32768
#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 512
#define CHANNELS 1
//#define SAMPLE_FORMAT SND_PCM_FORMAT_FLOAT_LE
//typedef float SAMPLE;
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE
typedef int16_t SAMPLE;

// for echo
#define MAX_POLL_FDS 4
#define INTERNAL_BUFFER_FRAMES (FRAMES_PER_BUFFER * 4) // Bigger internal buffer
#define MAX_ECHOES 5
//#define ECHO_DELAY_FRAMES 2205  // ~50ms at 44.1kHz
#define ECHO_DELAY_FRAMES 4410  // ~100ms at 44.1kHz
typedef int16_t SAMPLE;


#ifdef __cplusplus
extern "C" {
#endif

/* Simple echo effect */
SAMPLE simple_echo_effect(SAMPLE input);

/* Simple fuzz effect */
SAMPLE fuzz_effect(SAMPLE input);

#ifdef __cplusplus
}
#endif
