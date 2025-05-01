/*
 * alsa_effects.c - just effects
 */

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

SAMPLE internal_buffer[INTERNAL_BUFFER_FRAMES];
int internal_buffer_head = 0;
int internal_buffer_tail = 0;

SAMPLE echo_buffer[ECHO_DELAY_FRAMES] = {0};
int echo_index = 0;


/* Simple echo effect */
SAMPLE simple_echo_effect(SAMPLE input)
{
    SAMPLE delayed = echo_buffer[echo_index];

    // Basic integer mix with feedback
    int32_t mixed = input + (delayed / 2);  // Mix with 50% delayed signal

    // Clip to int16 range
    if (mixed > MAX_INT16) mixed = MAX_INT16;
    if (mixed < MIN_INT16) mixed = MIN_INT16;

    // Store back into echo buffer with some decay
    echo_buffer[echo_index] = mixed / 2;  // Feedback with decay

    echo_index = (echo_index + 1) % ECHO_DELAY_FRAMES;
    return (SAMPLE)mixed;
}

/* repeatin echo effect*/
int num_echoes = 2;
SAMPLE echo_effect(SAMPLE input)
{
    int32_t mixed = input;

    for (int i = 1; i <= num_echoes; i++) {
        int delay_pos = (echo_index + ECHO_DELAY_FRAMES * i) % (ECHO_DELAY_FRAMES * MAX_ECHOES);
        mixed += echo_buffer[delay_pos] / (i + 1);  // Attenuate later echoes more
    }

    // Clip to int16 range
    if (mixed > MAX_INT16) mixed = MAX_INT16;
    if (mixed < MIN_INT16) mixed = MIN_INT16;

    echo_buffer[echo_index] = mixed / 2;  // Store decayed version for feedback
    echo_index = (echo_index + 1) % (ECHO_DELAY_FRAMES * MAX_ECHOES);

    return (SAMPLE)mixed;
}

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
    int16_t silence[FRAMES_PER_BUFFER] = {0};

    printf("Starting real-time fuzz audio passthrough... Press Ctrl+C to stop.\n");
    snd_pcm_start(capture_handle);
    snd_pcm_writei(playback_handle, silence, FRAMES_PER_BUFFER);

  //while (1) {
  //    /* Capture audio */
  //    err = snd_pcm_readi(capture_handle, buffer, FRAMES_PER_BUFFER);
  //    if (err == -EPIPE) {
  //        fprintf(stderr, "Overrun occurred during capture\n");
  //        snd_pcm_prepare(capture_handle);
  //        continue;
  //    } else if (err < 0) {
  //        fprintf(stderr, "Error from read: %s\n", snd_strerror(err));
  //        snd_pcm_prepare(capture_handle);
  //        continue;
  //    }
  //
  //    /* Apply fuzz effect */
  //    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
  //        //buffer[i] = fuzz_effect(buffer[i]);
  //        buffer[i] = echo_effect(buffer[i]);
  //        //buffer[i] = buffer[i];
  //    }
  //
  //    /* Playback audio */
  //    err = snd_pcm_writei(playback_handle, buffer, FRAMES_PER_BUFFER);
  //    if (err == -EPIPE) {
  //        fprintf(stderr, "Underrun occurred during playback\n");
  //        snd_pcm_prepare(playback_handle);
  //        continue;
  //    } else if (err < 0) {
  //        fprintf(stderr, "Error from write: %s\n", snd_strerror(err));
  //        snd_pcm_prepare(playback_handle);
  //        continue;
  //    }
  //}


  /* Setup poll descriptors */
struct pollfd fds[MAX_POLL_FDS];
unsigned int nfds_capture = snd_pcm_poll_descriptors_count(capture_handle);
unsigned int nfds_playback = snd_pcm_poll_descriptors_count(playback_handle);
snd_pcm_poll_descriptors(capture_handle, fds, nfds_capture);
snd_pcm_poll_descriptors(playback_handle, fds + nfds_capture, nfds_playback);

  while (1)
  {
      int poll_ret = poll(fds, nfds_capture + nfds_playback, -1);
      if (poll_ret < 0) {
          perror("poll");
          break;
      }
  
      unsigned short revents;
  
      /* Check capture device */
      snd_pcm_poll_descriptors_revents(capture_handle, fds, nfds_capture, &revents);
      if (revents & POLLIN) {
          SAMPLE temp_buf[FRAMES_PER_BUFFER];
  
          int frames = snd_pcm_readi(capture_handle, temp_buf, FRAMES_PER_BUFFER);
          if (frames == -EPIPE) {
              fprintf(stderr, "Overrun occurred during capture\n");
              snd_pcm_prepare(capture_handle);
              continue;
          } else if (frames < 0) {
              fprintf(stderr, "Error from read: %s\n", snd_strerror(frames));
              snd_pcm_prepare(capture_handle);
              continue;
          }
  
          for (int i = 0; i < frames; i++) {
              //temp_buf[i] = fuzz_effect(temp_buf[i]);
              temp_buf[i] = simple_echo_effect(temp_buf[i]);
              //temp_buf[i] = simple_echo_effect(temp_buf[i]);
              //temp_buf[i] = simple_echo_effect(temp_buf[i]);
              //temp_buf[i] = (temp_buf[i]);
  
              internal_buffer[internal_buffer_head] = temp_buf[i];
              internal_buffer_head = (internal_buffer_head + 1) % INTERNAL_BUFFER_FRAMES;
  
              if (internal_buffer_head == internal_buffer_tail) {
                  fprintf(stderr, "Warning: internal buffer overrun\n");
                  internal_buffer_tail = (internal_buffer_tail + 1) % INTERNAL_BUFFER_FRAMES;
              }
          }
      }
  
      /* Check playback device */
      snd_pcm_poll_descriptors_revents(playback_handle, fds + nfds_capture, nfds_playback, &revents);
      if (revents & POLLOUT) {
          int frames_available = (internal_buffer_head - internal_buffer_tail + INTERNAL_BUFFER_FRAMES) % INTERNAL_BUFFER_FRAMES;
  
          if (frames_available >= FRAMES_PER_BUFFER) {
              SAMPLE temp_out[FRAMES_PER_BUFFER];
  
              for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
                  temp_out[i] = internal_buffer[internal_buffer_tail];
                  internal_buffer_tail = (internal_buffer_tail + 1) % INTERNAL_BUFFER_FRAMES;
              }
  
              int frames = snd_pcm_writei(playback_handle, temp_out, FRAMES_PER_BUFFER);
              if (frames == -EPIPE) {
                  fprintf(stderr, "Underrun occurred during playback\n");
                  snd_pcm_prepare(playback_handle);
                  continue;
              } else if (frames < 0) {
                  fprintf(stderr, "Error from write: %s\n", snd_strerror(frames));
                  snd_pcm_prepare(playback_handle);
                  continue;
              }
          }
      }
  }




    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);

    return 0;
}
