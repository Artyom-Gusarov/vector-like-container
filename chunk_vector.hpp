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
    class chunk_iterator {
    private:
        using Owner = std::conditional_t<
            is_const,
            const chunk_vector<T, chunk_size, Alloc>,
            chunk_vector<T, chunk_size, Alloc>>;
        using Value = std::conditional_t<is_const, const T, T>;
        Owner *m_chunk_vector_ptr;
        std::size_t m_index;

    public:
        using difference_type = typename Owner::difference_type;
        using value_type = Value;
        using pointer = std::conditional_t<is_const, const Value *, Value *>;
        using reference = Value &;
        using iterator_category = std::random_access_iterator_tag;

        chunk_iterator(Owner *ptr = nullptr, std::size_t index = 0)
            : m_chunk_vector_ptr(ptr), m_index(index) {
        }

        chunk_iterator(const chunk_iterator &other) = default;

        chunk_iterator &operator=(const chunk_iterator &other) = default;

        chunk_iterator(chunk_iterator &&other) noexcept = default;

        chunk_iterator &operator=(chunk_iterator &&other) noexcept = default;

        ~chunk_iterator() = default;

        operator chunk_iterator<true>() const noexcept {
            return chunk_iterator<true>(m_chunk_vector_ptr, m_index);
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

    const_pointer get_ptr_by_index(size_type index) const {
        return v_chunks[index / chunk_size] + index % chunk_size;
    }

    void check_out_of_bound(size_type index) const {
        if (index >= size()) {
            throw std::out_of_range(
                "Requested index: " + std::to_string(index) +
                ", size: " + std::to_string(capacity())
            );
        }
    }

    void elements_shift(size_type start_pos, difference_type shift) {
        if (shift > 0) {
            reserve(v_size + shift);
            if (v_size == 0) {
                return;
            }
            for (size_type current_pos = v_size - 1; current_pos > start_pos;
                 --current_pos) {
                if (current_pos + shift >= v_size) {
                    new (get_ptr_by_index(current_pos + shift))
                        value_type(std::move(operator[](current_pos)));
                } else {
                    operator[](current_pos + shift) =
                        std::move(operator[](current_pos));
                }
            }
            if (start_pos != v_size) {
                if (start_pos + shift >= v_size) {
                    new (get_ptr_by_index(start_pos + shift))
                        value_type(std::move(operator[](start_pos)));
                } else {
                    operator[](start_pos + shift) =
                        std::move(operator[](start_pos));
                }
            }
        } else if (shift < 0) {
            for (size_type current_pos = start_pos; current_pos < v_size;
                 ++current_pos) {
                operator[](current_pos + shift) =
                    std::move(operator[](current_pos));
            }
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

    chunk_vector(chunk_vector &&other) noexcept
        : v_size(std::exchange(other.v_size, 0)),
          v_chunks(std::move(other.v_chunks)) {
    }

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

    // Capacity
    [[nodiscard]] bool empty() const noexcept {
        return v_size == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return v_size;
    }

    [[nodiscard]] size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    void reserve(size_type k) & {
        while (capacity() < k) {
            pointer new_chunk = allocator_type().allocate(chunk_size);
            v_chunks.push_back(new_chunk);
        }
    }

    [[nodiscard]] size_type capacity() const noexcept {
        return v_chunks.size() * chunk_size;
    }

    void shrink_to_fit() & {
        while (capacity() - v_size >= chunk_size) {
            allocator_type().deallocate(v_chunks.back(), chunk_size);
            v_chunks.pop_back();
        }
        v_chunks.shrink_to_fit();
    }

    // Modifiers
    void clear() & noexcept {
        for (size_type ind = 0; ind < v_size; ++ind) {
            operator[](ind).~value_type();
        }
        v_size = 0;
    }

    iterator insert(const_iterator pos, const_reference value) & {
        elements_shift(pos - begin(), 1);
        if (pos == end()) {
            new (get_ptr_by_index(v_size)) value_type(value);
        } else {
            operator[](pos - begin()) = value;
        }
        ++v_size;
        return iterator(this, pos - begin());
    }

    iterator insert(const_iterator pos, rvalue_reference value) & {
        elements_shift(pos - begin(), 1);
        if (pos == end()) {
            new (get_ptr_by_index(v_size)) value_type(std::move(value));
        } else {
            operator[](pos - begin()) = std::move(value);
        }
        ++v_size;
        return iterator(this, pos - begin());
    }

    iterator insert(const_iterator pos, size_type count, const_reference value)
        & {
        elements_shift(pos - begin(), count);
        for (size_type current_pos = pos - begin();
             current_pos < count + (pos - begin()); ++current_pos) {
            if (current_pos >= v_size) {
                new (get_ptr_by_index(current_pos)) value_type(value);
            } else {
                operator[](current_pos) = value;
            }
        }
        v_size += count;
        return iterator(this, pos - begin());
    }

    template <
        class InputIt,
        std::enable_if_t<
            std::is_base_of_v<
                std::input_iterator_tag,
                typename std::iterator_traits<InputIt>::iterator_category>,
            bool> = true>
    iterator insert(const_iterator pos, InputIt first, InputIt last) & {
        size_type count = std::distance(first, last);
        elements_shift(pos - begin(), count);
        for (size_type current_pos = pos - begin();
             current_pos < count + (pos - begin()); ++current_pos) {
            if (current_pos >= v_size) {
                new (get_ptr_by_index(current_pos)) value_type(*first);
            } else {
                operator[](current_pos) = *first;
            }
            ++first;
        }
        v_size += count;
        return iterator(this, pos - begin());
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) & {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args &&...args) {
        elements_shift(pos - begin(), 1);
        if (pos == end()) {
            allocator_type tmp_alloc;
            std::allocator_traits<allocator_type>::construct(
                tmp_alloc, get_ptr_by_index(v_size), std::forward<Args>(args)...
            );
        } else {
            operator[](pos - begin()) = value_type(std::forward<Args>(args)...);
        }
        ++v_size;
        return iterator(this, pos - begin());
    }

    iterator erase(const_iterator pos) {
        elements_shift(pos - begin() + 1, -1);
        operator[](--v_size).~value_type();
        return iterator(this, pos - begin());
    }

    iterator erase(const_iterator first, const_iterator last) {
        elements_shift(last - begin(), first - last);
        for (size_type i = 0; i < static_cast<size_type>(last - first); ++i) {
            operator[](--v_size).~value_type();
        }
        return iterator(this, first - begin());
    }

    void push_back(const_reference t) & {
        reserve(v_size + 1);
        pointer pos_for_new_value = get_ptr_by_index(v_size++);
        new (pos_for_new_value) value_type(t);
    }

    void push_back(rvalue_reference t) & {
        reserve(v_size + 1);
        pointer pos_for_new_value = get_ptr_by_index(v_size++);
        new (pos_for_new_value) value_type(std::move(t));
    }

    template <class... Args>
    reference emplace_back(Args &&...args) {
        reserve(v_size + 1);
        pointer pos_for_new_value = get_ptr_by_index(v_size++);
        allocator_type tmp_alloc;
        std::allocator_traits<allocator_type>::construct(
            tmp_alloc, pos_for_new_value, std::forward<Args>(args)...
        );
        return back();
    }

    void pop_back() & noexcept {
        operator[](--v_size).~value_type();
    }

    void resize(size_type count) & {
        while (count < v_size) {
            pop_back();
        }
        reserve(count);
        while (count > v_size) {
            push_back(T());
        }
    }

    void resize(size_type count, const_reference t) & {
        while (count < v_size) {
            pop_back();
        }
        reserve(count);
        while (count > v_size) {
            push_back(t);
        }
    }

    void resize(size_type count, rvalue_reference t) & {
        while (count < v_size) {
            pop_back();
        }
        reserve(count);
        while (count > v_size + 1) {
            push_back(t);
        }
        if (count == v_size + 1) {
            push_back(std::move(t));
        }
    }

    void swap(chunk_vector &other) noexcept {
        v_chunks.swap(other.v_chunks);
        std::swap(v_size, other.v_size);
    }

    // Friend functions
    friend iterator operator+(difference_type n, const iterator &it) noexcept {
        return it + n;
    }

    friend const_iterator
    operator+(difference_type n, const const_iterator &it) noexcept {
        return it + n;
    }
};
}  // namespace CustomVector

namespace std {
template <typename T, std::size_t chunk_size, typename Alloc>
void swap(
    CustomVector::chunk_vector<T, chunk_size, Alloc> &lhs,
    CustomVector::chunk_vector<T, chunk_size, Alloc> &rhs
) noexcept {
    lhs.swap(rhs);
}
}  // namespace std

#endif  // CHUNK_VECTOR_HPP
