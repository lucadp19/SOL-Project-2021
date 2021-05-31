#include "util/hash/hashtable.h"
#include <math.h>

static inline int _hashtbl_insert(hashtbl_t* table, long item){
    long hash = (table->hash_funct(item, table->nlist)) % table->nlist;

    if( list_push_back(table->list[hash], NULL, (void*)item) == -1) {
        return -1;
    }

    table->nelem++;
    return 0;
}

static inline void _node_cleaner(node_t* to_clean){
    free(to_clean);
}

static int hashtbl_expand(hashtbl_t** table){
    if(*table == NULL){
        errno = EINVAL;
        return -1;
    }
    
    hashtbl_t* new_table = NULL;

    if(hashtbl_init(&new_table, (*table)->nlist * 2, (*table)->hash_funct) != 0){
        return -1;
    }

    for(int i = 0; i < (*table)->nlist; i++){
        node_t* curr = (*table)->list[i]->head;

        while(curr != NULL){
            if( _hashtbl_insert(new_table, (long)curr->data) == -1)
                return -1;
            curr = curr->next;
        }
    } 

    hashtbl_free(table);
    *table = new_table;

    return 0;
}

int hashtbl_init(hashtbl_t** table, int nlist, unsigned long (*hash_funct)(long, long)){
    (*table) = NULL;

    if( ((*table) = (hashtbl_t*)malloc(sizeof(hashtbl_t))) == NULL){
        errno = ENOMEM;
        return -1;
    }

    (*table)->nelem = 0;
    (*table)->nlist = nlist;
    (*table)->hash_funct = hash_funct;

    if( ((*table)->list = (list_t**)malloc((*table)->nlist * sizeof(list_t*))) == NULL){
        errno = ENOMEM;
        return -1;
    }

    for(int i = 0; i < (*table)->nlist; i++){
        if( ((*table)->list[i] = empty_list()) == NULL){
            return -1;
        }
    }

    return 0;
}

int hashtbl_insert(hashtbl_t** table, long item){
    if((*table) == NULL){
        errno = EINVAL;
        return -1;
    }

    if((*table)->nelem + 1 > (*table)->nlist/2){
        if( hashtbl_expand(table) == -1){
            return -1;
        }
    }

    if( _hashtbl_insert(*table, item) == -1){
        return -1;
    }
    return 0;
}

bool hashtbl_contains(hashtbl_t* table, long item){
    if(table == NULL) return false;

    unsigned long hash = table->hash_funct(item, table->nlist) % table->nlist;
    node_t* curr = table->list[hash]->head;
    while(curr != NULL){
        if((long)curr->data == item)
            return true;
        curr = curr->next;
    }

    return false;
}

int hashtbl_remove(hashtbl_t* table, long item){
    if(table == NULL){
        errno = EINVAL;
        return -1;
    }

    long hash = (table->hash_funct(item, table->nlist));

    node_t* curr = table->list[hash]->head;

    while(curr != NULL){
        if((long)curr->data == item){
            // is head?
            if(curr->prev != NULL)
                curr->prev->next = curr->next;
            else table->list[hash]->head = NULL;
            // is tail?
            if(curr->next != NULL)
                curr->next->prev = curr->prev;
            else table->list[hash]->tail = NULL;

            free(curr);
            table->list[hash]->nelem--;
            table->nelem--;

            return 0;
        }
        curr = curr->next;
    }

    return 1;
}

void hashtbl_free(hashtbl_t** table){
    if(*table == NULL) return;

    for(int i = 0; i < (*table)->nlist; i++){
        list_delete(&(*table)->list[i], _node_cleaner);
    }
    free((*table)->list);
    free(*table);
}

// Taken from Cormen and the following link:
// https://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/homework10/hashfuncs.html
unsigned long default_hashtbl_hash(long val, long nlist){
    double A = 0.5*(sqrt(5) - 1);
    double useless;
    return (unsigned long)floor(nlist*modf(val*A, &useless));
}
