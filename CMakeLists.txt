cmake_minimum_required(VERSION 3.10)
project(RealTimeAudio CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable strict compiler warnings
add_compile_options(-Wall -Werror -pedantic)

# Find ALSA
find_package(ALSA REQUIRED)

# Add include directories
include_directories(include ${ALSA_INCLUDE_DIRS})

# Add subdirectories for your modules
add_subdirectory(lib/DataStructures)
add_subdirectory(lib/AudioOutput)
add_subdirectory(lib/AudioCapture)
add_subdirectory(lib/Filters)

# Define the executable (use C++ source files)
add_executable(realtimeaudio src/main.cpp src/Sequencer.cpp)

# Link your internal libraries and ALSA
target_link_libraries(realtimeaudio PRIVATE AudioOutput AudioCapture Filters ${ALSA_LIBRARIES})
