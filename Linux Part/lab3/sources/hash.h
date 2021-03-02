#ifndef __HASH_H__
#define __HASH_H__

#include "list.h"

typedef struct __hash_t {
    list_t **arr;
    unsigned size;
    // no lock in hash
} hash_t;

void hash_init(hash_t *hash, int size);
void hash_insert(hash_t *hash, unsigned int key);
void hash_delete(hash_t *hash, unsigned int key);
void *hash_lookup(hash_t *hash, unsigned int key);

void hash_clear(hash_t *hash);
unsigned int hash_size(hash_t *hash);
int hash_change_lock(hash_t *hash, char* lock_type);

#endif