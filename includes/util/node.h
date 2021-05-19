#ifndef _NODE_H
#define _NODE_H

#include <stdlib.h>

/**
 * A node of a doubly-linked list, with a constant string key
 * and a generic data field.
 */
typedef struct _node_t {
    const char* key;
    void* data;
    struct _node_t* prev;
    struct _node_t* next;
} node_t;

/**
 * Creates a new node with the given key and data.
 * If allocation fails, it returns NULL and sets errno (ENOMEM).
 */
node_t* create_node(const char* key, void* data);
/**
 * Deletes the contents of a node and the node itself.
 */
void free_node(node_t* node);

#endif