/* deque.hpp
 * 【双端队列】允许以O(1)时间在两端增/删且支持随机访问[]的队列
 * STL当中 <stl_deque.h>/<deque> 部分内容的简化版
 */
#ifndef __DEQUE__
#define __DEQUE__
#include "alloc.hpp"
#include "traits.hpp"
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
    // __DequeIterator(): 
    //     cur(nullptr), buf(nullptr) {}
    // __DequeIterator(const iterator& other):
    //     cur(other.cur), buf(other.buf) {}
    // self-other, ++/--self, self++/--, self==other, self!=other, *self, ->self ...
    Type& operator*() const { return *cur; }
    Type* operator->()  const { return cur; }
    bool operator==(const iterator& other) const { return cur==other.cur; }
    bool operator!=(const iterator& other) const { return cur!=other.cur; }
    bool operator<(const iterator& other) const 
        { return (buf==other.buf) ? (cur<other.cur) : (buf<other.buf); }
    bool operator>(const iterator& other) const 
        { return (buf==other.buf) ? (cur>other.cur) : (buf>other.buf); }
    iterator& operator++() {
        if (++cur == *buf+buf_size) 
            { ++buf; cur=*buf; }
        return *this;
    }
    iterator operator++(int) {
        iterator tmp = *this;
        this->operator++();
        return tmp;
    }
    iterator& operator--() {
        if (cur-- == *buf)
            { --buf; cur=(*buf)+buf_size-1; }
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        this->operator--();
        return tmp;
    }
    difference_type operator-(const iterator& other) const { 
        return difference_type(buf_size) * (buf-other.buf-1) 
               + (cur - *buf) + (other.*buf+buf_size - other.cur); }
    iterator& operator+=(difference_type n) {}
    iterator operator+(difference_type n) {}
    iterator& operator-=(difference_type n) {}
    iterator operator-(difference_type n) {}
    Type& operator[](size_t n) {}
};


// """双端队列Deque"""
template < class Type, class Alloc = FirstAlloc<Type> >
class Deque {
public:     // ...
    static size_t buffer_size() const 
        { return sizeof(Type)<512 ? 512/sizeof(Type) : 1; }

public:     // 【内部类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef __DequeIterator<Type, buffer_size()> iterator;   // 【迭代器】

private:    // 【成员变量】
    iterator    _start;
    iterator    _finish;
    Type**      _map;
    size_t      _map_size;
};


#endif // __DEQUE