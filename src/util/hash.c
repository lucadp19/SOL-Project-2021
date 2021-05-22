#include "util/hash.h"

int hash_iter_init(hash_iter_t* iter){
    if(iter == NULL) {
        errno = EINVAL;
        return -1;
    }

    iter->current_list = -1;
    iter->current_pos = NULL;

    return 0;
}

int hashtbl_iter_get_next(hash_iter_t* iter, hashtbl_t* table){
    if(table == NULL || iter == NULL){
        errno = EINVAL;
        return -1;
    }

    if(iter->current_pos != NULL){
        iter->current_pos = iter->current_pos->next;
        
        if(iter->current_pos != NULL){ // found next element in the same list
            return 0;
        }
    }

    // list ended: must go to next list
    // searching for next non-empty list
    while(iter->current_list + 1 < table->nlist && iter->current_pos == NULL){
        iter->current_list++;
        iter->current_pos = table->list[iter->current_list]->head;
    }

    if(iter->current_pos != NULL){ // finally found it
        return 0;
    }

    // end of hashtable
    iter->current_pos = NULL;
    iter->current_list = -2;

    return 1;
}

int hashmap_iter_get_next(hash_iter_t* iter, hashmap_t* map){
    if(map == NULL || iter == NULL){
        errno = EINVAL;
        return -1;
    }

    if(iter->current_pos != NULL){
        iter->current_pos = iter->current_pos->next;
        
        if(iter->current_pos != NULL){ // found next element in the same list
            return 0;
        }
    }

    // list ended: must go to next list
    // searching for next non-empty list
    while(iter->current_list + 1 < map->nlist && iter->current_pos == NULL){
        iter->current_list++;
        iter->current_pos = map->list[iter->current_list]->head;
    }

    if(iter->current_pos != NULL){ // finally found it
        return 0;
    }

    // end of hashtable
    iter->current_pos = NULL;
    iter->current_list = -2;

    return 1;
}