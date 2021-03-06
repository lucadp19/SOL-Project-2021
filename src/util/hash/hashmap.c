#include "util/hash/hashmap.h"

static inline int _hashmap_insert(hashmap_t* map, const char* key, void* data){
    unsigned long hash = (map->hash_funct(key, map->nlist)) % (map->nlist);

    if( list_push_back(map->list[hash], key, data) == -1) {
        return -1;
    }

    map->nelem++;
    return 0;
}

static void _hashmap_free_nodes(hashmap_t** map){
     if(*map == NULL) return;

    for(int i = 0; i < (*map)->nlist; i++){
        list_delete(&(*map)->list[i], free_only_node);
    }
    free((*map)->list);
    free(*map);

    *map = NULL;
}

static int hashmap_expand(hashmap_t** map){
    if(*map == NULL){
        errno = EINVAL;
        return -1;
    }
    
    hashmap_t* new_map = NULL;

    if(hashmap_init(&new_map, (*map)->nlist * 2, (*map)->hash_funct, (*map)->node_cleaner) != 0){
        return -1;
    }

    for(int i = 0; i < (*map)->nlist; i++){
        node_t* curr = (*map)->list[i]->head;

        while(curr != NULL){
            if( _hashmap_insert(new_map, curr->key, curr->data) == -1)
                return -1;
            curr = curr->next;
        }
    } 

    _hashmap_free_nodes(map);
    *map = new_map;

    return 0;
}

int hashmap_init(hashmap_t** map, int nlist, unsigned long (*hash_funct)(const char*, long), void (*node_cleaner)(node_t*)){
    (*map) = NULL;

    if( ((*map) = (hashmap_t*)malloc(sizeof(hashmap_t))) == NULL){
        errno = ENOMEM;
        return -1;
    }

    (*map)->nelem = 0;
    (*map)->nlist = nlist;
    (*map)->hash_funct = hash_funct;
    (*map)->node_cleaner = node_cleaner;


    if( ((*map)->list = (list_t**)malloc((*map)->nlist * sizeof(list_t*))) == NULL){
        errno = ENOMEM;
        return -1;
    }

    for(int i = 0; i < (*map)->nlist; i++){
        if( ((*map)->list[i] = empty_list()) == NULL){
            return -1;
        }
    }

    return 0;
}

int hashmap_insert(hashmap_t** map, const char* key, void* item){
    if((*map) == NULL){
        errno = EINVAL;
        return -1;
    }

    if((*map)->nelem + 1 > (*map)->nlist/2){
        if( hashmap_expand(map) == -1){
            return -1;
        }
    }

    if( _hashmap_insert(*map, key, item) == -1){
        return -1;
    }
    return 0;
}

int hashmap_remove(hashmap_t* map, const char* key, const char** key_ptr, void** data_ptr){
    if(map == NULL){
        errno = EINVAL;
        return -1;
    }

    unsigned long hash = (map->hash_funct(key, map->nlist)) % (map->nlist);

    node_t* curr = map->list[hash]->head;

    while(curr != NULL){
        if(strcmp(curr->key, key) == 0){

            // is head?
            if(curr->prev != NULL)
                curr->prev->next = curr->next;
            else map->list[hash]->head = curr->next;

            // is tail?
            if(curr->next != NULL)
                curr->next->prev = curr->prev;
            else map->list[hash]->tail = curr->prev;

            if(data_ptr != NULL)
                *data_ptr = curr->data;
            if(key_ptr != NULL)
                *key_ptr = curr->key;

            free(curr);
            map->list[hash]->nelem--;
            map->nelem--;

            return 0;
        }
        curr = curr->next;
    }

    return 1;
}

void hashmap_free(hashmap_t** map){
    if(*map == NULL) return;

    for(int i = 0; i < (*map)->nlist; i++){
        list_delete(&(*map)->list[i], (*map)->node_cleaner);
    }
    free((*map)->list);
    free(*map);

    *map = NULL;
}

int hashmap_get_by_key(hashmap_t* map, const char* key, void** data){
    if(map == NULL) {
        errno = EINVAL;
        return -1;
    }

    unsigned long hash = (map->hash_funct(key, map->nlist)) % (map->nlist);
    node_t* curr = map->list[hash]->head;
    *data = NULL;

    while(curr != NULL){
        if(strcmp(key, curr->key) == 0){
            *data = curr->data;
            return 0;
        }
        curr = curr->next;
    }

    errno = ENOENT;
    return -1;
}

bool hashmap_contains(hashmap_t* map, const char* key){
    if(map == NULL) return false;

    unsigned long hash = (map->hash_funct(key, map->nlist)) % (map->nlist);
    node_t* curr = map->list[hash]->head;

    while(curr != NULL) {
        if(strcmp(key, curr->key) == 0)
            return true;    
        curr = curr->next;
    }

    return false;
}

// Taken from the following link:
// http://www.cse.yorku.ca/~oz/hash.html
unsigned long default_hashmap_hash(const char* str, long nlist){
    unsigned long hash = 5381;
    int c;

    while( (c = *(str++)) )
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

