#include "CircBuff.hpp"
#include <cstring> // for memcpy
#include "AudioCapture.hpp"
#include <stdio.h>

CircularBuffer::CircularBuffer(size_t bufferSize)
    : buffer(bufferSize), bufferSize(bufferSize),
      writeIndex(0), readIndex(0), bufferFull(false)
{}

size_t CircularBuffer::advanceIndex(size_t index) const
{
    return (index + 1) % bufferSize;
}


bool CircularBuffer::write(const AudioData& data)
{
    if (bufferFull && (writeIndex == readIndex)) {
        return false; // Buffer overrun
    }

    buffer[writeIndex] = data;  // <-- just a normal assignment!
    writeIndex = advanceIndex(writeIndex);

    if (writeIndex == readIndex) {
        bufferFull = true;
    }

    return true;
}

bool CircularBuffer::read(AudioData& dest)
{
    if (!bufferFull && (readIndex == writeIndex)) {
        return false; // Buffer underrun
    }

    dest = buffer[readIndex];  // <-- normal assignment again!
    readIndex = advanceIndex(readIndex);

    bufferFull = false;

    return true;
}
