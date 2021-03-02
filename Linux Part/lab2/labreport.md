# Project 2a: Malloc and Free

10185102223 汪杰



### 数据结构定义

定义了两个结构体，用于管理空闲块链表（node_t）和已分配内存（header_t）

需要注意的是，为了后续实现方便，这里的两个结构体需要有相同的大小。指针大小总和编译器位数一致，但是整形的大小是随平台变化的：

|   类型    | Linux: 32-bit/64-bit(byte) | Windows: 32-bit/64-bit(byte) |
| :-------: | :------------------------: | :--------------------------: |
|    int    |             4              |              4               |
|   long    |            4/8             |              4               |
| long long |             8              |              8               |
|  pointer  |            4/8             |             4/8              |

Linux用LP64规则（long, pointers are 64-bit）

Windows用LLP64规则（long long, pointers are 64-bit），暗示long是32-bit

如果是在Windows平台上，就没有和指针大小始终相同的基本类型。好在，这是在Linux平台上的项目，用long就可以了。

为了适应更多的不确定性，我在网上了解到 `uintptr_t` 是足够放下指针的整型。这里采用了这个类型。（定义在 `#include <inttypes.h>` 中）



### mem_alloc()

该函数需要支持不同的算法。本实验中，为每个算法定义了一个函数，封装算法，提供统一接口，使代码逻辑更清晰。

```c
switch (style) {
case M_BESTFIT:
    find_best_fit(size, &pre, &cur);
    break;
case M_WORSTFIT:
    find_worst_fit(size, &pre, &cur);
    break;
case M_FIRSTFIT:
    find_first_fit(size, &pre, &cur);
    break;
}
```

由于后面需要用到指向该空闲块和该块前一个位置的指针。不能通过返回值返回两个指针，所以通过传入二级指针来返回结果。

无论通过什么算法找到空闲块，之后的处理都是一样的：

根据剩余空间的大小：

- 若不够建立一个新的节点（即剩余大小不超过空闲块链表），则把剩余空间一并分配出去。（虽然用户不知道）
  - 若不一并分配，在回收时就可能找不到这段空间，内存泄漏
- 若能够新建一个节点，那么相应地维护链表

最后，在原地把。并返回。



### mem_free()

- 先判断MAGIC是否相等。
- 通过比较指针大小，找到当前块在空闲块链表中的位置。
- 把已分配内存头（header_t）转换成空闲块链表头（node_t）
- 根据左右两边的邻接情况，考虑合并。



### 坑

指针可以相减，但是减之前需要转换成 **void\*** ，否则会得到与预期不同的结果。