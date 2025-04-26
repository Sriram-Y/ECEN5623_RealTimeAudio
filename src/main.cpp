#include <stdio.h>
#include "AudioOutput.h"
#include "AudioCapture.h"
#include "Filters.h"
#include "Sequencer.hpp"
#include "../lib/AudioPlaybackService/include/AudioPlaybackService.hpp"
#include "../lib/AudioCaptureService/include/AudioCaptureService.hpp"
#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"

int main() {
    SharedAudioBuffer sharedBuffer;
    Sequencer sequencer;

    sequencer.addService(new AudioCaptureService(1, 99, 100), &sharedBuffer); // Every 100 ms
    sequencer.addService(new AudioPlaybackService(1, 98, 100), &sharedBuffer);

    // Start services and signal handlers like before
    sequencer.startServices();
    while (exit_flag == 0) sleep(1);
    sequencer.stopServices();
    return 0;
}

