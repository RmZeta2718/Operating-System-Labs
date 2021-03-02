#ifndef __HASH_FULL_LOCK_H__
#define __HASH_FULL_LOCK_H__

#include "list_node.h"

typedef void (*lock_func_t)(void*);

typedef struct __hash_t {
    node_t **arr;
    unsigned size;
    void *lock;
    lock_func_t acquire;
    lock_func_t release;
} hash_t;

void hash_init(hash_t *hash, int size);
void hash_insert(hash_t *hash, unsigned int key);
void hash_delete(hash_t *hash, unsigned int key);
void *hash_lookup(hash_t *hash, unsigned int key);

void hash_clear(hash_t *hash);
unsigned int hash_size(hash_t *hash);
int hash_change_lock(hash_t *hash, char* lock_type);

#endif