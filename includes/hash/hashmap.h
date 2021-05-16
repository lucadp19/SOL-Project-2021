#ifndef _HASHMAP_H
#define _HASHMAP_H

#include "util.h"
#include "node.h"
#include "list.h"

/**
 * An hashmap mapping strings to generic void* values.
 */
typedef struct {
    long nlist;
    long nelem;
    list_t** list;
    long (*hash_funct)(const char*);
    void (*node_cleaner)(node_t*);
} hashmap_t;

/**
 * Creates and allocates a new hashmap with nlist positions,
 * the given hashing function and a function to deallocate a single node of the map.
 * On success returns 0 and returns the new hashmap in map,
 * on error returns -1 and sets errno.
 */
int hashmap_init(hashmap_t** map, int nlist, long (*hash_funct)(const char*), void (*node_cleaner)(node_t*));

/**
 * Inserts a new pair <key, data> into the hashmap.
 * On success returns 0, on error returns -1 and sets errno.
 */
int hashmap_insert(hashmap_t** map, const char* key, void* item);

/**
 * Removes an item with the given key from the hashmap map.
 * If the element is found and removed, or if the map did not contain item, it returns 0;
 * On error the function returns -1 and sets errno.
 */
int hashmap_remove(hashmap_t* map, const char* key);

/**
 * Deletes map, deallocating all memory.
 */
void hashmap_free(hashmap_t** map);

#endif
