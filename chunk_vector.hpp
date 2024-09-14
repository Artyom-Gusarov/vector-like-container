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
            return m_chunk_vector_ptr->get_ptr_by_index(m_index);
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

public:
    // Member types
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using rvalue_reference = value_type &&;
    using const_reference = const value_type &;
    using pointer = typename std::allocator_traits<Alloc>::pointer;
    using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
    using iterator = chunk_iterator<false>;
    using const_iterator = chunk_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    size_type v_size;
    std::vector<pointer> v_chunks;

    pointer get_ptr_by_index(size_type index) {
        return v_chunks[index / chunk_size] + index % chunk_size;
    }

    void check_out_of_bound(size_type index) {
        if (index >= capacity()) {
            throw std::out_of_range(
                "Requested index: " + std::to_string(index) +
                ", capacity: " + std::to_string(capacity())
            );
        }
    }

public:
    // Constructors
    chunk_vector() : v_size(0) {
    }

    explicit chunk_vector(size_type n) : v_size(0) {
        for (size_type i = 0; i < n; ++i) {
            push_back(value_type());
        }
    }

    chunk_vector(size_type n, const_reference t) : v_size(0) {
        for (size_type i = 0; i < n; ++i) {
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
        for (size_type i = 0; i < v_size; ++i) {
            this->operator[](i).~value_type();
        }
        for (pointer chunk : v_chunks) {
            allocator_type().deallocate(chunk, chunk_size);
        }
    }

    void reserve(size_type k) & {
        while (capacity() < k) {
            pointer new_chunk = allocator_type().allocate(chunk_size);
            v_chunks.push_back(new_chunk);
        }
    }

    void push_back(const_reference t) & {
        reserve(v_size + 1);
        pointer cur_chunk = v_chunks[v_size / chunk_size];
        new (cur_chunk + (v_size++ % chunk_size)) value_type(t);
    }

    void push_back(rvalue_reference t) & {
        reserve(v_size + 1);
        pointer cur_chunk = v_chunks[v_size / chunk_size];
        new (cur_chunk + (v_size++ % chunk_size)) value_type(std::move(t));
    }

    void pop_back() & noexcept {
        pointer cur_chunk = v_chunks[(v_size - 1) / chunk_size];
        cur_chunk[--v_size % chunk_size].~value_type();
    }

    // Element access
    reference at(size_type pos) & {
        check_out_of_bound(pos);
        return *get_ptr_by_index(pos);
    }

    [[nodiscard]] const_reference at(size_type pos) const & {
        check_out_of_bound(pos);
        return *get_ptr_by_index(pos);
    }

    [[nodiscard]] rvalue_reference at(size_type pos) && {
        check_out_of_bound(pos);
        return std::move(*get_ptr_by_index(pos));
    }

    reference operator[](size_type pos) & noexcept {
        return *get_ptr_by_index(pos);
    }

    [[nodiscard]] const_reference operator[](size_type pos) const & noexcept {
        return *get_ptr_by_index(pos);
    }

    [[nodiscard]] rvalue_reference operator[](size_type pos) && noexcept {
        return std::move(*get_ptr_by_index(pos));
    }

    reference front() & noexcept {
        return *v_chunks[0];
    }

    [[nodiscard]] const_reference front() const & noexcept {
        return *v_chunks[0];
    }

    [[nodiscard]] rvalue_reference front() && noexcept {
        return std::move(*v_chunks[0]);
    }

    reference back() & noexcept {
        return *get_ptr_by_index(v_size - 1);
    }

    [[nodiscard]] const_reference back() const & noexcept {
        return *get_ptr_by_index(v_size - 1);
    }

    [[nodiscard]] rvalue_reference back() && noexcept {
        return std::move(*get_ptr_by_index(v_size - 1));
    }

    // Iterators
    iterator begin() noexcept {
        return iterator(this, 0);
    }

    const_iterator begin() const noexcept {
        return const_iterator(this, 0);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(this, 0);
    }

    iterator end() noexcept {
        return iterator(this, v_size);
    }

    const_iterator end() const noexcept {
        return const_iterator(this, v_size);
    }

    const_iterator cend() const noexcept {
        return const_iterator(this, v_size);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    [[nodiscard]] size_type size() const noexcept {
        return v_size;
    }

    [[nodiscard]] bool empty() const noexcept {
        return v_size == 0;
    }

    [[nodiscard]] size_type capacity() const noexcept {
        return v_chunks.size() * chunk_size;
    }
};
}  // namespace CustomVector

#endif  // CHUNK_VECTOR_HPP
