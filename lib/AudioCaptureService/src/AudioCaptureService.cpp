#include "AudioCapture.h"
#include "Sequencer.hpp"

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

