/* deque.hpp
 * 【双端队列】允许以O(1)时间在两端增/删，且支持随机访问[]
 * STL当中 <stl_deque.h>/<deque> 部分内容的简化版
 */
#ifndef __DEQUE__
#define __DEQUE__
#include <initializer_list>
#include "alloc.hpp"    // FirstAlloc, Allocator
#include "traits.hpp"   // ...
#include "utils.hpp"    // mystl_max()
using namespace std;


// 双端队列迭代器【立大功】
template < class Type, size_t buf_size >
struct __DequeIterator {
    // 内部类型定义
    typedef RandomIteratorTag       iterator_category;
    typedef Type                    value_type;
    typedef Type*                   pointer;
    typedef Type&                   reference;
    typedef ptrdiff_t               difference_type;
    typedef __DequeIterator<Type, buf_size> iterator;
    // 成员变量
    Type*   cur;    // 当前数据的指针
    Type**  buf;    // 当前缓冲区(Type*)的指针
    // 构造函数
    __DequeIterator(): 
        cur(nullptr), buf(nullptr) {}
    // *self, ->self, self==other, self<other, self++, ++self, self-other, self+=n, self+n, self[] ...
    Type& operator*()   const { return *cur; }
    Type* operator->()  const { return cur; }

    bool operator==(const iterator& other) const { return cur==other.cur; }
    bool operator!=(const iterator& other) const { return cur!=other.cur; }

    bool operator<(const iterator& other) const 
        { return (buf==other.buf) ? (cur<other.cur) : (buf<other.buf); }
    bool operator>(const iterator& other) const 
        { return (buf==other.buf) ? (cur>other.cur) : (buf>other.buf); }

    iterator& operator++() {
        if (++cur == *buf+buf_size)             // ++cur后到达当前缓冲区buf的finish 【finish即最后一个元素的后一个的指针】
            { ++buf; cur=*buf; }                // 跳到下一个缓冲区
        return *this;
    }
    iterator operator++(int) {
        iterator tmp = *this;
        this->operator++();
        return tmp;
    }

    iterator& operator--() {
        if (cur-- == *buf)                      // cur到达当前缓冲区buf的begin 【begin即第一个元素的指针，*buf】
            { --buf; cur=(*buf)+buf_size-1; }   // 跳到上一个缓冲区
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        this->operator--();
        return tmp;
    }

    difference_type operator-(const iterator& other) const 
        { return difference_type(buf_size) * (buf-other.buf-1) 
            + (cur - *buf) + (other.*buf+buf_size - other.cur); }

    iterator& operator+=(difference_type n) {
        difference_type offset = (cur-*buf) + n;  // 从当前缓冲区buf的begin开始算起的偏移量
        // 在同一个缓冲区内 
        if (0 <= offset  &&  offset < buf_size)
            cur += n;  // 或*buf+=offset
        // 在不同缓冲区
        else {
            difference_type buf_offset = offset>0 ? offset/(difference_type)buf_size : 
                ((offset+1) / (difference_type)buf_size) - 1 ;          // 缓冲区的偏移量
            buf += buf_offset;
            cur = *buf + (offset - buf_offset*(difference_type)buf_size);
        }
        return *this;
    }
    iterator operator+(difference_type n) const { return iterator(*this) += n; }
    iterator& operator-=(difference_type n)     { return this->operator+=(-n); }
    iterator operator-(difference_type n) const { return iterator(*this) -= n; }

    Type& operator[](size_t n) 
        { return *((this->operator+(n)).cur); }     // 可修改
    const Type& operator[](size_t n) const  
        { return *(this->operator+(n)); }           // 不可修改
};


// """双端队列Deque"""
template < class Type, class Alloc = FirstAlloc >
class Deque {
public:     // 一些常量
    static const size_t buffer_size = sizeof(Type)<512 ? 512/sizeof(Type) : 1;  // 每个缓冲区buffer的元素个数【跟Type挂钩，以static const声明】
    static const size_t init_map_size = 8ULL;                                   // 中控器map的初始长度

public:     // 【内部类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef __DequeIterator<Type, buffer_size>  iterator;           // 【迭代器】
    typedef Allocator<Type, Alloc>              buffer_allocator;   // 【用于分配每个缓冲区的空间】
    typedef Allocator<Type*, Alloc>             map_allocator;      // 【用于分配中控器_map的空间】 

private:    // 【成员变量】
    iterator    _start;
    iterator    _finish;
    Type**      _map;
    size_t      _map_size;

private:    // 【重新调整/分配map】
    void _adjust_map(size_t add_buffers, bool add_at_front) {}

public:     // 【构造/析构函数】
    Deque(): 
        _start(), _finish(), _map(nullptr), _map_size(0) {}
    Deque(size_t n, const Type& value = Type()) {
        // 分配中控器_map空间
        size_t needed_buffers = n / buffer_size + 1;
        _map_size = mystl_max(size_t(init_map_size), needed_buffers+2); // ???
        _map = map_allocator::allocate(_map_size);
        // 分配各个所需缓冲区的空间
        Type** first = _map + (_map_size-needed_buffers)/2;
        Type** last = first + needed_buffers;
        for (Type** cur=first; cur<last; ++cur)
            *cur = buffer_allocator::allocate(buffer_size);
        // 填充各个缓冲区
        _start.cur=*first;  _start.buf=first;
        _finish.cur=*first; _finish.buf=first;
        for (size_t i=0; i<n; ++i) { 
            new (_finish.cur) Type(value); 
            ++_finish;
        }
    }
    Deque(initializer_list<Type> init_list) {}
    // Deque(Type* first, Type* last) {}
    // Deque(const Deque<Type, Alloc>& other) {}
    // Deque(Deque<Type, Alloc>&& other) {}
    ~Deque() {}

public:     // 【Basic Accessor】
    size_t size()   const { return _finish-_start; }
    bool empty()    const { return _start==_finish; }
    iterator begin()const { return _start; }
    iterator end()  const { return _finish; }

public:     // 【改、查】
    Type& front() { return *(_start.cur); }
    Type& back()  { iterator tmp=_finish; --tmp; return *(tmp.cur); }
    Type& operator[](size_t i) { return _start[i]; }
    const Type& front() const { return *_start; }
    const Type& back()  const { iterator tmp=_finish; --tmp; return *tmp; }
    const Type& operator[](size_t i) const { return _start[i]; }
    iterator find(const Type& value) {
        for (iterator tmp=_start; tmp!=_finish; ++tmp)
            if (*tmp == value) return tmp;
        return _finish;
    }

public:     // 【增】
    void push_back(const Type& item) {}
    void push_front(const Type& item) {}

public:     // 【删】
    Type pop_back() {}
    Type pop_front() {}
};

template <class Type>
ostream& operator<<(ostream& out, const Deque<Type>& deq) {
    out << "[ ";
    for (const Type& item : deq) out << item << " ";
    return out << "]";
}

#endif // __DEQUE