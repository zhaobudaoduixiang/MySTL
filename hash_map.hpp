#ifndef __HASH_MAP__
#define __HASH_MAP__
#include "alloc.hpp"
#include "utils.hpp"
using namespace std;

// 宏：节点颜色、负载因子
#define RED     true
#define BLACK   false
#define UP_TOL  10ULL
#define LOW_TOL 1ULL

// ...
template <class Key, class Value>
struct __HashMapNode {
    Key                         _key;   // 键
    Value                       _val;   // 值
    __HashMapNode<Key, Value>*  _right; // 这里即next，继承到TreeNode后即右孩子（父类指针可无条件指向派生类对象）
};

// ...
template <class Key, class Value>
struct __HashMapTreeNode: __HashMapNode<Key, Value> {
    __HashMapTreeNode<Key, Value>* _left;   // 左孩子
    bool _color;                            // 颜色：红/黑
};

// ...
template <class Key, class Value>
struct __HashMapIterator {};

// """哈希映射[STL unordered_map<>]"""
template <class Key, 
          class Value, 
          class KeyHasher   = HashCode<Key>, 
          class KeyCompare  = Compare<Key>, 
          class TableAlloc  = FirstAlloc, 
          class NodeAlloc   = SecondAlloc>
class HashMap {

public:     // 【类型定义】
    // pointer, reference, size_type, difference_type...
    typedef Pair<Key, Value>                value_type;
    typedef __HashMapIterator<Key, Value>   iterator;
    typedef __HashMapNode<Key, Value>       _Node;
    typedef __HashMapTreeNode<Key, Value>   _TreeNode;
    typedef Allocator<_Node*, TableAlloc>   table_allocator;
    typedef Allocator<_Node, NodeAlloc>     node_allocator;
    typedef Allocator<bool, TableAlloc>     treetag_allocator;
    typedef Allocator<_TreeNode, NodeAlloc> treenode_allocator;
    typedef unsigned long                   ulong_t;

private:    // 【成员变量】
    static constexpr ulong_t __table_sizes[28] = { 53, 97, 193, 389, 769, 1543, 3079, 6151, 
                                                12289, 24593, 49157, 98317, 196613, 393241, 
                                                786433, 1572869, 3145739, 6291469, 12582917, 
                                                25165843, 50331653, 100663319, 201326611, 
                                                402653189, 805306457, 1610612741, 3221225473UL,
                                                4294967291UL };
    KeyCompare  _compare;
    KeyHasher   _hasher;
    _Node**     _hash_table;
    ulong_t     _table_size;
    size_t      _size;
    bool*       _istree;

private:    // 【rehash】
    ulong_t _next_tbsize() const {
        for (const ulong_t* cur=__table_sizes; cur<__table_sizes+28; ++cur)
            if (*cur == _table_size) return *(++cur);
        return (ulong_t)0;
    }
    void _rehash(ulong_t tbsize) {;}

public:     // 【构造/析构函数】
    HashMap(): 
        _hash_table(table_allocator::clallocate(__table_sizes[0])),
        _table_size(__table_sizes[0]), _size(0ULL),
        _istree(treetag_allocator::clallocate(__table_sizes[0])) {}
    ~HashMap() {
        clear();
        table_allocator::deallocate(_hash_table);
    }

public:     // 【Basic Accessor】
    size_t size() const { return _size; }
    bool empty()  const { return _size==0; }
    ulong_t hash(const Key& key) const 
        { return (_hasher(key) & _table_size); }  // 充分利用局部性原理

public:     // 【增、改、查】
    void insert(const Key& key, const Value& value) {}
    const Value& operator[](const Key& key) const {}
    Value& operator[](const Key& key) {}

public:     // 【删】
    void earse(const Key& key) {}
    void clear() {}
};


#endif // __HASH_MAP__