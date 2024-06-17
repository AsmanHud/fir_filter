cmake_minimum_required(VERSION 3.22.1)
project(FIR_Filter_Project C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include_directories(include)

# Check if the platform is Windows
if(WIN32)
    message(STATUS "Detected Windows platform")
    add_definitions(-D_USE_MATH_DEFINES)
else()
    message(STATUS "Detected Unix-based platform")
endif()

add_subdirectory(googletest)
include_directories(googletest/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(fir_filter src/main.c src/fir_filter.c src/fir_filter_cli.c)
if(NOT WIN32)
    target_link_libraries(fir_filter m)
endif()

add_executable(runTests tests/fir_filter_tests.cpp src/fir_filter.c)
if(NOT WIN32)
    target_link_libraries(runTests gtest gtest_main m)
else()
    target_link_libraries(runTests gtest gtest_main)
endif()