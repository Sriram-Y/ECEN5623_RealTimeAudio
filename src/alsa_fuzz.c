/*
 * alsa_fuzz.c - Rewritten from pa_fuzz.c to use ALSA instead of PortAudio
 */

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PCM_INPUT_DEVICE "hw:3,0"
#define PCM_OUTPUT_DEVICE "hw:2,0"
//#define SAMPLE_RATE 44100
#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 512
#define CHANNELS 1
//#define SAMPLE_FORMAT SND_PCM_FORMAT_FLOAT_LE
//typedef float SAMPLE;
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE
typedef int16_t SAMPLE;



/* Simple fuzz effect */
//static SAMPLE fuzz_effect(SAMPLE input)
//{
//
//    float gain = 10.0f;
//    float output = input * gain;
//    if (output > 1.0f) output = 1.0f;
//    if (output < -1.0f) output = -1.0f;
//    return output;
//}
static SAMPLE fuzz_effect(SAMPLE input)
{
    float x = input / 32768.0f;
    //float gain = 10.0f;
    float y = x * x * x - 1.0f;
    if (y > 1.0f) y = 1.0f;
    if (y < -1.0f) y = -1.0f;
    return (SAMPLE)(y * 32767.0f);
}

#define FUZZ(x) CubicAmplifier(CubicAmplifier(CubicAmplifier(CubicAmplifier(x))))

int main(void)
{
    snd_pcm_t *capture_handle;
    snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    SAMPLE buffer[FRAMES_PER_BUFFER];
    int err;

    /* Open capture device */
    if ((err = snd_pcm_open(&capture_handle, PCM_INPUT_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "cannot open audio capture device (%s)\n", snd_strerror(err));
        exit(1);
    }

    /* Open playback device */
    if ((err = snd_pcm_open(&playback_handle, PCM_OUTPUT_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "cannot open audio playback device (%s)\n", snd_strerror(err));
        exit(1);
    }

    /* Set hardware parameters for capture */
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, SAMPLE_FORMAT);
    snd_pcm_hw_params_set_rate(capture_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);
    snd_pcm_hw_params(capture_handle, hw_params);
    snd_pcm_hw_params_free(hw_params);
    snd_pcm_prepare(capture_handle);

    /* Set hardware parameters for playback */
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_any(playback_handle, hw_params);
    snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(playback_handle, hw_params, SAMPLE_FORMAT);
    snd_pcm_hw_params_set_rate(playback_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(playback_handle, hw_params, CHANNELS);
    snd_pcm_hw_params(playback_handle, hw_params);
    snd_pcm_hw_params_free(hw_params);
    snd_pcm_prepare(playback_handle);

    printf("Starting real-time fuzz audio passthrough... Press Ctrl+C to stop.\n");

    //while (1)
    //{
    //  /* Capture audio */
    //  err = snd_pcm_readi(capture_handle, buffer, FRAMES_PER_BUFFER);
    //  if (err == -EPIPE) {
    //      fprintf(stderr, "Overrun occurred during capture\n");
    //      snd_pcm_prepare(capture_handle);
    //      continue;
    //  } else if (err < 0) {
    //      fprintf(stderr, "Error from read: %s\n", snd_strerror(err));
    //      snd_pcm_prepare(capture_handle);
    //      continue;
    //  }

    //  /* Apply fuzz effect */
    //  for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
    //      //buffer[i] = fuzz_effect(buffer[i]);
    //      buffer[i] = buffer[i];
    //  }

    //  /* Playback audio */
    //  err = snd_pcm_writei(playback_handle, buffer, FRAMES_PER_BUFFER);
    //  if (err == -EPIPE) {
    //      fprintf(stderr, "Underrun occurred during playback\n");
    //      snd_pcm_prepare(playback_handle);
    //      continue;
    //  } else if (err < 0) {
    //      fprintf(stderr, "Error from write: %s\n", snd_strerror(err));
    //      snd_pcm_prepare(playback_handle);
    //      continue;
    //  }
    //}
  while (1)
  {
      /* Capture audio */
      err = snd_pcm_readi(capture_handle, buffer, FRAMES_PER_BUFFER);
      if (err == -EPIPE) {
          fprintf(stderr, "Overrun occurred during capture\n");
          snd_pcm_prepare(capture_handle);
          continue;
      } else if (err < 0) {
          fprintf(stderr, "Error from read: %s\n", snd_strerror(err));
          snd_pcm_prepare(capture_handle);
          continue;
      }
  
      /* Apply fuzz effect */
      for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
          //buffer[i] = fuzz_effect(buffer[i]);
          buffer[i] = buffer[i];
      }
  
      /* Playback audio */
      err = snd_pcm_writei(playback_handle, buffer, FRAMES_PER_BUFFER);
      if (err == -EPIPE) {
          fprintf(stderr, "Underrun occurred during playback\n");
          snd_pcm_prepare(playback_handle);
          continue;
      } else if (err < 0) {
          fprintf(stderr, "Error from write: %s\n", snd_strerror(err));
          snd_pcm_prepare(playback_handle);
          continue;
      }
  }



    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);

    return 0;
}
