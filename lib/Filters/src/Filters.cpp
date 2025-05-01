#include <stdio.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include "Filters.hpp"

void Filters_function()
{
    printf("Hello from Filters_function!\n");
}


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
SAMPLE fuzz_effect(SAMPLE input)
{
    float x = input / 32768.0f;
    //float gain = 10.0f;
    float y = x * x * x - 1.0f;
    if (y > 1.0f) y = 1.0f;
    if (y < -1.0f) y = -1.0f;
    return (SAMPLE)(y * 32767.0f);
}
