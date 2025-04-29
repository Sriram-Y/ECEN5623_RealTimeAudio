#pragma once
#include <cstddef>
#include <vector>
#include "AudioCapture.hpp" // Include your struct

class CircularBuffer
{
public:
    CircularBuffer(size_t bufferSize);

    bool write(const AudioData& data);
    bool read(AudioData& dest);

private:
    std::vector<AudioData> buffer;  // <-- CHANGED from uint8_t to AudioData
    size_t bufferSize;

    size_t writeIndex;
    size_t readIndex;
    bool bufferFull;

    size_t advanceIndex(size_t index) const;
};
