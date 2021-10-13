/* vector.hpp
 * 动态数组
 * STL当中 <stl_vector.h>/<vector> 部分内容的简化/优化版
 * Vector<> 与 STL vector<> 不同之处：
 * insert()/erase()中的元素移动、_resize()扩容/缩容，都是通过内存拷贝(memmove/memcpy)实现
 * 即使Type类型对象带指向其它空间的指针，这样做也是完全没问题的！！！无需频繁地进行构造/解构对象！
 */
/* 关于深/浅拷贝：
 * 如果参数是右值引用，绝对要深拷贝 —— 右值引用是临时对象，若只复制指针，操作完后临时对象析构，这个刚构建的对象也没了
 * 如果参数是常引用，也要深拷贝！不然对象析构的时候会free()同一个指针多次造成程序崩溃！
 * 综上，总之深拷贝就对了！
 * (1)像如下情况，编译器 不会 进行多余的深拷贝：
 * template<class Type>
 * Vector<Type>& generate_Vector() { Vector<Type> tmp; ...; return tmp; }
 * Vector<xx> vec = generate_Vector<xx>()
 * 将tmp构造好后，就直接将三个指针塞到vec了，而不会进行深拷贝构造！
 * (2)像如下情况，则会提示无法操作：
 * Vector<xx> vec;
 * vec = generate_Vector();
 * 此时generate_Vector()产生的是临时的右值引用，只能重载operater=(&&)进行深拷贝！
 */

#ifndef __VECTOR__
#define __VECTOR__
#include <iostream>         // cout, cerr, ostream... 以及 <new>/new, <cstring>/memmove, <cstdlib>/malloc, <windows.h>/system 等
#include <initializer_list> // initializer_list<>
#include "alloc.hpp"        // FirstAlloc<>
#include "traits.hpp"       // TypeTraits<>
using namespace std;


// """动态数组Vector[STL vector<>]"""
template < class Type, class Alloc = FirstAlloc<Type> >
class Vector {
public:     // 【迭代器等内部类型定义】
    static const size_t default_capacity = 31ULL;   // 默认初始大小(已经演变成缩容的“下界”了。。。)
    typedef Type        value_type;         // 
    typedef Type*       iterator;           // 迭代器(Type*)
    typedef Type&       reference;          // 
    typedef Type*       pointer;            // 
    typedef size_t      size_type;          // unsigned long long
    typedef ptrdiff_t   difference_type;    // 两个迭代器之间的距离(long long)
    // 除上述以外，还应该有const_iterator, reverse_iterator, const_reference......

protected:  // 【成员变量】
    // [@][@][@][@][@][@][@][@][ ][ ][ ][ ][ ][ ][ ][ ]
    // ↑                       ↑                       ↑
    // _start                  _finish                 _end_of_storage
    Type* _start;               // 起始地址
    Type* _finish;              // 已初始地址的最后一个+1
    Type* _end_of_storage;      // 可用地址的最后一个+1

            // 【扩/缩容】
    void _resize(size_t n) {
        // 其实并不需要像STL vector那样进行：再分配->深拷贝...
        // 直接用realloc调整空间或浅拷贝即可，即使Type类对象另带指针也没问题的！
        Type* new_start = Alloc::reallocate(_start, n);
        _end_of_storage = new_start + n;
        _finish = new_start + size();  // 不加if(new_start != _start)，尽量不破坏流水线
        _start = new_start;
    }

public:     // 【构造、析构函数】
    // 缺省构造，采用延迟构造(_start, ... = nullptr)
    Vector(): _start(nullptr), 
             _finish(nullptr), 
             _end_of_storage(nullptr) {}
    // 构造 [ value, value, value..., (n个元素) ] 这样的一个数组对象
    // 若n=0则延迟构造(_start, ... = nullptr)，value默认为Type()
    Vector(size_t n, const Type& value = Type()): 
        _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
        // 【注：STL vector带explicit，防止隐式转换。这里没有int/long n类型的重载，不要explicit】
        if (n != 0) {
            _start = Alloc::allocate(n);
            _finish = _start;
            _end_of_storage = _start + n;
            while (_finish != _end_of_storage)
                new (_finish++) Type(value);    // mystl_construct(...)
            // cout << "construct: " << _start << endl;
        }
    }
    // 以[first, last)指针区域的内容构造一个数组对象
    // 若first>=last则延迟构造(_start, ... = nullptr)
    Vector(iterator first, iterator last): 
        _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {
        if (first < last) {
            size_t n = size_t(last - first);
            _start = Alloc::allocate(n);
            _finish = _start;
            _end_of_storage = _start + n;
            while (first < last)
                new (_finish++) Type(*first++);     // 无论*和++运算级别如何，*first++结果都一样；优先级++高于*
        }
    }
    // 以字面量Vector<Type>({x, xx, xxx...})构造一个数组
    // 若init_list为空，会报错！！！
    Vector(initializer_list<Type> init_list) {
        _start = Alloc::allocate(init_list.size());
        _finish = _start;
        _end_of_storage = _start + init_list.size();
        for (const auto& item : init_list) new (_finish++) Type(item);
        // cout << "construct: " << _start << endl;
    }
    // 析构并释放空间
    ~Vector() {
        // cout << "destroy" << _start << endl;
        mystl_destroy(_start, _finish);     // 依次析构
        Alloc::deallocate(_start);          // 释放空间
    }
    // 拷贝构造函数，默认深拷贝
    Vector(const Vector<Type>& other, bool deepcopy=true):
        _start(other._start), _finish(other._finish), _end_of_storage(other._end_of_storage) {
        if (deepcopy) {
            _start = Alloc::allocate(other.size());
            _end_of_storage = _start + other.size();
            _finish = _start;
            for (const Type& item : other) new (_finish++) Type(item);
            // cout << "copy construct: " << _start << endl;
        }
    }
    // obj = other，采用深拷贝
    Vector<Type>& operator=(const Vector<Type>& other) {
        if (&other != this) {               // 确保不是“自己=自己”... ←.←
            if (!_start) this->~Vector();   // ...
            _start = Alloc::allocate(other.size());
            _end_of_storage = _start + other.size();
            _finish = _start;
            for (const Type& item : other) new (_finish++) Type(item);
            // cout << "op=() copy construct: " << _start << endl;
        }
        return *this;
    }

public:     // 【改、查】类定义中不超一行自动内联
    // 已有的元素个数
    size_t size() const { return size_t(_finish - _start); }
    // 总共可容纳的元素个数
    size_t capacity() const { return size_t(_end_of_storage - _start); }
    // 是否为空
    bool empty() const { return _start == _finish; }
    // 
    iterator begin()    const { return _start; }            // 更标准的定义应该是const_iterator begin() const {...}
    iterator end()      const { return _finish; }           // 且应该重载多一个iterator begin() {...}
    iterator rbegin()   const { return _finish-1; }
    iterator rend()     const { return _start-1; }
    Type& front()       const { return *_start; }           // 同理，原本应该是const_reference front() const {...}
    Type& back()        const { return *(_finish-1); }      // 以及reference front() {...}
    // 
    Type& operator[](size_t i) { return *(_start+i); }
    const Type& operator[](size_t i) const { return *(_start+i); }
    // 遍历查找与item相等的元素，返回第一个相等元素的指针
    iterator find(const Type& item) {
        for (Type* ptr=_start; ptr!=_finish; ++ptr)
            if (*ptr == item) 
                return ptr;
        return _finish; // 即end()，此时ptr==_finish，然鹅ptr已释放
    }

public:     // 【增】
    // 在末端添加元素item
    void push_back(const Type& item) {
        if (_finish == _end_of_storage)     // 【容量不足，自动扩容为2倍+1（防止原本空间为0），均摊时间复杂度O(1)】
            _resize(capacity()*2+1);
        new (_finish++) Type(item);         // 【placement new】【在更高级的语言中 data[count++] = item 即可】
    }
    // 在position指针处插入n个值为value的元素
    void insert(iterator position, size_t n, const Type& value) {
        if (position<_start || position>=_finish) {         // position越界
            cerr << "warning: position(" << position << ") is out of range!" << endl; 
            return; 
        }
        while (_finish+n > _end_of_storage)                 // 容量不足，扩容
            _resize(capacity()*2+1);
        memmove(position+n, position,                       // 从后向前，将[position, _finish)依次后移n格
                sizeof(Type)*size_t(_finish-position));     // memmove(void* dst, void* src, size_t nbytes)，通过char*逐个字节拷贝可复现
        _finish += n;
        for (size_t i=0; i<n; ++i)                             // 从position开始依次以value值构造n个对象
            new (position++) Type(value);
    }
    // 在position指针处插入元素item
    void insert(iterator position, const Type& item) { insert(position, 1, item); }

public:     // 【删】
    // 弹出末端元素
    Type pop_back() {
        if (_finish <= _start) {        // 数组为空情况
            cerr << "warning: " << "Vector(at " << this << ") is empty!" << endl;
            return Type();
        }
        Type tmp(*(_finish-1));         // 暂存back()用于返回
        (--_finish)->~Type();           // 对back()析构
        if (size() < capacity()/4  &&  capacity()/2 > default_capacity)
            _resize(capacity()/2);      // 【容量少于1/4即冗余，自动缩容为1/2；注意防止操作时间复杂度振荡】
        return tmp;
    }
    // 将指针区域[first, last)元素全部删除
    void erase(iterator first, iterator last) {
        if (_finish <= _start) {                        // 数组为空情况
            cerr << "warning: " << "Vector(at " << this << ") is empty!" << endl;
            return;
        }
        if (last < first  ||  last > _finish  ||        // first/last越界
            first < _start  ||  first >= _finish) {
            cerr << "[first, last) is out of range!" << endl;
            return;
        }
        mystl_destroy(first, last);             // 对[first, last)的对象析构
        const size_t n = size_t(last - first);
        memmove(first, last, sizeof(Type)*n);   // 从前向后，将last开始后边剩余元素依次前移n格
        _finish -= n;
        while (size() < capacity()/4  &&  capacity()/2 > default_capacity)
            _resize(capacity()/2);              // 当n > capacity()/8时，会缩容两次或以上
    }
    // 将position指针处的元素删除
    void erase(iterator position) { erase(position, position+1); }

public:     // 【特殊：交换两个Vector<Type>，浅拷贝交换！】
    void swap(Vector<Type>& other) {
        if (&other == this) return;
        Vector<Type> tmp[1]; memcpy(tmp, this, sizeof(Vector<Type>));
        memcpy(this, &other, sizeof(Vector<Type>));
        memcpy(&other, tmp, sizeof(Vector<Type>));
        // Type* tmp_start=_start;     Type* tmp_finish=_finish;   Type* tmp_end=_end_of_storage;
        // _start=other._start;        _finish=other._finish;      _end_of_storage=other._end_of_storage;
        // other._start=tmp_start;     other._finish=tmp_finish;   other._end_of_storage=tmp_end;
    }
};

template <class Type>
ostream& operator<<(ostream& out, const Vector<Type>& vec) {
    out << "[ ";
    for (const Type& item : vec) out << item << " ";
    return out << "]";
}


#endif // __VECTOR__





/* // 【***** 测 试 *****】
#include <vector>
#include <ext/pool_allocator.h>     // std::allocator ==> __gnu_cxx::__pool_alloc<Type>
#include <ctime>
#include "utils.hpp"
// “增、删、改、查” 测试函数
template < typename Seq, typename Type >
void __test(Seq& seq, const Type& ele) {
    const int n = int(6e6);
    // 增
    clock_t t1 = clock();
    for (int i=0; i<n; ++i)
        seq.push_back(ele);
    clock_t t2 = clock(); 
    cout << "push: " << t2 - t1 << " ms" << "\tsize=" << seq.size() << endl;
    // 改、查
    t1 = clock();
    for (int i=0; i<n; ++i) seq[i];
    t2 = clock();
    cout << "op[]: " << t2 - t1 << " ms" << "\t[0]=" << seq.front() << endl;
    // 删
    t1 = clock();
    while (!seq.empty())
        seq.pop_back();
    t2 = clock();
    cout << "pop: " << t2 - t1 << " ms" << "\tsize=" << seq.size() << endl;
}
// 开始测试
int main(int argc, char const *argv[]) {
    // 一维数组初步测试
    {
        Vector<double> tmp(10);
        cout << tmp << endl;
    }
    // 二维数组初步测试
    {
        Vector< Vector<int> > arr2d({
            {1, 2, 3, 4}, 
            {666, 666, 666, 666}, 
            {11, 12, 13, 14}
        });
        cout << arr2d << endl;
        // for (int i=0; i<arr2d.size(); ++i) {
        //     for (int j=0; j<arr2d[0].size(); ++j) cout << arr2d[i][j] << "\t";
        //     cout << endl;
        // }
        cout << "------------------" << endl;
        arr2d.erase(&arr2d[1]);             // [[1,2,3,4], [11,12,13,14]]
        arr2d.erase(&arr2d.back());         // [[1,2,3,4]]
        cout << arr2d << endl;
        cout << "------------------" << endl;
        arr2d.insert(arr2d.begin(), {11, 12, 13, 14});      // [[11,12,13,14], [1,2,3,4]]
        cout << arr2d << endl;
        arr2d.insert(arr2d.begin(), {66, 66, 66, 66});      // [[66,66,66,66], [1,2,3,4], [1,2,3,4]]
        cout << arr2d << endl;
        arr2d.back().swap(arr2d.front());
        cout << arr2d << endl;
        cout << "------------------" << endl;
        for (int i=0; i<5; ++i) arr2d.push_back({99, 99});  // 后边加5个[99, 99]，触发_resize
        cout << arr2d << endl;
        cout << "------------------" << endl;
    }
    // 性能测试
    {
        cout << "=================================" << endl;
        {
            Vector<int> my_vec;
            vector<int> stl_vec;
            cout << "Vector<int>" << endl; 
            __test(my_vec, 666);
            cout << "------------------" << endl;
            cout << "vector<int>" << endl;
            __test(stl_vec, 666);
        }
        cout << "=================================" << endl;
        {
            Vector<Pair<int, int>> my_vec;
            vector<Pair<int, int>> stl_vec;
            cout << "Vector<Pair>" << endl; 
            __test(my_vec, Pair<int, int>(66, 66));
            cout << "------------------" << endl;
            cout << "vector<Pair>" << endl;
            __test(stl_vec, Pair<int, int>(66, 66));
        }
        cout << "=================================" << endl;
    }
    system("pause");
    return 0;
}
 */




/* // 摸索深/浅拷贝
#include <vector>
Vector<int> func() {
    Vector<int> farr;
    farr.push_back(1);
    farr.push_back(2);
    return farr;
}
int main(int argc, char const *argv[]) {
    Vector<int> tmp1({1, 2, 3, 4});
    Vector<int> tmp2 = tmp1;     // 调用Vector(const &)
    Vector<int> tmp3 = func();   // 调用Vector(const &)
    // tmp3 = func();  // 这样调用的是operator=(&&)
    tmp3 = tmp2;
    // tmp3 = Vector<int>({1, 2, 3});
    cout << "===" << endl;
    cout << tmp1.begin() << endl;
    cout << tmp1._end_of_storage << endl;
    cout << tmp1._end_of_storage - tmp1._start << endl;
    cout << tmp2.begin() << endl;
    cout << tmp2._end_of_storage << endl;
    cout << tmp2._end_of_storage - tmp2._start << endl;
    cout << tmp3.begin() << endl;
    cout << tmp3._end_of_storage << endl;
    cout << tmp3._end_of_storage - tmp3._start << endl; 
    system("pause");
    return 0;
}
 */
