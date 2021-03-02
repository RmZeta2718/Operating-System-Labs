#include "list.h"
#include "change_lock.h"
#include <malloc.h>

void list_init(list_t *list) {
    list->head = NULL;
    list->lock = NULL;
    list_change_lock(list, "pthread_mutex");
}

void list_insert(list_t *list, unsigned int key) {
    list->acquire(list->lock);
    node_insert(&list->head, key);
    list->release(list->lock);
}

void list_delete(list_t *list, unsigned int key) {
    list->acquire(list->lock);
    node_delete(&list->head, key);
    list->release(list->lock);
}

void *list_lookup(list_t *list, unsigned int key) {
    void *ptr;
    list->acquire(list->lock);
    ptr = node_lookup(list->head, key);
    list->release(list->lock);
    return ptr;
}

void list_clear(list_t *list) {
    list->acquire(list->lock);
    node_clear(list->head);
    list->head = NULL;
    list->release(list->lock);
}

unsigned int list_size(list_t *list) {
    unsigned int len;
    list->acquire(list->lock);
    len = node_size(list->head);
    list->release(list->lock);
    return len;
}

int list_change_lock(list_t *list, char *lock_type) {
    return change_lock(lock_type, &list->lock, &list->acquire, &list->release);
}
