#include "util/hash/hashmap.h"

static inline int _hashmap_insert(hashmap_t* map, const char* key, void* data){
    long hash = (map->hash_funct(key)) % (map->nlist);

    if( list_push_back(map->list[hash], key, data) == -1) {
        return -1;
    }

    map->nelem++;
    return 0;
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

    hashmap_free(map);
    *map = new_map;

    return 0;
}

int hashmap_init(hashmap_t** map, int nlist, long (*hash_funct)(const char*), void (*node_cleaner)(node_t*)){
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

int hashmap_remove(hashmap_t* map, const char* key){
    if(map == NULL){
        errno = EINVAL;
        return -1;
    }

    long hash = (map->hash_funct(key)) % (map->nlist);

    node_t* curr = map->list[hash]->head;

    while(curr != NULL){
        if(curr->key == key){

            // is head?
            if(curr->prev != NULL)
                curr->prev->next = curr->next;
            else map->list[hash]->head = NULL;

            // is tail?
            if(curr->next != NULL)
                curr->next->prev = curr->prev;
            else map->list[hash]->tail = NULL;

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


