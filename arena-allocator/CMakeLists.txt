cmake_minimum_required(VERSION 3.16)

project(arena_allocator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library (your allocator)
add_library(arena
    src/arena.cpp
)

target_include_directories(arena
    PUBLIC include
)

# Executable (test)
add_executable(arena_test
    src/main.cpp
)

target_link_libraries(arena_test
    PRIVATE arena
)