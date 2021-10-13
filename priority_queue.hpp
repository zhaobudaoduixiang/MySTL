/* priority_queue.hpp
 * 优先队列
 * STL当中 <stl_queue.h>/<queue> 的 priority_queue<> 部分内容的简化/优化版
 * PriorityQueue<> 与 STL priority_queue<> 的不同之处：
 * 内部元素移动全部通过memcpy实现，而不是原本低效的深拷贝！
 * 这样当 Type 为 String / string 等带指针的序列类型，也能确保高效效率！
 */
/* 基于“最大二叉堆”实现的优先队列示意图
 * 【PriorityCompare = Less<Type>就变成最小堆了，可自定指定优先级比较函数子】
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
template < class Type, class PriorityCompare = Greater<Type>, class Alloc = FirstAlloc<Type> >
class PriorityQueue {
public:     // 【“优先队列”没有迭代器！！！】
    static const size_t default_capacity = 31ULL;   // 默认初始大小(已经演变成缩容的“下界”了。。。)

private:    // 【成员变量】
    Type* _start;           // 队首地址
    Type* _finish;          // 待入队位置
    Type* _end_of_storage;  // ...
    PriorityCompare _prior; // 比较器，_prior(a, b)即 a优先级 > b优先级 ?

            // 【扩/缩容】
    void _resize(size_t n) {
        Type* new_start = Alloc::reallocate(_start, n);
        _end_of_storage = new_start + n;
        _finish = new_start + size();   // size() = _finish - _start
        _start = new_start;
    }

public:     // 【构造/析构函数】
    // 延迟构造
    PriorityQueue(): 
        _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {}
    // 对字面量数组heapify
    PriorityQueue(initializer_list<Type> init_list) {
        size_t init_size = init_list.size() * 2;
        _start = Alloc::allocate(init_size);
        _finish = _start;
        _end_of_storage = _start + init_size;
        for (const auto& item : init_list)
            new (_finish++) Type(item);
        // heapify【从最后一个节点的父节点开始，依次往前shift down】
        for (size_t index=((_finish-1-_start)-1)/2 ; index>0; --index)
            _shift_down(index);
    }
    // 指定初始总容量
    PriorityQueue(size_t capacity):
        _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
        if (capacity > 0) {
            _start = Alloc::allocate(capacity);
            _finish = _start;
            _end_of_storage = _start + capacity;
        }
    }
    ~PriorityQueue() {
        mystl_destroy(_start, _finish);
        Alloc::deallocate(_start);
    }

public:     // 【查】
    size_t size()       const { return size_t(_finish - _start); }
    size_t capacity()   const { return size_t(_end_of_storage - _start); }
    bool empty()        const { return _start == _finish; }
    const Type& top()   const { return *_start; }

public:     // 【改】
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
    private: void _shift_up(size_t index) {
        Type tmp[1]; memcpy(tmp, _start+index, sizeof(Type));       // *tmp暂存刚入队那个
        size_t parent_idx = (index-1) / 2;
        while ( index > 0  &&  _prior(*tmp, _start[parent_idx]) ) { // *tmp优先级高于其父节点，还需上移
            memcpy(_start+index, _start+parent_idx, sizeof(Type));  // 即_start[index] = _start[parent_idx]
            index = parent_idx;                                     // 将父节点下移，等价于*tmp交换上移
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
            _resize(capacity() / 2);  // 延迟缩容，即：上一次_pop()操作后，即使容量冗余，也不会立即缩容
        }
        Type tmp(*_start);
        _start->~Type();
        memcpy(_start, --_finish, sizeof(Type));
        _shift_down(0);
        return tmp;
    }
    private: void _shift_down(size_t index) {
        Type tmp[1]; memcpy(tmp, _start+index, sizeof(Type));           // *tmp暂存刚入队那个
        size_t finish_idx = _finish - _start;
        size_t child_idx = index * 2 + 1;
        while (child_idx < finish_idx) {
            // 选出左右孩子中优先级较高的一个（的索引）
            if ( child_idx+1 < finish_idx  &&   
                _prior(_start[child_idx+1], _start[child_idx]) ) ++child_idx;
            // 优先级较高的孩子比*tmp更优先？
            if (_prior(_start[child_idx], *tmp)) {
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