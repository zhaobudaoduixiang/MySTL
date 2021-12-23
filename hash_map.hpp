#ifndef __HASH_MAP__
#define __HASH_MAP__
#include "alloc.hpp"
#include "utils.hpp"
using namespace std;

// 节点颜色
#define RED     true
#define BLACK   false

// ...
template <class Key>
struct __HashSetNode {
    Key                 key;
    __HashSetNode<Key>* right;  // 即next，继承到TreeNode后即右孩子（父类指针可无条件指向派生类对象）
};

// ...
template <class Key>
struct __HashSetTreeNode: __HashSetNode<Key> {
    __HashSetTreeNode<Key>* left;
    bool                    color;
};

// ...
template <class Key, class Value>
struct __HashMapNode { 
    Key                         key;
    Value                       value;
    __HashMapNode<Key, Value>*  right;
};

// ...
template <class Key, class Value>
struct __HashMapTreeNode: __HashMapNode<Key, Value> {
    __HashMapTreeNode<Key, Value>*  left;
    bool                            color;
};

// 迭代器
template <class Key, class Value>
struct __HashMapIterator {
    typedef __HashMapNode<Key, Value> Node;
    Node**      bucket;
    Node*       cur;
    void*       obj;
};

// """哈希映射[STL unordered_map<>]"""
template <class Key, 
          class Value, 
          class KeyHasher   = HashCode<Key>, 
          class KeyCompare  = Compare<Key>, 
          class TableAlloc  = FirstAlloc, 
          class NodeAlloc   = SecondAlloc>
class HashMap {

public:     // 【类型定义】
    typedef Pair<Key, Value>    value_type;
    typedef size_t              size_type;          // 64位编译器为unsigned long long，32位编译器为unsigned long
    typedef ptrdiff_t           difference_type;    // 64位编译器为long long，32位编译器为long，表示两个迭代器间的距离
    // pointer? reference?
    typedef __HashMapIterator<Key, Value>   iterator;
    typedef __HashMapNode<Key, Value>       Node;
    typedef __HashMapTreeNode<Key, Value>   TreeNode;
    typedef Allocator<Node*, TableAlloc>    table_allocator;
    typedef Allocator<Node, NodeAlloc>      node_allocator;
    typedef Allocator<bool, TableAlloc>     treetag_allocator;
    typedef Allocator<TreeNode, NodeAlloc>  treenode_allocator;

private:    // 【成员变量】
    static const size_type __up_tol = 10;   // 负载因子上限，即当node/bucket > 10时，rehash(next_table_size())
    static const size_type __low_tol = 1;   // 负载因子下限，即当node/bucket < 1时，rehash(prev_table_size())
    static constexpr size_type __table_sizes[28] = { 53, 97, 193, 389, 769, 1543, 3079, 6151, 
                                                     12289, 24593, 49157, 98317, 196613, 393241, 
                                                     786433, 1572869, 3145739, 6291469, 12582917, 
                                                     25165843, 50331653, 100663319, 201326611, 
                                                     402653189, 805306457, 1610612741, 3221225473UL,
                                                     4294967291UL };  // 哈希表长度【都是质数，用于取模散列】
    KeyCompare  _compare;       // ...
    KeyHasher   _hasher;        // ...
    Node**      _hash_table;    // ...
    size_type   _table_size;    // ...
    size_type   _size;          // ...
    bool*       _istree;        // ...

private:    // 【rehash】
    size_type _next_table_size() const {
        for (const size_type* cur=__table_sizes; cur<__table_sizes+28; ++cur)
            if (*cur == _table_size) return *(++cur);
        return (size_type)0;
    }
    size_type _prev_table_size() const {
        for (const size_type* cur=__table_sizes; cur<__table_sizes+28; ++cur)
            if (*cur == _table_size) return *(--cur);
        return (size_type)0;
    }
    void _rehash(size_type table_size) {;}

public:     // 【构造/析构函数】
    HashMap(): 
        _hash_table(table_allocator::clallocate(__table_sizes[0])),
        _table_size(__table_sizes[0]), _size(0),
        _istree(treetag_allocator::clallocate(__table_sizes[0])) {}
    ~HashMap() {
        clear();
        table_allocator::deallocate(_hash_table);
    }

public:     // 【Basic Accessor】
    size_type size() const { return _size; }
    bool empty()     const { return _size==0; }
    size_type hash(const Key& key) const            // 减少访问静态区的__table_sizes[]，
        { return (_hasher(key) & _table_size); }    // 充分利用局部性原理

public:     // 【增、改、查】
    void insert(const Key& key, const Value& value) {}
    const Value& operator[](const Key& key) const {}
    Value& operator[](const Key& key) {}

public:     // 【删】
    void earse(const Key& key) {}
    void clear() {}
};


#endif // __HASH_MAP__