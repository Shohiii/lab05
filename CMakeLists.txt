cmake_minimum_required(VERSION 3.10)

project(lab05 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_COVERAGE "Enable coverage" ON)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

add_subdirectory(banking)

if(BUILD_TESTS)
    enable_testing()

    add_subdirectory(third-party/gtest)
    add_subdirectory(tests)
endif()

# Compatibility of GoogleTest 1.8.1 with modern GCC.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(gtest PRIVATE -Wno-error=maybe-uninitialized)
endif()
