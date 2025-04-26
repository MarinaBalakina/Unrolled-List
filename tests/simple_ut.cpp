#pragma once

#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <algorithm>
#include <limits>
#include <cstddef>

struct Node_Tag {};

template<typename T, std::size_t NodeMaxSize = 10, typename Allocator = std::allocator<T>>
class unrolled_list {
public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type  = Allocator;

private:
    struct node_struct {
        node_struct* prev;
        node_struct* next;
        std::size_t count;
        alignas(T) unsigned char storage[NodeMaxSize * sizeof(T)];

        node_struct() : prev(nullptr), next(nullptr), count(0) {}

        T* get_ptr(std::size_t i) {
            return reinterpret_cast<T*>(storage + i * sizeof(T));
        }
        const T* get_ptr(std::size_t i) const {
            return reinterpret_cast<const T*>(storage + i * sizeof(T));
        }

        void construct_elem(std::size_t idx, const T& val) {
            new (static_cast<void*>(get_ptr(idx))) T(val);
        }
        void construct_elem(std::size_t idx, T&& val) {
            new (static_cast<void*>(get_ptr(idx))) T(std::move(val));
        }
        void destroy_elem(std::size_t idx) noexcept {
            get_ptr(idx)->~T();
        }
    };

    using node_alloc_type = typename std::allocator_traits<Allocator>::template rebind_alloc<Node_Tag>;
    node_alloc_type node_alloc;
    allocator_type val_alloc;
    node_struct* head;
    node_struct* tail;
    size_type size_;

public:
    template<bool is_const>
    class iterators_class {
    public:
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer           = std::conditional_t<is_const, const T*, T*>;
        using reference         = std::conditional_t<is_const, const T&, T&>;

        node_struct* node_ptr;
        std::size_t index;

        iterators_class(node_struct* n = nullptr, std::size_t i = 0)
            : node_ptr(n), index(i)
        {}

        // Преобразование iterator -> const_iterator
        template<bool B, typename = std::enable_if_t<!B && is_const>>
        iterators_class(const iterators_class<B>& other)
            : node_ptr(other.node_ptr), index(other.index)
        {}

        reference operator*() const {
            return *(node_ptr->get_ptr(index));
        }
        pointer operator->() const {
            return node_ptr->get_ptr(index);
        }

        iterators_class& operator++() {
            if (node_ptr) {
                if (index + 1 < node_ptr->count) {
                    ++index;
                } else {
                    if (node_ptr->next) {
                        node_ptr = node_ptr->next;
                        index = 0;
                    } else {
                        node_ptr = nullptr;
                        index = 0;
                    }
                }
            }
            return *this;
        }
        iterators_class operator++(int) {
            iterators_class tmp(*this);
            ++(*this);
            return tmp;
        }
        iterators_class& operator--() {
            if (node_ptr) {
                if (index > 0) {
                    --index;
                } else {
                    if (node_ptr->prev) {
                        node_ptr = node_ptr->prev;
                        index = node_ptr->count - 1;
                    } else {
                        node_ptr = nullptr;
                        index = 0;
                    }
                }
            }
            return *this;
        }
        iterators_class operator--(int) {
            iterators_class tmp(*this);
            --(*this);
            return tmp;
        }

        bool operator==(const iterators_class& other) const {
            return (node_ptr == other.node_ptr) && (index == other.index);
        }
        bool operator!=(const iterators_class& other) const {
            return !(*this == other);
        }
    };

    using iterator       = iterators_class<false>;
    using const_iterator = iterators_class<true>;

    template<bool is_const>
    class rev_iterator_class {
    public:
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer           = std::conditional_t<is_const, const T*, T*>;
        using reference         = std::conditional_t<is_const, const T&, T&>;

        iterators_class<is_const> base_iter;

        explicit rev_iterator_class(iterators_class<is_const> it)
            : base_iter(it)
        {}

        reference operator*() {
            auto tmp = base_iter;
            --tmp;
            return *tmp;
        }
        pointer operator->() {
            auto tmp = base_iter;
            --tmp;
            return tmp.operator->();
        }

        rev_iterator_class& operator++() {
            --base_iter;
            return *this;
        }
        rev_iterator_class operator++(int) {
            rev_iterator_class tmp(*this);
            --base_iter;
            return tmp;
        }
        rev_iterator_class& operator--() {
            ++base_iter;
            return *this;
        }
        rev_iterator_class operator--(int) {
            rev_iterator_class tmp(*this);
            ++base_iter;
            return tmp;
        }

        bool operator==(const rev_iterator_class& other) const {
            return base_iter == other.base_iter;
        }
        bool operator!=(const rev_iterator_class& other) const {
            return !(*this == other);
        }

        iterators_class<is_const> base() const {
            return base_iter;
        }
    };

    using reverse_iterator       = rev_iterator_class<false>;
    using const_reverse_iterator = rev_iterator_class<true>;

    iterator begin() noexcept {
        return iterator(head, 0);
    }
    const_iterator begin() const noexcept {
        return const_iterator(head, 0);
    }
    const_iterator cbegin() const noexcept {
        return const_iterator(head, 0);
    }
    iterator end() noexcept {
        return iterator(nullptr, 0);
    }
    const_iterator end() const noexcept {
        return const_iterator(nullptr, 0);
    }
    const_iterator cend() const noexcept {
        return const_iterator(nullptr, 0);
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

    unrolled_list()
        : node_alloc(), val_alloc(), head(nullptr), tail(nullptr), size_(0)
    {}
    explicit unrolled_list(const allocator_type& alloc)
        : node_alloc(alloc), val_alloc(alloc), head(nullptr), tail(nullptr), size_(0)
    {}
    template<typename input_iter>
    unrolled_list(input_iter first, input_iter last, const allocator_type& alloc = allocator_type())
        : unrolled_list(alloc)
    {
        while (first != last) {
            push_back(*first);
            ++first;
        }
    }
    unrolled_list(size_type n, const T& val, const allocator_type& alloc = allocator_type())
        : unrolled_list(alloc)
    {
        while (n--) {
            push_back(val);
        }
    }
    unrolled_list(std::initializer_list<T> il, const allocator_type alloc = allocator_type())
        : unrolled_list(alloc)
    {
        for (auto& elem : il) {
            push_back(elem);
        }
    }
    unrolled_list(const unrolled_list& other)
        : unrolled_list(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.val_alloc))
    {
        for (auto el : other) {
            push_back(el);
        }
    }
    unrolled_list(unrolled_list&& other, const allocator_type& alloc)
        : unrolled_list(alloc)
    {
        for (auto& el : other) {
            push_back(std::move(el));
        }
        other.clear();
    }
    unrolled_list(unrolled_list&& other) noexcept
        : node_alloc(std::move(other.node_alloc)),
          val_alloc(std::move(other.val_alloc)),
          head(other.head),
          tail(other.tail),
          size_(other.size_)
    {
        other.head = nullptr;
        other.tail = nullptr;
        other.size_ = 0;
    }
    ~unrolled_list() {
        clear();
    }

    unrolled_list& operator=(const unrolled_list& other) {
        if (this != &other) {
            clear();
            for (auto el : other) {
                push_back(el);
            }
        }
        return *this;
    }
    unrolled_list& operator=(unrolled_list&& other) noexcept {
        if (this != &other) {
            clear();
            node_alloc = std::move(other.node_alloc);
            val_alloc  = std::move(other.val_alloc);
            head       = other.head;
            tail       = other.tail;
            size_      = other.size_;
            other.head = nullptr;
            other.tail = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    unrolled_list& operator=(std::initializer_list<T> il) {
        clear();
        for (auto&& el : il) {
            push_back(el);
        }
        return *this;
    }

    // Для AllocatorAwareContainer
    allocator_type get_allocator() const {
        return val_alloc;
    }

    // Для Container / SequenceContainer
    size_type size() const noexcept {
        return size_;
    }
    bool empty() const noexcept {
        return (size_ == 0);
    }
    size_type max_size() const noexcept {
        return (std::numeric_limits<size_type>::max)();
    }

    // Операторы сравнения (для Container)
    bool operator==(const unrolled_list& rhs) const {
        if (size_ != rhs.size_) return false;
        auto it1 = begin();
        auto it2 = rhs.begin();
        while (it1 != end()) {
            if (*it1 != *it2) return false;
            ++it1;
            ++it2;
        }
        return true;
    }
    bool operator!=(const unrolled_list& rhs) const {
        return !(*this == rhs);
    }

    void clear() noexcept {
        node_struct* cur_node = head;
        while (cur_node) {
            node_struct* next_node = cur_node->next;
            for (std::size_t i = 0; i < cur_node->count; ++i) {
                cur_node->destroy_elem(i);
            }
            deallocate_node(cur_node);
            cur_node = next_node;
        }
        head = nullptr;
        tail = nullptr;
        size_ = 0;
    }

    void push_back(const T& val) {
        if (!tail) {
            node_struct* nd = allocate_node();
            nd->construct_elem(0, val);
            nd->count = 1;
            head = tail = nd;
            size_ = 1;
        } else {
            if (tail->count < NodeMaxSize) {
                tail->construct_elem(tail->count, val);
                tail->count++;
                size_++;
            } else {
                node_struct* nd = allocate_node();
                nd->construct_elem(0, val);
                nd->count = 1;
                tail->next = nd;
                nd->prev = tail;
                tail = nd;
                size_++;
            }
        }
    }
    void push_back(T&& val) {
        if (!tail) {
            node_struct* nd = allocate_node();
            nd->construct_elem(0, std::move(val));
            nd->count = 1;
            head = tail = nd;
            size_ = 1;
        } else {
            if (tail->count < NodeMaxSize) {
                tail->construct_elem(tail->count, std::move(val));
                tail->count++;
                size_++;
            } else {
                node_struct* nd = allocate_node();
                nd->construct_elem(0, std::move(val));
                nd->count = 1;
                tail->next = nd;
                nd->prev = tail;
                tail = nd;
                size_++;
            }
        }
    }

    void pop_back() noexcept {
        if (!tail) return;
        if (tail->count > 0) {
            tail->destroy_elem(tail->count - 1);
            tail->count--;
            size_--;
            if (tail->count == 0) {
                if (head == tail) {
                    deallocate_node(tail);
                    head = tail = nullptr;
                } else {
                    node_struct* tmp = tail;
                    tail = tail->prev;
                    tail->next = nullptr;
                    deallocate_node(tmp);
                }
            }
        }
    }

    void push_front(const T& val) {
        if (!head) {
            node_struct* nd = allocate_node();
            nd->construct_elem(0, val);
            nd->count = 1;
            head = tail = nd;
            size_ = 1;
        } else {
            if (head->count < NodeMaxSize) {
                for (std::size_t i = head->count; i > 0; i--) {
                    head->construct_elem(i, std::move(*(head->get_ptr(i - 1))));
                    head->destroy_elem(i - 1);
                }
                head->construct_elem(0, val);
                head->count++;
                size_++;
            } else {
                node_struct* nd = allocate_node();
                nd->construct_elem(0, val);
                nd->count = 1;
                nd->next = head;
                head->prev = nd;
                head = nd;
                size_++;
            }
        }
    }
    void push_front(T&& val) {
        if (!head) {
            node_struct* nd = allocate_node();
            nd->construct_elem(0, std::move(val));
            nd->count = 1;
            head = tail = nd;
            size_ = 1;
        } else {
            if (head->count < NodeMaxSize) {
                for (std::size_t i = head->count; i > 0; i--) {
                    head->construct_elem(i, std::move(*(head->get_ptr(i - 1))));
                    head->destroy_elem(i - 1);
                }
                head->construct_elem(0, std::move(val));
                head->count++;
                size_++;
            } else {
                node_struct* nd = allocate_node();
                nd->construct_elem(0, std::move(val));
                nd->count = 1;
                nd->next = head;
                head->prev = nd;
                head = nd;
                size_++;
            }
        }
    }
    void pop_front() noexcept {
        if (!head) return;
        if (head->count > 0) {
            head->destroy_elem(0);
            for (std::size_t i = 1; i < head->count; i++) {
                head->construct_elem(i - 1, std::move(*(head->get_ptr(i))));
                head->destroy_elem(i);
            }
            head->count--;
            size_--;
            if (head->count == 0) {
                if (head == tail) {
                    deallocate_node(head);
                    head = tail = nullptr;
                } else {
                    node_struct* tmp = head;
                    head = head->next;
                    if (head) head->prev = nullptr;
                    deallocate_node(tmp);
                }
            }
        }
    }

    T& front() {
        return *(head->get_ptr(0));
    }
    const T& front() const {
        return *(head->get_ptr(0));
    }
    T& back() {
        return *(tail->get_ptr(tail->count - 1));
    }
    const T& back() const {
        return *(tail->get_ptr(tail->count - 1));
    }

    template<typename input_iter>
    iterator insert(const_iterator pos, input_iter first, input_iter last) {
        iterator res;
        while (first != last) {
            res = insert(pos, *first);
            ++first;
            ++pos;
        }
        return res;
    }
    iterator insert(const_iterator pos, std::initializer_list<T> il) {
        iterator ret;
        for (auto&& val : il) {
            ret = insert(pos, val);
            ++pos;
        }
        return ret;
    }
    iterator insert(const_iterator pos, size_type n, T& val) {
        iterator ret;
        for (size_type i = 0; i < n; i++) {
            ret = insert(pos, val);
            ++pos;
        }
        return ret;
    }

    iterator insert(const_iterator pos, const T& val) {
        if (!pos.node_ptr) {
            push_back(val);
            if (tail) {
                return iterator(tail, tail->count - 1);
            } else {
                return iterator(nullptr, 0);
            }
        }
        return do_insert(pos, val);
    }
    iterator insert(const_iterator pos, T&& val) {
        if (!pos.node_ptr) {
            push_back(std::move(val));
            if (!tail) {
                return iterator(nullptr, 0);
            } else {
                return iterator(tail, tail->count - 1);
            }
        }
        return do_insert(pos, std::move(val));
    }

    iterator erase(const_iterator pos) noexcept {
        if (!pos.node_ptr) return end();
        return do_erase(pos);
    }
    iterator erase(const_iterator first_it, const_iterator last_it) {
        while (first_it != last_it) {
            first_it = erase(first_it);
        }
        return iterator(last_it.node_ptr, last_it.index);
    }

private:
    iterator do_insert(const_iterator pos, const T& val) {
        node_struct* n = pos.node_ptr;
        if (n->count < NodeMaxSize) {
            n->construct_elem(n->count, val);
            ++n->count;
            ++size_;
            return iterator(n, n->count - 1);
        } else {
            node_struct* nd = allocate_node();
            nd->construct_elem(0, val);
            nd->count = 1;
            ++size_;
            nd->next = n->next;
            if (n->next) {
                n->next->prev = nd;
            }
            n->next = nd;
            nd->prev = n;
            if (n == tail) {
                tail = nd;
            }
            return iterator(nd, 0);
        }
    }
    iterator do_insert(const_iterator pos, T&& val) {
        node_struct* n = pos.node_ptr;
        if (n->count < NodeMaxSize) {
            n->construct_elem(n->count, std::move(val));
            ++n->count;
            ++size_;
            return iterator(n, n->count - 1);
        } else {
            node_struct* nd = allocate_node();
            nd->construct_elem(0, std::move(val));
            nd->count = 1;
            ++size_;
            nd->next = n->next;
            if (n->next) {
                n->next->prev = nd;
            }
            n->next = nd;
            nd->prev = n;
            if (n == tail) {
                tail = nd;
            }
            return iterator(nd, 0);
        }
    }
    iterator do_erase(const_iterator pos) noexcept {
        node_struct* n = pos.node_ptr;
        std::size_t idx = pos.index;
        if (!n) return end();

        if (n->count > 1) {
            n->destroy_elem(idx);
            --n->count;
            --size_;
            return iterator(n, idx);
        } else {
            --n->count;
            --size_;
            if (n == head && n == tail) {
                deallocate_node(n);
                head = tail = nullptr;
                return end();
            } else if (n == head) {
                head = n->next;
                if (head) head->prev = nullptr;
                deallocate_node(n);
                return iterator(head, 0);
            } else if (n == tail) {
                tail = n->prev;
                if (tail) tail->next = nullptr;
                deallocate_node(n);
                return end();
            } else {
                node_struct* p = n->prev;
                node_struct* nx = n->next;
                if (p) p->next = nx;
                if (nx) nx->prev = p;
                deallocate_node(n);
                return iterator(nx, 0);
            }
        }
    }

    node_struct* allocate_node() {
        Node_Tag* raw_mem = node_alloc.allocate(1);
        void* raw_ptr = static_cast<void*>(raw_mem);
        node_struct* nd = new (raw_ptr) node_struct();
        return nd;
    }
    void deallocate_node(node_struct* nd) noexcept {
        nd->~node_struct();
        node_alloc.deallocate(reinterpret_cast<Node_Tag*>(nd), 1);
    }
};
