#include <stdio.h>
#include "AudioOutput.hpp"
#include "AudioCapture.hpp"
#include "Filters.hpp"
#include "Sequencer.hpp"

int main()
{
    AudioCapture cap;
    const int rate = 44100;
    const int channels = 1;
    const int seconds = 5;

    if (setup_capture(&cap, rate, channels) != 0)
    {
        return 1;
    }

    AudioData cap_data = start_capture(&cap, seconds);

    playback_audio(cap_data, rate, channels);

    free(cap_data.data);
    return 0;
}
