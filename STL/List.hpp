#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iterator>
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
            std::allocator_traits<Alloc>::template rebind_alloc<ListValueNode<T>>;

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
        mDummy.prev = prev;
        prev->next  = &mDummy;
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

  public:

    struct iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;

      private:
        ListNode *mCurr;

        friend List;

        explicit iterator(ListNode *curr) noexcept : mCurr(curr) { }

        // ++iterator
        iterator &operator++() noexcept
        {
            mCurr = mCurr->next;
            return *this;
        }

        // iterator++
        iterator operator++(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        // --iterator
        iterator &operator--() noexcept
        {
            mCurr = mCurr->prev;
            return *this;
        }

        // iterator--
        iterator operator--(int) noexcept
        {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        T &operator*() const noexcept { return mCurr->value(); }

        bool operator!=(const iterator &that) const noexcept
        {
            return mCurr != that.mCurr;
        }

        bool operator==(const iterator &that) const noexcept { return !(*this != that); }
    };

    struct const_iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T *;
        using reference         = const T &;

      private:
        const ListNode *mCurr;

        friend List;

        explicit const_iterator(const ListNode *curr) noexcept : mCurr(curr) { }

      public:
        const_iterator() = default;

        const_iterator(iterator that) noexcept : mCurr(that.mCurr) { }

        explicit operator iterator() noexcept
        {
            return iterator {const_cast<ListNode *>(mCurr)};
        }

        // ++iterator
        const_iterator &operator++() noexcept
        {
            mCurr = mCurr->next;
            return *this;
        }

        // iterator++
        const_iterator operator++(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        // iterator++
        const_iterator &operator--() noexcept
        {
            mCurr = mCurr->next;
            return *this;
        }

        // --iterator
        const_iterator operator--(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        const T &operator*() const noexcept { return mCurr->value(); }

        bool operator==(const const_iterator &that) const noexcept
        {
            return mCurr == that.mCurr;
        }

        bool operator!=(const const_iterator &that) const noexcept
        {
            return !(*this == that);
        }
    };

    iterator begin() noexcept { return iterator {mDummy.next}; }

    iterator end() noexcept { return iterator {mDummy.prev}; }

    const_iterator cbegin() const noexcept { return const_iterator {mDummy.next}; }

    const_iterator cend() const noexcept { return const_iterator {mDummy.prev}; }

    const_iterator begin() const noexcept { return cbegin(); }

    const_iterator end() const noexcept { return cend(); }

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }

    reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const noexcept
    {
        return std::make_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const noexcept
    {
        return std::make_reverse_iterator(cbegin());
    }

    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    const_reverse_iterator rend() const noexcept { return crend(); }

    iterator erase(const_iterator pos) noexcept
    {
        ListNode *node = const_cast<ListNode *>(pos.mCurr);
        auto next      = node->next;
        auto prev      = node->prev;
        prev->next     = next;
        next->prev     = prev;

        std::destroy_at(&node->value());
        --mSize;
        return iterator {next};
    }

    iterator erase(const_iterator first, const_iterator last) noexcept
    {
        while (first != last)
            first = erase(first);
        return iterator(first);
    }

    void pop_front() noexcept { erase(begin()); }

    void pop_back() noexcept { erase(std::prev(end())); }

    std::size_t remove(const T &value) noexcept
    {
        auto first = begin();
        auto last  = begin();

        std::size_t count = 0;
        while (first != last)
        {
            if (*first == value)
            {
                first = erase(first);
                ++count;
            }
            else
            {
                ++first;
            }
        }

        return count;
    }

    template <typename Predict>
    std::size_t remove_if(Predict &&pred) noexcept
    {
        auto first = begin();
        auto last  = begin();

        std::size_t count = 0;
        while (first != last)
        {
            if (pred(*first))
            {
                first = erase(first);
                ++count;
            }
            else
            {
                ++first;
            }
        }

        return count;
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args &&...args)
    {
        ListNode *curr = newNode();
        ListNode *next = const_cast<ListNode *>(pos.mCurr);
        ListNode *prev = next->prev;

        curr->prev = prev;
        prev->next = curr;
        curr->next = next;
        next->prev = curr;

        std::construct_at(&curr->value(), std::forward<Args>(args)...);
        ++mSize;

        return iterator {curr};
    }

    iterator insert(const_iterator pos, const T &value) { return emplace(pos, value); }

    iterator insert(const_iterator pos, T &&value)
    {
        return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator pos, std::size_t n, const T &value)
    {
        auto origin    = pos;
        bool hadOrigin = false;

        while (n)
        {
            pos = emplace(pos, value);
            if (!hadOrigin)
            {
                hadOrigin = true;
                origin    = pos;
            }
            ++pos;
            --n;
        }

        return iterator(origin);
    }

    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        auto origin    = pos;
        bool hadOrigin = false;

        while (first != last)
        {
            pos = emplace(pos, *first);
            if (!hadOrigin)
            {
                hadOrigin = true;
                origin    = pos;
            }
            ++pos;
            ++first;
        }

        return iterator(origin);
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist)
    {
        return insert(pos, ilist.begin(), ilist.end());
    }

    void splice(const_iterator pos, List &&that)
    {
        insert(pos,
               std::make_move_iterator(that.begin()),
               std::make_move_iterator(that.end()));
    }

    Alloc get_allocator() const { return mAlloc; }

    bool operator==(const List &that) noexcept
    {
        return std::equal(begin(), end(), that.begin(), that.end());
    }

    auto operator<=>(const List &that) noexcept
    {
        return std::lexicographical_compare_three_way(begin(),
                                                      end(),
                                                      that.begin(),
                                                      that.end());
    }
};
