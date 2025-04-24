#pragma once

//#include "AudioCapture.h"
#include "AudioCapture.h"
//#include "SharedAudioBuffer.hpp"
#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"
#include "../src/Sequencer.hpp"

class AudioCaptureService : public Service {
public:
    AudioCaptureService(int affinity, int priority, int period, AudioCapture* cap, SharedAudioBuffer* buffer);

protected:
    void _doService() override;

private:
    AudioCapture* cap;
    SharedAudioBuffer* shared;
};

