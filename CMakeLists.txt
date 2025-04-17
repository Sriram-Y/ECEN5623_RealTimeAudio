cmake_minimum_required(VERSION 3.10)
project(RealTimeAudio C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
add_subdirectory(lib/AudioOutput)
add_subdirectory(lib/AudioCapture)
add_subdirectory(lib/Filters)

add_executable(realtimeaudio src/main.c)
target_link_libraries(realtimeaudio PRIVATE AudioOutput AudioCapture Filters)
