// SharedAudioBuffer.cpp
#include "SharedAudioBuffer.hpp"
#include <DataStructures/DataStructures.h>  // Required for AudioData
#include <cstring>           // Required for memcpy
#include <mutex>                   // for std::mutex, std::lock_guard

void SharedAudioBuffer::write(const AudioData& data) {
    std::lock_guard<std::mutex> lock(mtx);
    buffer.assign(data.data, data.data + data.size);
}

AudioData SharedAudioBuffer::read() {
    std::lock_guard<std::mutex> lock(mtx);

    AudioData result;
    result.size = buffer.size();
    result.offset = 0;

    if (result.size == 0) {
        result.data = nullptr;
        return result;
    }

    result.data = static_cast<char*>(malloc(result.size));
    if (result.data) {
        std::memcpy(result.data, buffer.data(), result.size);
    } else {
        result.size = 0;
    }

    return result;
}

