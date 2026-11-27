cmake_minimum_required(VERSION 3.20)
project(P09FenwickTrees LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

if(BUILD_TESTING)
    foreach(test IN ITEMS test_correctness test_fenwick_advanced test_range_structures)
        add_executable(${test} tests/${test}.cpp)
        add_test(NAME ${test} COMMAND ${test})
    endforeach()
endif()

add_executable(benchmark_range_structures benchmarks/benchmark_range_structures.cpp)
foreach(dimension IN ITEMS 1d 2d)
    add_executable(benchmark_${dimension} benchmarks/benchmark_${dimension}.cpp)
    add_executable(profile_${dimension} profiling/profile_${dimension}.cpp)
endforeach()
