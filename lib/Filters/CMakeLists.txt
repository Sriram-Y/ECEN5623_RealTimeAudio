add_library(Filters
    src/Filters.cpp
)

set_target_properties(Filters PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Filters PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
