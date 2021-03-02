#ifndef __LIST_H__
#define __LIST_h__

#include "list_node.h"

typedef void (*lock_func_t)(void*);

typedef struct __list_t {
    node_t *head;
    void *lock;
    lock_func_t acquire;
    lock_func_t release;
} list_t;

void list_init(list_t *list);
void list_insert(list_t *list, unsigned int key);
void list_delete(list_t *list, unsigned int key);
void *list_lookup(list_t *list, unsigned int key);

void list_clear(list_t *list);
unsigned int list_size(list_t *list);
int list_change_lock(list_t *list, char* lock_type);

#endif