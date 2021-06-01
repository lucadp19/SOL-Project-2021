#ifndef _HASHMAP_H
#define _HASHMAP_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"

/**
 * An hashmap mapping strings to generic void* values.
 */
typedef struct {
    long nlist;
    long nelem;
    list_t** list;
    unsigned long (*hash_funct)(const char*, long);
    void (*node_cleaner)(node_t*);
} hashmap_t;

/**
 * Creates and allocates a new hashmap with nlist positions,
 * the given hashing function and a function to deallocate a single node of the map.
 * On success returns 0 and returns the new hashmap in map,
 * on error returns -1 and sets errno.
 * Possible errors are:
 *  * memory allocation failed, errno = [ENOMEM].
 */
int hashmap_init(hashmap_t** map, int nlist, unsigned long (*hash_funct)(const char*, long), void (*node_cleaner)(node_t*));

/**
 * Inserts a new pair <key, data> into the hashmap pointed by map.
 * On success returns 0, on error returns -1 and sets errno.
 * Possible errors are:
 *  * map is NULL, errno = [EINVAL];
 *  * memory allocation failed, errno = [ENOMEM].
 */
int hashmap_insert(hashmap_t** map, const char* key, void* item);

/**
 * Removes an item with the given key from the hashmap map.
 * If the element is found and removed, it returns 0
 * and puts the pointer to the removed key and data into key_ptr and data_ptr, unless they are NULL.
 * If the element is not found, it returns 1.
 * On error the function returns -1 and sets errno.
 * Possible errors are:
 *  * map is NULL, errno = [EINVAL]. 
 */
int hashmap_remove(hashmap_t* map, const char* key, const char** key_ptr, void** data_ptr);

/**
 * Deletes map through the given node_cleaner function.
 */
void hashmap_free(hashmap_t** map);

/**
 * Given a key, returns a pointer to the data of a node with the given key.
 * On success returns 0, on error returns -1 and sets errno.
 * Possible errors are:
 *  * map is NULL, errno = [EINVAL];
 *  * no element with the given key is found, errno = [ENOENT].
 */
int hashmap_get_by_key(hashmap_t* map, const char* key, void** data);

/**
 * Given a key returns true if map contains a node with the given key,
 * false otherwise. Returns false also when map is NULL.
 */
bool hashmap_contains(hashmap_t* map, const char* key);

/**
 * Default hashmap hashing function.
 */
unsigned long default_hashmap_hash(const char* key, long nlist);

#endif
