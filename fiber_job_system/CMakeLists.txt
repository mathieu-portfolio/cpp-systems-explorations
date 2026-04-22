cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(fiber_job_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED CONFIG COMPONENTS context)

# -----------------------------
# JobSystem dependency
# -----------------------------
add_subdirectory(../job_system job_system_build)

# -----------------------------
# Library
# -----------------------------
add_library(fiber_job_system
    src/fiber_job_system.cpp
)

target_link_libraries(fiber_job_system
    PUBLIC
        job_system
        Boost::context
)

target_include_directories(fiber_job_system
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------
# Demo
# -----------------------------
add_executable(fiber_job_system_demo
    demo/demo.cpp
)

target_link_libraries(fiber_job_system_demo
    PRIVATE fiber_job_system
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

    add_executable(fiber_job_system_tests
        tests/basic_tests.cpp
        tests/scheduling_tests.cpp
        tests/contract_tests.cpp
    )

    target_link_libraries(fiber_job_system_tests
        PRIVATE
            fiber_job_system
            GTest::gtest_main
    )

    target_include_directories(fiber_job_system_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    include(GoogleTest)
    gtest_discover_tests(fiber_job_system_tests)
endif()
