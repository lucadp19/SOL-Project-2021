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
 * May fail if:
 *  * allocation fails: the function returns NULL ad sets errno to ENOMEM.
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
 * On success returns 0; otherwise returns -1 and sets errno.
 * Possible errors are:
 *  * list is NULL, errno = [EINVAL];
 *  * node allocation fails, errno = [ENOMEM].
 */
int list_push_front(list_t* list, const char* key, void* data);
/**
 * Adds a new node with the given key and data to the back of list.
 * On success returns 0; otherwise returns -1 and sets errno.
 * Possible errors are:
 *  * list is NULL, errno = [EINVAL];
 *  * node allocation fails, errno = [ENOMEM].
 */
int list_push_back(list_t* list, const char* key, void* data);

/**
 * Takes the first node of list and returns its contents into key and data, if they are not NULL.
 * If they are NULL, those contents will be lost.
 * On success returns 0, on error returns -1 and sets errno.
 * Possible errors are:
 *  * list is NULL or empty, errno = [EINVAL].
 */
int list_pop_front(list_t* list, const char** key, void** data);
/**
 * Takes the last node of list and returns its contents into key and data, if they are not NULL.
 * If they are NULL, those contents will be lost.
 * On success returns 0, on error returns -1 and sets errno.
 * Possible errors are:
 *  * list is NULL or empty, errno = [EINVAL].
 */
int list_pop_back(list_t* list, const char** key, void** data);

#endif