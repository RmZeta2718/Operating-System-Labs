#include "hash_full_lock.h"
#include "change_lock.h"
#include <stdlib.h>

static void clear(hash_t *hash);

void hash_init(hash_t *hash, int size) {
    hash->arr = malloc(sizeof(node_t*) * size);
    for (unsigned i = 0; i < size; ++i)
        hash->arr[i] = NULL;
    hash->size = size;
    hash->lock = NULL;
    hash_change_lock(hash, "spinlock");
}

void hash_insert(hash_t *hash, unsigned int key) {
    hash->acquire(hash->lock);
    node_insert(&hash->arr[key % hash->size], key);
    hash->release(hash->lock);
}

void hash_delete(hash_t *hash, unsigned int key) {
    hash->acquire(hash->lock);
    node_delete(&hash->arr[key % hash->size], key);
    hash->release(hash->lock);
}

void *hash_lookup(hash_t *hash, unsigned int key) {
    void *ptr;
    hash->acquire(hash->lock);
    ptr = node_lookup(hash->arr[key % hash->size], key);
    hash->release(hash->lock);
    return ptr;
}

void hash_clear(hash_t *hash) {
    hash->acquire(hash->lock);
    clear(hash);
    hash->release(hash->lock);
}

unsigned int hash_size(hash_t *hash) {
    unsigned int size = 0;
    hash->acquire(hash->lock);
    for (unsigned i = 0; i < hash->size; ++i)
        size += node_size(hash->arr[i]);
    hash->release(hash->lock);
    return size;
}

int hash_change_lock(hash_t *hash, char *lock_type) {
    return change_lock(lock_type, &hash->lock, &hash->acquire, &hash->release);
}

static void clear(hash_t *hash) {
    for (unsigned i = 0; i < hash->size; ++i)
        node_clear(hash->arr[i]);
    free(hash->arr);
}
