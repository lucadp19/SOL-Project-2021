#ifndef _LIST_H
#define _LIST_H

#include <stdlib.h>
#include "node.h"
#include "util.h"

/** 
 * A doubly-linked list with pointers to both ends and a length field.
 */
typedef struct {
    node_t* head;
    node_t* tail;
    unsigned int length;
} list_t;

/** 
 * Returns a newly allocated list_t object. 
 * If allocation fails, it returns NULL ad sets errno (ENOMEM).
 */
list_t* empty_list();
/**
 * Deletes all the nodes in a list and the list itself..
 */
void list_delete(list_t* list);

/**
 * Adds node to the front of list.
 * If the given list or node is NULL it returns ENULLVAL,
 * whereas in case of success it returns 0.
 */
int list_push_front(list_t* list, node_t* node);
/**
 * Adds node to the back of list.
 * If the given list or node is NULL it returns ENULLVAL,
 * whereas in case of success it returns 0.
 */
int list_push_back(list_t* list, node_t* node);

/**
 * Takes the first node of list and returns it.
 * If list is empty it returns NULL.
 */
node_t* list_pop_front(list_t* list);
/**
 * Takes the last node of list and returns it.
 * If list is empty it returns NULL.
 */
node_t* list_pop_back(list_t* list);

#endif