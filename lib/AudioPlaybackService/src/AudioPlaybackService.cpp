#include "../lib/AudioOutput/include/AudioOutput.h"
#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"
#include "../lib/Sequencer/include/Sequencer.hpp"

class AudioPlaybackService : public Service {
public:
    AudioPlaybackService(int affinity, int priority, int period, SharedAudioBuffer* shared)
        : Service(affinity, priority, period), shared(shared) {}

protected:
    void _doService() override {
        AudioData data = shared->read();
        if (data.data && data.size > 0) {
            playback_audio(data, 20000, 1);
            free(data.data);
        }
    }

private:
    SharedAudioBuffer* shared;
};

