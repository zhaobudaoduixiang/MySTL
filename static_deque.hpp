/* static_deque.hpp
 * 【静态双端队列】允许以“均摊O(1)”的时间复杂度在两端进行增/删，支持[]随机访问
 * 一般双端队列的结构复杂，而在某些应用场景下需要的空间可提前预知，这时使用静态双端队列将更加高效
 * 虽宣称“静态”，不过也会自动扩容/缩容，规则与动态数组Vector<>一致
 */
#ifndef __STATIC_DEQUE__
#define __STATIC_DEQUE__
#include <initializer_list>
#include "alloc.hpp"    // ...
#include "traits.hpp"   // ...
using namespace std;


// 静态双端队列迭代器
template <class Type>
struct __StaticDequeIterator {};

// 静态双端队列
template < class Type, class Alloc = FirstAlloc>
class StaticDeque {

public:     // 【类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef __StaticDequeIterator<Type> iterator;
    typedef Allocator<Type, Alloc>      data_allocator; // 【内存分配器】
    static const size_type default_capacity = 31;

private:    // 【成员变量】
    Type*       _data;
    size_type   _capacity;
    size_type   _starti;
    size_type   _finishi;

private:    // 【扩/缩容】
    void _resize(size_type n) {}

public:     // 【构造函数】
    StaticDeque(size_type init_capa = default_capacity): 
        _data(data_allocator::allocate(init_capa)), _capacity(init_capa), _starti(0), _finishi(0) {}
    // StaticDeque(initializer_list<Type> init_list) {}
    // StaticDeque(const StaticDeque<Type, Alloc>& other) {}
    // StaticDeque(StaticDeque<Type, Alloc>&& other) {}

public:     // 【Basic Accessor】
    size_type size()     const { return _finishi>_starti ? (_finishi-_starti) : (_finishi+_capacity-_starti); }
    size_type capacity() const { return _capacity; }
    bool empty()         const { return _starti == _finishi; }

public:     // 【增】
    void push_back(const Type& item) {}
    void push_front(const Type& item) {}
    iterator insert(iterator pos, const Type& item);

public:     // 【删】
    Type pop_back();
    Type pop_front();
    iterator erase(iterator first, iterator last);
    iterator erase(iterator pos);
    void clear() {}
};

#endif // __STATIC_DEQUE__