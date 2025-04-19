cmake_minimum_required(VERSION 3.10)
project(RealTimeAudio C)

set(CMAKE_C_STANDARD 11)

# Find ALSA
find_package(ALSA REQUIRED)

# Add include directories
include_directories(include ${ALSA_INCLUDE_DIRS})

# Add subdirectories for your modules
add_subdirectory(lib/DataStructures)
add_subdirectory(lib/AudioOutput)
add_subdirectory(lib/AudioCapture)
add_subdirectory(lib/Filters)

# Define the executable
add_executable(realtimeaudio src/main.c)

# Link your internal libraries and ALSA
target_link_libraries(realtimeaudio PRIVATE AudioOutput AudioCapture Filters ${ALSA_LIBRARIES})
