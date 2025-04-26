add_library(AudioCapture
    src/AudioCapture.cpp
)

set_target_properties(AudioCapture PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(AudioCapture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AudioCapture PUBLIC DataStructures)

