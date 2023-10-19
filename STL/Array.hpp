#pragma once

#include <cstddef>     // size_t
#include <iterator>    // std::reverse_iterator
#include <stdexcept>   // std::runtime_error
#include <string>      // std::to_string
#include <type_traits>


#define _LIBPOWERCXX_THROW_OUT_OF_RANGE(__i, __n)                             \
    throw std::runtime_error("out of range at index " + std::to_string(__i) + \
                             ", size " + std::to_string(__n))

#if defined(_MSC_VER)
    #define _LIBPOWERCXX_UNREACHABLE() __assume(0)
#elif defined(__clang__) || defined(__GNUC__)
    #define _LIBPOWERCXX_UNREACHABLE() __builtin_unreachable()
#else
    #define _LIBPOWERCXX_UNREACHABLE() \
        do                             \
        {                              \
        } while (1)
#endif


template<typename T, std::size_t N>
class Array
{
  public:
    using value_type             = T;
    using pointer                = T *;
    using const_pointer          = const T *;
    using reference              = T &;
    using const_reference        = const T &;
    using iterator               = T *;
    using const_iterator         = const T *;
    using reverse_iterator       = std::reverse_iterator<T *>;
    using const_reverse_iterator = std::reverse_iterator<const T *>;

    T m_elements[N];

    T &operator[](std::size_t i) noexcept { return m_elements[i]; }

    const T &operator[](std::size_t i) const noexcept { return m_elements[i]; }

    T &at(std::size_t i)
    {
        if (i >= N) [[unlikely]]
            _LIBPOWERCXX_THROW_OUT_OF_RANGE(i, N);
        return m_elements[i];
    }

    const T &at(std::size_t i) const
    {
        if (i >= N) [[unlikely]]
            _LIBPOWERCXX_THROW_OUT_OF_RANGE(i, N);
        return m_elements[i];
    }

    void fill(const T &value) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        for (std::size_t i = 0; i < N; i++)
            m_elements[i] = value;
    }

    void swap(Array &that) noexcept(std::is_nothrow_swappable_v<T>)
    {
        for (std::size_t i = 0; i < N; i++)
            std::swap(m_elements[i], that.m_elements[i]);
    }

    T &front() noexcept { return m_elements[0]; }

    const T &front() const noexcept { return m_elements[0]; }

    T &back() noexcept { return m_elements[N - 1]; }

    const T &back() const noexcept { return m_elements[N - 1]; }

    static constexpr std::size_t empty() noexcept { return false; }

    static constexpr std::size_t size() noexcept { return N; }

    static constexpr std::size_t max_size() noexcept { return N; }

    T *data() noexcept { return m_elements; }

    const T *data() const noexcept { return m_elements; }

    const T *cdata() const noexcept { return m_elements; }

    T *begin() noexcept { return m_elements; }

    T *end() noexcept { return m_elements + N; }

    const T *begin() const noexcept { return m_elements; }

    const T *end() const noexcept { return m_elements + N; }

    const T *cbegin() const noexcept { return m_elements; }

    const T *cend() const noexcept { return m_elements + N; }

    reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(m_elements); }

    reverse_iterator rend() noexcept
    {
        return std::make_reverse_iterator(m_elements + N);
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return std::make_reverse_iterator(m_elements);
    }

    const_reverse_iterator rend() const noexcept
    {
        return std::make_reverse_iterator(m_elements + N);
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return std::make_reverse_iterator(m_elements);
    }

    const_reverse_iterator crend() const noexcept
    {
        return std::make_reverse_iterator(m_elements + N);
    }
};

template<typename T>
class Array<T, 0>
{
  public:
    using value_type             = T;
    using pointer                = T *;
    using const_pointer          = const T *;
    using reference              = T &;
    using const_reference        = const T &;
    using iterator               = T *;
    using const_iterator         = const T *;
    using reverse_iterator       = std::reverse_iterator<T *>;
    using const_reverse_iterator = std::reverse_iterator<const T *>;

    T &operator[](std::size_t i) noexcept { _LIBPOWERCXX_UNREACHABLE(); }

    const T &operator[](std::size_t i) const noexcept { _LIBPOWERCXX_UNREACHABLE(); }

    T &at(std::size_t i) { _LIBPOWERCXX_THROW_OUT_OF_RANGE(i, 0); }

    const T &at(std::size_t i) const { _LIBPOWERCXX_THROW_OUT_OF_RANGE(i, 0); }

    void fill(const T &value) noexcept(std::is_nothrow_copy_assignable_v<T>) { }

    void swap(Array &that) noexcept(std::is_nothrow_swappable_v<T>) { }

    T &front() noexcept { _LIBPOWERCXX_UNREACHABLE(); }

    const T &front() const noexcept { _LIBPOWERCXX_UNREACHABLE(); }

    T &back() noexcept { _LIBPOWERCXX_UNREACHABLE(); }

    const T &back() const noexcept { _LIBPOWERCXX_UNREACHABLE(); }

    static constexpr std::size_t empty() noexcept { return true; }

    static constexpr std::size_t size() noexcept { return 0; }

    static constexpr std::size_t max_size() noexcept { return 0; }

    T *data() noexcept { return nullptr; }

    const T *data() const noexcept { return nullptr; }

    const T *cdata() const noexcept { return nullptr; }

    T *begin() noexcept { return nullptr; }

    T *end() noexcept { return nullptr; }

    const T *begin() const noexcept { return nullptr; }

    const T *end() const noexcept { return nullptr; }

    const T *cbegin() const noexcept { return nullptr; }

    const T *cend() const noexcept { return nullptr; }

    reverse_iterator rbegin() noexcept { return nullptr; }

    reverse_iterator rend() noexcept { return nullptr; }

    const_reverse_iterator rbegin() const noexcept { return nullptr; }

    const_reverse_iterator rend() const noexcept { return nullptr; }

    const_reverse_iterator crbegin() const noexcept { return nullptr; }

    const_reverse_iterator crend() const noexcept { return nullptr; }
};

template<typename Tp, typename... Ts>
Array(Tp, Ts...) -> Array<Tp, 1 + sizeof...(Ts)>;
