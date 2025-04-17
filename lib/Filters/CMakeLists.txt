add_library(Filters
    src/Filters.c
)

target_include_directories(Filters PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
