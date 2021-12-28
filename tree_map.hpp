#ifndef __TREE_MAP__
#define __TREE_MAP__
#include <initializer_list>
#include "alloc.hpp"    // ...
#include "traits.hpp"   // ...
#include "utils.hpp"    // ...
#include "rb_tree.hpp"
using namespace std;


// 树状映射节点
template <class Key, class Value>
struct __TreeMapNode: public __RBTreeNode<Key> { Value value; };
// 迭代器...


// """..."""
template <class Key, class Value, class KeyCompare = Compare<Key>, class Alloc = SecondAlloc>
class TreeMap: public __RBTree<Key, KeyCompare, Alloc> {
public:
    void insert(const Key& key, const Value& value) {
        _insert(_root, key);
    }
// public:     // 【类型定义】
//     typedef Pair<Key, Value>    value_type;         // pointer? reference?
//     typedef size_t              size_type;
//     typedef ptrdiff_t           difference_type;
//     typedef __TreeMapNode<Key, Value>   Node;
//     typedef Allocator<Node, Alloc>      node_allocator;
//     static const bool red   = true;
//     static const bool black = false;

// private:    // 【成员变量】
//     Node*       _root;
//     size_type   _count;

// public:     // 【构造/析构函数】
//     TreeMap() {}
};




#endif // __TREE_MAP__