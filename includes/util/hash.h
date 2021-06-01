#ifndef _HASH_H
#define _HASH_H

#include "util/hash/hashmap.h"
#include "util/hash/hashtable.h"

/** Default number of positions for a hashtable or hashmap. */
#define HASH_N_LIST 8

/** Iterator for hashtables and hashmaps. */
typedef struct {
    node_t* current_pos;
    long current_list; 
} hash_iter_t;

/**
 * Initializes an already allocated iterator.
 * On success returns 0, on error returns -1 and sets errno.
 * Possible errors are:
 *  * iter is NULL, errno = [EINVAL].
 */ 
int hash_iter_init(hash_iter_t* iter);

/**
 * Gets the next element from the table.
 * It returns
 *  *  0    if a next element is found;
 *  * -1    if table or iter are NULL, errno = [EINVAL];
 *  *  1    if the end of the table is reached.
 */
int hashtbl_iter_get_next(hash_iter_t* iter, hashtbl_t* table); 

/**
 * Gets the next element from the map.
 * It returns
 *  *  0    if a next element is found;
 *  * -1    if map or iter are NULL, errno = [EINVAL];
 *  *  1    if the end of the table is reached.
 */
int hashmap_iter_get_next(hash_iter_t* iter, hashmap_t* map); 

#endif