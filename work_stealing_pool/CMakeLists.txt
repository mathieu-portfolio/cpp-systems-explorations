cmake_minimum_required(VERSION 3.16)

project(work_stealing_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Library
# -----------------------------
add_library(work_stealing_pool
    src/work_stealing_pool.cpp
)

target_include_directories(work_stealing_pool
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------
# Demo
# -----------------------------
add_executable(work_stealing_pool_demo
    demo/demo.cpp
)

target_link_libraries(work_stealing_pool_demo
    PRIVATE work_stealing_pool
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

    add_executable(work_stealing_pool_tests
        tests/basic_tests.cpp
        tests/submission_tests.cpp
        tests/contract_tests.cpp
    )

    target_link_libraries(work_stealing_pool_tests
        PRIVATE
            work_stealing_pool
            GTest::gtest_main
    )

    target_include_directories(work_stealing_pool_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    include(GoogleTest)
    gtest_discover_tests(work_stealing_pool_tests)
endif()
