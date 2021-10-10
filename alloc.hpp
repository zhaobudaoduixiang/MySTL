/*
 * STL当中的<stl_construct.h>和<stl_alloc.h>部分内容的简化版
 */
#ifndef __ALLOCATOR__
#define __ALLOCATOR__
// #include <new>      // ::operator new
#include <cstdlib>  // calloc, realloc, free
#include <cerrno>   // perror(即"print error")
#include "traits.hpp"
using namespace std;


// """在[first, last)区域内以value值构造对象[STL uninitialized_fill()]"""【似乎没什么必要？！】
// template<class ForwardIterator, class Type>
// inline void __mystl_construct(ForwardIterator first, 
//                               ForwardIterator last, 
//                               const Type& value, TpFalse) {}    // 一般非POD类型，for... new (ptr) Type(value)
// template<class ForwardIterator, class Type>
// inline void __mystl_construct(ForwardIterator first, 
//                               ForwardIterator last, 
//                               const Type& value, TpTrue) {}     // POD类型，for... * = value
// template <class ForwardIterator, class Type>
// inline void mystl_construct(ForwardIterator first, 
//                             ForwardIterator last, 
//                             const Type& value) {}
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
/* 注意：
 * SGI STL中惯用的inline void construct(T1*, const T2&)只是单纯的placement new，这里不进行包装
 * 单个的inline void destroy(T*)也只是单纯地调用析构函数，这里也不进行包装
 */


// """一级内存分配器FirstAlloc【满足::allocate, ::reallocate, ::deallocate接口】"""
template <class Type>
struct FirstAlloc {
    // 分配nobj个Type类对象的空间(malloc)
    static Type* allocate(size_t nobj = 1) {
        Type* mem = (Type*)malloc(nobj * sizeof(Type));
        if (!mem) { perror("out of memory!\n"); exit(1); }
        return mem;
    }
    // 只是free(mem)
    static void deallocate(Type* mem, size_t nobj = 0) { free(mem); }
    // 基于mem，重新分配nobj个Type类对象的空间(realloc)
    static Type* reallocate(Type* mem, size_t nobj) {               // 【_msize(ptr)运算符可知分配了多少内存给ptr】
        Type* new_mem = (Type*)realloc(mem, nobj*sizeof(Type));     // 【realloc自带拷贝和free！】
        if (!new_mem) {
            perror("out of memory!\n");
            free(mem);  // realloc失败，mem未释放！
            exit(1);    // 也可exit(EXIT_FAILURE)
        }
        return new_mem;
    }
    // 其实也可calloc/_recalloc组合，只是_recalloc某些编译器不兼容...
};


// """二级内存分配器SecAlloc——内存池分配器【只满足::allocate, ::reallocate】"""
#define __ALIGN             8       // 分配内存8字节对齐，STL中为enum {__ALIGN = 8};
#define __MAX_BYTES         128     // 大于128字节则调用FirstAlloc，STL中为enum {__MAX_BYTES = 128};
#define __N_MEM_LIST        16      // 128 ÷ 8 = 16，STL中为enum {__NFREELISTS = __MAX_BYTES / __ALIGN};
#define __N_BLOCK_PER_LIST  20      // 每次_mlist_alloc()获得的内存链表默认为20个对应大小的小区块
// 内存池【STL中的“二级内存分配器模板(__default_alloc_template)”】
template <bool threads = false>
struct __MemoryPool {
private:
    union mem_block {
        union mem_block* next_block;
        char any_data[1];
    };
    static char* _start_free;   // 内存池起始位置，只在_chunk_alloc()中变化
    static char* _end_free;     // 内存池结束位置，只在_chunk_alloc()中变化
    static size_t _heap_size;   // ...
    static mem_block* volatile _mem_lists[__N_MEM_LIST];    // 16条内存链表
    
    // 将nbytes上调至8的倍数：+7然后砍掉二进制的后3位（变成8的倍数）
    static size_t _block_size(size_t nbytes)
        { return ( (nbytes + __ALIGN-1) & ~(__ALIGN-1) ); }
    // nbytes的小区块在_mem_lists中位置 = ⌈nbytes / 8⌉-1
    static size_t _list_index(size_t nbytes)
        { return ( (nbytes + __ALIGN-1) / __ALIGN - 1 ); }

    // 从内存池中分配内存，然后切分/串联为__N_BLOCK_PER_LIST(20)个block_size字节的小内存块（内存链表）
    static mem_block* _mlist_alloc(size_t block_size) {
        size_t nblocks = __N_BLOCK_PER_LIST;
        char* chunk = _chunk_alloc(block_size, nblocks);    // nblocks存_chunk_alloc()分配得到区块个数，传引用！
        char* next_block = chunk + block_size;              // nblocks有可能不足__N_BLOCK_PER_LIST(20)个！
        mem_block* cur_block = (mem_block*)chunk;
        for (size_t i=0; i<nblocks-1; ++i) {        // 将新分配的内存切分/串联到各个mem_block
            cur_block->next_block = (mem_block*)next_block;
            cur_block = (mem_block*)next_block;
            next_block += block_size;
        }   cur_block->next_block = nullptr;        // 循环结束后，cur_block来到最后一块新分配内存，封尾(nullptr)
        // _mem_lists[_list_index(block_size)] = (mem_block*)chunk;
        return (mem_block*)chunk;
    }
    // 从内存池中分配nblocks个block_size字节的内存块所需的总内存【静态变量未初始化，放外边再定义】
    static char* _chunk_alloc(size_t block_size, size_t& nblocks);

public:
    // 分配nbytes个字节的空间
    static void* allocate(size_t nbytes) {
        if (nbytes > __MAX_BYTES)                   // 大于128字节，使用malloc分配
            return malloc(nbytes);
        size_t i = _list_index(nbytes);             // 定位到对应内存链表，拨出小内存块
        mem_block* cur_block = 
            _mem_lists[i]? _mem_lists[i]: _mlist_alloc(__ALIGN*(i+1));
        _mem_lists[i] = cur_block->next_block;
        // SGI STL写法：mem_block* volatile *mem_list_ptr = _mem_lists + _list_index(block_size);
        return (cur_block);
    }
    // 释放mem所指空间，其大小为nbytes个字节
    static void deallocate(void* mem, size_t nbytes) {
        if (nbytes > __MAX_BYTES)                   // 大于128字节，使用free释放
            return free(mem);
        size_t i = _list_index(nbytes);
        mem_block* next_block = _mem_lists[i];      // 即对应内存链表.push_front(mem)
        mem_block* cur_block = (mem_block*)mem;
        cur_block->next_block = next_block;
        _mem_lists[i] = cur_block;
    }
};
    // __MemoryPool静态成员变量初始化
    // __MemoryPool不能设置为<class Type>这样的模板类，否则内存池就没意义了！！！
    template <bool threads>
    char* __MemoryPool<threads>::_start_free = nullptr;
    template <bool threads>
    char* __MemoryPool<threads>::_end_free = nullptr;
    template <bool threads>
    size_t __MemoryPool<threads>::_heap_size = 0;
    template <bool threads>
    typename __MemoryPool<threads>::mem_block* volatile __MemoryPool<threads>::
    _mem_lists[__N_MEM_LIST] = {nullptr,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0};
    // 从内存池中分配nblocks个block_size字节的内存块所需的总内存
    template <bool threads>
    char* __MemoryPool<threads>::_chunk_alloc(size_t block_size, size_t& nblocks) {
        char* chunk;
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
            if (pool_bytes >= __ALIGN) {
                size_t i = _list_index(pool_bytes);
                ((mem_block*)_start_free)->next_block = _mem_lists[i];
                _mem_lists[i] = (mem_block*)_start_free;
            }
            // 【malloc()填充内存池】
            size_t fill_bytes = 2 * alloc_bytes + _block_size(_heap_size>>4);
            chunk = (char*)malloc(fill_bytes);
            if (!chunk) {   // malloc失败，尝试从别的内存链表分配区块？抛出异常？
                ;
            }
            _start_free = chunk + alloc_bytes;
            _end_free = chunk + fill_bytes;
            _heap_size += fill_bytes;
        }
        return chunk;
    }

// 二级内存分配器接口SecAlloc【只满足::allocate, ::reallocate】
template <class Type, bool threads = false>
struct SecAlloc {
    // 分配nobj个【默认nobj=1】Type类对象的空间
    static Type* allocate() 
        { return (Type*)__MemoryPool<threads>::allocate(sizeof(Type)); }
    static Type* allocate(size_t nobj) 
        { return (Type*)__MemoryPool<threads>::allocate(nobj*sizeof(Type)); }
    // 释放mem的空间，其大小为nobj个【默认nobj=1】Type类对象的大小
    static void deallocate(Type* mem) 
        { __MemoryPool<threads>::deallocate(mem, sizeof(Type)); }
    static void deallocate(Type* mem, size_t nobj) 
        { __MemoryPool<threads>::deallocate(mem, nobj*sizeof(Type)); }
    // ??????????????基于mem，重新分配nobj个Type类对象的空间
    // static Type* reallocate(Type* mem, size_t nobj) {}
};
/* 注意：
 * 原SGI STL的二级内存分配器适应STL标准后，现存于<ext/pool_allocator.h>
 * 使用时将std::allocator替换成__gnu_cxx::__pool_alloc<Type>即可
 * 不过，在大片内存管理上（比如vector和deque），std::allocator其实快很多！！！
 * 只是，对链式数据结构（比如list和map），二级内存分配器会让空间利用率更高 —— 
 * 因为每一次::operator new/malloc都会有标记部分，以供系统回收/对齐，二级内存分配器会让malloc次数大幅减少！
 */


#endif  // __ALLOCATOR__