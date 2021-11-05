/* priority_queue.hpp
 * 【优先队列】允许以O(logn)时间增/删“优先级最高元素”
 * STL当中 <stl_queue.h>/<queue> 的 priority_queue<> 部分内容的简化/优化版
 * 
 * PriorityQueue<> 与 STL priority_queue<> 的不同之处：
 * 内部元素移动全部通过memcpy实现，而不是原本低效的深拷贝！
 * 这样当 Type 为 String / string 等带指针的序列类型，也能确保高效效率！
 */

/* 基于“最大二叉堆”实现的优先队列示意图
 * 【PrioritySuperior = Less<Type>就变成最小堆了，可自定指定优先级比较函数子】
 * ////////////////////////////////////
 * //              62[0]             //
 * //             /     \            //
 * //         41[1]      30[2]       //
 * //        /    \     /     \      //
 * //    28[3]  16[4]  22[5]  13[6]  //
 * //   /    \     /                 //
 * // 19[7] 17[8] 15[9]              //
 * ////////////////////////////////////
 * 最大堆的基本性质：
 * (1)存储结构是一棵完全二叉树 —— 层序遍历地增/减
 * (2)父节点值比任一子节点的优先级高
 * (3)parent_index(i) = (i-1) / 2
 * (4)left_child_index(i) = 2 * i + 1
 * (5)right_child_index(i) = 2 * i + 2
 */

#ifndef __PRIORITY_QUEUE__
#define __PRIORITY_QUEUE__
#include <initializer_list>
#include <cstring>
#include "alloc.hpp"
#include "traits.hpp"
#include "utils.hpp"
using namespace std;


// """优先队列PriorityQueue [STL priority_queue<>] """
template < class Type, class PrioritySuperior = Greater<Type>, class Alloc = FirstAlloc >
class PriorityQueue {

public:     // 【“优先队列”没有迭代器！！！】
    typedef Type                    value_type;
    typedef Type*                   pointer;
    typedef Type&                   reference;
    typedef size_t                  size_type;
    typedef Allocator<Type, Alloc>  data_allocator; // 【内存分配器】
    static const size_type default_capacity = 31;   // 可缩容的容量下界

private:    // 【成员变量】
    Type* _start;               // 优先队列的队首指针
    Type* _finish;              // 优先队列的队尾后一个指针
    Type* _end_of_storage;      // ...
    PrioritySuperior _superior; // 比较器，_superior(a, b)即“a优先级 > b优先级”

private:    // 【扩/缩容】
    void _resize(size_type n) {
        Type* new_start = data_allocator::reallocate(_start, n);
        _end_of_storage = new_start + n;
        _finish = new_start + size();
        _start = new_start;
    }

public:     // 【构造/析构函数】
    // 指定初始总容量
    PriorityQueue(size_type init_size = default_capacity):
        _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
        if (init_size > 0) {
            _start = data_allocator::allocate(init_size);
            _finish = _start;
            _end_of_storage = _start + init_size;
        }
    }
    // 对字面量数组heapify
    PriorityQueue(initializer_list<Type> init_list) {
        size_type init_size = init_list.size() * 2;  // 分配init_list两倍大小的空间
        _start = data_allocator::allocate(init_size);
        _finish = _start;
        _end_of_storage = _start + init_size;
        for (const auto& item : init_list)
            new (_finish++) Type(item);
        // heapify【从最后一个节点的父节点开始往前，就都可以看作一个子堆了，依次shift down】
        for (size_type index=((_finish-1-_start)-1)/2 ; index>=0; --index)
            _shift_down(index);
    }
    ~PriorityQueue() {
        mystl::destroy(_start, _finish);
        data_allocator::deallocate(_start);
    }

public:     // 【查】
    size_type size()     const { return size_type(_finish - _start); }
    size_type capacity() const { return size_type(_end_of_storage - _start); }
    bool empty()         const { return _start == _finish; }
    const Type& top()    const { return *_start; }  // 不可修改

public:     // 【改】
    // 将堆顶修改为成new_top，然后_shift_down()维护堆性质【前K大/小】
    void replace(const Type& new_top) {
        *_start = new_top;
        _shift_down(0);
    }

public:     // 【增】
    void push(const Type& item) {
        if (_finish == _end_of_storage)
            _resize(capacity() * 2 + 1);
        new (_finish++) Type(item);
        _shift_up(_finish-1 - _start);
    }
    private: void _shift_up(size_type index) {
        char tmp[sizeof(Type)];
        memcpy(tmp, _start+index, sizeof(Type));    // *tmp用于暂存刚入队那个
        size_type parent_idx = (index-1) / 2;
        while ( index > 0  &&  _superior(*(Type*)tmp, _start[parent_idx]) ) {     // *tmp优先级高于其父节点，还需上移
            memcpy(_start+index, _start+parent_idx, sizeof(Type));      // 即_start[index] = _start[parent_idx]
            index = parent_idx;                                         // 将父节点下移，等价于*tmp交换上移
            parent_idx = (index-1) / 2;
        }                                           // 循环退出后index==0或*tmp优先级不高于父亲节点了
        memcpy(_start+index, tmp, sizeof(Type));    // 此时index正是*tmp应该呆的地方（参考插入排序逻辑）
    }

public:     // 【删】
    Type pop() {
        if (_finish <= _start) {
            cerr << "warning: " << "PriorityQueue(at " << this << ") is empty!" << endl;
            return Type();
        }
        if (size() < capacity()/4  &&  capacity()/2 > default_capacity) {
            _resize(capacity() / 2);                // 延迟缩容：上一次pop()操作后，即使容量冗余也要拖到这次才缩容
        }
        Type tmp(*_start);                          // 暂存堆顶元素，用于返回
        _start->~Type();
        memcpy(_start, --_finish, sizeof(Type));    // 将末端元素 覆盖到 堆顶
        _shift_down(0);
        return tmp;
    }
    private: void _shift_down(size_type index) {
        char tmp[sizeof(Type)];
        memcpy(tmp, _start+index, sizeof(Type));    // *tmp用于暂存刚入队那个
        size_type finish_idx = _finish - _start;
        size_type child_idx = index * 2 + 1;
        while (child_idx < finish_idx) {
            // 选出左右孩子中优先级较高的一个（的索引）
            if ( child_idx+1 < finish_idx  &&   
                _superior(_start[child_idx+1], _start[child_idx]) ) ++child_idx;
            // 优先级较高的孩子比*tmp更优先？
            if ( _superior(_start[child_idx], *(Type*)tmp) ) {
                memcpy(_start+index, _start+child_idx, sizeof(Type));   // 即_start[index] = _start[child_idx]
                index = child_idx;                                      // 孩子节点上移，等价于*tmp下移
                child_idx = index * 2 + 1;
            }
            else { break; }
        }                                           // 循环退出后*tmp没有孩子(child_idx>=finish_idx)
        memcpy(_start+index, tmp, sizeof(Type));    // 或*tmp优先级不低于孩子节点，index就是*tmp应该呆的地方
    }
};


#endif // __PRIORITY_QUEUE__





/* // 测试(OK)
#include <ctime>
#include <queue>
int main(int argc, char const *argv[]) {

    {
        srand((unsigned)time(nullptr));
        PriorityQueue<double> pq;
        double xx[10];
        for (int i=0; i<10; ++i) {
            xx[i] = (double)rand();
            cout << xx[i] << " ";
            pq.push(xx[i]);
        }
        cout << endl;
        while (!pq.empty())
            cout << pq.pop() << " ";
        cout << "\n===" << endl;

        clock_t st = clock();
        PriorityQueue<int> my_pq;
        for (int i=0; i<int(1e6); ++i)
            my_pq.push(i);
        cout << my_pq.top() << ", " << my_pq.size() << endl;
        while (!my_pq.empty())
            my_pq.pop();
        clock_t et = clock();
        cout << my_pq.size() << endl;
        cout << et - st << " ms" << endl;

        st = clock();
        priority_queue<int> stl_pq;
        for (int i=0; i<int(1e6); ++i)
            stl_pq.push(i);
        cout << stl_pq.top() << ", " << stl_pq.size() << endl;
        while (!stl_pq.empty())
            stl_pq.pop();
        et = clock();
        cout << stl_pq.size() << endl;
        cout << et - st << " ms" << endl;
    }

    system("pause");
    return 0;
}
 */