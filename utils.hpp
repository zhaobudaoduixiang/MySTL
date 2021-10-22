/* utils.hpp
 * 【各种小工具函数/类】
 * STL当中<stl_function.h> <stl_hash_fun.h> <stl_algo.h> <stl_pair.h>部分内容的简化版
 */
#ifndef __UTILITIES__
#define __UTILITIES__
#include <iostream>  // ostream


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
};


using namespace std;
// [STL greater<>]
template <class Type> 
struct Greater { 
    bool operator()(const Type& a, const Type& b) const { return a > b; }
};

// [STL less<>]
template <class Type> 
struct Less {
    bool operator()(const Type& a, const Type& b) const { return a < b; }
};


// [STL pair<>]
template <class T1, class T2>
struct Pair {
    T1 first;
    T2 second;
    Pair(): first(T1()), second(T2()) {}
    Pair(const T1& _first, const T2& _second):
        first(_first), second(_second) {}
    // bool operator>(const Pair<T1, T2>& other) {
    //     if (first > other.first) return true;
    //     else if (first < other.first) return false;
    //     else {
    //         if (second > other.second) return true;
    //         else if (second < other.second) return false;
    //         else return (this > &other);
    //     }
    // }
};
template <class T1, class T2>
ostream& operator<<(ostream& out, const Pair<T1, T2>& pr_obj) 
    { return out << "(" << pr_obj.first << ", " << pr_obj.second << ")"; }


#endif // __UTILITIES__