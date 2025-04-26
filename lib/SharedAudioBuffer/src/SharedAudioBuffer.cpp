#include "../lib/SharedAudioBuffer/include/SharedAudioBuffer.hpp"
#include <mutex>  // For std::lock_guard

// Constructor (optional, empty)
//SharedAudioBuffer::SharedAudioBuffer() {
//}

//// Write new audio data into the buffer
//void SharedAudioBuffer::write(const std::vector<int16_t>& data) {
//    std::lock_guard<std::mutex> lock(mtx);
//    buffer = data;  // Overwrite current buffer
//}
//
//// Read current audio data out of the buffer
//std::vector<int16_t> SharedAudioBuffer::read() {
//    std::lock_guard<std::mutex> lock(mtx);
//    return buffer;  // Return a copy
//}


void SharedAudioBuffer::write(const AudioData& data) {
    std::lock_guard<std::mutex> lock(mtx);
    buffer.assign(data.data, data.data + data.size);
}

AudioData SharedAudioBuffer::read() {
    std::lock_guard<std::mutex> lock(mtx);

    AudioData data;
    data.data = buffer.data();
    data.size = buffer.size();
    return data;
}


