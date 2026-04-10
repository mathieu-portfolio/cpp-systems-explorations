cmake_minimum_required(VERSION 3.16)

project(arena_allocator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library
add_library(arena
    src/arena.cpp
)

target_include_directories(arena
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Tests
add_executable(arena_tests
    tests/tests.cpp
)

target_link_libraries(arena_tests
    PRIVATE arena
)

# Demo
add_executable(arena_demo
    demo/demo.cpp
)

target_link_libraries(arena_demo
    PRIVATE arena
)