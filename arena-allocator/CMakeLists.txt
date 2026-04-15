cmake_minimum_required(VERSION 3.16)

project(arena_allocator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Library
# -----------------------------
add_library(arena
    src/arena.cpp
)

target_include_directories(arena
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------
# Demo
# -----------------------------
add_executable(arena_demo
    demo/demo.cpp
)

target_link_libraries(arena_demo
    PRIVATE arena
)

# -----------------------------
# Standalone debug executable
# -----------------------------
add_executable(arena_standalone_invalid_alignment
    tests/standalone_invalid_alignment.cpp
)

target_link_libraries(arena_standalone_invalid_alignment
    PRIVATE arena
)

# -----------------------------
# Testing
# -----------------------------
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )

    FetchContent_MakeAvailable(googletest)

    add_executable(arena_tests
        tests/allocation_tests.cpp
        tests/state_tests.cpp
        tests/rewind_tests.cpp
        tests/death_tests.cpp
    )

    target_link_libraries(arena_tests
        PRIVATE
            arena
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(arena_tests)
endif()