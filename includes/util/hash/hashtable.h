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
    long (*hash_funct)(long, long);
} hashtbl_t;

/**
 * Creates and allocates a new hashtable with nlist positions.
 * On success returns 0 and returns the new hashtable in table,
 * on error returns -1 and sets errno.
 */
int hashtbl_init(hashtbl_t** table, int nlist, long (*hash_funct)(long, long));

/**
 * Inserts a new element into the hashtable table.
 * On success returns 0, on error returns -1 and sets errno.
 */
int hashtbl_insert(hashtbl_t** table, long item);

/**
 * Removes item from the hashtable table.
 * If the element is found and removed, or if the table did not contain item, it returns 0;
 * On error the function returns -1 and sets errno.
 */
int hashtbl_remove(hashtbl_t* table, long item);

/**
 * Deletes table deallocating all memory. 
 * At the end of this function, table will not point to a meaningful location of memory.
 */
void hashtbl_free(hashtbl_t** table);

#endif