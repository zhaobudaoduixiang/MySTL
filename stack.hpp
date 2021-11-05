/* stack.hpp
 * 【栈】后进先出
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


template < class Type, class Seq = SList<Type> >
class Stack {
public:
    typedef Type    value_type;
    typedef Type*   pointer;
    typedef Type&   reference;
    typedef size_t  size_type;
private:
    Seq _self;
public:
    Stack(): _self(Seq()) {}
    // ~Stack() { /* 不需要析构函数，_self生存时间到了会自动析构 */ }
public:
    size_type size() const { return _self.size(); }
    bool empty()     const { return _self.empty(); }
    Type& top()             { return _self.front(); }
    const Type& top() const { return _self.front(); }
    void push(const Type& item) { _self.push_front(item); }
    Type pop()                  {return _self.pop_front();}
};


template <class Type>
class Stack< Type, Vector<Type> > {
private:
    Vector<Type> _self;
public:
    Stack(): 
        _self(Vector<Type>()) {}
    Stack(size_t init_capa):
        _self(Vector<Type>::static_construct(init_capa)) {}
    // ~Stack() { /* 不需要析构函数，_self生存时间到了会自动析构 */ }
public:
    size_t size() const { return _self.size(); }
    bool empty()  const { return _self.empty(); }
    Type& top()             { return _self.back(); }
    const Type& top() const { return _self.back(); }
    void push(const Type& item) { _self.push_back(item); }
    Type pop()                  {return _self.pop_back();}
    // void swap(Stack<Type, Vector<Type>>& other);
};


#endif // __STACK__