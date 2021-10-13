/* deque.hpp
 * 双端队列
 * STL当中 <stl_deque.h>/<deque> 部分内容的简化版
 */
#ifndef __DEQUE__
#define __DEQUE__
#include "alloc.hpp"
#include "traits.hpp"
using namespace std;


template <class Type>
struct __DequeIterator {
    typedef Type value_type;
    // ...
    Type*   first;
    Type*   last;
    Type*   cur;
    Type**  node;
};


template < class Type, class Alloc = FirstAlloc<Type> >
class Deque {};


#endif // __DEQUE