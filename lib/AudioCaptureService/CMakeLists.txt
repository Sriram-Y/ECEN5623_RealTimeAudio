add_library(AudioCaptureService
  src/AudioCaptureService.cpp
)

target_include_directories(AudioCaptureService PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AudioCaptureService PUBLIC DataStructures)

