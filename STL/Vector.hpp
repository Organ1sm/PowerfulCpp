#pragma once

#include <compare>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
struct Vector
{
    using value_type             = T;
    using allocator_type         = Alloc;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using pointer                = T *;
    using const_pointer          = const T *;
    using reference              = T &;
    using const_reference        = const T &;
    using iterator               = T *;
    using const_iterator         = const T *;
    using reverse_iterator       = std::reverse_iterator<T *>;
    using const_reverse_iterator = std::reverse_iterator<const T *>;

    T *mData;
    std::size_t mSize;
    std::size_t mCap;
    [[no_unique_address]] Alloc mAlloc;

    Vector() noexcept : mData(nullptr), mSize(0), mCap(0) { }

    Vector(std::initializer_list<T> ilist, const Alloc &alloc = Alloc())
        : Vector(ilist.begin(), ilist.end, alloc)
    { }

    explicit Vector(std::size_t n, const Alloc &alloc = Alloc()) : mAlloc(alloc)
    {
        mData = mAlloc.allocate(n);
        mCap = mSize = n;
        for (std::size_t i = 0; i != n; i++)
            std::construct_at(&mData[i]);   // m_data[i] = 0
    }

    Vector(std::size_t n, const T &value, const Alloc &alloc = Alloc()) : mAlloc(alloc)
    {
        mData = mAlloc.allocate(n);
        mCap = mSize = n;
        for (std::size_t i = 0; i != n; i++)
            std::construct_at(&mData[i], value);   // m_data[i] = value
    }

    template <std::random_access_iterator InputIt>
    Vector(InputIt first, InputIt last, const Alloc &alloc = Alloc()) : mAlloc(alloc)
    {
        std::size_t n = last - first;
        mData         = mAlloc.allocate(n);
        mCap = mSize = n;
        for (std::size_t i = 0; i != n; i++)
        {
            std::construct_at(&mData[i], *first);
            ++first;
        }
    }

    Vector(Vector &&that) noexcept : mAlloc(std::move(that.mAlloc))
    {
        mData      = that.mData;
        mSize      = that.mSize;
        mCap       = that.mCap;
        that.mData = nullptr;
        that.mSize = 0;
        that.mCap  = 0;
    }

    Vector(Vector &&that, const Alloc &alloc) noexcept : mAlloc(alloc)
    {
        mData      = that.mData;
        mSize      = that.mSize;
        mCap       = that.mCap;
        that.mData = nullptr;
        that.mSize = 0;
        that.mCap  = 0;
    }

    Vector &operator=(Vector &&that) noexcept
    {
        if (&that == this) [[unlikely]]
            return *this;

        for (std::size_t i = 0; i != mSize; i++)
            std::destroy_at(&mData[i]);

        if (mCap != 0)
            mAlloc.deallocate(mData, mCap);


        mData      = that.mData;
        mSize      = that.mSize;
        mCap       = that.mCap;
        that.mData = nullptr;
        that.mSize = 0;
        that.mCap  = 0;

        return *this;
    }

    Vector(const Vector &that) : mAlloc(that.mAlloc)
    {
        mCap = mSize = that.mSize;
        if (mSize != 0)
        {
            mData = mAlloc.allocate(mSize);
            for (std::size_t i = 0; i != mSize; i++)
                std::construct_at(&mData[i], std::as_const(that.mData[i]));
        }
        else
            mData = nullptr;
    }

    Vector(const Vector &that, const Alloc &alloc) : mAlloc(alloc)
    {
        mCap = mSize = that.mSize;
        if (mSize != 0)
        {
            mData = mAlloc.allocate(mSize);
            for (std::size_t i = 0; i != mSize; i++)
                std::construct_at(&mData[i], std::as_const(that.mData[i]));
        }
        else
            mData = nullptr;
    }

    Vector &operator=(const Vector &that)
    {
        if (&that == *this) [[unlikely]]
            return *this;

        reserve(that.mSize);
        mSize = that.mSize;
        for (std::size_t i = 0; i != mSize; i++)
            std::construct_at(&mData[i], std::as_const(that.mData[i]));

        return *this;
    }

    Vector &operator=(std::initializer_list<T> ilist)
    {
        assgin(ilist.begin(), ilist.end());
    }

    ~Vector() noexcept
    {
        for (std::size_t i = 0; i != mSize; i++)
            std::destroy_at(&mData[i]);

        if (mCap != 0)
            mAlloc.deallocate(mData, mCap);
    }

    void clear() noexcept
    {
        for (std::size_t i = 0; i != mSize; i++)
            std::destroy_at(&mData[i]);
        mSize = 0;
    }

    void reserve(std::size_t n)
    {
        if (n <= mCap)
            return;

        n = std::max(n, mCap * 2);
        std::cout << std::format("grow from {} to {}\n", mCap, n);

        auto oldData = mData;
        auto oldCap  = mCap;
        if (n == 0)
        {
            mData = nullptr;
            mCap  = 0;
        }
        else
        {
            mData = mAlloc.allocate(n);
            mCap  = n;
        }

        if (oldCap != 0)
        {
            for (std::size_t i = 0; i != mSize; i++)
                std::construct_at(&mData[i], std::move_if_noexcept(oldData[i]));

            for (std::size_t i = 0; i != mSize; i++)
                std::destroy_at(&oldData[i]);

            mAlloc.deallocate(oldData, oldCap);
        }
    }

    void resize(std::size_t n)
    {
        if (n < mSize)
        {
            for (std::size_t i = n; i != mSize; i++)
                std::destroy_at(&mData[i]);
            mSize = n;
        }
        else if (n > mSize)
        {
            reserve(n);
            for (std::size_t i = 0; i != mSize; i++)
                std::construct_at(&mData[i]);
        }
        mSize = n;
    }

    void resize(std::size_t n, const T &value)
    {
        if (n < mSize)
        {
            for (std::size_t i = n; i != mSize; i++)
                std::destroy_at(&mData[i]);
            mSize = n;
        }
        else if (n > mSize)
        {
            reserve(n);
            for (std::size_t i = 0; i != mSize; i++)
                std::construct_at(&mData[i], value);
        }
        mSize = n;
    }

    void shrink_to_fit() noexcept
    {
        auto oldData = mData;
        auto oldCap  = mCap;
        mCap         = mSize;

        if (mSize == 0)
            mData = nullptr;
        else
            mData = mAlloc.allocate(mSize);

        if (oldCap != 0) [[likely]]
        {
            for (std::size_t i = 0; i != mSize; i++)
            {
                std::construct_at(&mData[i], std::move_if_noexcept(oldData[i]));
                std::destroy_at(&oldData[i]);
            }
            mAlloc.deallocate(oldData, oldCap);
        }
    }

    std::size_t capacity() const noexcept { return mCap; }

    std::size_t size() const noexcept { return mSize; }

    bool empty() const noexcept { return mSize == 0; }

    static constexpr std::size_t max_size() noexcept
    {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    void swap(Vector &that) noexcept
    {
        std::swap(mData, that.mData);
        std::swap(mSize, that.mSize);
        std::swap(mCap, that.mCap);
        std::swap(mAlloc, that.mAlloc);
    }

    T &operator[](std::size_t i) noexcept { return mData[i]; }

    const T &operator[](std::size_t i) const noexcept { return mData[i]; }

    T &at(std::size_t i)
    {
        if (i >= mSize) [[unlikely]]
            throw std::out_of_range("Vector at function");
        return mData[i];
    }

    const T &at(std::size_t i) const
    {
        if (i >= mSize) [[unlikely]]
            throw std::out_of_range("Vector at function");
        return mData[i];
    }

    T &front() noexcept { return *mData; }

    const T &front() const noexcept { return *mData; }

    T &back() noexcept { return mData[mSize - 1]; }

    const T &back() const noexcept { return mData[mSize - 1]; }

    void push_back(const T &value)
    {
        if (mSize + 1 >= mCap) [[unlikely]]
            reserve(mSize + 1);

        std::construct_at(&mData[mSize], value);
        mSize = mSize + 1;
    }

    void push_back(T &&value)
    {
        if (mSize + 1 >= mCap) [[unlikely]]
            reserve(mSize + 1);

        std::construct_at(&mData[mSize], std::move(value));
        mSize = mSize + 1;
    }

    template <typename... Args>
    T &emplace_back(Args &&...args)
    {
        if (mSize + 1 >= mCap) [[unlikely]]
            reserve(mSize + 1);

        T *p = &mData[mSize];
        std::construct_at(p, std::forward<Args>(args)...);
        mSize = mSize + 1;

        return *p;
    }

    T *data() noexcept { return mData; }

    const T *data() const noexcept { return mData; }

    const T *cdata() const noexcept { return mData; }

    T *begin() noexcept { return mData; }

    T *end() noexcept { return mData + mSize; }

    const T *begin() const noexcept { return mData; }

    const T *end() const noexcept { return mData + mSize; }

    const T *cbegin() const noexcept { return mData; }

    const T *cend() const noexcept { return mData + mSize; }

    std::reverse_iterator<T *> rbegin() noexcept
    {
        return std::make_reverse_iterator(mData + mSize);
    }

    std::reverse_iterator<T *> rend() noexcept
    {
        return std::make_reverse_iterator(mData);
    }

    std::reverse_iterator<const T *> rbegin() const noexcept
    {
        return std::make_reverse_iterator(mData + mSize);
    }

    std::reverse_iterator<const T *> rend() const noexcept
    {
        return std::make_reverse_iterator(mData);
    }

    std::reverse_iterator<const T *> crbegin() const noexcept
    {
        return std::make_reverse_iterator(mData + mSize);
    }

    std::reverse_iterator<const T *> crend() const noexcept
    {
        return std::make_reverse_iterator(mData);
    }

    void pop_back() noexcept
    {
        mSize -= 1;
        std::destroy_at(&mData[mSize]);
    }

    T *erase(const T *it) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        std::size_t i = it - mData;
        for (std::size_t j = i + 1; j != mSize; j++)
            mData[j - 1] = std::move(mData[j]);

        mSize -= 1;
        std::destroy_at(&mData[mSize]);

        return const_cast<T *>(it);
    }

    /// 100, 2, 301, 99, 20
    ///  0   1   2   3   4
    ///  diff => 3 - 1 = 2;
    ///  j => 3
    ///  mdata[1] = mdata[3]
    ///  mdata[2] = mdata[4]
    T *erase(const T *first, const T *last) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        std::size_t diff = last - first;
        for (std::size_t j = last - mData; j != mSize; j++)
            mData[j - diff] = std::move(mData[j]);

        mSize -= diff;
        for (std::size_t j = mSize; j != mSize + diff; j++)
            std::destroy_at(&mData[j]);

        return const_cast<T *>(first);
    }

    void assgin(std::size_t n, const T &value)
    {
        clear();
        reserve(n);
        mSize = n;
        for (std::size_t i = 0; i != n; i++)
            std::construct_at(&mData[i], value);
    }

    template <std::random_access_iterator InputIt>
    void assgin(InputIt first, InputIt last)
    {
        std::size_t n = last - first;
        reserve(n);
        mSize = n;
        for (std::size_t i = 0; i != n; i++)
        {
            std::construct_at(mData[i], *first);
            ++first;
        }
    }

    void assgin(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }

    template <typename... Args>
    T *emplace(const T *it, Args &&...args)
    {
        std::size_t j = it - mData;
        reserve(mSize + 1);
        // [j, msize] => [j + 1, msize + 1]
        for (std::size_t i = mSize; i != j; i--)
        {
            std::construct_at(&mData[i], std::move(mData[i - 1]));
            std::destroy_at(&mData[i - 1]);
        }

        mSize += 1;
        std::construct_at(&mData[j], std::forward<Args>(args)...);
        return mData + j;
    }

    T *insert(const T *it, const T &value)
    {
        std::size_t j = it - mData;
        reserve(mSize + 1);
        // [j, msize] => [j + 1, msize + 1]
        for (std::size_t i = mSize; i != j; i--)
        {
            std::construct_at(&mData[i], std::move(mData[i - 1]));
            std::destroy_at(&mData[i - 1]);
        }
        mSize += 1;
        std::construct_at(&mData[j], value);
        return mData + j;
    }

    T *insert(const T *it, T &&value)
    {
        std::size_t j = it - mData;
        reserve(mSize + 1);
        // [j, msize] => [j + 1, msize + 1]
        for (std::size_t i = mSize; i != j; i--)
        {
            std::construct_at(&mData[i], std::move(mData[i - 1]));
            std::destroy_at(&mData[i - 1]);
        }
        mSize += 1;
        std::construct_at(&mData[j], std::move(value));
        return mData + j;
    }

    T *insert(const T *it, std::size_t n, const T &value)
    {
        std::size_t j = it - mData;
        if (n == 0) [[unlikely]]
            return const_cast<T *>(it);

        reserve(n + mSize);

        // [j, msize] => [j + n, msize + n]
        for (std::size_t i = mSize; i != j; i--)
        {
            std::construct_at(&mData[i + n - 1], std::move(mData[i - 1]));
            std::destroy_at(&mData[i - 1]);
        }

        mSize += n;
        for (std::size_t i = j; i != j + n; i++)
            std::construct_at(&mData[i], value);

        return mData + j;
    }

    template <std::random_access_iterator InputIt>
    T *insert(const T *it, InputIt first, InputIt last)
    {
        std::size_t j = it - mData;
        std::size_t n = last - first;
        if (n == 0) [[unlikely]]
            return const_cast<T *>(it);

        reserve(mSize + n);

        // [j, msize] => [j + n, msize + n]
        for (std::size_t i = mSize; i != j; i--)
        {
            std::construct_at(&mData[i + n - 1], std::move(mData[i - 1]));
            std::destroy_at(&mData[i - 1]);
        }

        mSize += n;
        for (std::size_t i = j; i != j + n; i++)
        {
            std::construct_at(&mData[i], *first);
            ++first;
        }

        return mData + j;
    }

    T *insert(const T *it, std::initializer_list<T> ilist)
    {
        return insert(it, ilist.begin(), ilist.end());
    }

    Alloc get_allocator() const noexcept { return mAlloc; }

    bool operator==(const Vector &that) noexcept
    {
        return std::equal(begin(), end(), that.begin(), that.end());
    }

    auto operator<=>(const Vector &that) noexcept
    {
        return std::lexicographical_compare_three_way(begin(),
                                                      end(),
                                                      that.begin(),
                                                      that.end());
    }
};
