#include "../lib/AudioCaptureService/include/AudioCaptureService.hpp"
#include "../lib/AudioCapture/include/AudioCapture.h"
#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"
#include "../lib/Sequencer/include/Sequencer.hpp"

//AudioCaptureService::AudioCaptureService(int affinity, int priority, int period, AudioCapture* cap, SharedAudioBuffer* shared)
//    : Service(affinity, priority, period), capture(cap), sharedBuffer(shared)
//{
//}

class AudioCaptureService : public Service {
public:
    AudioCaptureService(int affinity, int priority, int period, AudioCapture* cap, SharedAudioBuffer* shared)
        : Service(affinity, priority, period), cap(cap), shared(shared) {}

protected:
    void _doService() override {
        AudioData data = start_capture(cap, 1);
        shared->write(data);
        free(data.data);
    }

private:
    AudioCapture* cap;
    SharedAudioBuffer* shared;
};

