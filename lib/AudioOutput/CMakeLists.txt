add_library(AudioOutput
    src/AudioOutput.cpp
)

target_include_directories(AudioOutput PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AudioOutput PUBLIC DataStructures)