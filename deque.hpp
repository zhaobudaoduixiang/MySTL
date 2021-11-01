/* deque.hpp
 * 【双端队列】允许以O(1)时间在两端增/删，支持[]随机访问
 * STL当中 <stl_deque.h>/<deque> 部分内容的简化版
 */
#ifndef __DEQUE__
#define __DEQUE__
#include <initializer_list>
#include "alloc.hpp"    // ...
#include "traits.hpp"   // ...
using namespace std;


// 双端队列迭代器【立大功】
template < class Type, size_t buf_size >
struct __DequeIterator {
    // 内部类型定义
    typedef RandomIteratorTag       iterator_category;  // 支持随机访问的迭代器
    typedef Type                    value_type;
    typedef Type*                   pointer;
    typedef Type&                   reference;
    typedef size_t                  size_type;
    typedef ptrdiff_t               difference_type;
    typedef __DequeIterator<Type, buf_size> iterator;
    // 成员变量
    Type*   cur;    // 当前数据的指针
    Type**  buf;    // 当前缓冲区(Type*)的指针
    Type* buf_start()  const { return *buf; }           // 缓冲区的第一个位置
    Type* buf_finish() const { return *buf+buf_size; }  // 缓冲区最后一个位置+1
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
    Type& operator[](size_type n) 
        { return *((this->operator+(n)).cur); } // 可修改
    const Type& operator[](size_type n) const  
        { return *(this->operator+(n)); }       // 不可修改
};


// """双端队列Deque"""
template < class Type, class Alloc = FirstAlloc >
class Deque {

public:     // 【类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    // 每个缓冲区buffer的元素个数【每个缓冲区默认512字节】【跟Type挂钩，以static const声明】
    static const size_type buffer_size = sizeof(Type)<512 ? 512/sizeof(Type) : 1;
    // 中控器的默认长度
    static const size_type default_map_size = 8;
    typedef __DequeIterator<Type, buffer_size>  iterator;           // 【迭代器】
    typedef Allocator<Type, Alloc>              buffer_allocator;   // 【用于分配缓冲区的空间】
    typedef Allocator<Type*, Alloc>             map_allocator;      // 【用于分配中控器的空间】

private:    // 【成员变量】
    iterator    _start;         // 第一个位置
    iterator    _finish;        // 最后一个+1位置
    Type**      _map_start;     // 中控器起始
    size_type   _map_size;      // 中控器长度

private:    // 【...】
    // 分配中控器的空间【全0初始化】
    Type** _allocate_map(size_type mapsz)   { return map_allocator::clallocate(mapsz); }
    // 释放中控器的空间
    void _deallocate_map(Type** mapp)    { map_allocator::deallocate(mapp); }
    // 分配一个缓冲区的空间
    Type* _allocate_buffer(size_type bufsz) { return buffer_allocator::allocate(bufsz); }
    // 释放bufp所指缓冲区的空间，并将bufp指向0
    void _deallocate_buffer(Type** bufp) { buffer_allocator::deallocate(*bufp); *bufp = nullptr; }
    // 重新分配中控器的空间【全0】，并调整居中
    void _readjust_map(size_type add_buffers, bool add_front) {
        size_type old_buffers = _finish.buf-_start.buf+1;
        size_type need_buffers = old_buffers + add_buffers;
        Type** new_start_buf;
        // 中控器空间充足，直接居中即可
        if (_map_size >= 2*need_buffers) {
            new_start_buf = _map_start + (_map_size-need_buffers)/2;    // 居中时预留了add_buffers，而并未真正添加
            if (add_front) new_start_buf += add_buffers;                // 因此前端添加时要注意“恢复”start到未预留状态
            size_type buf_offset = new_start_buf - _start.buf;                      // 缓冲区偏移量
            memmove(new_start_buf, _start.buf, old_buffers*sizeof(Type*));
            if (buf_offset < 0)  // 【置0！保持中控器上未分配的缓冲区指向nullptr！】
                memset(new_start_buf+old_buffers, 0, (-buf_offset)*sizeof(Type*));  // 后端添加，前移
            else
                memset(_start.buf, 0, buf_offset*sizeof(Type*));                    // 前端添加，后移
            _start.buf = new_start_buf;
            _finish.buf = new_start_buf + old_buffers - 1;
        }
        // 需要分配更大的中控器【2倍大】
        else {
            _map_size += (add_buffers>_map_size ? add_buffers : _map_size);
            Type** new_map_start = _allocate_map(_map_size);                // 分配新中控器
            new_start_buf = new_map_start + (_map_size-need_buffers)/2;
            if (add_front) new_start_buf += add_buffers;
            memcpy(new_start_buf, _start.buf, old_buffers*sizeof(Type*));   // 拷贝
            _deallocate_map(_map_start);                                    // 释放原中控器
            _map_start = new_map_start;
            _start.buf = new_start_buf;
            _finish.buf = new_start_buf + old_buffers - 1;
        }
    }

public:     // 【构造/析构函数】
    Deque() {
        _map_size = default_map_size;
        _map_start = _allocate_map(_map_size);
        _start.buf = _map_start + _map_size/2;
        *(_start.buf) = _allocate_buffer(buffer_size);      // 至少留一个缓冲区
        _start.cur = _start.buf_start();
        _finish = _start;
    }
    Deque(initializer_list<Type> init_list) {
        // 分配中控器空间
        size_type nbufs = init_list.size()/buffer_size+1;   // 所需缓冲区数，恰好整除时会+1
        _map_size = nbufs + default_map_size;
        _map_start = _allocate_map(_map_size);              // 前后各留“默认缓冲区数的一半”
        // 分配所需缓冲区的空间
        Type** first_buffer = _map_start + default_map_size/2;
        for (Type** bufp=first_buffer; bufp<first_buffer+nbufs; ++bufp)
            *bufp = _allocate_buffer(buffer_size);
        // 填充
        _start.buf = first_buffer;
        _start.cur = _start.buf_start();
        _finish = _start;
        for (const auto& item : init_list) 
            { new (_finish.cur) Type(item);  ++_finish; }
    }
    // Deque(const Deque<Type, Alloc>& other) {}
    // Deque(Deque<Type, Alloc>&& other) {}
    // Deque(size_type n, const Type& value) {}
    ~Deque() { clear(); _deallocate_map(_map_start); }

public:     // 【Basic Accessor】
    size_type size()const { return _finish-_start; }
    bool empty()    const { return _start==_finish; }
    iterator begin()const { return _start; }
    iterator end()  const { return _finish; }

public:     // 【改、查】
    Type& front() { return *(_start.cur); }
    Type& back()  { iterator tmp=_finish; --tmp; return *(tmp.cur); }
    const Type& front() const { return *_start; }
    const Type& back()  const { iterator tmp=_finish; --tmp; return *tmp; }
    Type& operator[](size_type i) { return _start[i]; }
    const Type& operator[](size_type i) const { return _start[i]; }
    iterator find(const Type& value) {
        for (iterator tmp=_start; tmp!=_finish; ++tmp)
            if (*tmp == value) return tmp;
        return _finish;
    }

public:     // 【增】
    // 后端添加
    void push_back(const Type& item) {
        if (_finish.cur != _finish.buf_finish()-1) {    // _finish的缓冲区即使添加完后还有一个或以上空间
            new (_finish.cur++) Type(item);
        }
        else {                                          // _finish已在缓冲区最后一个，添加完后需要转到下一个缓冲区
            new (_finish.cur) Type(item);
            if (_finish.buf+1 == _map_start+_map_size)              // 中控器尾部空间不足，重整
                _readjust_map(1ULL, false);
            if (*(_finish.buf+1) == nullptr)                        // 下一个缓冲区空间未分配
                *(_finish.buf+1) = _allocate_buffer(buffer_size);
            ++_finish.buf;                                          // 转到下一个缓冲区（头）
            _finish.cur = _finish.buf_start();
        }
    }
    // 前端添加
    void push_front(const Type& item) {
        if (_start.cur != _start.buf_start()) {         // _start的缓冲区未到头
            new (--_start.cur) Type(item);
        }
        else {                                          // _start的缓冲区到头了，要翻页再添加
            if (_start.buf == _map_start)                           // 中控器前端空间不足，重整
                _readjust_map(1ULL, true);
            if (*(_start.buf-1) == nullptr)                         // 前一个缓冲区未分配
                *(_start.buf-1) = _allocate_buffer(buffer_size);
            --_start.buf;                                           // 转到前一个缓冲区
            _start.cur = _start.buf_finish()-1;
            new (_start.cur) Type(item);                            // 别忘记添加
        }
    }
    // 在pos处插入item
    // iterator insert(iterator pos, const Type& item);

public:     // 【删】
    // 后端弹出
    Type pop_back() {
        if (empty()) { // 空情况
            cout << "warning: Deque(at " << this << ") is empty!" << endl; 
            return Type();
        }
        if (_finish.cur != _finish.buf_start()) {   // _finish缓冲区还有一个或以上空间
            --_finish.cur;
            Type tmp = *_finish;
            (_finish.cur)->~Type();
            return tmp;
        }
        else {                                      // _finish已在缓冲区头，即缓冲区为空，需要转到前一个缓冲区
            if (*(_finish.buf+1) && _finish.buf+1<_map_start+_map_size)
                _deallocate_buffer(_finish.buf+1);  // 【延迟释放缓冲区：释放跳转前的下一个缓冲区】
            --_finish.buf;
            _finish.cur = _finish.buf_finish()-1;   // 这两行即无判断的--_finish
            Type tmp = *_finish;
            (_finish.cur)->~Type();
            return tmp;
        }
    }
    // 前端弹出
    Type pop_front() {
        if (empty()) { // 空情况
            cout << "warning: Deque(at " << this << ") is empty!" << endl; 
            return Type();
        }
        if (_start.cur != _start.buf_finish()-1) {
            Type tmp = *_start;
            (_start.cur++)->~Type();
            return tmp;
        }
        else {
            if (*(_start.buf-1) && _start.buf>=_map_start)
                _deallocate_buffer(_start.buf-1);
            Type tmp = *_start;
            (_start.cur)->~Type();
            ++_start.buf;
            _start.cur = _start.buf_start();
            return tmp;
        }
    }
    // 删除pos处元素
    // iterator erase(iterator pos);
    // iterator erase(iterator first, iterator last);
    // 清空整个双端队列
    void clear() {
        mystl::destroy(_start, _finish);                // 析构（这个不是制约速度的关键）
        for (Type** bufp=_start.buf+1; *bufp; ++bufp) 
            _deallocate_buffer(bufp);                   // 除_start所在缓冲区，取余缓冲区全部释放
        _start.cur = _start.buf_start();
        _finish = _start;                               // ...
    }
    // 没必要搞得这么复杂，其实速度差不多...
    // void clear() {
    //     if (_start.buf == _finish.buf) 
    //         mystl::destroy(_start.cur, _finish.cur);
    //     else {
    //         mystl::destroy(_start.cur, _start.buf_finish());
    //         for (Type** bufp=_start.buf+1; bufp<_finish.buf; ++bufp) {
    //             mystl::destroy(*bufp, *bufp+buffer_size);
    //             _deallocate_buffer(bufp);
    //         }
    //         mystl::destroy(_finish.buf_start(), _finish.cur);
    //         _deallocate_buffer(_finish.buf);
    //         if (*(_finish.buf+1))           // _finish.buf的下一个缓冲区因为延迟释放机制，还未释放
    //             _deallocate_buffer(_finish.buf+1);
    //     }
    //     _start.cur = _start.buf_start();    // 只留下_start所在缓冲区
    //     _finish = _start;
    // }
};
// cout << deq;
template <class Type>
ostream& operator<<(ostream& out, const Deque<Type>& deq) {
    out << "[ ";
    for (const Type& item : deq) out << item << " ";
    return out << "]";
}


#endif // __DEQUE