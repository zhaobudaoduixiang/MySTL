#ifndef __RB_TREE__
#define __RB_TREE__
#include <initializer_list>
#include "alloc.hpp"
#include "traits.hpp"
#include "utils.hpp"


// """红黑树节点"""
// 【红黑树节点基类】
struct __TreeNodeBase {
    __TreeNodeBase *left, *right;
    bool color;
};
// 【只包含键，用于TreeSet】
template < class Key, class Alloc = SecondAlloc >
struct __TreeSetNode: __TreeNodeBase { 
    Key key;
    void* operator new(size_t nbytes) { return Alloc::allocate(nbytes); }
    void operator delete(void* ptr)   { Alloc::deallocate(ptr); }
};
// 【包含键-值，用于TreeMap】
template < class Key, class Value, class Alloc = SecondAlloc >
struct __TreeMapNode: __TreeNodeBase { 
    Key key; 
    Value value; 
    void* operator new(size_t nbytes) { return Alloc::allocate(nbytes); }
    void operator delete(void* ptr)   { Alloc::deallocate(ptr); }
};


// """红黑树"""
template < class NodeType, class Key, class KeyCompare >
struct __RBTree {
    // 【类型定义】
    static const bool red   = true;
    static const bool black = false;
    
    // 【成员变量】
    KeyCompare  _compare;
    NodeType*   _root;
    size_t      _count;

    // 【构造/析构函数】
    __RBTree(): _root(nullptr), _count(0) {}
    ~__RBTree() { _clear(_root); _count=0; }

    // 【构造/析构节点】
    static NodeType* _make_node(const Key& key);
    static void _destroy_node(NodeType* opnode);

    // 【左/右旋转】
    static NodeType* _left_rotate(NodeType* opnode);
    static NodeType* _right_rotate(NodeType* opnode);

    // 【增】
    NodeType* _insert(NodeType* opnode, const Key& key);

    // 【删】
    NodeType* _erase(NodeType* opnode, const Key& key);
    static void _clear(NodeType* opnode);

    // 【改、查】
    static NodeType* _find(NodeType* opnode, const Key& key);
};


#endif // __RB_TREE__