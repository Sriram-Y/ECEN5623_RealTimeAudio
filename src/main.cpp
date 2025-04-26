#include <stdio.h>
#include "AudioOutput.h"
#include "AudioCapture.h"
#include "Filters.h"
#include "../lib/Sequencer/include/Sequencer.hpp"
#include "../lib/AudioPlaybackService/include/AudioPlaybackService.hpp"
#include "../lib/AudioCaptureService/include/AudioCaptureService.hpp"
#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"

std::atomic<bool> exit_flag{false};
void signal_handler(int signum) {
    exit_flag = true;
}

int main() {
    SharedAudioBuffer sharedBuffer;
    AudioCapture capture;
    Sequencer sequencer;

    sequencer.addService(new AudioCaptureService(1, 99, 100, &capture, &sharedBuffer)); // needs capture + shared buffer
sequencer.addService(new AudioPlaybackService(1, 98, 100, &sharedBuffer));          // only needs shared buffer


    // Start services and signal handlers like before
    sequencer.startServices();
    while (exit_flag == 0) sleep(1);
    sequencer.stopServices();
    return 0;
}

