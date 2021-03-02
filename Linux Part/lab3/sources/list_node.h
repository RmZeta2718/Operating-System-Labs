#ifndef __LIST_NODE_H__
#define __LIST_NODE_H__

typedef struct __node_t {
    unsigned key;
    struct __node_t *next;
} node_t;

void node_insert(node_t **head, unsigned int key);
void node_delete(node_t **head, unsigned int key);
void *node_lookup(node_t *head, unsigned int key);
void node_clear(node_t *head);
unsigned int node_size(node_t *head);

#endif