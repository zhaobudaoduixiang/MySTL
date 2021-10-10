#ifndef __QUEUE__
#define __QUEUE__
#include "deque.hpp"
#include "slist.hpp"
using namespace std;


template < class Type, class Seq = Deque<Type> >
struct Queue {};


template <class Type, class Seq>
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
    void push() {_self->push_back();}
    Type pop()  {return _self->pop_front();}
    // void swap(const Queue<Type, Seq>& other);
};


#endif // __QUEUE__