#ifndef __SORT__
#define __SORT__
#include "utils.hpp"


// """基本的排序算法"""
namespace mystl {
    // 冒泡排序：对[left, right]区间进行
    template <class Type>
    void bubble_sort(Type* left, Type* right) {
        for (; right>=left; --right)            // [left, right]无序
            for (Type* cur=left; cur<right; ++cur) 
                if (*cur > *(cur+1)) iter_swap(cur, cur+1);
    }

    // 选择排序：对[left, right]区间进行
    template <class Type>
    void selection_sort(Type* left, Type* right) {
        for (; left<=right; ++left) {           // [原left, left)有序，[left, right]无序
            Type* cur_min = left;               // [left, right]最小元素的指针
            for (Type* cur=left; cur<=right; ++cur)
                if (*cur < *cur_min) cur_min=cur;
            iter_swap(left, cur_min);
        }
    }

    // 插入排序：对[left, right]区间进行
    template <class Type>
    void insertion_sort(Type* left, Type* right) {
        for (Type* cur=left; cur<=right; ++cur) {
            Type tmp=*cur, *prev=cur-1;         // [left, cur)已是有序的
            while (prev>=left && *prev>tmp)     // 从后往前，将大于tmp的各数逐步后移
                { *(prev+1)=*prev; --prev; }
            *(prev+1) = tmp;
        }
    }

    // 希尔排序：对[left, right]区间进行
    template <class Type>
    void shell_sort(Type* left, Type* right) {
    // 【采用】Knuth: 1, 4, 13, 40, 121, 364, 1093... (next(gap) = 3 * gap + 1)
    // Hibbard: 1, 3, 7, 15... (next(gap) = 2 * gap -1)
    // Sedgewick: 1, 5, 19, 41, 109... (next(gap) = 9*4^gap - 9*2^gap + 1  //  k = 4^gap - 3*2^gap + 1)
        size_t gap = 1;
        while (gap < (right-left+1)/3) gap=3*gap+1;
        // 预处理：通过以gap为跨度进行分组，分别进行插入排序，以避免最终插入排序的长距离插入
        // 每组：array[i, i+gap, i+2*gap...]；每组的插入排序是“交替”进行的
        while (gap > 1) {
            for (Type* cur=left; cur<=right; ++cur) {
                Type tmp=*cur, *prev=cur-gap;
                while (prev>=left && *prev>tmp)
                    { *(prev+gap)=*prev; prev-=gap; }
                *(prev+gap) = tmp;
            }
            gap /= 3;
        }
        // 最终插入排序：【如果上边是 while(gap>=1){...} 就可以不要这个】
        insertion_sort(left, right);
    }
};


// """快速排序"""
namespace mystl {
    // [left, right]区间的头、中、尾三元素的“中位数”(指针返回)
    template <class Type>
    inline Type* __median(Type* left, Type* right) {
        Type* mid = left + (right-left)/2;
        if (*left < *mid) {							
            if (*mid < *right) return mid;          // *left < *mid < *right                            ==>  left, mid, right  ==>  mid
            else if (*left < *right) return right;  // *left < *mid, *mid >= *right, *left < *right     ==>  left, right, mid  ==>  right
            else return left;                       // *left < *mid, *mid >= *right, *left >= *right    ==>  right, left, mid  ==>  left
        }
        else if (*mid > *right) return mid;         // *left >= *mid > *right                           ==>  right, mid, left  ==>  mid
        else if (*left < *right) return left;       // *left >= *mid, *mid <= *right, *left < *right    ==>  mid, left, right  ==>  left
        else return right;                          // *left >= *mid, *mid <= *right, *left <= *right   ==>  mid, right, left  ==>  right
    }

    // 快速排序：对[left, right]区间进行
    template <class Type>
    void quick_sort(Type* left, Type* right) {
        if (right - left < 17)                      // 区间长度<=16，调用插入排序
            return insertion_sort(left, right);     // 递归到底：if (left >= right) return;
        // "begin partition"
        iter_swap(left, __median(left, right));     // *left即pivot
        Type *l=left+1, *r=right;
        while (1) {
            while (*l < *left  &&  l <= r) ++l;     // l<=r和l>r，确保l和r最后停止时错开（不重叠）——
            while (*r > *left  &&  l <= r) --r;     // r停在“最后一个比pivot小的数”，而l停在“第一个比pivot大的数”
            if (l > r) break;
            iter_swap(l++, r--);
        }
        iter_swap(r, left);
        // "end partition"
        quick_sort(left, r-1);
        quick_sort(r+1, right);
    }

    // 三路快排：对[left, right]区间进行，适用于有大量重复元素的序列
    template <class Type>
    void quick_sort_3ways(Type* left, Type* right) {
        if (right - left < 17)
            return insertion_sort(left, right);
        // "begin partition"
        iter_swap(left, __median(left, right));
        Type *lt=left, *gt=right, *cur=left+1;
        // TODO: .......
        // "end partition"
        // quick_sort_3ways(left, lt-1);
        // quick_sort_3ways(gt, right);
    }
};


// """合并排序"""
namespace mystl {
    // 合并有序的[left, mid]和[mid+1, right]【参考leetcode88】
    template <class Type>
    void __merge(Type* left, Type* mid, Type* right, Type* aux) {
        memcpy(aux, left, (mid-left+1)*sizeof(Type));
        Type *l=aux, *r=mid+1;                          // 注意：左游标l在aux上！
        mid = aux + (mid-left);                         // mid用作左游标l的右边界
        while (l<=mid && r<=right) 
            *left++ = (*r < *l) ? *r++ : *l++;          // left用作结果游标
        if (l <= mid) 
            memcpy(left, l, (mid-l+1)*sizeof(Type));    // 拷贝剩余部分【l未到mid要拷贝到left，r未到right则不用管】
        // while (l<=mid)   *left++=*l++;
        // while (r<=right) *left++=*r++;
    }

    // 合并排序（递归函数）：...
    template <class Type>
    void __merge_sort(Type* left, Type* right, Type* aux) {
        if (right - left < 17) 
            return insertion_sort(left, right);         // 递归到底：if (left >= right) return;
        Type* mid = left + (right-left)/2;
        __merge_sort(left, mid, aux);
        __merge_sort(mid+1, right, aux);
        if (*mid > *(mid+1)) __merge(left, mid, right, aux);
    }

    // 合并排序（主调函数）：对[left, right]区间进行
    template <class Type>
    void merge_sort(Type* left, Type* right) {
        Type* auxiliary = (Type*)malloc( ((right-left+1)/2+1) * sizeof(Type) );
        __merge_sort(left, right, auxiliary);
        free(auxiliary);
    }

    // 合并排序（自底向上bottom up）：对[left, right]区间进行
    template <class Type>
    void merge_sort_bu(Type* left, Type* right) {
        Type* auxiliary = (Type*)malloc((right-left+1) * sizeof(Type));
        for (size_t sz=1; sz<right-left+1; sz*=2)       // sz是每个待合并部分的长度【严格sz<n，若等于则可以结束了】
            for (Type* l=left; l+sz<=right; l+=2*sz) {
                Type* mid = l + sz - 1;                 // 左部分[l, mid]，长度为sz
                Type* r   = min(l+(2*sz)-1, right);     // 右部分[mid+1, r]，长度可能不足sz
                if (*mid > *(mid+1)) 
                    __merge(l, mid, r, auxiliary);
                // 【末尾长度不足sz的那些残余部分会在最上层，即最后得到合并】
            }
        free(auxiliary);
    }
};


// """堆排序"""
namespace mystl {
    // 将heap_start[idx]下沉，维护heap_start开始的最大堆
    template <class Type>
    void __shift_down(Type* heap_start, size_t heap_size, size_t idx) {
        Type tmp = heap_start[idx];
        size_t child_idx = idx * 2 + 1;
        while (child_idx < heap_size) {
            // 选出左右孩子中优先级较高的一个（的索引）
            if (child_idx+1 < heap_size  &&  
                heap_start[child_idx+1] > heap_start[child_idx]) ++child_idx;
            // 优先级较高的孩子比tmp更优先？
            if (heap_start[child_idx] > tmp) {
                heap_start[idx] = heap_start[child_idx];
                idx = child_idx;
                child_idx = idx * 2 + 1;
            }
            else { break; }
        }
        heap_start[idx] = tmp;
    }

    // 堆排序：对[left, right]区间进行
    template <class Type>
    void heap_sort(Type* left, Type* right) {
        // heapify成最大堆
        size_t heap_size = right - left + 1;
        for (ptrdiff_t i=((right-left)-1)/2; i>=0; --i)     // ((right-left)-1)/2是最后一个非叶子节点
            __shift_down(left, heap_size, i);
        // 逐步弹出元素，将元素从后向前存储于数组尾部
        while (heap_size > 0) {
            mystl::iter_swap(left, left+(--heap_size));
            __shift_down(left, heap_size, 0);
        }
    }
};


// """sort函数及其泛化"""
// 一般情况：快速排序
// 递归层数>2logn：堆排序
namespace mystl {
    // sort函数：对[first, last)进行
    template <class BidirectIterator>
    void sort(BidirectIterator first, BidirectIterator last);

    // 泛化的插入排序：对[left, right]区间进行
    template <class BidirectIterator, class ItemCompare>
    void insertion_sort(BidirectIterator left, BidirectIterator right, ItemCompare comp);

    // 泛化的快速排序：对[left, right]区间进行
    template <class ForwardIterator, class ItemCompare>
    ForwardIterator __median(ForwardIterator left, ForwardIterator right, ItemCompare comp);
    template <class BidirectIterator, class ItemCompare>
    void quick_sort(BidirectIterator left, BidirectIterator right, ItemCompare comp);

    // 泛化的堆排序：对[left, right]区间进行
    template <class BidirectIterator, class ItemCompare>
    void __shift_down(BidirectIterator heap_start, size_t heap_size, size_t idx, ItemCompare comp);
    template <class BidirectIterator, class ItemCompare>
    void heap_sort(BidirectIterator left, BidirectIterator right, ItemCompare comp);

    // 泛化的sort函数：对[first, last)进行
    template <class BidirectIterator, class ItemCompare>
    void sort(BidirectIterator first, BidirectIterator last, ItemCompare comp);
};


// """arg sort"""
// 思路：
// 用哪种排序算法来实现都可以
// 比较的时候用array[indexes[i]]和array[indexes[j]]比，而移动的时候是像indexes[i] = indexes[j]这样即可


#endif // __SORT__