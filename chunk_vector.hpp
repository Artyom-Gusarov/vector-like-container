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
    template <bool is_const>
    class chunk_iterator
        : public std::iterator<std::random_access_iterator_tag, T> {
    private:
        using Owner = chunk_vector<T, chunk_size, Alloc>;
        using Value = std::conditional_t<is_const, const T, T>;
        Owner *m_chunk_vector_ptr;
        std::size_t m_index;

    public:
        chunk_iterator(Owner ptr = nullptr, std::size_t index = 0)
            : m_chunk_vector_ptr(ptr), m_index(index) {
        }

        bool operator==(const chunk_iterator &other) const noexcept {
            return m_chunk_vector_ptr == other.m_chunk_vector_ptr &&
                   m_index == other.m_index;
        }

        bool operator!=(const chunk_iterator &other) const noexcept {
            return m_chunk_vector_ptr != other.m_chunk_vector_ptr ||
                   m_index != other.m_index;
        }

        Value &operator*() const noexcept {
            return m_chunk_vector_ptr->operator[](m_index);
        }

        Value *operator->() const noexcept {
            return get_ptr_by_index(m_index);
        }

        chunk_iterator &operator++() noexcept {
            ++m_index;
            return *this;
        }

        chunk_iterator operator++(int) noexcept {
            chunk_iterator tmp = *this;
            ++m_index;
            return tmp;
        }

        chunk_iterator &operator--() noexcept {
            --m_index;
            return *this;
        }

        chunk_iterator operator--(int) noexcept {
            chunk_iterator tmp = *this;
            --m_index;
            return tmp;
        }

        chunk_iterator &operator+=(std::ptrdiff_t n) noexcept {
            m_index += n;
            return *this;
        }

        chunk_iterator &operator-=(std::ptrdiff_t n) noexcept {
            m_index -= n;
            return *this;
        }

        chunk_iterator operator+(std::ptrdiff_t n) const noexcept {
            return chunk_iterator(m_chunk_vector_ptr, m_index + n);
        }

        chunk_iterator operator-(std::ptrdiff_t n) const noexcept {
            return chunk_iterator(m_chunk_vector_ptr, m_index - n);
        }

        std::ptrdiff_t operator-(const chunk_iterator &other) const noexcept {
            return m_index - other.m_index;
        }

        Value &operator[](std::ptrdiff_t n) const noexcept {
            return m_chunk_vector_ptr->operator[](m_index + n);
        }

        bool operator<(const chunk_iterator &other) const noexcept {
            return m_index < other.m_index;
        }

        bool operator>(const chunk_iterator &other) const noexcept {
            return m_index > other.m_index;
        }

        bool operator<=(const chunk_iterator &other) const noexcept {
            return m_index <= other.m_index;
        }

        bool operator>=(const chunk_iterator &other) const noexcept {
            return m_index >= other.m_index;
        }
    };

    std::size_t v_size;
    std::size_t v_capacity;
    std::vector<T *> v_chunks;

    T *get_ptr_by_index(std::size_t index) {
        return v_chunks[index / chunk_size] + index % chunk_size;
    }

public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename std::allocator_traits<Alloc>::pointer;
    using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
    using iterator = chunk_iterator<false>;
    using const_iterator = chunk_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
