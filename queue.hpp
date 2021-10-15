/* queue.hpp
 * 【队列】先进先出
 * STL当中 <stl_queue.h>/<queue> 的 queue<> 部分内容的简化/优化版
 * 只是一个 “先进先出(FIFO)” 的 “接口” 
 * 任意能够一端进/另一端出的数据结构都可作为其下层
 * 比如：Deque<> SList<> 等
 */
#ifndef __QUEUE__
#define __QUEUE__
#include "deque.hpp"
#include "slist.hpp"
using namespace std;


template < class Type, class Seq = Deque<Type> >
struct Queue {};


template <class Type>
struct Queue <Type, SList<Type>> {
private:
    SList<Type>* _self;
public:
    Queue(): _self(new SList<Type>()) {}
    ~Queue() { delete _self; }
    bool size()     const {return _self->size();}
    bool empty()    const {return _self->empty();}
    Type& front()   const {return _self->front();}
    Type& back()    const {return _self->back();}
    void push(const Type& item) {_self->push_back(item);}
    Type pop()                  {return _self->pop_front();}
    // void swap(const Queue<Type, SList<Type>>& other);
};


#endif // __QUEUE__