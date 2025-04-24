add_library(SharedAudioBuffer
    src/SharedAudioBuffer.cpp
)

target_include_directories(SharedAudioBuffer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(SharedAudioBuffer PUBLIC DataStructures)

