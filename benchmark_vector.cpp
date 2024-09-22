#include <benchmark/benchmark.h>
#include <string>

#ifdef TEST_STL_VECTOR
#include <vector>
template <typename T>
using vector = std::vector<T>;
#elif TEST_STL_DEQUE
#include "deque"
template <typename T>
using vector = std::deque<T>;
#elif TEST_CHUNK_VECTOR
#include "chunk_vector.hpp"
template <typename T>
using vector = CustomVector::chunk_vector<T>;
#elif TEST_CHUNK_VECTOR_V2
#include "chunk_vector_v2.hpp"
template <typename T>
using vector = CustomVector::chunk_vector<T>;
#endif

namespace {

class NonTriviallyCopyableInt {
public:
    NonTriviallyCopyableInt(int value = 0) : value_(value) {
    }

    NonTriviallyCopyableInt(const NonTriviallyCopyableInt &other)
        : value_(other.value_) {
    }

    NonTriviallyCopyableInt &operator=(const NonTriviallyCopyableInt &other) {
        if (this != &other) {
            value_ = other.value_;
        }
        return *this;
    }

    ~NonTriviallyCopyableInt() {
    }

    int getValue() const {
        return value_;
    }

private:
    int value_;
};

template <std::size_t size>
class BigSizeClass {
public:
    char data[size];
};

template <std::size_t size>
class NonTriviallyCopyableBigSizeClass {
public:
    char data[size];

    NonTriviallyCopyableBigSizeClass() {
    }

    NonTriviallyCopyableBigSizeClass(
        const NonTriviallyCopyableBigSizeClass &other
    ) {
        for (std::size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }

    NonTriviallyCopyableBigSizeClass &operator=(
        const NonTriviallyCopyableBigSizeClass &other
    ) {
        if (this != &other) {
            for (std::size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }
};

template <typename T = int, std::size_t count = 1000>
void reserve_BM(benchmark::State &state) {
    for (auto _ : state) {
        vector<T> v;
        for (std::size_t i = 0; i < count; ++i) {
            v.reserve(count);
        }
    }
}

template <typename T = int, std::size_t count = 1000>
void reserve_by_one_BM(benchmark::State &state) {
    for (auto _ : state) {
        vector<T> v;
        for (std::size_t i = 1; i <= count; ++i) {
            v.reserve(i);
        }
    }
}

template <typename T = int, std::size_t iterations = 1000>
void push_back_BM(benchmark::State &state) {
    T obj = T();

    for (auto _ : state) {
        vector<T> v;
        for (std::size_t i = 0; i < iterations; ++i) {
            v.push_back(obj);
        }
        benchmark::DoNotOptimize(v);
    }
}

template <typename T = int, std::size_t iterations = 1000>
void push_back_after_reserve_BM(benchmark::State &state) {
    T obj = T();

    for (auto _ : state) {
        vector<T> v;
#ifndef TEST_STL_DEQUE
        v.reserve(iterations);
#endif
        for (std::size_t i = 0; i < iterations; ++i) {
            v.push_back(obj);
        }
        benchmark::DoNotOptimize(v);
    }
}

template <typename T = int, std::size_t size = 1000>
void access_BM(benchmark::State &state) {
    vector<T> v(size);

    for (auto _ : state) {
        for (std::size_t i = 0; i < size; ++i) {
            benchmark::DoNotOptimize(v[i]);
        }
    }
}

template <typename T = int, std::size_t size = 1000>
void random_access_BM(benchmark::State &state) {
    vector<T> v(size);

    for (auto _ : state) {
        for (std::size_t i = 0; i < size; ++i) {
            if (i % 2) {
                benchmark::DoNotOptimize(v[i]);
            } else {
                benchmark::DoNotOptimize(v[size - 1 - i]);
            }
        }
    }
}

}  // namespace

BENCHMARK(reserve_BM<int, 1000>);
BENCHMARK(reserve_BM<int, 100000>);

BENCHMARK(reserve_BM<BigSizeClass<512>, 1000>);
BENCHMARK(reserve_BM<BigSizeClass<512>, 100000>);

BENCHMARK(reserve_BM<BigSizeClass<1024>, 1000>);
BENCHMARK(reserve_BM<BigSizeClass<1024>, 100000>);

BENCHMARK(reserve_by_one_BM<int, 1000>);
BENCHMARK(reserve_by_one_BM<int, 100000>);

BENCHMARK(reserve_by_one_BM<BigSizeClass<512>, 1000>);
BENCHMARK(reserve_by_one_BM<BigSizeClass<512>, 100000>);

BENCHMARK(reserve_by_one_BM<BigSizeClass<1024>, 1000>);
BENCHMARK(reserve_by_one_BM<BigSizeClass<1024>, 100000>);

BENCHMARK(push_back_BM<int, 1000>);
BENCHMARK(push_back_BM<NonTriviallyCopyableInt, 1000>);
BENCHMARK(push_back_BM<int, 100000>);
BENCHMARK(push_back_BM<NonTriviallyCopyableInt, 100000>);

BENCHMARK(push_back_BM<BigSizeClass<512>, 1000>);
BENCHMARK(push_back_BM<NonTriviallyCopyableBigSizeClass<512>, 1000>);
BENCHMARK(push_back_BM<BigSizeClass<512>, 100000>);
BENCHMARK(push_back_BM<NonTriviallyCopyableBigSizeClass<512>, 100000>);

BENCHMARK(push_back_BM<BigSizeClass<1024>, 1000>);
BENCHMARK(push_back_BM<NonTriviallyCopyableBigSizeClass<1024>, 1000>);
BENCHMARK(push_back_BM<BigSizeClass<1024>, 100000>);
BENCHMARK(push_back_BM<NonTriviallyCopyableBigSizeClass<1024>, 100000>);

BENCHMARK(push_back_after_reserve_BM<int, 1000>);
BENCHMARK(push_back_after_reserve_BM<NonTriviallyCopyableInt, 1000>);
BENCHMARK(push_back_after_reserve_BM<int, 100000>);
BENCHMARK(push_back_after_reserve_BM<NonTriviallyCopyableInt, 100000>);

BENCHMARK(push_back_after_reserve_BM<BigSizeClass<512>, 1000>);
BENCHMARK(push_back_after_reserve_BM<
          NonTriviallyCopyableBigSizeClass<512>,
          1000>);
BENCHMARK(push_back_after_reserve_BM<BigSizeClass<512>, 100000>);
BENCHMARK(push_back_after_reserve_BM<
          NonTriviallyCopyableBigSizeClass<512>,
          100000>);

BENCHMARK(push_back_after_reserve_BM<BigSizeClass<1024>, 1000>);
BENCHMARK(push_back_after_reserve_BM<
          NonTriviallyCopyableBigSizeClass<1024>,
          1000>);
BENCHMARK(push_back_after_reserve_BM<BigSizeClass<1024>, 100000>);
BENCHMARK(push_back_after_reserve_BM<
          NonTriviallyCopyableBigSizeClass<1024>,
          100000>);

BENCHMARK(access_BM<int, 1000>);
BENCHMARK(access_BM<int, 100000>);
BENCHMARK(access_BM<BigSizeClass<512>, 1000>);
BENCHMARK(access_BM<BigSizeClass<512>, 100000>);
BENCHMARK(access_BM<BigSizeClass<1024>, 1000>);
BENCHMARK(access_BM<BigSizeClass<1024>, 100000>);

BENCHMARK(random_access_BM<int, 1000>);
BENCHMARK(random_access_BM<int, 100000>);
BENCHMARK(random_access_BM<BigSizeClass<512>, 1000>);
BENCHMARK(random_access_BM<BigSizeClass<512>, 100000>);
BENCHMARK(random_access_BM<BigSizeClass<1024>, 1000>);
BENCHMARK(random_access_BM<BigSizeClass<1024>, 100000>);

BENCHMARK_MAIN();