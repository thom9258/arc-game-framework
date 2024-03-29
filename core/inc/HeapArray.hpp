#pragma once

#include <type_traits>
#include <memory>
#include <exception>
#include <initializer_list>

namespace arc {
namespace core {

/* @brief HeapArray is a heap-allocated imitation of std::array.
 *
 * @template T: the value_type for the HeapArray container.
 *
 * HeapArray is designed to alleviate the pitfall of needing to know the size of
 * std::array at compile time, size specification is optional for HeapArray, and
 * is specified in the constructor.
 *
 * HeapArray is non-resizeable and non-growable, and is always considered at
 * full capacity, this makes HeapArray unsuiatable to use as a buffer, but
 * ensures its memory allocation to be handled only once.
 *
 * Members of HeapArray are considered unique, and are therefore not shared
 * between multiple instances of HeapArray's.
 *
 * HeapArray provides random access iterators of types:
 * - iterator
 * - const_iterator
 * - reverse_iterator
 * - const_reverse_iterator
 *
 * @refrences
 * Making a full-bodied container:
 * https://computingonplains.wordpress.com/custom-c-container-classes-with-iterators/
 * Real std::array:
 * https://en.cppreference.com/w/cpp/container/array
 * std requirements:
 * https://en.cppreference.com/w/cpp/named_req/Container
 *
 */
template <typename T>
class HeapArray {

//   static_assert(std::is_default_constructible_v<T>,
//                 "value_type must be default-constructible");
//   static_assert(std::is_copy_constructible_v<T>,
//                 "value_type must by copyable");
//   static_assert(std::is_nothrow_copy_constructible_v<T> &&
//                     std::is_nothrow_copy_assignable_v<T>,
//                 "value_type must be nothrow copy-/assign'able");

  public:
    /* Template specifications for metaprogramming, allows for type retrieval
     * in templates:
     *
     *     template <typename C> void foo() {
     *         typename C::value_type my_value{};
     *     }
     *
     * Where 'typename C' is an arbitrary HeapArray<T>, with 'my_value' being
     * a default-constructed value of type T.
     */
    using size_type = size_t;
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using refrence = value_type&;
    using const_refrence = const value_type&;

    /* @brief Sized construction with default values
     *
     * default values of members are not guaranteed and generated by 'new'
     *
     * @param _len: length of specified HeapArray
     */
    constexpr HeapArray(const size_t _len) noexcept
        : m_size((_len) ? _len : 1), m_data(new T[m_size]) {}

    /* @brief Construction from initializer_list
     *
     * Construct an array with specified default values from initializer_list
     * length of array is the same as the provided initializer_list.
     *
     * @param _il: initializer_list with specified values
     */
    constexpr HeapArray(const std::initializer_list<T>& _il) noexcept
        : HeapArray(_il.size()) {
        for (auto it = _il.begin(); it < _il.end(); it++)
            at(std::distance(_il.begin(), it)) = *it;
    }

    /* @brief copy construction by duplication
     *
     * It should not be possible to have two HeapArray objects point to the
     * same internal memroy, so we deep copy the data.
     *
     * @param _ha: HeapArray to construct a copy from
     */
    constexpr HeapArray(const HeapArray& _ha) noexcept
        : HeapArray(_ha.size()) {
        std::copy(_ha.cbegin(), _ha.cend(), begin());
    }

    /* @brief move construction by duplication
     *
     * @param _ha: HeapArray to construct directly without data dublication
     */
    //constexpr HeapArray(HeapArray&& _ha) noexcept
    //    : m_size(_ha.size()), m_data(_ha.data()) {}
    constexpr HeapArray(HeapArray&& _ha) = delete;

    /* @brief Cleans up owned memory on destruction
     */
    ~HeapArray() noexcept { delete[] m_data; }

    /* @brief assignment operator creates a copy
     */
    constexpr HeapArray& operator=(const HeapArray& _ha) const noexcept {
        ~HeapArray();
        HeapArray(std::copy(_ha)); /*calls copy construction*/
        return this;
    }

    /* @brief Equality comparison by comparing members
     */
    constexpr friend bool operator==(const HeapArray& _a,
                                     const HeapArray& _b) noexcept {
        if (_a.size() != _b.size())
            return false;
        return std::equal(_a.cbegin(), _a.cend(), _b.cbegin());
    }

    /* @brief Equality comparison by comparing members
     */
    constexpr friend bool operator!=(const HeapArray& _a,
                                     const HeapArray& _b) noexcept {
        return !(_a == _b);
    }

    /* @brief mathematical comparison not supported
     */
    constexpr friend bool operator<=(const HeapArray& _a,
                                     const HeapArray& _b) = delete;

    /* @brief mathematical comparison not supported
     */
    constexpr friend bool operator<(const HeapArray& _a,
                                    const HeapArray& _b) = delete;

    /* @brief mathematical comparison not supported
     */
    constexpr friend bool operator>=(const HeapArray& _a,
                                     const HeapArray& _b) = delete;

    /* @brief mathematical comparison not supported
     */
    constexpr friend bool operator>(const HeapArray& _a,
                                    const HeapArray& _b) = delete;

    /* @brief stream contents of array
     *
     * @todo Line-breaks are used after each element of the array,
     * it should be possible to count the characters of the stream and break,
     * at a specified break size.
     *
     * @return the output stream to stream to
     */
    friend std::ostream& operator<<(std::ostream& out, const HeapArray& _ha) {
        out << "[" << _ha.size() << "]{ ";
        size_t i = 0;
        for (; i < _ha.size() - 1; i++)
            out << _ha.at(i) << ", ";
        out << _ha.at(i) << " }";
        return out;
    }

    /* @brief value input streaming is not supported
     */
    friend std::istream& operator>>(std::istream& in,
                                    const HeapArray& _ha) = delete;

    /* @brief HeapArray::operator[]
     * @param _idx: index to wanted member
     *
     * Access array member by refrence.
     * Is considered unsafe, and does not check bounds.
     * For safe access, see HeapArray::at
     *
     * @return Refrence to member given by index
     */
    constexpr T& operator[](size_t _idx) const { return data()[_idx]; }

    /* @brief Raw array pointer
     *
     * @return raw pointer to start of owned data
     */
    constexpr T* data(void) const noexcept { return m_data; }

    /* @brief Size of array
     *
     * @return size of array
     */
    constexpr size_t size(void) const noexcept { return m_size; }

    /* @brief Maximum size of array
     *
     * Guaranteed to be the same as HeapArray::size
     *
     * @return size of array
     */
    constexpr size_t max_size(void) const noexcept { return m_size; }

    /* @brief HeapArray::at
     * @param _idx: index to wanted member
     *
     * Access array member by refrence.
     * Is considered safe, and does will throw std::out_of_range if
     * out of bounds.
     *
     * @return Refrence to member given by index
     */
    constexpr T& at(size_t _idx) const {
        if (/*_idx < 0 ||*/ _idx >= m_size)
            throw std::out_of_range(":(");
        return data()[_idx];
    }

    /* @brief HeapArray::front
     *
     * Access first array member by refrence.
     * Is guaranteed to exist, with safe access.
     *
     * @return Refrence to first member
     */
    constexpr T& front() const { return at(0); }

    /* @brief HeapArray::back
     *
     * Access last array member by refrence.
     * Is guaranteed to exist, with safe access.
     * If HeapArray::size == 1, then  HeapArray::front == HeapArray::back
     *
     * @return Refrence to last member
     */
    constexpr T& back() const { return at(size() - 1); }

    /* @brief HeapArray::empty
     *
     * Guaranteed to be false, as a instance of HeapArray is always considered
     * 'full'.
     *
     * @return false
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return false; }

    /* @brief HeapArray::swap
     *
     * Swaps contents of this with other instance of HeapArray with same
     * value_type.
     *
     * @return refrence to this
     */
    constexpr HeapArray& swap(HeapArray& _other) noexcept {
        std::swap(this, _other);
        return this;
    }

    /* @brief HeapArray::fill
     *
     * Fills all members with specified copied values.
     */
    void fill(const T& _v) { std::fill(begin(), end(), _v); }

    /* @brief HeapArray::clear
     *
     * Fills all members with default value of T.

     * @note breaks if typename T is not default-constructable.
     */
    void clear() { fill(T{}); }

    /*Iterator forward declatations*/
    struct iterator;
    struct const_iterator;
    struct reverse_iterator;
    struct const_reverse_iterator;

    iterator begin() noexcept { return iterator(m_data); }

    iterator end() noexcept { return iterator(m_data + m_size); }

    const_iterator cbegin() const noexcept { return const_iterator(m_data); }

    const_iterator cend() const noexcept {
        return const_iterator(m_data + m_size);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(m_data + m_size - 1);
    }

    reverse_iterator rend() noexcept { return reverse_iterator(m_data - 1); }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(m_data + m_size - 1);
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(m_data - 1);
    }

  private:
    /*length of heap-allocated array of T*/
    size_t m_size{0};
    /*pointer to start of heap-allocated array of T*/
    T* m_data{nullptr};

    /*Iterator Definition*/
  public:
    struct iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        explicit iterator(pointer ptr) : m_ptr(ptr) {}
        reference operator*() const { return *m_ptr; }
        pointer operator->() const { return m_ptr; }

        iterator& operator++() {
            ++m_ptr;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++m_ptr;
            return tmp;
        }
        iterator& operator--() {
            --m_ptr;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --m_ptr;
            return tmp;
        }
        friend bool operator==(const iterator& a, const iterator& b) {
            return a.m_ptr == b.m_ptr;
        };
        friend bool operator!=(const iterator& a, const iterator& b) {
            return a.m_ptr != b.m_ptr;
        };
        friend size_t operator-(const iterator& a, const iterator& b) {
            return a.m_ptr - b.m_ptr;
        };
        friend bool operator<(const iterator& a, const iterator& b) {
            return a.m_ptr < b.m_ptr;
        };

      private:
        pointer m_ptr;
    };

    struct const_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = const T*;
        using reference = const T&;

        explicit const_iterator(pointer ptr) : m_ptr(ptr) {}
        const_iterator(const iterator& it) : m_ptr(it.m_ptr) {}
        reference operator*() const { return *m_ptr; }
        pointer operator->() const { return m_ptr; }

        const_iterator& operator++() {
            m_ptr++;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++m_ptr;
            return tmp;
        }
        const_iterator& operator--() {
            m_ptr--;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --m_ptr;
            return tmp;
        }

        friend bool operator==(const const_iterator& a,
                               const const_iterator& b) {
            return a.m_ptr == b.m_ptr;
        };
        friend bool operator!=(const const_iterator& a,
                               const const_iterator& b) {
            return a.m_ptr != b.m_ptr;
        };
        friend size_t operator-(const const_iterator& a,
                                const const_iterator& b) {
            return a.m_ptr - b.m_ptr;
        };

      private:
        pointer m_ptr;
    };

    struct reverse_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        explicit reverse_iterator(pointer ptr) : m_ptr(ptr) {}
        reference operator*() const { return *m_ptr; }
        pointer operator->() const { return m_ptr; }

        reverse_iterator& operator++() {
            --m_ptr;
            return *this;
        }
        reverse_iterator operator++(int) {
            reverse_iterator tmp = *this;
            --m_ptr;
            return tmp;
        }
        reverse_iterator& operator--() {
            ++m_ptr;
            return *this;
        }
        reverse_iterator operator--(int) {
            reverse_iterator tmp = *this;
            ++m_ptr;
            return tmp;
        }
        friend bool operator==(const reverse_iterator& a,
                               const reverse_iterator& b) {
            return a.m_ptr == b.m_ptr;
        };
        friend bool operator!=(const reverse_iterator& a,
                               const reverse_iterator& b) {
            return a.m_ptr != b.m_ptr;
        };
        friend size_t operator-(const reverse_iterator& a,
                                const reverse_iterator& b) {
            return a.m_ptr - b.m_ptr;
        };
        friend bool operator<(const reverse_iterator& a,
                              const reverse_iterator& b) {
            return a.m_ptr < b.m_ptr;
        };

      private:
        pointer m_ptr;
    };

    struct const_reverse_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = const T*;
        using reference = const T&;

        explicit const_reverse_iterator(pointer ptr) : m_ptr(ptr) {}
        const_reverse_iterator(const iterator& it) : m_ptr(it.m_ptr) {}
        reference operator*() const { return *m_ptr; }
        pointer operator->() const { return m_ptr; }

        const_reverse_iterator& operator++() {
            m_ptr--;
            return *this;
        }
        const_reverse_iterator operator++(int) {
            const_reverse_iterator tmp = *this;
            --m_ptr;
            return tmp;
        }
        const_reverse_iterator& operator--() {
            m_ptr++;
            return *this;
        }
        const_reverse_iterator operator--(int) {
            const_reverse_iterator tmp = *this;
            ++m_ptr;
            return tmp;
        }

        friend bool operator==(const const_reverse_iterator& a,
                               const const_reverse_iterator& b) {
            return a.m_ptr == b.m_ptr;
        };
        friend bool operator!=(const const_reverse_iterator& a,
                               const const_reverse_iterator& b) {
            return a.m_ptr != b.m_ptr;
        };
        friend size_t operator-(const const_reverse_iterator& a,
                                const const_reverse_iterator& b) {
            return a.m_ptr - b.m_ptr;
        };

      private:
        pointer m_ptr;
    };
};

} /*ns*/
} /*ns*/
