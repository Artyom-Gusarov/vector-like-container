cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILD_TYPE Release)

project(MyVector)

set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined,leak -g")
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
        benchmark-chunk-vector
        benchmark_vector.cpp
)
target_compile_definitions(benchmark-chunk-vector PRIVATE TEST_CHUNK_VECTOR)
target_link_libraries(benchmark-chunk-vector benchmark::benchmark)

foreach(CHUNK_SIZE RANGE 1024 65536)
    if(CHUNK_SIZE EQUAL 1024 OR CHUNK_SIZE EQUAL 2048 OR CHUNK_SIZE EQUAL 4096 OR CHUNK_SIZE EQUAL 8192 OR
            CHUNK_SIZE EQUAL 16384 OR CHUNK_SIZE EQUAL 32768 OR CHUNK_SIZE EQUAL 65536)
        set(EXE_NAME "benchmark-chunk-vector-v2-${CHUNK_SIZE}")
        add_executable(${EXE_NAME} benchmark_vector.cpp)
        target_compile_definitions(${EXE_NAME} PRIVATE TEST_CHUNK_VECTOR_V2 CHUNK_SIZE=${CHUNK_SIZE})
        target_link_libraries(${EXE_NAME} benchmark::benchmark)
    endif()
endforeach()

# Custom target to run benchmarks for chunk vector of different versions
add_custom_target(run-chunk-vector-benchmarks
        DEPENDS benchmark-chunk-vector benchmark-chunk-vector-v2-1024 benchmark-chunk-vector-v2-2048 benchmark-chunk-vector-v2-4096 benchmark-chunk-vector-v2-8192 benchmark-chunk-vector-v2-16384 benchmark-chunk-vector-v2-32768 benchmark-chunk-vector-v2-65536
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_1024.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-1024 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_1024.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_2048.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-2048 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_2048.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_4096.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-4096 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_4096.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_8192.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-8192 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_8192.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_16384.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-16384 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_16384.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_32768.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-32768 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_32768.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector --benchmark_out=../BM_results/benchmark_chunk_vector_65536.json --benchmark_out_format=json
        COMMAND benchmark-chunk-vector-v2-65536 --benchmark_out=../BM_results/benchmark_chunk_vector_v2_65536.json --benchmark_out_format=json
)