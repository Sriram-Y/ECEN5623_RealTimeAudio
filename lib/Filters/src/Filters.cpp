#include <stdio.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include "Filters.hpp"

#define FILTER_LEVELS 10
#define SAMPLE_RATE 48000

// Precomputed alpha values for 11 cutoff presets (logarithmic-ish scale)
float filter_alpha[FILTER_LEVELS] = {
    0.01f, 0.05f, 0.1f, 0.15f, 0.2f, 0.3f, 0.4f, 0.6f, 0.8f, 1.0f
};

SAMPLE reverb_buffer[ECHO_DELAY_FRAMES] = {0};
SAMPLE echo_buffer[ECHO_DELAY_FRAMES] = {0};
uint8_t reverb_index = 0;
uint8_t echo_index = 0;

void Filters_function()
{
    printf("Hello from Filters_function!\n");
}


/* Simple reverb effect */
SAMPLE simple_reverb_effect(SAMPLE input_sample)
{
    SAMPLE delayed = reverb_buffer[reverb_index];

    // Basic integer mix with feedback
    int32_t mixed = input_sample + (delayed / 2);  // Mix with 50% delayed signal

    // Clip to int16 range
    if (mixed > MAX_INT16) mixed = MAX_INT16;
    if (mixed < MIN_INT16) mixed = MIN_INT16;

    // Store back into reverb buffer with some decay
    reverb_buffer[reverb_index] = mixed / 2;  // Feedback with decay

    reverb_index = (reverb_index + 1) % REVERB_DELAY_FRAMES;
    return (SAMPLE)mixed;
}

/* Filter effect's input 0-10 defines the cutoff*/
SAMPLE filter_effect(SAMPLE input_sample, uint8_t cutoff_preset)
{
  // single pole filter
  // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]

  static float y_prev = 0.0f;

  if (cutoff_preset >= FILTER_LEVELS)
    cutoff_preset = FILTER_LEVELS - 1;

  float alpha = filter_alpha[cutoff_preset];
  float x = (float)input_sample;

  float y = alpha * x + (1.0f - alpha) * y_prev;
  y_prev = y;

  // Clamp result and return
  if (y > MAX_INT16) y = MAX_INT16;
  if (y < MIN_INT16) y = MIN_INT16;

  return (SAMPLE)y;
}



/* repeatin echo effect*/
int num_echoes = 2;
SAMPLE echo_effect(SAMPLE input_sample)
{
    int32_t mixed = input_sample;

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
//static SAMPLE fuzz_effect(SAMPLE input_sample)
//{
//
//    float gain = 10.0f;
//    float output = input_sample * gain;
//    if (output > 1.0f) output = 1.0f;
//    if (output < -1.0f) output = -1.0f;
//    return output;
//}
SAMPLE fuzz_effect(SAMPLE input_sample)
{
    float x = input_sample / 32768.0f;
    //float gain = 10.0f;
    float y = x * x * x - 1.0f;
    if (y > 1.0f) y = 1.0f;
    if (y < -1.0f) y = -1.0f;
    return (SAMPLE)(y * 32767.0f);
}
