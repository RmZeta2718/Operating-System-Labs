#include "hash.h"
#include "change_lock.h"
#include <stdlib.h>

static void clear(hash_t *hash);

void hash_init(hash_t *hash, int size) {
    hash->arr = malloc(sizeof(list_t*) * size);
    for (unsigned i = 0; i < size; ++i) {
        hash->arr[i] = malloc(sizeof(list_t));
        list_init(hash->arr[i]);
    }
    hash->size = size;
}

void hash_insert(hash_t *hash, unsigned int key) {
    list_insert(hash->arr[key % hash->size], key);
}

void hash_delete(hash_t *hash, unsigned int key) {
    list_delete(hash->arr[key % hash->size], key);
}

void *hash_lookup(hash_t *hash, unsigned int key) {
    void *ptr;
    ptr = list_lookup(hash->arr[key % hash->size], key);
    return ptr;
}

void hash_clear(hash_t *hash) {
    clear(hash);
}

int hash_change_lock(hash_t *hash, char *lock_type) {
    if (list_change_lock(hash->arr[0], lock_type) < 0)
        return -1;
    for (int i = 1; i < hash->size; ++i)
        list_change_lock(hash->arr[i], lock_type);
    return 0;
}

unsigned int hash_size(hash_t *hash) {
    unsigned int size = 0;
    for (unsigned i = 0; i < hash->size; ++i)
        size += list_size(hash->arr[i]);
    return size;
}

static void clear(hash_t *hash) {
    for (unsigned i = 0; i < hash->size; ++i)
        list_clear(hash->arr[i]);
    free(hash->arr);
}
