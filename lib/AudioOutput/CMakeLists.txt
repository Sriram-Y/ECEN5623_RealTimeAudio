add_library(AudioOutput
    src/AudioOutput.c
)

target_include_directories(AudioOutput PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
