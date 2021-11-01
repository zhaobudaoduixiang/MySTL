/* utils.hpp
 * 【各种小工具函数/类】
 * STL当中<stl_function.h> <stl_hash_fun.h> <stl_algo.h> <stl_pair.h>部分内容的简化版
 */
#ifndef __UTILITIES__
#define __UTILITIES__
#include <iostream> // ostream
#include <cstring>  // strcmp
using namespace std;


// """HashCode —— 输入对象，返回size_t"""
// 【无论什么编译器，size_t都足以覆盖全部指针地址】
// 【注意：未偏特化的HashCode什么也不做！！！】
template <class Type> 
struct HashCode {};
// HashCode<字符串> —— 原本还应加上<const char*>的偏特化，不过这个可实例时自行调整模板
template<> struct HashCode<char*> {
    size_t operator()(const char* key) const { 
        size_t hash_code = 0;
        for (; *key; ++key)  // 把字符串看作一个31进制的数
            hash_code = hash_code * 31 + *key;
        return hash_code;
    }
};
template<> struct HashCode<string> {
    size_t operator()(const string& key) const { 
        size_t hash_code = 0;
        for (const auto& tmp : key)
            hash_code = hash_code * 31 + tmp;
        return hash_code;
    }
};
// HashCode<小于等于size_t的整型> —— 返回自己即可
template<> struct HashCode<char> {
    size_t operator()(char key) const { return (size_t)key; }
};
template<> struct HashCode<signed char> {
    size_t operator()(signed char key) const { return (size_t)key; }  
};
template<> struct HashCode<unsigned char> {
    size_t operator()(unsigned char key) const { return (size_t)key; }  
};
template<> struct HashCode<short> {
    size_t operator()(short key) const { return (size_t)key; }
};
template<> struct HashCode<unsigned short> {
    size_t operator()(unsigned short key) const { return (size_t)key; }
};
template<> struct HashCode<int> {  // java中int固定32位，其哈希函数整体逻辑为(int_obj & 0x7fffffff) ^ ((unsigned)int_obj >> 16)
    size_t operator()(int key) const { return (size_t)key; }
};
template<> struct HashCode<unsigned> {
    size_t operator()(unsigned key) const { return (size_t)key; }
};
template<> struct HashCode<long> {
    size_t operator()(long key) const { return (size_t)key; }
};
template<> struct HashCode<unsigned long> {
    size_t operator()(unsigned long key) const { return (size_t)key; }
};
// HashCode<大于等于size_t的整型>
template<> struct HashCode<long long> {
    size_t operator()(long long key)         // 后32位 ^ 前32位
        const { return (size_t)(key ^ ((unsigned long long)key>>32)); }
};
template<> struct HashCode<unsigned long long> {
    size_t operator()(unsigned long long key)// 后32位 ^ 前32位
        const { return (size_t)(key ^ (key>>32)); }
};
// HashCode<浮点型> —— 读作整数即可
template<> struct HashCode<float> {
    size_t operator()(float key)  // 无论编译器，uint32_t始终与float位数相同
        const { return (size_t)(*((uint32_t*)&key)); }
};
template<> struct HashCode<double> {
    size_t operator()(double key) // 无论编译器，uint64_t始终与double位数相同
        const { return (*((uint64_t*)&key)) ^ (*((uint64_t*)&key)>>32); }
};
// template<> struct HashCode<long double> {};  // TODO: 有待填坑..........


// """Compare —— a==b返回0，a>b返回int>0，a<b返回int<0【参考java的obj.compare()】"""
// Compare<未偏特化泛型> —— 判断两次，效率较差
template <class Type>
struct Compare {
    int operator()(const Type& a, const Type& b) 
        const { return a==b ? 0 : (a>b ? 1 : -1); }
};
// Compare<字符串> —— 使用strcmp
template<> struct Compare<char*> {
    int operator()(const char* a, const char* b) const { return strcmp(a, b); }
};
template<> struct Compare<string> {
    int operator()(const string& a, const string& b) const { return strcmp(a.c_str(), b.c_str()); }
};
// Compare<小于等于int的整型> —— a-b即可
template<> struct Compare<char> {
    int operator()(char a, char b) const { return (int)(a-b); }
};
template<> struct Compare<signed char> {
    int operator()(signed char a, signed char b) const { return (int)(a-b); }
};
template<> struct Compare<unsigned char> {
    int operator()(unsigned char a, unsigned char b) const { return (int)(a-b); }
};
template<> struct Compare<short> {
    int operator()(short a, short b) const { return (int)(a-b); }
};
template<> struct Compare<unsigned short> {
    int operator()(unsigned short a, unsigned short b) const { return (int)(a-b); }
};
template<> struct Compare<int> {
    int operator()(short a, short b) const { return (int)(a-b); }
};
template<> struct Compare<unsigned int> {
    int operator()(unsigned int a, unsigned int b) const { return (int)(a-b); }
};
// Compare<大于int的整型> —— TODO: ..........
// Compare<浮点型> —— TODO: ..........


// [STL greater<>]
template <class Type> 
struct Greater { 
    bool operator()(const Type& a, const Type& b) const { return a > b; }
};
template<> struct Greater<char*> {
    bool operator()(const char* a, const char* b) const { return strcmp(a, b) > 0; }
};
// [STL less<>]
template <class Type> 
struct Less {
    bool operator()(const Type& a, const Type& b) const { return a < b; }
};
template<> struct Less<char*> {
    bool operator()(const char* a, const char* b) const { return strcmp(a, b) < 0; }
};
// [STL greater_equal<>, less_equal<>, equal<>...]


// [STL pair<>]
template <class T1, class T2>
struct Pair {
    T1 first;
    T2 second;
    Pair(): first(T1()), second(T2()) {}
    Pair(const T1& _first, const T2& _second):
        first(_first), second(_second) {}
    bool operator==(const Pair<T1, T2>& other) const { 
        return first==other.first && second==other.second; 
    }
    bool operator!=(const Pair<T1, T2>& other) const { 
        return first!=other.first || second!=other.second; 
    }
    bool operator>(const Pair<T1, T2>& other) const {
        if (first > other.first) return true;
        else if (first < other.first) return false;
        else {
            if (second > other.second) return true;
            else return false;
        }
    }
    bool operator<(const Pair<T1, T2>& other) const {
        if (first < other.first) return true;
        else if (first > other.first) return false;
        else {
            if (second < other.second) return true;
            else return false;
        }
    }
};
template<class T1, class T2>
struct Compare<Pair<T1, T2>> {
    int operator()(const Pair<T1, T2>& a, const Pair<T1, T2>& b) const {
        if (a.first==b.first && a.second==b.second)
            return 0;
        else {
            if (a.first > b.first) return 1;
            else if (a.first < b.first) return -1;
            else if (a.second > b.second) return 1;
            else return -1;
        }
    }
};
template <class T1, class T2>
ostream& operator<<(ostream& out, const Pair<T1, T2>& pr_obj) 
    { return out << "(" << pr_obj.first << ", " << pr_obj.second << ")"; }


// 重名，以mystl命名空间加以区分
namespace mystl {
    // [STL swap()]
    template <class Type>
    inline void swap(const Type& a, const Type& b) { Type tmp=a; a=b; b=tmp; }

    // [STL max()]
    template <class Type>
    inline const Type& max(const Type& a, const Type& b) { return b>a ? b : a; }
    template <class Type, class Compare>
    inline const Type& max(const Type& a, const Type& b, Compare comp) { return comp(b, a) ? b : a; }

    // [STL min()]
    template <class Type>
    inline const Type& min(const Type& a, const Type& b) { return b<a ? b : a; }
    template <class Type, class Compare>
    inline const Type& min(const Type& a, const Type& b, Compare comp) { return comp(b, a) ? b : a; }

    // []
    template <class T1, class T2>
    inline bool same_type(const T1& a, const T2& b) { return false; }
    template <class Type>
    inline bool same_type(const Type& a, const Type& b) { return true; }

    // []
    int power(int base, int exponent);
};


#endif // __UTILITIES__