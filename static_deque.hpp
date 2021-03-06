/* static_deque.hpp
 * 【静态双端队列】允许以“均摊O(1)”的时间复杂度在两端进行增/删，支持[]随机访问
 * 一般双端队列的结构复杂，而在某些应用场景下需要的空间可提前预知，这时使用静态双端队列将更加高效
 * 虽宣称“静态”，不过也会自动扩容/缩容，规则与动态数组Vector<>类似
 */
#ifndef __STATIC_DEQUE__
#define __STATIC_DEQUE__
#include <initializer_list>
#include <iostream>
#include "alloc.hpp"
#include "traits.hpp"
using namespace std;


// 静态双端队列迭代器
template <class Type>
struct __StaticDequeIterator {
    // 类型定义
    typedef RandomIteratorTag           iterator_category;  // 支持随机访问的迭代器
    typedef Type                        value_type;
    typedef Type*                       pointer;
    typedef Type&                       reference;
    typedef size_t                      size_type;
    typedef ptrdiff_t                   difference_type;
    typedef __StaticDequeIterator<Type> iterator;
    // 成员变量
    Type* left;
    Type* right;
    Type* cur;
    // 构造函数
    __StaticDequeIterator(): 
        left(nullptr), right(nullptr), cur(nullptr) {}
    __StaticDequeIterator(Type* left_bound, Type* right_bound, Type* cur_ptr):
        left(left_bound), right(right_bound), cur(cur_ptr) {}
    // *self, ->self
    Type& operator*()   const { return *cur; }
    Type* operator->()  const { return cur; }
    // self==other, self!=other
    bool operator==(const iterator& other) const { return cur==other.cur; }
    bool operator!=(const iterator& other) const { return cur!=other.cur; }
    // ++self, self++
    iterator& operator++() { 
        if (++cur==right) cur=left;
        return *this; 
    }
    iterator operator++(int) {
        iterator tmp = *this;
        if (++cur==right) cur=left;
        return tmp;
    }
    // --self, self--
    iterator& operator--() {
        if (cur--==left) cur=right-1;
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        if (cur--==left) cur=right-1;
        return tmp;
    }
    // self+=n, self-=n, self+n, self-n
    iterator& operator+=(difference_type n) {
        if (n >= 0) {
            if (cur+n < right) cur += n;
            else               cur = left + (n - (right-1 - cur));
        }
        else {  // n < 0
            n = -n;
            if (cur-n <= left) cur -= n;
            else               cur = (right-1) - (n - (cur-left+1));  // 前后的-1,+1其实可省略
        }
        return *this;
    }
    iterator& operator-=(difference_type n)     { return this->operator+=(n); }
    iterator operator+(difference_type n) const { return iterator(*this) += n; }
    iterator operator-(difference_type n) const { return iterator(*this) -= n; }
    // self[n]
    Type& operator[](size_type i) 
        { return cur+i>=right ? left[cur+i-right] : cur[i]; }
    const Type& operator[](size_type i) const 
        { return cur+i>=right ? left[cur+i-right] : cur[i]; }
    // self<other, self>other, self-other无法实现
};


// """双端队列"""
template < class Type, class Alloc = FirstAlloc >
class StaticDeque {

public:     // 【类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef __StaticDequeIterator<Type> iterator;
    typedef Allocator<Type, Alloc>      data_allocator;
    static const size_type default_capacity = 31;

private:    // 【成员变量】
    // [ ][ ][@][@][@][@][@][@][@][@][ ][ ][ ][ ][ ][ ]
    // ↑     ↑                       ↑                 ↑
    // _left _start                  _finish           _right   (_size=8)
    // ---进行5次push_front(...)---
    // [@][@][@][@][@][@][@][@][@][@][ ][ ][ ][@][@][@]
    // ↑                             ↑        ↑        ↑
    // _left                         _finish  _start   _right   (_size=13)
    Type*       _left;
    Type*       _right;
    Type*       _start;
    Type*       _finish;
    size_type   _size;

private:    // 【扩/缩容】
    void _resize(size_type n) {
        Type* new_left = data_allocator::clallocate(n);             // 全0再分配
        if (_finish<_start) {               // 表明_start或_finish翻过页，分两部分拷贝
            Type* new_cur = new_left;
            memcpy(new_cur, _start, (_right-_start)*sizeof(Type));  // 拷贝[_start, _right)
            new_cur += (_right-_start);
            memcpy(new_cur, _left, (_finish-_left)*sizeof(Type));   // 拷贝[_left, _finish)
        }
        else {                              // _finish>=_start，即空/未满且无翻页，直接拷贝[_start, _finish)即可
            memcpy(new_left, _start, _size*sizeof(Type));
        }
        data_allocator::deallocate(_left);
        _left   = new_left;
        _right  = new_left + n;
        _start  = new_left;
        _finish = new_left + _size;
    }

public:     // 【构造函数】
    StaticDeque(size_type init_size = default_capacity) {
        _left   = data_allocator::clallocate(init_size);    // 全0初始化
        _right  = _left + init_size;
        _start  = _left + init_size/2;                      // 居中
        _finish = _start;
        _size   = 0;
    }
    StaticDeque(initializer_list<Type> init_list, 
                size_type init_size = default_capacity) {
        while (init_size < init_list.size())                // 注意自定义的init_size可能不足
            init_size = init_size * 2 + 1;
        _left   = data_allocator::clallocate(init_size);
        _right  = _left + init_size;
        _start  = _left;                                    // 不必居中了...
        _finish = _left;
        for (const auto& item : init_list)
            new (_finish++) Type(item);
        _size   = init_list.size();
    }
    StaticDeque(const StaticDeque<Type, Alloc>& other) {
        _left   = data_allocator::clallocate(other.capacity());
        _right  = _left + other.capacity();
        _start  = _left;
        _finish = _start;
        for (const auto& item : other)
            new (_finish++) Type(item);
        _size   = other._size;
    }
    // StaticDeque(StaticDeque<Type, Alloc>&& other) {}        // 同上...
    // operator=(const StaticDeque<Type, Alloc>& other) {}
    ~StaticDeque() { clear(); data_allocator::deallocate(_left); }

public:     // 【Basic Accessor】
    size_type size()     const { return _size; }
    size_type capacity() const { return _right - _left; }
    bool empty()         const { return _size == 0; }
    iterator begin()  { return iterator(_left, _right, _start); }
    iterator end()    { return iterator(_left, _right, _finish); }
    iterator rbegin() { return iterator(_left, _right, _finish).operator--(); }
    iterator rend()   { return iterator(_left, _right, _start).operator--(); }
    const iterator begin()  const { return iterator(_left, _right, _start); }
    const iterator end()    const { return iterator(_left, _right, _finish); }
    const iterator rbegin() const { return iterator(_left, _right, _finish).operator--(); }
    const iterator rend()   const { return iterator(_left, _right, _start).operator--(); }

public:     // 【改、查】
    Type& front() { return *_start; }
    Type& back()  { return _finish==_left ? *(_right-1) : *(_finish-1); }
    Type& operator[](size_type i)       // 可以if(i >= _size)检查越界
        { return _start+i>=_right ? _left[_start+i-_right] : _start[i]; }
    const Type& front() const { return *_start; }
    const Type& back()  const { return _finish==_left ? *(_right-1) : *(_finish-1); }
    const Type& operator[](size_type i) const 
        { return _start+i>=_right ? _left[_start+i-_right] : _start[i]; }

public:     // 【增】
    void push_back(const Type& item) {
        if (_size+1 == capacity())      // 此时|_finish - _start| = 1，不能完全满，否则end()==begin()
            _resize(capacity()*2+1);    // 扩容为2倍+1
        new (_finish) Type(item);
        if (++_finish==_right) _finish=_left;
        ++_size;
    }
    void push_front(const Type& item) {
        if (_size+1 == capacity())      // 此时|_finish - _start| = 1，不能完全满，否则end()==begin()
            _resize(capacity()*2+1);    // 扩容为2倍+1
        if (_start--==_left) _start=_right-1;
        new (_start) Type(item);
        ++_size;
    }
    // insert()和erase()在这里太难搞了，效率不高(O(n)时间)且极少用到，
    // 也可以通过外部进行若干次pop_front()/pop_back()然后push_front()/push_back()实现，这里就不实现了

public:     // 【删】
    Type pop_back() {
        if (_size < capacity()/4  &&  capacity()/2 > default_capacity)
            _resize(capacity()/2);
        if (--_finish==_left-1) _finish=_right-1;   // _finish先往前走一格，
        Type tmp = *_finish;                        // 即_finish暂时充当“即将被弹出元素的指针”
        _finish->~Type();
        --_size;
        return tmp;
    }
    Type pop_front() {
        if (_size < capacity()/4  &&  capacity()/2 > default_capacity)
            _resize(capacity()/2);
        Type tmp = *_start;
        _start->~Type();
        --_size;
        if (++_start==_right-1) _start=_left;
        return tmp;
    }

    void clear() {
        if (_finish > _start) 
            mystl::destroy(_start, _finish);
        else 
            { mystl::destroy(_start, _right); mystl::destroy(_left, _finish); }
        _finish = _start;   // _start不必居中，反正是循环的
        _size   = 0;
    }
};


// cout << sdeq;
template <class Type>
ostream& operator<<(ostream& out, const StaticDeque<Type>& sdeq) {
    out << "[ ";
    for (const Type& item : sdeq) out << item << " ";
    return out << "]";
}


#endif // __STATIC_DEQUE__