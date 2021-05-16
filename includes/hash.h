#ifndef _HASH_H
#define _HASH_H

#include "hash/hashmap.h"
#include "hash/hashtable.h"

typedef struct {
    node_t* current_pos;
    long current_list; 
} hash_iter_t;

/**
 * Initializes a iterator (already allocated).
 * It returns 0 on success, -1 and sets errno if iter == NULL.
 */ 
int hash_iter_init(hash_iter_t* iter);

/**
 * Gets the next element from the table.
 * It returns
 *      0 if a next element is found;
 *      -1 and sets errno if there is an error;
 *      1 if the end of the table is reached.
 */
int hashtbl_iter_get_next(hash_iter_t* iter, hashtbl_t* table); 

#endif