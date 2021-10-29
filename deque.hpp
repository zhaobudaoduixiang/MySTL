/* deque.hpp
 * 【双端队列】允许以O(1)时间在两端增/删，且支持随机访问[]
 * STL当中 <stl_deque.h>/<deque> 部分内容的简化版
 */
#ifndef __DEQUE__
#define __DEQUE__
#include <initializer_list>
#include "alloc.hpp"    // ...
#include "traits.hpp"   // ...
#include "utils.hpp"    // mystl::max()
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
    Type* buf_start()  const { return *buf; }
    Type* buf_finish() const { return *buf+buf_size; }
    // 构造函数
    __DequeIterator(): cur(nullptr), buf(nullptr) {}
    // *self, ->self
    Type& operator*()   const { return *cur; }
    Type* operator->()  const { return cur; }
    // self==other, self!=other
    bool operator==(const iterator& other) const { return cur==other.cur; }
    bool operator!=(const iterator& other) const { return cur!=other.cur; }
    // self<other, self>other
    bool operator<(const iterator& other) const 
        { return (buf==other.buf) ? (cur<other.cur) : (buf<other.buf); }
    bool operator>(const iterator& other) const 
        { return (buf==other.buf) ? (cur>other.cur) : (buf>other.buf); }
    // ++self, self++
    iterator& operator++() {
        if (++cur == buf_finish())          // ++cur后到达当前缓冲区buf的finish 【finish即最后一个元素的后一个的指针】
            { ++buf; cur=buf_start(); }     // 跳到下一个缓冲区
        return *this;
    }
    iterator operator++(int) {
        iterator tmp = *this;
        this->operator++();
        return tmp;
    }
    // --self, self--
    iterator& operator--() {
        if (cur-- == buf_start())           // cur到达当前缓冲区buf的start 【start即第一个元素的指针，*buf】
            { --buf; cur=buf_finish()-1; }  // 跳到上一个缓冲区
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        this->operator--();
        return tmp;
    }
    // 【self - other】
    difference_type operator-(const iterator& other) const { 
        return difference_type(buf_size) * (buf-other.buf-1) 
            + (cur - buf_start()) + (other.buf_finish() - other.cur); 
    }
    // self+=n, self-=n, self+n, self-n
    iterator& operator+=(difference_type n) {
        difference_type offset = (cur-buf_start()) + n;   // 从当前缓冲区buf的begin开始算起的偏移量
        if (0 <= offset && offset < buf_size)   // 在不同缓冲区
            cur += n;
        else {                                  // 在不同缓冲区
            difference_type buf_offset = offset>0 ? offset/(difference_type)buf_size : 
                ((offset+1) / (difference_type)buf_size) - 1 ;  // 缓冲区的偏移量
            buf += buf_offset;
            cur = buf_start() + (offset - buf_offset*(difference_type)buf_size);
        }
        return *this;
    }
    iterator& operator-=(difference_type n)     { return this->operator+=(-n); }
    iterator operator+(difference_type n) const { return iterator(*this) += n; }
    iterator operator-(difference_type n) const { return iterator(*this) -= n; }
    // self[n]
    Type& operator[](size_t n) 
        { return *((this->operator+(n)).cur); }     // 可修改
    const Type& operator[](size_t n) const  
        { return *(this->operator+(n)); }           // 不可修改
};


// """双端队列Deque"""
template < class Type, class Alloc = FirstAlloc >
class Deque {
public:
    // 每个缓冲区buffer的元素个数【每个缓冲区默认512字节】【跟Type挂钩，以static const声明】
    static const size_t buffer_size = sizeof(Type)<512ULL ? 512ULL/sizeof(Type) : 1ULL;
    // 中控器的默认长度
    static const size_t default_map_size = 8ULL;   

public:     // 【类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef __DequeIterator<Type, buffer_size>  iterator;           // 【迭代器】
    typedef Allocator<Type, Alloc>              buffer_allocator;   // 【用于分配缓冲区的空间】
    typedef Allocator<Type*, Alloc>             map_allocator;      // 【用于分配中控器的空间】 

private:    // 【成员变量】
    iterator    _start;         // ...
    iterator    _finish;        // ...
    Type**      _map_start;     // 中控器起始
    Type**      _map_finish;    // ...

private:    // 【...】
    // 分配中控器的空间【全0初始化】
    Type** _allocate_map(size_t mapsz)   { return map_allocator::clallocate(mapsz); }
    // 释放中控器的空间
    void _deallocate_map(Type** mapp)    { map_allocator::deallocate(mapp); }
    // 分配一个缓冲区的空间
    Type* _allocate_buffer(size_t bufsz) { return buffer_allocator::allocate(bufsz); }
    // 释放bufp所指缓冲区的空间，并将bufp指向0
    void _deallocate_buffer(Type** bufp) { buffer_allocator::deallocate(bufp); *bufp = nullptr; }
    // 重新分配中控器的空间【全0】，并调整居中
    void _readjust_map(size_t add_buffers, bool add_at_front) {}

public:     // 【构造/析构函数】
    Deque() {
        _map_start = _allocate_map(default_map_size);
        _map_finish = _map_start + default_map_size;
        _start.buf = _map_start + default_map_size/2;
        _start.cur = *(_start.buf);
        _finish = _start;
    }
    Deque(initializer_list<Type> init_list) {
        // 分配中控器空间
        size_t nbufs = init_list.size()/buffer_size+1;
        _map_start = _allocate_map(nbufs + 2);  // 前后各留一个缓冲区
        _map_finish = _map_start + nbufs + 2;
        // 分配所需缓冲区的空间
        for (Type** bufp=_map_start+1; bufp<_map_start+1+nbufs; ++bufp)
            *bufp = _allocate_buffer(buffer_size);
        // 填充
        _start.buf=_map_start+1; _start.cur=*(_start.buf);
        _finish = _start;
        for (const auto& item : init_list) 
            { new (_finish.cur) Type(item);  ++_finish; }
    }
    // Deque(const Deque<Type, Alloc>& other) {}
    // Deque(Deque<Type, Alloc>&& other) {}
    // Deque(size_t n, const Type& value) {}
    // Deque(Type* first, Type* last) {}
    ~Deque() { clear(); _deallocate_map(_map_start); }

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
    void push_back(const Type& item) {
        if (_finish.cur == _finish.buf_finish()) {  // 到达缓冲区尾，翻页
            if (_finish.buf+1 == _map_finish) {
                ;
            }
            else if (++_finish.buf) 
                *(_finish.buf) = _allocate_buffer(buffer_size);
            _finish.cur = *(_finish.buf);
        }
        new (_finish.cur++) Type(item);
    }
    void push_front(const Type& item) {}

public:     // 【删】
    Type pop_back() {}
    Type pop_front() {}
    void clear() {
        mystl::destroy(_start, _finish);                        // 依次析构
        for (Type** bufp=_start.buf; *bufp; ++bufp)             // 释放各缓冲区空间
            _deallocate_buffer(*bufp);
        _start.buf = _map_start + (_map_finish-_map_start)/2;   // _start和_finish居中
        _start.cur = *(_start.buf);
        _finish = _start;
    }
};

template <class Type>
ostream& operator<<(ostream& out, const Deque<Type>& deq) {
    out << "[ ";
    for (const Type& item : deq) out << item << " ";
    return out << "]";
}


#endif // __DEQUE