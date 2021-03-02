#include "list_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

void node_insert(node_t **head, unsigned int key) {
    node_t *new = malloc(sizeof(node_t));
    if (!new) {
        // if no enough space when malloc
        // print error message and do nothing on the list
        perror("node_insert");
        return;
    }
    new->key = key;
    new->next = *head;
    *head = new;
}
void node_delete(node_t **head, unsigned int key) {
    node_t *now, *pre;
    now = *head;
    while (now) {
        if (now->key == key) {
            if (now == *head)
                *head = now->next;
            else
                pre->next = now->next;
            free(now);
            return;
        }
        pre = now;
        now = now->next;
    }
    assert(0);
}

void *node_lookup(node_t *head, unsigned int key) {
    node_t *ptr = head;
    while (ptr) {
        if (ptr->key == key) break;
        ptr = ptr->next;
    }
    return ptr;
}

void node_clear(node_t *head) {
    node_t *ptr = head, *nxt;
    while (ptr) {
        nxt = ptr->next;
        free(ptr);
        ptr = nxt;
    }
}

unsigned int node_size(node_t *ptr) {
    unsigned int len = 0;
    while (ptr) {
        ++len;
        ptr = ptr->next;
    }
    return len;
}