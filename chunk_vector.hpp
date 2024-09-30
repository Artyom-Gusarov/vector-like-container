#ifndef CHUNK_VECTOR_HPP
#define CHUNK_VECTOR_HPP
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace CustomVector {
template <typename T, typename Alloc, typename>
struct alloc_wrapper {
private:
    Alloc alloc;

public:
    alloc_wrapper() : alloc() {
    }

    alloc_wrapper(const Alloc &alloc) : alloc(alloc) {
    }

    alloc_wrapper(Alloc &&moved_alloc) : alloc(std::move(moved_alloc)) {
    }

    T *allocate(std::size_t n) {
        return std::allocator_traits<Alloc>::allocate(alloc, n);
    }

    void deallocate(T *p, std::size_t n) {
        std::allocator_traits<Alloc>::deallocate(alloc, p, n);
    }

    template <typename... Args>
    void construct(T *p, Args &&...args) {
        std::allocator_traits<Alloc>::construct(
            alloc, p, std::forward<Args>(args)...
        );
    }

    Alloc get_alloc_copy() const {
        return alloc;
    }

    Alloc &get_alloc_ref() {
        return alloc;
    }
};

// Specialization for stateless allocators
template <typename T, typename Alloc>
struct alloc_wrapper<
    T,
    Alloc,
    std::enable_if_t<std::allocator_traits<Alloc>::is_always_equal::value>> {
    alloc_wrapper() = default;

    alloc_wrapper(const Alloc &){};

    alloc_wrapper(Alloc &&){};

    static T *allocate(std::size_t n) {
        auto tmp = Alloc();
        return std::allocator_traits<Alloc>::allocate(tmp, n);
    }

    static void deallocate(T *p, std::size_t n) {
        auto tmp = Alloc();
        std::allocator_traits<Alloc>::deallocate(tmp, p, n);
    }

    template <typename... Args>
    static void construct(T *p, Args &&...args) {
        auto tmp = Alloc();
        std::allocator_traits<Alloc>::construct(
            tmp, p, std::forward<Args>(args)...
        );
    }

    static Alloc get_alloc_copy() {
        return Alloc();
    }

    static Alloc &get_alloc_ref() {
        static Alloc alloc;
        return alloc;
    }
};

template <
    typename T,
    std::size_t chunk_size = (sizeof(T) > 8192 ? 1 : 8192 / sizeof(T)),
    typename Alloc = std::allocator<T>>
class chunk_vector : private alloc_wrapper<T, Alloc, void> {
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
    chunk_vector() noexcept(noexcept(std::vector<pointer>())) : v_size(0) {
    }

    explicit chunk_vector(const allocator_type &alloc) noexcept
        : alloc_wrapper<T, Alloc, void>(alloc), v_size(0) {
    }

    chunk_vector(
        size_type count,
        const_reference value,
        const allocator_type &alloc = allocator_type()
    )
        : alloc_wrapper<T, Alloc, void>(alloc), v_size(0) {
        resize(count, value);
    }

    explicit chunk_vector(
        size_type count,
        const allocator_type &alloc = allocator_type()
    )
        : alloc_wrapper<T, Alloc, void>(alloc), v_size(0) {
        resize(count);
    }

    template <
        class InputIt,
        std::enable_if_t<
            std::is_base_of_v<
                std::input_iterator_tag,
                typename std::iterator_traits<InputIt>::iterator_category>,
            bool> = true>
    chunk_vector(
        InputIt first,
        InputIt last,
        const allocator_type &alloc = allocator_type()
    )
        : alloc_wrapper<T, Alloc, void>(alloc), v_size(0) {
        assign(first, last);
    }

    chunk_vector(const chunk_vector &other)
        : chunk_vector(
              other.begin(),
              other.end(),
              std::allocator_traits<allocator_type>::
                  select_on_container_copy_construction(other.get_allocator())
          ) {
    }

    chunk_vector(const chunk_vector &other, const allocator_type &alloc)
        : chunk_vector(other.begin(), other.end(), alloc) {
    }

    chunk_vector(chunk_vector &&other) noexcept
        : alloc_wrapper<T, Alloc, void>(std::move(other.get_alloc_ref())),
          v_size(std::exchange(other.v_size, 0)),
          v_chunks(std::move(other.v_chunks)) {
    }

    chunk_vector(chunk_vector &&other, const allocator_type &alloc)
        : alloc_wrapper<T, Alloc, void>(alloc), v_size(0) {
        reserve(other.v_size);
        for (; v_size < other.v_size; ++v_size) {
            new (get_ptr_by_index(v_size)) value_type(std::move(other[v_size]));
        }
    }

    chunk_vector(
        std::initializer_list<value_type> init,
        const allocator_type &alloc = allocator_type()
    )
        : chunk_vector(init.begin(), init.end(), alloc) {
    }

    chunk_vector &operator=(const chunk_vector &other) {
        if (this == &other) {
            return *this;
        }
        assign(other.begin(), other.end());
        return *this;
    }

    chunk_vector &operator=(chunk_vector &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        std::swap(v_size, other.v_size);
        v_chunks.swap(other.v_chunks);
        return *this;
    }

    chunk_vector &operator=(std::initializer_list<value_type> ilist) {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    ~chunk_vector() {
        for (size_type i = 0; i < v_size; ++i) {
            this->operator[](i).~value_type();
        }
        for (pointer chunk : v_chunks) {
            this->deallocate(chunk, chunk_size);
        }
    }

    void assign(size_type count, const_reference value) & {
        size_type old_size = v_size;
        resize(count, value);
        for (size_type i = 0; i < std::min(old_size, v_size); ++i) {
            operator[](i) = value;
        }
    }

    template <
        class InputIt,
        std::enable_if_t<
            std::is_base_of_v<
                std::input_iterator_tag,
                typename std::iterator_traits<InputIt>::iterator_category>,
            bool> = true>
    void assign(InputIt first, InputIt last) {
        size_type count = std::distance(first, last);

        if (count <= v_size) {
            while (count < v_size) {
                pop_back();
            }
            for (size_type i = 0; i < v_size; ++i) {
                operator[](i) = *(first++);
            }
        } else {
            reserve(count);
            for (size_type i = 0; i < v_size; ++i) {
                operator[](i) = *(first++);
            }
            for (size_type i = v_size; i < count; ++i) {
                new (get_ptr_by_index(i)) value_type(*(first++));
                ++v_size;
            }
        }
    }

    void assign(std::initializer_list<value_type> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    allocator_type get_allocator() const noexcept {
        return this->get_alloc_copy();
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

    std::unique_ptr<value_type[]> copy_data() const {
        std::unique_ptr<value_type[]> data_copy(new value_type[v_size]);
        for (size_type i = 0; i < v_size; ++i) {
            data_copy[i] = operator[](i);
        }
        return data_copy;
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
            pointer new_chunk = this->allocate(chunk_size);
            v_chunks.push_back(new_chunk);
        }
    }

    [[nodiscard]] size_type capacity() const noexcept {
        return v_chunks.size() * chunk_size;
    }

    void shrink_to_fit() & {
        while (capacity() - v_size >= chunk_size) {
            this->deallocate(v_chunks.back(), chunk_size);
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

    iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
        & {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args &&...args) {
        elements_shift(pos - begin(), 1);
        if (pos == end()) {
            this->construct(
                get_ptr_by_index(v_size), std::forward<Args>(args)...
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
        this->construct(pos_for_new_value, std::forward<Args>(args)...);
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

    friend bool
    operator==(const chunk_vector &lhs, const chunk_vector &rhs) noexcept {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (size_type i = 0; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }

    friend bool
    operator!=(const chunk_vector &lhs, const chunk_vector &rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool
    operator<(const chunk_vector &lhs, const chunk_vector &rhs) noexcept {
        size_type min_size = std::min(lhs.size(), rhs.size());
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs[i] < rhs[i]) {
                return true;
            }
            if (lhs[i] > rhs[i]) {
                return false;
            }
        }
        return lhs.size() < rhs.size();
    }

    friend bool
    operator>(const chunk_vector &lhs, const chunk_vector &rhs) noexcept {
        return rhs < lhs;
    }

    friend bool
    operator<=(const chunk_vector &lhs, const chunk_vector &rhs) noexcept {
        return !(lhs > rhs);
    }

    friend bool
    operator>=(const chunk_vector &lhs, const chunk_vector &rhs) noexcept {
        return !(lhs < rhs);
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
