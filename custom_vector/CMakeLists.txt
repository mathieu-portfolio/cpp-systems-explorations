cmake_minimum_required(VERSION 3.16)

project(custom_vector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Library
# -----------------------------
add_library(vector INTERFACE)

target_include_directories(vector
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------
# Demo
# -----------------------------
add_executable(vector_demo
    demo/demo.cpp
)

target_link_libraries(vector_demo
    PRIVATE vector
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

    add_executable(vector_tests
        tests/basic_tests.cpp
        tests/copy_move_tests.cpp
        tests/lifetime_tests.cpp
        tests/death_tests.cpp
        tests/iterator_tests.cpp
    )

    target_link_libraries(vector_tests
        PRIVATE
            vector
            GTest::gtest_main
    )

    target_include_directories(vector_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    include(GoogleTest)
    gtest_discover_tests(vector_tests)
endif()

# -----------------------------
# Benchmark
# -----------------------------
add_executable(vector_bench
bench/bench.cpp
)

target_link_libraries(vector_bench
PRIVATE vector
)

target_include_directories(vector_bench
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
