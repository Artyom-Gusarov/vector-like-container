cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILD_TYPE Release)

project(MyVector)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined,leak -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(
        benchmark-stl-vector
        benchmark_vector.cpp
)
target_compile_definitions(benchmark-stl-vector PRIVATE TEST_STL_VECTOR)
target_link_libraries(benchmark-stl-vector benchmark::benchmark)

add_executable(
        benchmark-deque-as-vector
        benchmark_vector.cpp
)
target_compile_definitions(benchmark-deque-as-vector PRIVATE TEST_STL_DEQUE)
target_link_libraries(benchmark-deque-as-vector benchmark::benchmark)

add_executable(
        benchmark-chunk-vector
        benchmark_vector.cpp
)
target_compile_definitions(benchmark-chunk-vector PRIVATE TEST_CHUNK_VECTOR)
target_link_libraries(benchmark-chunk-vector benchmark::benchmark)

add_executable(
        test-chunk-vector-trivially-copyable
        test_vector.cpp
)
target_compile_definitions(test-chunk-vector-trivially-copyable PRIVATE TEST_CHUNK_VECTOR TRIVIALLY_COPYABLE)
target_link_libraries(test-chunk-vector-trivially-copyable gtest_main)

add_executable(
        test-chunk-vector-non-trivially-copyable
        test_vector.cpp
)
target_compile_definitions(test-chunk-vector-non-trivially-copyable PRIVATE TEST_CHUNK_VECTOR NON_TRIVIALLY_COPYABLE)
target_link_libraries(test-chunk-vector-non-trivially-copyable gtest_main)

# Custom target to run all benchmarks
add_custom_target(run-all-benchmarks
        DEPENDS benchmark-stl-vector benchmark-chunk-vector benchmark-deque-as-vector
        COMMAND benchmark-stl-vector --benchmark_out=../BM_results/benchmark_stl_vector.txt --benchmark_out_format=console
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector.txt --benchmark_out_format=console
        COMMAND benchmark-deque-as-vector --benchmark_out=../BM_results/benchmark_deque_as_vector.txt --benchmark_out_format=console
)