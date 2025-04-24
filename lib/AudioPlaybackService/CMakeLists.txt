add_library(AudioPlaybackService
  src/AudioPlaybackService.cpp
)

target_include_directories(AudioPlaybackService PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AudioPlaybackService PUBLIC DataStructures)

