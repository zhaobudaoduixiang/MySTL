/* utils.hpp
 * STL当中<stl_function.h> <stl_hash_fun.h> <stl_algo.h> <stl_pair.h>部分内容的简化版
 */
#ifndef __UTILITIES__
#define __UTILITIES__
#include <iostream>  // ostream
using namespace std;


// [STL swap]
template <class Type>
inline void mystl_swap(const Type& a, const Type& b) { Type tmp=a; a=b; b=tmp; }
// same_type
template <class T1, class T2>
inline bool same_type(const T1& a, const T2& b) { return false; }
template <class Type>
inline bool same_type(const Type& a, const Type& b) { return true; }


// functors
template <class Type> 
struct Greater { 
    bool operator()(const Type& a, const Type& b) const { return a > b; }
};
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