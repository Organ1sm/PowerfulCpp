#pragma once

#include <concepts>
#include <cstdio>
#include <utility>

template<typename T>
struct DefaultDeleter
{
    void operator()(T *p) const { delete p; }
};

template<typename T>
struct DefaultDeleter<T[]>
{
    void operator()(T *p) const { delete[] p; }
};

template<>
struct DefaultDeleter<std::FILE>
{
    void operator()(std::FILE *p) const { fclose(p); }
};

template<typename T, typename U>
T exchange(T &dest, U &&value)
{
    T tmp = std::move(dest);
    dest  = std::forward<U>(value);
    return tmp;
}

template<typename T, typename Deleter = DefaultDeleter<T>>
struct UniquePtr
{
  private:
    T *m_p;

    template<typename U, typename UDeleter>
    friend struct UniquePtr;

  public:
    // Defalut Constructor
    UniquePtr(std::nullptr_t dummy = nullptr) : m_p(nullptr) { }

    explicit UniquePtr(T *p) : m_p(p) { }

    // Before C++20
    // template <class U, class UDeleter, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    template<typename U, typename UDeleter>
        requires(std::convertible_to<U *, T *>)
    UniquePtr(UniquePtr<U, UDeleter> &&that)
    {
        this->m_p = exchange(that.m_p, nullptr);
    }

    // Destructor
    ~UniquePtr()
    {
        if (this->m_p)
            Deleter {}(this->m_p);
    }

    UniquePtr(const UniquePtr &)            = delete;
    UniquePtr &operator=(const UniquePtr &) = delete;

    UniquePtr(UniquePtr &&that) { this->m_p = exchange(that.m_p, nullptr); }

    UniquePtr &operator==(UniquePtr &&that)
    {
        if (this != &that) [[likely]]
        {
            if (this->m_p)
                Deleter {}(this->m_p);
            this->m_p = exchange(that.m_p, nullptr);
        }
        return *this;
    }

    T *get() const { return this->m_p; }

    T *release() { return exchange(this->m_p, nullptr); }

    void reset(T *p = nullptr)
    {
        if (this->m_p)
            Deleter {}(this->m_p);
        this->m_p = p;
    }

    T &operator*() const { return *this->m_p; }

    T *operator->() const { return this->m_p; }
};

template<typename T, typename Deleter>
struct UniquePtr<T[], Deleter> : UniquePtr<T, Deleter>
{ };

template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args &&...args)
{
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
UniquePtr<T> makeUniqueForOverwrite()
{
    return UniquePtr<T>(new T);
}
