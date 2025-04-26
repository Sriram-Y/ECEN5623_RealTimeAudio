add_library(Sequencer
    src/Sequencer.cpp
)

target_include_directories(Sequencer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Sequencer PUBLIC DataStructures)

