#include <stdio.h>
#include <signal.h>
#include <syslog.h>

// Student-generated headers
#include "AudioOutput.hpp"
#include "AudioCapture.hpp"
#include "Filters.hpp"
#include "Sequencer.hpp"
//#include "../src/alsa_effects.h"

#define PCM_INPUT_DEVICE "hw:3,0"
#define PCM_OUTPUT_DEVICE "hw:2,0"
#define SAMPLE_RATE 48000

// Definition of a sample
#define SAMPLE_FORMAT SND_PCM_FORMAT_S16_LE
typedef int16_t SAMPLE;
snd_pcm_t *capture_handle;
snd_pcm_t *playback_handle;
snd_pcm_hw_params_t *hw_params;


// This works on "normal" computers
//#define SAMPLE_FORMAT SND_PCM_FORMAT_FLOAT_LE
//typedef float SAMPLE;

/* Echo Parameters */
#define INTERNAL_BUFFER_FRAMES (FRAMES_PER_BUFFER * 4) // Bigger internal buffer for this windowed effect
#define MAX_ECHOES 5
//#define ECHO_DELAY_FRAMES 2205  // ~50ms at 44.1kHz
#define ECHO_DELAY_FRAMES 4410  // ~100ms at 44.1kHz

SAMPLE internal_buffer[INTERNAL_BUFFER_FRAMES];
int internal_buffer_head = 0;
int internal_buffer_tail = 0;

//AudioCapture cap;
const int channels = 1;
const int seconds = 5;
//AudioData cap_data ;
//AudioData cap_data2 ;
//extern Sequencer sequencer;
int run_once = 0;

std::jthread _service;  // Global thread for service execution

void serviceCapture(){
  SAMPLE temp_buf[FRAMES_PER_BUFFER];

  // Using fprin
  int frames = snd_pcm_readi(capture_handle, temp_buf, FRAMES_PER_BUFFER);
  if (frames == -EPIPE) {
    syslog(LOG_PERROR, "Overrun occurred during capture\n");
  } 
  else if (frames < 0) {
    syslog(LOG_PERROR, "Error from read\n");
  }
  snd_pcm_prepare(capture_handle);

  for (int i = 0; i < frames; i++) {
    //temp_buf[i] = fuzz_effect(temp_buf[i]);
    temp_buf[i] = simple_echo_effect(temp_buf[i]);
    //temp_buf[i] = simple_echo_effect(temp_buf[i]);
    //temp_buf[i] = simple_echo_effect(temp_buf[i]);
    //temp_buf[i] = (temp_buf[i]);

    internal_buffer[internal_buffer_head] = temp_buf[i];
    internal_buffer_head = (internal_buffer_head + 1) % INTERNAL_BUFFER_FRAMES;

    if (internal_buffer_head == internal_buffer_tail) {
      syslog(LOG_PERROR, "Warning: internal buffer overrun\n");
      internal_buffer_tail = (internal_buffer_tail + 1) % INTERNAL_BUFFER_FRAMES;
    }
  }
  if(run_once == 0)
    run_once++;
}

void serviceEffect(){
  if(run_once > 0){

  }
}

void servicePlayback(){
  if(run_once > 0){
    int frames_available = (internal_buffer_head - internal_buffer_tail + INTERNAL_BUFFER_FRAMES) % INTERNAL_BUFFER_FRAMES;

    if (frames_available >= FRAMES_PER_BUFFER) {
      SAMPLE temp_out[FRAMES_PER_BUFFER];

      for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        temp_out[i] = internal_buffer[internal_buffer_tail];
        internal_buffer_tail = (internal_buffer_tail + 1) % INTERNAL_BUFFER_FRAMES;
      }

      int frames = snd_pcm_writei(playback_handle, temp_out, FRAMES_PER_BUFFER);
      if (frames == -EPIPE) {
        syslog(LOG_PERROR, "Underrun occurred during playback\n");
      } 
      else if (frames < 0) {
        syslog(LOG_PERROR, "Error from write: %s\n", snd_strerror(frames));
      }
      snd_pcm_prepare(playback_handle);
    }
  }
}

void serviceKeyboard() {
    static bool initialized = false;
    if (!initialized) {
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        openlog("KeyWatcher", LOG_PID | LOG_CONS, LOG_USER);
        initialized = true;
    }

    char buf[256];
    ssize_t len = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        syslog(LOG_INFO, "Key(s) pressed: %s", buf);
    }
}

//void serviceKeyboard() {
//  static bool initialized = false;
//  if (!initialized) {
//    // Set stdin to non-blocking
//    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
//    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
//
//    // Set terminal to raw mode (optional)
//    struct termios term;
//    tcgetattr(STDIN_FILENO, &term);
//    term.c_lflag &= ~(ICANON | ECHO);
//    tcsetattr(STDIN_FILENO, TCSANOW, &term);
//
//    // Open syslog
//    openlog("KeyWatcher", LOG_PID | LOG_CONS, LOG_USER);
//    initialized = true;
//  }
//
//  char c;
//  while (read(STDIN_FILENO, &c, 1) > 0) {
//    char buf[64];
//    snprintf(buf, sizeof(buf), "Key pressed: %c", c);
//    syslog(LOG_INFO, "%s", buf);
//  }
//}


int audio_setup(snd_pcm_t **capture_handle, snd_pcm_t **playback_handle, snd_pcm_hw_params_t **hw_params) {
    int err;

    // Open capture device
    if ((err = snd_pcm_open(capture_handle, PCM_INPUT_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        //fprintf(stderr, "cannot open audio capture device (%s)\n", snd_strerror(err));
        syslog(LOG_PERROR, "cannot open audio capture device (%s)\n", snd_strerror(err));
        return 1;
    }

    // Open playback device
    if ((err = snd_pcm_open(playback_handle, PCM_OUTPUT_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        //fprintf(stderr, "cannot open audio playback device (%s)\n", snd_strerror(err));
        syslog(LOG_PERROR, "cannot open audio playback device (%s)\n", snd_strerror(err));
        return 1;
    }

    // Capture device setup
    snd_pcm_hw_params_malloc(hw_params);
    snd_pcm_hw_params_any(*capture_handle, *hw_params);
    snd_pcm_hw_params_set_access(*capture_handle, *hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(*capture_handle, *hw_params, SAMPLE_FORMAT);
    snd_pcm_hw_params_set_rate(*capture_handle, *hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(*capture_handle, *hw_params, CHANNELS);
    snd_pcm_hw_params(*capture_handle, *hw_params);
    snd_pcm_hw_params_free(*hw_params);
    snd_pcm_prepare(*capture_handle);

    // Playback device setup
    snd_pcm_hw_params_malloc(hw_params);
    snd_pcm_hw_params_any(*playback_handle, *hw_params);
    snd_pcm_hw_params_set_access(*playback_handle, *hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(*playback_handle, *hw_params, SAMPLE_FORMAT);
    snd_pcm_hw_params_set_rate(*playback_handle, *hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(*playback_handle, *hw_params, CHANNELS);
    snd_pcm_hw_params(*playback_handle, *hw_params);
    snd_pcm_hw_params_free(*hw_params);
    snd_pcm_prepare(*playback_handle);

    return 0;
}


int main(){
    Sequencer sequencer{};
    //SAMPLE buffer[FRAMES_PER_BUFFER];
    int err;

    /* start up syslog */
    openlog("audio_effects", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);

    /* audio setup (not yet rt)*/

    if ((err = audio_setup(&capture_handle, &playback_handle, &hw_params))) {
        //fprintf(stderr, "Cannot setup the audio (%s)\n", snd_strerror(err));
        syslog(LOG_PERROR, "Cannot setup the audio (%s)\n", snd_strerror(err));
        //syslog(LOG_PERROR, "Try to Add Services");
        exit(1);
    }
    syslog(LOG_INFO, "Success of setup capture\n");


    //struct sigaction sa;
    //sa.sa_handler = handle_sigint;
    //sa.sa_flags = 0; // No special flags
    //sigemptyset(&sa.sa_mask); // No blocked signals


  

    syslog(LOG_INFO, "Try to Add Services");
    sequencer.addService(serviceCapture, 1, 5, 5);
    sequencer.addService(serviceEffect, 1, 6, 5000);
    sequencer.addService(servicePlayback, 1, 10, 5);

    // change in and out effects
    sequencer.addService(serviceKeyboard, 1, 98, 1000);
    
    // Register signal handler
    //sigaction(SIGINT, &sa, NULL);
    //sigaction(SIGTERM, &sa, NULL); // Handle termination signal
    
    syslog(LOG_INFO, "Services Added");

    sequencer.startServices();
    // todo: wait for ctrl-c or some other terminating condition
    //while (sequencer.exit_flag == 0) {
    //    sleep(1); // Sleep to reduce CPU usage
    //}

    //while(sequencer.exit_flag <= 1) {
    //    sleep(1); // Sleep to reduce CPU usage
    //}
    std::thread waitForEnterThread([]()
    {
        std::string input;
        syslog(LOG_INFO, "Press ENTER to stop services...");
        //std::cout << "Press ENTER to stop services..." << std::endl;
        std::getline(std::cin, input);
    });
    waitForEnterThread.join();

    sequencer.stopServices();
    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);

    syslog(LOG_INFO, "Services stopped");
    closelog();
    return 0;
}
