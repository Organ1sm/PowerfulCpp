#pragma once

#include <cstddef>
#include <deque>
#include <initializer_list>
#include <limits>
#include <memory>
#include <utility>
#ifdef NDEBUG
    #define DEBUG_INIT_DEADBEAF(T)
#else
    #define DEBUG_INIT_DEADBEAF(T) \
        {                          \
            (T *) 0xdeadbeaf       \
        }
#endif   // DEBUG

template <typename T>
struct ListBaseNode
{
    ListBaseNode *next DEBUG_INIT_DEADBEAF(ListBaseNode);
    ListBaseNode *prev DEBUG_INIT_DEADBEAF(ListBaseNode);

    inline T &value();
    inline const T &value() const;
};

template <typename T>
struct ListValueNode : ListBaseNode<T>
{
    union
    {
        T mValue;
    };
};

template <typename T>
inline T &ListBaseNode<T>::value()
{
    return static_cast<ListValueNode<T> &>(*this).mValue;
}

template <typename T>
inline const T &ListBaseNode<T>::value() const
{
    return static_cast<const ListValueNode<T> &>(*this).mValue;
}

template <typename T, typename Alloc = std::allocator<T>>
struct List
{
    using value_type      = T;
    using allocator_type  = Alloc;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T *;
    using const_pointer   = const T *;
    using reference       = T &;
    using const_reference = const T &;

  private:
    using ListNode = ListBaseNode<T>;
    using AllocNode =
            std::allocator_traits<Alloc>::template rebindAlloc<ListValueNode<T>>;

    ListNode mDummy;
    std::size_t mSize;
    [[no_unique_address]] Alloc mAlloc;

    ListNode *newNode() { return AllocNode {mAlloc}.allocate(1); }

    void deleteNode(ListNode *node) noexcept
    {
        AllocNode {mAlloc}.deallocate(static_cast<ListValueNode<T> *>(node), 1);
    }

  public:
    List() noexcept
    {
        mSize       = 0;
        mDummy.prev = mDummy.next = &mDummy;
    }

    explicit List(const Alloc &alloc) noexcept : mAlloc(alloc)
    {
        mSize       = 0;
        mDummy.prev = mDummy.next = &mDummy;
    }

    explicit List(std::size_t n, const Alloc &alloc = Alloc()) : mAlloc(alloc)
    {
        uninitAssign(n);
    }

    explicit List(std::size_t n, const T &value, const Alloc &alloc = Alloc())
        : mAlloc(alloc)
    {
        uninitAssign(n, value);
    }

    List(List &&that) : mAlloc(std::move(that.mAlloc))
    {
        uninitMoveAssgin(std::move(that));
    }

    List(List &&that, const Alloc &alloc) : mAlloc(alloc)
    {
        uninitMoveAssgin(std::move(that));
    }

    List &operator=(List &&that)
    {
        mAlloc = std::move(that.mAlloc);
        clear();
        uninitMoveAssgin(std::move(that));
    }

    List(const List &that) : mAlloc(that.mAlloc)
    {
        uninitAssign(that.cbegin(), that.cend());
    }

    List(const List &that, const Alloc &alloc) : mAlloc(alloc)
    {
        uninitAssign(that.cbegin(), that.cend());
    }

    List &operator=(const List &that) { assign(that.cbegin(), that.cend()); }

    // input_iterator = *it it++ ++it it!=it it==it
    // output_iterator = *it=val it++ ++it it!=it it==it
    // forward_iterator = *it *it=val it++ ++it it!=it it==it
    // bidirectional_iterator = *it *it=val it++ ++it it-- --it it!=it it==it
    // random_access_iterator = *it *it=val it[n] it[n]=val it++ ++it it-- --it it+=n it-=n it+n it-n it!=it it==it

    template <std::input_iterator InputIt>
    List(InputIt first, InputIt last, const Alloc &alloc = Alloc())
    {
        uninitAssign(first, last);
    }

    List(std::initializer_list<T> ilist, const Alloc &alloc = Alloc())
        : List(ilist.begin(), ilist.end(), alloc)
    { }

    List &operator=(std::initializer_list<T> ilist) { assign(ilist); }

    ~List() noexcept { clear(); }

    bool empty() noexcept { return mDummy.prev == mDummy.next; }

    T &front() noexcept { return mDummy.next->value(); }

    const T &front() const noexcept { return mDummy.next->value(); }

    T &back() noexcept { return mDummy.prev->value(); }

    const T &back() const noexcept { return mDummy.prev->value(); }

    std::size_t size() const noexcept { return mSize; }

    static constexpr std::size_t max_size() noexcept
    {
        return std::numeric_limits<std::size_t>::max();
    }

    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last)
    {
        clear();
        uninitAssign(first, last);
    }

    void assign(std::initializer_list<T> ilist)
    {
        clear();
        uninitAssign(ilist.begin(), ilist.end());
    }

    void assign(std::size_t n, const T &value)
    {
        clear();
        uninitAssign(n, value);
    }

    template <typename... Args>
    T &emplace_back(Args &&...args)
    {
        ListNode *node = newNode();
        ListNode *prev = mDummy.prev;

        prev->next = node;
        node->prev = prev;
        node->next = &mDummy;

        std::construct_at(&node->value(), std::forward<Args>(args)...);
        mDummy.prev = node;
        ++mSize;
        return node->value();
    }

    template <typename... Args>
    T &emplace_front(Args &&...args)
    {
        ListNode *node = newNode();
        ListNode *next = mDummy.next;

        node->next = next;
        node->prev = &mDummy;
        next->prev = node;

        std::construct_at(&node->value(), std::forward<Args>(args)...);
        mDummy.next = node;
        ++mSize;
        return node->value();
    }

    void push_back(const T &value) { emplace_back(value); }

    void push_back(T &&value) { emplace_back(std::move(value)); }

    void push_front(const T &value) { emplace_front(value); }

    void push_front(T &&value)
    {   // don't repeat yourself (DRY)
        emplace_front(std::move(value));
    }

    void clear() noexcept
    {
        ListNode *curr = mDummy.next;
        while (curr != &mDummy)
        {
            std::destroy_at(&curr->value());
            auto next = curr->next;
            deleteNode(curr);
            curr = next;
        }
        mDummy.prev = mDummy.next = &mDummy;
        mSize                     = 0;
    }

  private:
    void uninitMoveAssgin(List &&that)
    {
        auto prevNode = that.mDummy.prev;
        auto nextNode = that.mDummy.next;

        prevNode->next   = &mDummy;
        nextNode->prev   = &mDummy;
        mDummy           = that.mDummy;
        that.mDummy.prev = that.mDummy.next = &that.mDummy;
        mSize                               = that.mSize;
        that.mSize                          = 0;
    }

    template <std::input_iterator InputIt>
    void uninitAssign(InputIt first, InputIt last)
    {
        mSize          = 0;
        ListNode *prev = &mDummy;
        while (first != last)
        {
            ListNode *node = newNode();
            prev->next     = node;
            node->prev     = prev;
            std::construct_at(&node->value(), *first);
            prev = node;
            ++first;
            ++mSize;
        }
    }

    void uninitAssign(std::size_t n)
    {
        ListNode *prev = &mDummy;
        while (n)
        {
            ListNode *node = newNode();
            prev->next     = node;
            node->prev     = prev;
            std::construct_at(&node->value());
            prev = node;
            --n;
        }

        mDummy.prev = prev;
        prev->next  = &mDummy;
        mSize       = n;
    }

    void uninitAssign(std::size_t n, const T &value)
    {
        ListNode *prev = &mDummy;
        while (n)
        {
            ListNode *node = newNode();
            prev->next     = node;
            node->prev     = prev;
            std::construct_at(&node->value(), value);
            prev = node;
            --n;
        }

        mDummy.prev = prev;
        prev->next  = &mDummy;
        mSize       = n;
    }
};
