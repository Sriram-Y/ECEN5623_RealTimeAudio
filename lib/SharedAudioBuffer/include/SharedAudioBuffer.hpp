// Implement a shared audio buffer
#ifndef __SHAREDAUDIOBUFFER_H_H
#define __SHAREDAUDIOBUFFER_H_H

// SharedAudioBuffer.hpp

#include <vector>
#include <mutex>
#include <DataStructures/DataStructures.h>  // Required for AudioData

class SharedAudioBuffer {
public:
    void write(const AudioData& data);
    AudioData read();

private:
    std::vector<char> buffer;
    std::mutex mtx;
};

#endif // __SHAREDAUDIOBUFFER_H_H
