/* slist.hpp
 * 【单向链表(slist, single list)】
 * STL当中 <stl_slist.h>/<slist>/新版<forward_list> 部分内容的简化版
 * 
 * SList<> 与 STL slist<>/forward_list<> 的不同之处：
 * (1)除“头端插入/删除push_front()/pop_front()”外，也支持高效的尾端插入push_back()
 * (2)类设计上没有采用复杂继承关系，因为个人认为真没必要...
 * 当然了，假如是只要求push_front()/pop_front()的话，则有必要这样 —— 
 * 
 * struct __SListLink {__SListLink* link;};
 * template <class Type>
 * struct __SListNode: public __SListLink {Type data;};
 * ...
 * template <class Type>
 * class SList {
 *  typedef __SListLink         _Link;
 *  typedef __SListNode<Type>   _Node;
 *  ...
 *  _Link  _dummy_head;  // 虚拟头节点，只有link没有data
 *  size_t _count;       // 而指向_Link的指针，也可无条件指向_Node，因_Node继承自_Link
 * };
 * 
 * 这样可以相当优雅地将空/非空的增删操作都统一起来！
 * dummy_head技巧在刷leetcode时常会用到！！！
 */
#ifndef __SINGLE_LIST__
#define __SINGLE_LIST__
#include <iostream>         // cout, ostream
#include <initializer_list> // ...
#include "alloc.hpp"
using namespace std;


// SList节点
template <class Type> 
struct __SListNode { 
    Type data;                  // ...
    __SListNode<Type>* next;    // ...
};
// SList迭代器
template <class Type>
struct __SListIterator {
    // 内部类型定义
    typedef ForwardIteratorTag      iterator_category;
    typedef Type                    value_type;
    typedef Type*                   pointer;
    typedef Type&                   reference;
    typedef ptrdiff_t               difference_type;
    typedef __SListIterator<Type>   iterator;
    typedef __SListNode<Type>       _Node;
    // ！！！本体！！！
    _Node* node_p;
    // 构造函数
    __SListIterator(_Node* slist_node_p = nullptr): node_p(slist_node_p) {}
    __SListIterator(const iterator& other): node_p(other.node_p) {}
    // ++self, self++, self==other, self!=other, *self, self->
    iterator& operator++() 
        { node_p=node_p->next;  return *this; }
    iterator operator++(int) 
        { iterator tmp(*this);  node_p=node_p->next;  return tmp; }
    bool operator==(const iterator& other)  const { return node_p == other.node_p; }
    bool operator!=(const iterator& other)  const { return node_p != other.node_p; }
    Type& operator*()                       const { return node_p->data; }
    _Node& operator->()                     const { return *node_p; }   // 为什么STL的要返回Type*???
    operator _Node*()                       const { return node_p; }
};


// """单链表SList[STL forward_list<>]"""
template < class Type, class Alloc = SecAlloc<__SListNode<Type>> >
class SList {

public:     // 【内部类型定义】
    typedef Type        value_type;
    typedef Type*       pointer;
    typedef Type&       reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef __SListIterator<Type>   iterator;  // 【迭代器】
    typedef __SListNode<Type>       _Node;

private:    // 【成员变量】
    _Node* _head;       // 头节点指针
    _Node* _tail;       // 尾节点指针
    size_t _count;      // 节点数

public:     // 【构造/析构函数】
    SList(): _head(nullptr), _tail(nullptr), _count(0) {}
    SList(initializer_list<Type> init_list): 
        _head(nullptr), _tail(nullptr), _count(0) {
        for (const auto& item : init_list)
            push_back(item);
    }
    ~SList() { clear(); }
    static _Node* make_node(const Type& data, void* next) {
        _Node* new_node = Alloc::allocate();
        new_node->data = data;
        new_node->next = (_Node*)next;
        return new_node;
    }
    static void destroy_node(_Node* node_p) {
        (node_p->data).~Type();
        Alloc::deallocate(node_p);
    }

public:     // 【改、查】类定义中不超一行自动内联
    size_t size()       const { return _count; }
    bool empty()        const { return _count==0; }
    iterator begin()    const { return iterator(_head); }
    iterator end()      const { return iterator(); }
    Type& front()       const { return _head->data; }
    Type& back()        const { return _tail->data; }
    iterator find(const Type& value) {
        for (iterator cur=begin(); cur!=end(); ++cur)
            if (*cur == value) return cur;
        return iterator();
    }

public:     // 【增】
    // 头端加入一个值为value的节点
    void push_front(const Type& value) {
        if (empty())
            _head = _tail = make_node(value, nullptr);  // 右结合，即_tail=make_node(..); _head=_tail;
        else            
            _head = make_node(value, _head);
        ++_count;
    }
    // 尾端加入一个值为value的节点
    void push_back(const Type& value) {
        if (empty())
            _head = _tail = make_node(value, nullptr);
        else
            { _tail->next=make_node(value, nullptr);  _tail=_tail->next; }
        ++_count; 
    }
    // 在迭代器node_p后插入一个值为value的节点【注：迭代器可转换为_Node*，故可输入节点指针】
    void insert_after(iterator node_p, const Type& value) {
        node_p->next = make_node(value, (_Node*)node_p);
        ++_count;
    }

public:     // 【删】
    // 头端弹出一个节点，返回该节点的数据
    Type pop_front() {
        if (empty()) {  // 空，warning，返回Type()
            cerr << "warning: SList(at " << this << ") is empty!" << endl; 
            return Type(); 
        }
        _Node* tmp_node = _head;
        Type tmp_data = tmp_node->data;
        _head = _head->next;
        destroy_node(tmp_node);
        --_count;
        return tmp_data;
    }
    // 删除迭代器node_p后的一个节点【注：迭代器可转换为_Node*，故可输入节点指针】
    void erase_after(iterator node_p) {
        _Node* cur_node = (_Node*)node_p;
        cur_node->next = cur_node->next->next;
        destroy_node(cur_node->next);
        --_count;
    }
    // 清除所有节点
    void clear() {
        _Node *next_node, *cur_node=_head;
        for ( ; cur_node ; cur_node=next_node) {
            next_node = cur_node->next;
            destroy_node(cur_node);
        }
        _count = 0;
    }
};

template <class Type>
ostream& operator<<(ostream& out, const SList<Type>& slst) {
    for (const Type& item : slst) out << item << " -> ";
    return out << "null";
}


#endif // __SINGLE_LIST__





/* // 测试(OK)
int main(int argc, char const *argv[]) {

    // {
    //     // clock_t st = clock();
    //     // SList<Pair<int, double>> my_slist;
    //     // for (int i=0; i<int(1e6); ++i)
    //     //     my_slist.push_front({1, 2});
    //     // clock_t et = clock();
    //     // cout << my_slist.front() << ", " << my_slist.back() << ", " << my_slist.size() << endl;
    //     // cout << et - st << " ms" << endl;
    //     // cout << (__SListNode<Pair<int, double>>*)my_slist.begin() << endl;
    //     // STL的单链表有bug???
    //     clock_t st = clock();
    //     // list<Pair<int, double>> stl_list;
    //     forward_list<Pair<int, double>> stl_list;
    //     for (int i=0; i<int(1e6); ++i)
    //         stl_list.push_front({1, 2});
    //     clock_t et = clock();
    //     cout << stl_list.front() << ", " << endl;
    //     cout << et - st << " ms" << endl;
    // }

    // {
    //     clock_t st = clock();
    //     SList<int> my_slist;
    //     for (int i=0; i<int(1e6); ++i)
    //         my_slist.push_front(i);
    //     clock_t et = clock();
    //     cout << my_slist.front() << ", " << my_slist.back() << ", " << my_slist.size() << endl;
    //     cout << et - st << " ms" << endl;
    // }
    // {
    //     clock_t st = clock();
    //     Vector<int> my_vec;
    //     for (int i=0; i<int(1e6); ++i)
    //         my_vec.push_back(i);
    //     clock_t et = clock();
    //     cout << my_vec.front() << ", " << my_vec.back() << ", " << my_vec.size() << endl;
    //     cout << et - st << " ms" << endl;
    // }

    system("pause");
    return 0;
}
 */