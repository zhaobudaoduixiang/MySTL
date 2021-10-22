/* alloc.hpp
 * 【内存分配器】
 * STL当中 <stl_construct.h> <stl_alloc.h> 部分内容的简化版
 * 
 * 一级内存分配器：
 * 即malloc()/free()/realloc()，适用于大片连续空间的分配
 * 
 * 二级内存分配器：
 * SGI STL的私房菜【GCC4.9开始被废弃...】，对链式数据结构的效率有极大提升！
 * 因为 —— 每次::operator new/malloc()所得内存块都会有标记/冗余部分，以供系统回收/对齐，
 * 而对于链式数据结构，其节点所占空间一致，因此这些标记部分都是多余的，二级内存分配器可让malloc()次数大幅减少！
 * 原SGI STL的二级内存分配器适应STL标准后，现存于<ext/pool_allocator.h>，名为__gnu_cxx::__pool_alloc<>
 * 
 * 注意：
 * 个人认为，原SGI STL将两级内存分配器通过simple_alloc<>接口“合二为一”，从而对外完全屏蔽的做法并不好！
 * 由于二级内存分配器难以实现reallocate()，故simple_alloc<>只有allocate()和deallocate()，这样会大幅降低vector<>和priority_queue<>等的性能！
 * 所以，这里我将二级内存分配器分开，Vector<>/PriorityQueue<>/Deque<>等默认使用一级内存分配器，SList<>/TreeMap<>等则默认使用二级内存分配器！
 */
#ifndef __ALLOCATOR__
#define __ALLOCATOR__
#include <new>
#include <cstdlib>      // malloc, realloc, free
#include <cerrno>       // perror(print error)
#include "traits.hpp"   // TypeTraits<>, IteratorTraits<>
using namespace std;


// """在[first, last)区域内以value值构造对象[STL uninitialized_fill()]"""
// 似乎真没什么必要... uninitialized_fill_n()的源码在《STL源码剖析》的3.7
template<class ForwardIterator, class Type>
inline void __mystl_construct(ForwardIterator first, 
                              ForwardIterator last, 
                              const Type& value, TpFalse)   // 一般非POD类型
    { for(; first!=last; ++first) new (first) Type(value); }
template<class ForwardIterator, class Type>
inline void __mystl_construct(ForwardIterator first, 
                              ForwardIterator last, 
                              const Type& value, TpTrue)    // POD类型
    { for(; first!=last; ++first) *first=value; }
template <class ForwardIterator, class Type>
inline void mystl_construct(ForwardIterator first, 
                            ForwardIterator last, 
                            const Type& value)
    { __mystl_construct(first, last, value, 
                        typename TypeTraits<typename IteratorTraits<ForwardIterator>::value_type>
                        ::is_POD_type()); }

// """解构[first, last)区域的全部对象[STL destroy()]"""
template <class ForwardIterator, class Type>
inline void __mystl_destroy(ForwardIterator first, 
                            ForwardIterator last, TpTrue) {}    // 有trivial_destructor的类型
template <class ForwardIterator, class Type>
inline void __mystl_destroy(ForwardIterator first, 
                            ForwardIterator last, TpFalse)      // 无trivial_destructor的类型
    { for(; first != last; ++first) first->~Type(); }
template <class ForwardIterator>
inline void mystl_destroy(ForwardIterator first, 
                          ForwardIterator last) {
    __mystl_destroy<ForwardIterator, typename IteratorTraits<ForwardIterator>::value_type>(
        first, last, 
        typename TypeTraits<typename IteratorTraits<ForwardIterator>::value_type>
        ::has_trivail_destructor()
    );
}
/* SGI STL中惯用的inline void construct(T1*, const T2&)只是单纯的placement new，这里不进行包装
 * 单个的inline void destroy(T*)也只是单纯地调用析构函数，这里也不进行包装
 */


// """一级内存分配器FirstAlloc【适用于大片连续空间分配，如Vector<>等】"""
// 具有 ::allocate()即malloc() / ::deallocate()即free() /::reallocate()即realloc()
struct FirstAlloc {
    // malloc()分配nbytes字节的空间
    static void* allocate(size_t nbytes) {
        void* mem = malloc(nbytes);
        if (!mem) { perror("out of memory!\n"); exit(1); }
        return mem;
    }
    // 即free(mem)
    static void deallocate(void* mem) { free(mem); }
    // realloc()为mem重新分配nbytes字节的空间
    static void* reallocate(void* mem, size_t nbytes) {     // _msize(mem)可知分配了多少内存给mem
        void* new_mem = realloc(mem, nbytes);               // realloc自带memcpy和free
        if (!new_mem) {
            perror("out of memory!\n");
            free(mem);  // realloc失败，mem未释放！
            exit(1);    // 也可exit(EXIT_FAILURE)
        }
        return new_mem;
    }
    // 其实也可calloc/_recalloc组合，只是_recalloc某些编译器不兼容...
};


// """二级内存分配器SecondAlloc【适用于链式数据结构，如SList<>等】"""
// 只具有 ::allocate() / ::deallocate()，基于 “内存池” 实现[STL __default_alloc_template]
class SecondAlloc {
    // 一些常量
    static const size_t __align             = 8;    // 分配的内存大小为“8字节对齐”
    static const size_t __max_bytes         = 128;  // 二级分配器最多分配128字节，否则将自动调用malloc()/free()
    static const size_t __n_mem_lists       = 16;   // 128 / 8 = 16，16条内存链表
    static const size_t __n_blocks_per_list = 20;   // 内存链表每次填充20个内存块

    // 内存块：平时存着下一块内存块的地址，用时可覆盖
    union mem_block {
        union mem_block* next_block;
        char any_data[1];
    };

    // 内存池【原本还应考虑多线程情况，这里只是简单实现，就不搞这么多了。。。】
    static char* _start_free;   // 内存池起始位置，只在_chunk_alloc()中变化
    static char* _end_free;     // 内存池结束位置，只在_chunk_alloc()中变化
    static size_t _heap_size;   // ...
    static mem_block* volatile _mem_lists[__n_mem_lists];  // 16条内存链表
    
    // 将nbytes上调至8的倍数：+7然后砍掉二进制的后3位（变成8的倍数）
    static size_t _block_size(size_t nbytes)
        { return ( (nbytes + __align-1) & ~(__align-1) ); }
    // nbytes的区块在_mem_lists中位置 = ⌈nbytes / 8⌉-1
    static size_t _list_index(size_t nbytes)
        { return ( (nbytes + __align-1) / __align - 1 ); }

    // 从内存池中分配内存，然后切分/串联为__n_blocks_per_list(20)个block_size字节的小内存块（内存链表）
    static mem_block* _mlist_alloc(size_t block_size) {
        size_t nblocks = __n_blocks_per_list;
        char* chunk = _chunk_alloc(block_size, nblocks);    // nblocks存_chunk_alloc()分配得到区块个数，传引用！
        char* next_block = chunk + block_size;              // nblocks有可能不足__n_blocks_per_list(20)个！
        mem_block* cur_block = (mem_block*)chunk;
        for (size_t i=0; i<nblocks-1; ++i) {        // 将新分配的内存切分/串联到各个mem_block
            cur_block->next_block = (mem_block*)next_block;
            cur_block = (mem_block*)next_block;
            next_block += block_size;
        }   cur_block->next_block = nullptr;        // 循环结束后，cur_block来到最后一块新分配内存，封尾(nullptr)
        // _mem_lists[_list_index(block_size)] = (mem_block*)chunk;
        return (mem_block*)chunk;
    }
    // 从内存池中分配nblocks个block_size字节的内存块所需的总内存【太长了，放外边再定义】
    static char* _chunk_alloc(size_t block_size, size_t& nblocks);

public:
    // 分配nbytes个字节的空间
    static void* allocate(size_t nbytes) {
        if (nbytes > __max_bytes)                   // 大于128字节，使用malloc()分配
            return FirstAlloc::allocate(nbytes);
        size_t i = _list_index(nbytes);             // 定位到对应内存链表，然后拨出内存块
        mem_block* cur_block = 
            _mem_lists[i]? _mem_lists[i]: _mlist_alloc(__align*(i+1));
        _mem_lists[i] = cur_block->next_block;
        // SGI STL写法：mem_block* volatile *mem_list_ptr = _mem_lists + _list_index(block_size);
        return (cur_block);
    }
    // 释放mem所指空间，其大小为nbytes个字节
    static void deallocate(void* mem, size_t nbytes) {
        if (nbytes > __max_bytes)                   // 大于128字节，使用free()释放
            return FirstAlloc::deallocate(mem);
        size_t i = _list_index(nbytes);
        mem_block* next_block = _mem_lists[i];      // 以下即对应内存链表.push_front(mem)
        mem_block* cur_block = (mem_block*)mem;
        cur_block->next_block = next_block;
        _mem_lists[i] = cur_block;
    }
};

// SecondAlloc静态成员变量的初始化
// 正是因为这些静态成员，即要求不同类型的空间在同一个内存池分配，所以不能设置为<class Type>这样的模板类！
char* SecondAlloc::_start_free  = nullptr;
char* SecondAlloc::_end_free    = nullptr;
size_t SecondAlloc::_heap_size  = 0;
typename SecondAlloc::mem_block* volatile SecondAlloc::
_mem_lists[__n_mem_lists] = {nullptr,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0};

// 从内存池中分配nblocks个block_size字节的内存块所需的总内存
char* SecondAlloc::_chunk_alloc(size_t block_size, size_t& nblocks) {
    char* chunk;    // 返回的空间起始地址
    size_t alloc_bytes = block_size * nblocks;
    size_t pool_bytes = _end_free - _start_free;
    if (pool_bytes >= alloc_bytes) {        // 内存池剩余空间完全满足需求
        chunk = _start_free;
        _start_free += alloc_bytes;
    }
    else if (pool_bytes >= block_size) {    // 内存池剩余空间不完全满足需求，但可以提供一个以上区块
        nblocks = pool_bytes / block_size;
        chunk = _start_free;
        _start_free += block_size*nblocks;
    }
    else {                                  // 内存池剩余空间不足，连一个区块都拿不出
        // 充分利用内存池剩余空间
        if (pool_bytes >= __align) {
            size_t i = _list_index(pool_bytes);
            ((mem_block*)_start_free)->next_block = _mem_lists[i];
            _mem_lists[i] = (mem_block*)_start_free;
        }
        // malloc()填充内存池
        size_t fill_bytes = 2 * alloc_bytes + _block_size(_heap_size>>4);
        chunk = (char*)malloc(fill_bytes);
        if (!chunk) {
            ;   // malloc失败，尝试从别的内存链表分配区块？抛出异常？
        }
        _start_free = chunk + alloc_bytes;  // 【注：chunk前alloc_bytes字节算是已分配出去的，即将被用】
        _end_free = chunk + fill_bytes;
        _heap_size += fill_bytes;
    }
    return chunk;
}


// """内存分配器接口【可自行偏特化】"""
// 默认为一级内存分配器
template <class Type, class Alloc = FirstAlloc>
struct Allocator {
    static Type* allocate(size_t nobjs) 
        { return (Type*)Alloc::allocate(nobjs*sizeof(Type)); }
    static void deallocate(Type* mem) 
        { Alloc::deallocate(mem); }
    static Type* reallocate(Type* mem, size_t nobjs) 
        { return (Type*)Alloc::reallocate(mem, nobjs*sizeof(Type)); }
};
// 二级内存分配器偏特化的Allocator
template <class Type>
struct Allocator<Type, SecondAlloc> {
    static Type* allocate()
        { return (Type*)SecondAlloc::allocate(sizeof(Type)); }
    static void deallocate(Type* mem)
        { SecondAlloc::deallocate(mem, sizeof(Type)); }
};


#endif  // __ALLOCATOR__