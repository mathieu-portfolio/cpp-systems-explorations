cmake_minimum_required(VERSION 3.16)

project(job_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# ThreadPool dependency
# -----------------------------
add_subdirectory(../thread_pool thread_pool_build)

# -----------------------------
# Library
# -----------------------------
add_library(job_system
    src/job_system.cpp
)

target_link_libraries(job_system
    PUBLIC thread_pool
)

target_include_directories(job_system
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------
# Demo
# -----------------------------
add_executable(job_system_demo
    demo/demo.cpp
)

target_link_libraries(job_system_demo
    PRIVATE job_system
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

    add_executable(job_system_tests
        tests/basic_tests.cpp
        tests/dependency_tests.cpp
        tests/wait_tests.cpp
        tests/contract_tests.cpp
        tests/exception_tests.cpp
    )

    target_link_libraries(job_system_tests
        PRIVATE
            job_system
            GTest::gtest_main
    )

    target_include_directories(job_system_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    include(GoogleTest)
    gtest_discover_tests(job_system_tests)
endif()
