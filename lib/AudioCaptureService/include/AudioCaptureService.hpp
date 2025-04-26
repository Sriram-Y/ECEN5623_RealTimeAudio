#pragma once

//#include "AudioCapture.h"
#include "../lib/AudioCapture/include/AudioCapture.h"
//#include "SharedAudioBuffer.hpp"
#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"
#include "../lib/Sequencer/include/Sequencer.hpp"



class AudioCaptureService : public Service {
public:
    AudioCaptureService(int affinity, int priority, int period, AudioCapture* cap, SharedAudioBuffer* shared);
    void _doService() override;

private:
    AudioCapture* capture;
    SharedAudioBuffer* sharedBuffer;
};

