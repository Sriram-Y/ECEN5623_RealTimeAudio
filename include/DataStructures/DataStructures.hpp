#ifndef __DATASTRUCTURES_H_H
#define __DATASTRUCTURES_H_H

#include <alsa/asoundlib.h>

// TODO (SY): Reorg needed
typedef struct
{
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    char *frame_buffer;
    int frame_buffer_size;

    char *audio_buffer;
    size_t audio_buffer_size;
    size_t audio_buffer_offset;

    int rate;
    int channels;
} AudioCapture;

typedef struct
{
    char *data;
    size_t size;
    size_t offset;
} AudioData;

#endif // __DATASTRUCTURES_H_H
