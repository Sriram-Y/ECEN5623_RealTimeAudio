#include <stdio.h>
#include <signal.h>
#include <syslog.h>

// Keyboard capture
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdatomic.h>

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

/* reverb Parameters */
#define capture_buffer_FRAMES (FRAMES_PER_BUFFER * 4) // Bigger internal buffer for this windowed effect
#define effects_buffer_FRAMES (FRAMES_PER_BUFFER * 4) // Bigger internal buffer for this windowed effect
#define MAX_reverbES 5

SAMPLE capture_buffer[capture_buffer_FRAMES];
int capture_buffer_head = 0;
int capture_buffer_tail = 0;

SAMPLE effects_buffer[effects_buffer_FRAMES];
int effects_buffer_head = 0;
int effects_buffer_tail = 0;

static uint8_t filter_effect_level = 10, filter_enabled = 0;
static uint8_t reverb_effect_level = 10,   reverb_enabled = 0;
static uint8_t fuzz_effect_level = 10,   fuzz_enabled = 0;

//AudioCapture cap;
const int channels = 1;
const int seconds = 5;
//AudioData cap_data ;
//AudioData cap_data2 ;
//extern Sequencer sequencer;
atomic_int run_once = 0;
atomic_int keyboard_running = 1;

std::jthread _service;  // Global thread for service execution
                        //
/* Keyboard stuff*/
struct termios orig_termios;
static effects_mode current_effect = EFFECT_NONE;

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void handle_sigint(int sig) {
    restore_terminal();
    syslog(LOG_INFO, "\n[Terminal restored on signal %d]\n", sig);
    exit(0);
}

void set_raw_mode() {
    struct termios new_termios;

    // Save original settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);                // Restore at normal exit
    signal(SIGINT, handle_sigint);           // Restore on Ctrl-C

    new_termios = orig_termios;

    // Set raw mode: no canonical, no echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

/* Services */
void serviceCapture(){
  SAMPLE temp_buf[FRAMES_PER_BUFFER];

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
    //temp_buf[i] = simple_reverb_effect(temp_buf[i]);
    //temp_buf[i] = simple_reverb_effect(temp_buf[i]);
    //temp_buf[i] = simple_reverb_effect(temp_buf[i]);
    // Passthru
    temp_buf[i] = (temp_buf[i]);

    capture_buffer[capture_buffer_head] = temp_buf[i];
    capture_buffer_head = (capture_buffer_head + 1) % capture_buffer_FRAMES;

    if (capture_buffer_head == capture_buffer_tail) {
      syslog(LOG_PERROR, "Warning: internal buffer overrun\n");
      capture_buffer_tail = (capture_buffer_tail + 1) % capture_buffer_FRAMES;
    }
  }
  if(run_once == 0)
    run_once++;
}

void serviceEffect(){
  if(run_once > 0){
    int frames_available = (capture_buffer_head - capture_buffer_tail + capture_buffer_FRAMES) % capture_buffer_FRAMES;

    if (frames_available >= FRAMES_PER_BUFFER) {
      SAMPLE temp_dry_fx[FRAMES_PER_BUFFER] = {0};
      SAMPLE temp_filter_fx[FRAMES_PER_BUFFER];
      SAMPLE temp_reverb_fx[FRAMES_PER_BUFFER];
      SAMPLE temp_fuzz_fx[FRAMES_PER_BUFFER];
      SAMPLE temp_mixed_fx[FRAMES_PER_BUFFER];

      for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        temp_dry_fx[i] = capture_buffer[capture_buffer_tail];
        capture_buffer_tail = (capture_buffer_tail + 1) % capture_buffer_FRAMES;

        // Apply effects

        if(fuzz_enabled){
          temp_fuzz_fx[i] = fuzz_effect(temp_dry_fx[i], fuzz_effect_level);
          temp_dry_fx[i] = temp_fuzz_fx[i];
        }
        else
          temp_fuzz_fx[i] = 0;

        if(filter_enabled){
          temp_filter_fx[i] = filter_effect(temp_dry_fx[i], filter_effect_level );
          temp_dry_fx[i] = temp_filter_fx[i];
        }
        else
          temp_filter_fx[i] = 0;

        if(reverb_enabled){
          //temp_reverb_fx[i] = simple_reverb_effect(temp_dry_fx[i], reverb_effect_level);
          //temp_reverb_fx[i] = reverb_effect(temp_dry_fx[i], reverb_effect_level);
          temp_reverb_fx[i] = echo_effect(temp_dry_fx[i], reverb_effect_level);
          temp_dry_fx[i] = temp_reverb_fx[i];
        }
        else
          temp_reverb_fx[i] = 0;
        // Mix effects
        if(reverb_enabled == 0 && fuzz_enabled == 0 && filter_enabled == 0)
          temp_mixed_fx[i] = (temp_dry_fx[i]);
        else if(reverb_enabled == 1 && fuzz_enabled == 0 && filter_enabled == 0)
          temp_mixed_fx[i] = temp_reverb_fx[i];
        else if(reverb_enabled == 0 && fuzz_enabled == 0 && filter_enabled == 1)
          temp_mixed_fx[i] = temp_filter_fx[i];
        else if(reverb_enabled == 0 && fuzz_enabled == 1 && filter_enabled == 0)
          temp_mixed_fx[i] = temp_fuzz_fx[i];
        else
          temp_mixed_fx[i] = (temp_dry_fx[i]);
        
        /*else{
          temp_mixed_fx[i] += temp_fuzz_fx[i] >> 2;
          temp_mixed_fx[i] += temp_filter_fx[i] >> 2;
          temp_mixed_fx[i] += temp_reverb_fx[i] >> 2;
          */
        //}

        // Add to Effects bufer
        effects_buffer[effects_buffer_head] = temp_mixed_fx[i];
        effects_buffer_head = (effects_buffer_head + 1) % effects_buffer_FRAMES;

        if (effects_buffer_head == effects_buffer_tail) {
          syslog(LOG_PERROR, "Warning: internal buffer overrun\n");
          effects_buffer_tail = (effects_buffer_tail + 1) % effects_buffer_FRAMES;
        }
      }
    }
  }
}

void servicePlayback(){
  if(run_once > 0){
    // take from effects buffer
    int frames_available = (effects_buffer_head - effects_buffer_tail + effects_buffer_FRAMES) % effects_buffer_FRAMES;

    if (frames_available >= FRAMES_PER_BUFFER) {
      SAMPLE temp_out[FRAMES_PER_BUFFER];

      for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        temp_out[i] = effects_buffer[effects_buffer_tail];
        effects_buffer_tail = (effects_buffer_tail + 1) % effects_buffer_FRAMES;
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
  if(run_once > 0 && keyboard_running){
    set_raw_mode();

    char ch;
    while (keyboard_running && read(STDIN_FILENO, &ch, 1) > 0) {
      switch (ch) {

        case 'h': // help
          current_effect = EFFECT_NONE;
          syslog(LOG_INFO, "USAGE:\n\
                           -----------------------------------\n\
                           h -- prints this message\n\
                           p -- pauses all effects\n\
                           ] -- INCReases selected effect\n\
                           [ -- DECreases selected effect\n\
                           -----------------------------------\n\
                           Effects\n\
                           Pressing one of the following keys\n\
                           selects the effect and toggles it\n\
                           -----------------------------------\n\
                           r  -- Reverb effect toggle\n\
                           f  -- Filter effect toggle\n\
                           d  -- Fuzzy distortion effect toggle\n\
                           -----------------------------------\n");
          break;

        case 'p': // pause effects
            current_effect = EFFECT_NONE;
            filter_enabled = 0;
            fuzz_enabled = 0;
            reverb_enabled = 0;
            syslog(LOG_INFO, "Pausing all effects...\n");
          syslog(LOG_INFO, "Current Mode: %d\n", current_effect);
          break;

        case 'f':
          current_effect = EFFECT_FILTER;
          syslog(LOG_INFO, "Current Mode: Filter %d\n", current_effect);
          filter_enabled = !filter_enabled;
          syslog(LOG_INFO, "Filter toggled %s\n", filter_enabled ? "ON" : "OFF");
          break;

        case 'd':
          current_effect = EFFECT_FUZZ;
          syslog(LOG_INFO, "Current Mode: Fuzz %d\n", current_effect);
          fuzz_enabled = !fuzz_enabled;
          syslog(LOG_INFO, "Fuzzy distortion toggled %s\n", fuzz_enabled ? "ON" : "OFF");
          break;


        case 'r':
          current_effect = EFFECT_REVERB;
          syslog(LOG_INFO, "Current Mode: Reverb %d\n", current_effect);
          reverb_enabled = !reverb_enabled;
          syslog(LOG_INFO, "Reverb toggled %s\n", reverb_enabled ? "ON" : "OFF");
          break;

        // decrease effect
        case '[':
          syslog(LOG_INFO, "Current Mode: %d\n", current_effect);
          if(current_effect == EFFECT_FILTER){
            if(filter_effect_level > 0)
              filter_effect_level--;
            syslog(LOG_INFO, "Filter effect level: %d\n", filter_effect_level);
          }
          else if(current_effect == EFFECT_FUZZ){
            if(fuzz_effect_level > 0)
              fuzz_effect_level--;
            syslog(LOG_INFO, "Fuzz effect level: %d\n", fuzz_effect_level);
          }
          else if(current_effect == EFFECT_REVERB){
            if(reverb_effect_level > 0)
              reverb_effect_level--;
            syslog(LOG_INFO, "Reverb effect level: %d\n", reverb_effect_level);
          }
          break;

        // increase effect
        case ']':
          syslog(LOG_INFO, "Current Mode: %d\n", current_effect);
          if(current_effect == EFFECT_FILTER){
            if(filter_effect_level < 10)
              filter_effect_level++;
            syslog(LOG_INFO, "Filter effect level: %d\n", filter_effect_level);
          }
          else if(current_effect == EFFECT_FUZZ){
            if(fuzz_effect_level < 10)
              fuzz_effect_level++;
            syslog(LOG_INFO, "Fuzz effect level: %d\n", fuzz_effect_level);
          }
          else if(current_effect == EFFECT_REVERB){
            if(reverb_effect_level < 10)
              reverb_effect_level++;
            syslog(LOG_INFO, "Reverb effect level: %d\n", reverb_effect_level);
          }
          break;

        case '\n':
          keyboard_running = 0;
          break;

        default:
          syslog(LOG_INFO, "Key pressed: %c\n", ch);
          syslog(LOG_INFO, "For usage, press 'h'");
          break;
      }
    }
    //syslog(LOG_INFO,"You typed: %c\n", ch);
  }
}

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
    sequencer.addService(serviceEffect, 1, 6, 5);
    sequencer.addService(servicePlayback, 1, 10, 5);

    // change in and out effects
    sequencer.addService(serviceKeyboard, 1, 98, 100);
    
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

    keyboard_running = 0;
    sequencer.stopServices();
    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);

    syslog(LOG_INFO, "Services stopped");
    closelog();
    atexit(restore_terminal);                // Restore at normal exit
    //signal(SIGINT, handle_sigint);           // Restore on ctrl-c
    return 0;
}
