/* traits.hpp
 * 【迭代器、各类的 “特性萃取器” 】
 * SGI STL当中<type_traits.h>的iterator_traits<>和__type_traits<>部分内容的简化版
 */
#ifndef __TRAITS__
#define __TRAITS__
using namespace std;


// """迭代器特性"""
// 迭代器类型标记（类）
struct InputIteratorTag {};                                 // input_iterator_tag，任意可变迭代器
struct OutputIteratorTag {};                                // ouput_iterator_tag，任意不可变迭代器
struct ForwardIteratorTag: public InputIteratorTag {};      // forward_iterator_tag，前向迭代器
struct BidirectIteratorTag: public ForwardIteratorTag {};   // bidirectional_iterator_tag，双向迭代器
struct RandomIteratorTag: public BidirectIteratorTag {};    // random_access_iterator_tag，随机访问迭代器
// 迭代器基类（自定义的迭代器应继承自这个类）
template <class Category,
          class Type,
          class Pointer,
          class Reference,
          class Distance>
struct Iterator {  // iterator<...>
    typedef Category    iterator_category;
    typedef Type        value_type;
    typedef Pointer     pointer;
    typedef Reference   reference;
    typedef Distance    difference_type;
};
// 一般迭代器类型【为什么要搁这搁这？因为要考虑到天然的指针也可以是迭代器呀！】
template <class Ite>
struct IteratorTraits {     // iterator_traits<>
    typedef typename Ite::iterator_category iterator_category;
    typedef typename Ite::value_type        value_type;
    typedef typename Ite::pointer           pointer;
    typedef typename Ite::reference         reference;
    typedef typename Ite::difference_type   difference_type;
};
// 泛型指针类型（偏特化）【<const Type*>可实例化时自行调整！！！】
template <class Type>
struct IteratorTraits<Type*> {
    typedef RandomIteratorTag   iterator_category;
    typedef Type                value_type;
    typedef Type*               pointer;
    typedef Type&               reference;
    typedef ptrdiff_t           difference_type;
};


// """类型特性"""
struct TpTrue {};       // __true_type
struct TpFalse {};      // __false_type
// 一般类型
template <class Type>
struct TypeTraits {     // __type_traits<...>
    typedef TpTrue  this_dummy_member_must_be_first;    // ???
    typedef TpFalse has_trivial_default_constructor;    // Type()
    typedef TpFalse has_trivial_copy_constructor;       // Type(const Type&)
    typedef TpFalse has_trivial_assignment_operator;    // operator=()
    typedef TpFalse has_trivail_destructor;             // ~Type()
    typedef TpFalse is_POD_type;                        // POD, plain old data
};
// 泛化指针类型（偏特化）【<const Type*>可实例化时自行调整！！！】
template <class Type>
struct TypeTraits<Type*> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
// 特化内置类型（偏特化）【<const xx>可实例化时自行调整！！！】
template<> struct TypeTraits<char> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<signed char> {  // signed/unsigned char表示单个字节的整型，而不是字符
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<unsigned char> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<short> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<unsigned short> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<int> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<unsigned int> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<long> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<unsigned long> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<long long> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<unsigned long long> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<float> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<double> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};
template<> struct TypeTraits<long double> {
    typedef TpTrue has_trivial_default_constructor;
    typedef TpTrue has_trivial_copy_constructor;
    typedef TpTrue has_trivial_assignment_operator;
    typedef TpTrue has_trivail_destructor;
    typedef TpTrue is_POD_type;
};


#endif // __TRAITS__