#ifndef __FILTERS_H_H
#define __FILTERS_H_H

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
#define REVERB_DELAY_FRAMES 4410  // ~100ms at 44.1kHz
#define ECHO_DELAY_FRAMES 44100  // ~1000ms at 44.1kHz
typedef int16_t SAMPLE;

typedef enum {
    EFFECT_NONE = 0,
    EFFECT_FILTER = 1,
    EFFECT_REVERB = 2,
    EFFECT_FUZZ = 3
} effects_mode;

/* Simple echo effect */
SAMPLE simple_reverb_effect(SAMPLE input_sample);

/* Filter effect */
SAMPLE filter_effect(SAMPLE input_sample, uint8_t cutoff_preset);

/* Fuzz effect */
SAMPLE fuzz_effect(SAMPLE input_sample);

void Filters_function();

#endif // __FILTERS_H_H
