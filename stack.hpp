/* stack.hpp
 * 栈
 * STL当中 <stl_stack.h>/<stack> 部分内容的简化/优化版
 * 只是一个 “后进先出(LIFO)” 的 “接口” 
 * 任意能够一端进/出的数据结构都可作为其下层
 * 比如：Deque<> SList<> Vector<>等
 */
#ifndef __STACK__
#define __STACK__
#include "deque.hpp"
#include "slist.hpp"
#include "vector.hpp"
using namespace std;


template < class Type, class Seq = Deque<Type> >
class Stack {};


template <class Type>
class Stack< Type, SList<Type> > {
private:
    SList<Type>* _self;
public:
    Stack(): _self(new SList<Type>()) {}
    ~Stack() { delete _self; }
    bool size()     const {return _self->size();}
    bool empty()    const {return _self->empty();}
    Type& front()   const {return _self->front();}
    Type& back()    const {return _self->back();}
    void push(const Type& item) {_self->push_front(item);}
    Type pop()                  {return _self->pop_front();}
    // void swap(const& other);
};
template <class Type>
class Stack< Type, Vector<Type> > {
private:
    Vector<Type>* _self;
public:
    Stack(): _self(new Vector<Type>()) {}
    ~Stack() { delete _self; }
    bool size()     const {return _self->size();}
    bool empty()    const {return _self->empty();}
    Type& front()   const {return _self->front();}
    Type& back()    const {return _self->back();}
    void push(const Type& item) {_self->push_back();}
    Type pop()                  {return _self->pop_back();}
    // void swap(const& other);
};


#endif // __STACK__