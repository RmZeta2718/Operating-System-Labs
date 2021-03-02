#include "mem.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// a fancy way to debug
// #define DEBUG
#ifdef DEBUG
#define here printf("Passing [%s] in Line %d\n", __FUNCTION__, __LINE__);
#else
#define here ;
#endif


// data structure definition
typedef struct __header_t {
    // manage allocated mem
    size_t size;
    uintptr_t magic;
} header_t;
typedef struct __node_t {
    // manage free blocks
    size_t size;      // exclude head size
    struct __node_t *next;
} node_t;
/*
header_t and node_t should be aligned
Therefore header_t.magic has to be uintptr_t
(A integer type defined in <inttypes.h> that is large enough to hold pointers)

Note that, pointer size is 4bytes in 32-bit compiler, 8bytes in 64-bit compiler.
As a result, sizeof(node_t) == sizeof(header_t) == 64-bit ? 16 : 8
See more details in labreport.md
*/


// local functions for implementation convenience
void printErr(const char message[]);
void find_best_fit(int size, node_t **pre_ans, node_t **cur_ans);
void find_worst_fit(int size, node_t **pre_ans, node_t **cur_ans);
void find_first_fit(int size, node_t **pre_ans, node_t **cur_ans);


// global variables and macros
#define MAGIC_NUMBER 12345678
#define MEM_ALIGN 8
int m_error = 0;
int init_flag = 0;
node_t *free_head;


// implement main functionalities
int mem_init(int size_of_region) {
    m_error = 0;
    if (sizeof(node_t) != sizeof(header_t)) {
        printErr("node_t and header_t should have the same size.");
        return -1;
    }
    if (init_flag) {
        printErr("mem_init() should be called once and only once.");
        m_error = E_BAD_ARGS;
        return -1;
    }
    if (size_of_region <= 0) {
        printErr("size of region <= 0");
        m_error = E_BAD_ARGS;
        return -1;
    }

    // round up size of region to page size
    int size_of_page = getpagesize();
    if (size_of_region % size_of_page)
        size_of_region += size_of_page - size_of_region % size_of_page;
    
    // open the /dev/zero device
    int fd = open("/dev/zero", O_RDWR);
    // size_of_region (in bytes) needs to be evenly divisible by the page size
    void *ptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE, fd, 0);
    // close the device (don't worry, mapping should be unaffected)
    close(fd);

    if (ptr == MAP_FAILED) {
        m_error = E_BAD_ARGS;
        perror("mmap");
        return -1;
    }

    // init free block list head
    free_head = ptr;
    free_head->size = size_of_region - sizeof(node_t);
    free_head->next = NULL;
    init_flag = 1;
    return 0;
}

/*
 memory before and after allocation:
 |--node_t--|-free/alloc-|--node_t--|-------------free space------------|-|--node_t--|
 |--node_t--|-free/alloc-|-header_t-|-allocated-|--node_t--|-free space-|-|--node_t--|
 ^pre                    ^cur                   ^new                      ^nxt
*/
void* mem_alloc(int size, int style) {
    m_error = 0;
    if (!init_flag) {
        printErr("Please call mem_init() before mem_alloc().");
        return NULL;
    }
    // align up to 8 bytes
    if (size % MEM_ALIGN)
        size += MEM_ALIGN - size % MEM_ALIGN;
    node_t *pre, *cur, *new, *nxt;
    switch (style) {
    case M_BESTFIT:
        here
        find_best_fit(size, &pre, &cur);
        break;
    case M_WORSTFIT:
        here
        find_worst_fit(size, &pre, &cur);
        break;
    case M_FIRSTFIT:
        here
        find_first_fit(size, &pre, &cur);
        break;
    default:
        printErr("Invalid style in mem_alloc().");
        return NULL;
    }
    if (!cur) {
        m_error = E_NO_SPACE;
        printErr("No enough space found.");
        return NULL;
    }
    // enough free space found, maintain linked list:
    nxt = cur->next;        // store *nxt
    if (cur->size - size <= sizeof(node_t)) {
        // new block has size 0
        here
        size = cur->size;   // alloc the whole current free space
        if (pre)
            pre->next = nxt;
        else
            free_head = nxt;
    } else {
        // enough space for a new block
        // build *new
        here
        new = (void*)cur + sizeof(node_t) + size;
        new->next = nxt;
        new->size = cur->size - size - sizeof(node_t);
        if (pre)
            pre->next = new;
        else
            free_head = new;
    }
    // (node_t*)cur ==> (header_t*)alloc_head
    header_t *alloc_head = (header_t*)cur;
    alloc_head->size = size;
    alloc_head->magic = MAGIC_NUMBER;
    return alloc_head + 1;  // or (void*)alloc_head + sizeof(header_t);
}

/*
 memory before and one case after free:
 |--node_t--|--free--|-[alloc]-|-header_t-|--alloc--|-[alloc]-|--node_t--|--free--|
 |--node_t--|--free--|-[alloc]-|--node_t--|--------------free space---------------|
 ^pre                          ^cur                           ^nxt
*/
int mem_free(void* ptr) {
    m_error = 0;
    if (!init_flag) {
        printErr("Please call mem_init() before mem_free().");
        return -1;
    }
    if (!ptr) return 0;
    header_t *head = (void*)ptr - sizeof(header_t);
    // header_t *head = ptr - sizeof(header_t);
    if (head->magic != MAGIC_NUMBER) {
        m_error = E_BAD_POINTER;
        printErr("Pointer passed to mem_free() is not obtained from mem_alloc().");
        return -1;
    }
    node_t *pre, *cur, *nxt;
    cur = (node_t*)head;
    if (!free_head) {
        // all memory is allocated
        free_head = cur;
        free_head->next = NULL;
        // no need to reset size
        here
        return 0;
    }
    if (cur < free_head) {
        // cur before free list head:
        pre = NULL;
        nxt = free_head;
        free_head = cur;
    } else {
        // find pre < cur < nxt
        pre = free_head, nxt = free_head->next;
        for (; nxt && nxt < cur; pre = pre->next, nxt = nxt->next);
    }
    here
    // add cur to linked list
    if (pre) pre->next = cur;
    cur->next = nxt;
    // Coalesce
    if (pre) { // pre != NULL
        // Coalescing pre/cur
        if ((void*)cur - (void*)pre == sizeof(node_t) + pre->size) {
            pre->size += sizeof(node_t) + cur->size;
            pre->next = nxt;
            cur = pre;      // "delete" pre, move cur to pre
        }
    } // else: cur is free list head
    if (nxt) {
        here
        // Coalescing cur/nxt
        if ((void*)nxt - (void*)cur == sizeof(node_t) + cur->size) {
            cur->size += sizeof(node_t) + nxt->size;
            cur->next = nxt->next;
        }
    } // else: cur is free list tail
    return 0;
}

void mem_dump(void) {
    puts("Memory dump:");
    for (node_t* ptr = free_head; ptr != NULL; ptr = ptr->next)
        printf("%p: %9lu\n", ptr, ptr->size);
}


// implement local functions
void printErr(const char message[]) {
    // a simple interface for printing error message with an trailing newline
    static const char err[] = "Error: ";
    write(STDERR_FILENO, err, strlen(err));
    write(STDERR_FILENO, message, strlen(message));
    write(STDERR_FILENO, "\n", 1);
}

void find_best_fit(int size, node_t **pre_ans, node_t **cur_ans) {
    // find the smallest block where blocksize >= size
    *pre_ans = *cur_ans = NULL;
    if (!free_head) {
        return;
    }
    // handle list head
    *cur_ans = free_head->size >= size ? free_head : NULL;
    node_t *pre = free_head, *cur = free_head->next;
    for (; cur; pre = pre->next, cur = cur->next) {
        if (cur->size >= size) {
            if (!(*cur_ans) || cur->size < (*cur_ans)->size) {
                *pre_ans = pre;
                *cur_ans = cur;
            }
        }
    }
}
void find_worst_fit(int size, node_t **pre_ans, node_t **cur_ans) {
    // find the biggest block where blocksize >= size
    *pre_ans = *cur_ans = NULL;
    if (!free_head) {
        return;
    }
    // handle list head
    *cur_ans = free_head->size >= size ? free_head : NULL;
    node_t *pre = free_head, *cur = free_head->next;
    for (; cur; pre = pre->next, cur = cur->next) {
        if (cur->size >= size) {
            if (!(*cur_ans) || cur->size > (*cur_ans)->size) {
                *pre_ans = pre;
                *cur_ans = cur;
            }
        }
    }

}
void find_first_fit(int size, node_t **pre_ans, node_t **cur_ans) {
    // find the first block where blocksize >= size
    *pre_ans = *cur_ans = NULL;
    if (!free_head) {
        return;
    }
    if (free_head->size >= size) {
        *cur_ans = free_head;
        return;
    }
    node_t *pre = free_head, *cur = free_head->next;
    for (; cur; pre = pre->next, cur = cur->next) {
        if (cur->size >= size) {
            *pre_ans = pre;
            *cur_ans = cur;
            return;
        }
    }
    // reach here if no appropriate block is found.
    // ans are NULL by default.
}
