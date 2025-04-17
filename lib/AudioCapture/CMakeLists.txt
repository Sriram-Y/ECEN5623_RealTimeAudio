add_library(AudioCapture
    src/AudioCapture.c
)

target_include_directories(AudioCapture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
