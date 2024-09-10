#ifndef CHUNK_VECTOR_HPP
#define CHUNK_VECTOR_HPP
#include <memory>
#include <vector>

namespace CustomVector {
template <
    typename T,
    std::size_t chunk_size = 4096 / sizeof(T),
    typename Alloc = std::allocator<T>>
class chunk_vector {
private:
    std::size_t v_size;
    std::size_t v_capacity;
    std::vector<T *> v_chunks;

public:
    chunk_vector() : v_size(0), v_capacity(0) {
    }

    explicit chunk_vector(std::size_t n) : v_size(0), v_capacity(0) {
        for (std::size_t i = 0; i < n; ++i) {
            push_back(T());
        }
    }

    chunk_vector(std::size_t n, const T &t) : v_size(0), v_capacity(0) {
        for (std::size_t i = 0; i < n; ++i) {
            push_back(t);
        }
    }

    // delete copy constructor
    chunk_vector(const chunk_vector &other) = delete;

    // delete copy assignment
    chunk_vector &operator=(const chunk_vector &other) = delete;

    // delete move constructor
    chunk_vector(chunk_vector &&other) = delete;

    // delete move assignment
    chunk_vector &operator=(chunk_vector &&other) = delete;

    ~chunk_vector() {
        for (std::size_t i = 0; i < v_size; ++i) {
            this->operator[](i).~T();
        }
        for (T *chunk : v_chunks) {
            Alloc().deallocate(chunk, chunk_size);
        }
    }

    void reserve(std::size_t k) & {
        while (v_capacity < k) {
            T *new_chunk = Alloc().allocate(chunk_size);
            v_chunks.push_back(new_chunk);
            v_capacity += chunk_size;
        }
    }

    void push_back(const T &t) & {
        reserve(v_size + 1);
        T *cur_chunk = v_chunks[v_size / chunk_size];
        new (cur_chunk + (v_size++ % chunk_size)) T(t);
    }

    void push_back(T &&t) & {
        reserve(v_size + 1);
        T *cur_chunk = v_chunks[v_size / chunk_size];
        new (cur_chunk + (v_size++ % chunk_size)) T(std::move(t));
    }

    void pop_back() & noexcept {
        T *cur_chunk = v_chunks[(v_size - 1) / chunk_size];
        cur_chunk[--v_size % chunk_size].~T();
    }

    T &operator[](std::size_t i) & noexcept {
        return v_chunks[i / chunk_size][i % chunk_size];
    }

    [[nodiscard]] const T &operator[](std::size_t i) const & noexcept {
        return v_chunks[i / chunk_size][i % chunk_size];
    }

    [[nodiscard]] T &&operator[](std::size_t i) && noexcept {
        return std::move(v_chunks[i / chunk_size][i % chunk_size]);
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return v_size;
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return v_capacity;
    }
};
}  // namespace CustomVector

#endif  // CHUNK_VECTOR_HPP
