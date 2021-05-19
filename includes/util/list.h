#ifndef _LIST_H
#define _LIST_H

#include <stdlib.h>
#include "util/node.h"
#include "util/util.h"

/** 
 * A doubly-linked list with pointers to both ends and a number-of-elements field.
 */
typedef struct {
    node_t* head;
    node_t* tail;
    unsigned int nelem;
} list_t;

/** 
 * Returns a newly allocated list_t object. 
 * If allocation fails, it returns NULL ad sets errno (ENOMEM).
 */
list_t* empty_list();
/**
 * Deletes all the nodes using the node_cleaner function
 * and then deletes the list itself.
 * After calling this function, *list will point to NULL.
 * If node_cleaner == NULL uses free_node as a default. 
 */
void list_delete(list_t** list, void (*node_cleaner) (node_t*));

/**
 * Adds a new node with the given key and data to the front of list.
 * If the given list is NULL it returns -1 and sets errno (EINVAL),
 * whereas in case of success it returns 0.
 */
int list_push_front(list_t* list, const char* key, void* data);
/**
 * Adds a new node with the given key and data to the back of list.
 * If the given list or node is NULL it returns -1 and sets errno (EINVAL),
 * whereas in case of success it returns 0.
 */
int list_push_back(list_t* list, const char* key, void* data);

/**
 * Takes the first node of list and returns its contents into key and data.
 * If list is empty it returns 0 and sets errno (EINVAL), otherwise it returns 0.
 */
int list_pop_front(list_t* list, const char** key, void** data);
/**
 * Takes the last node of list and returns its contents into key and data.
 * If list is empty it returns 0 and sets errno (EINVAL), otherwise it returns 0.
 */
int list_pop_back(list_t* list, const char** key, void** data);

#endif