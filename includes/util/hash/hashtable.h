#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"

/**
 * An hashtable containing integer (long) values.
 */
typedef struct {
    long nlist;
    long nelem;
    list_t** list;
    unsigned long (*hash_funct)(long, long);
} hashtbl_t;

/**
 * Creates and allocates a new hashtable with nlist positions with a given hash function.
 * On success returns 0 and returns the new hashtable in table,
 * on error returns -1 and sets errno.
 * Possible errors are:
 *  * memory allocation failed, errno = [ENOMEM].
 */
int hashtbl_init(hashtbl_t** table, int nlist, unsigned long (*hash_funct)(long, long));

/**
 * Inserts a new element into the hashtable pointed by table.
 * In doing so, the function may double the size of the hashtable.
 * On success returns 0, on error returns -1 and sets errno.
 * Possible errors are:
 *  * table is NULL, errno = [EINVAL];
 *  * memory allocation failed, errno = [ENOMEM].
 */
int hashtbl_insert(hashtbl_t** table, long item);

/**
 * Removes item from the hashtable table.
 * If the element is found and removed, it returns 0.
 * If the element is not found, it returns 1.
 * On error the function returns -1 and sets errno.
 * Possible errors are:
 *  * table is NULL, errno = [EINVAL].
 */
int hashtbl_remove(hashtbl_t* table, long item);

/**
 * Given an hashtable and an item, checks if item is in table.
 * If item is found returns true, otherwise returns false.
 */
bool hashtbl_contains(hashtbl_t* table, long item);

/**
 * Deletes table deallocating all memory. 
 * At the end of this function, table will not point to a meaningful location of memory.
 */
void hashtbl_free(hashtbl_t** table);

/**
 * Default hashtable hashing function.
 */
unsigned long default_hashtbl_hash(long val, long nlist);

#endif