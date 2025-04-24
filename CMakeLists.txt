cmake_minimum_required(VERSION 3.10)
project(RealTimeAudio C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Find ALSA
find_package(ALSA REQUIRED)

# Add include directories
include_directories(include ${ALSA_INCLUDE_DIRS})

# Add subdirectories for your modules
add_subdirectory(lib/DataStructures)
add_subdirectory(lib/AudioOutput)
add_subdirectory(lib/AudioCapture)
add_subdirectory(lib/Filters)
# Add subdirs for cpp code
add_subdirectory(lib/SharedAudioBuffer)
add_subdirectory(lib/AudioPlaybackService)
add_subdirectory(lib/AudioCaptureService)

# Define the executable
# add_executable(realtimeaudio src/main.c)
add_executable(realtimeaudio src/main.cpp)

# Link your internal libraries and ALSA
target_link_libraries(realtimeaudio PRIVATE AudioOutput AudioCapture Filters ${ALSA_LIBRARIES})
