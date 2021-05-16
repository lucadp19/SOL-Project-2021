#include "hash/hashtable.h"

static inline int _hashtbl_insert(hashtbl_t* table, long item){
    long hash = (table->hash_funct(item)) % (table->nlist);

    // printf("hash: %ld\n", hash);
    if( list_push_back(table->list[hash], NULL, (void*)item) == -1) {
        // printf("list push back failed\n");
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
    // printf("table nelem = <%ld>, nlist = <%ld>\n", new_table->nelem, new_table->nlist);

    for(int i = 0; i < (*table)->nlist; i++){
        node_t* curr = (*table)->list[i]->head;

        while(curr != NULL){
            if( _hashtbl_insert(new_table, (long)curr->data) == -1)
                return -1;
            curr = curr->next;
            // printf("table nelem = <%ld>, nlist = <%ld>\n", new_table->nelem, new_table->nlist);
        }
    } 

    // printf("new_table nelem = <%ld>, nlist = <%ld>\n", new_table->nelem, new_table->nlist);
    // printf("table nelem = <%ld>, nlist = <%ld>\n", (*table)->nelem, (*table)->nlist);
    hashtbl_free(table);
    *table = new_table;

    // printf("table nelem = <%ld>, nlist = <%ld>\n", (*table)->nelem, (*table)->nlist);
    // printf("Finished expansion.\n");
    return 0;
}

int hashtbl_init(hashtbl_t** table, int nlist, long (*hash_funct)(long)){
    (*table) = NULL;

    if( ((*table) = (hashtbl_t*)malloc(sizeof(hashtbl_t))) == NULL){
        errno = ENOMEM;
        return -1;
    }

    (*table)->nelem = 0;
    (*table)->nlist = nlist;
    (*table)->hash_funct = hash_funct;

    // printf("Yay 1\n");
    // fflush(stdout);

    if( ((*table)->list = (list_t**)malloc((*table)->nlist * sizeof(list_t*))) == NULL){
        errno = ENOMEM;
        return -1;
    }
    // printf("Yay 2\n");
    // fflush(stdout);

    for(int i = 0; i < (*table)->nlist; i++){
        if( ((*table)->list[i] = empty_list()) == NULL){
            return -1;
        }
    }
    // printf("Yay 3\n");
    // fflush(stdout);

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
        // printf("INSERT FAILED\n");
        return -1;
    }
    return 0;
}

int hashtbl_remove(hashtbl_t* table, long item){
    if(table == NULL){
        errno = EINVAL;
        return -1;
    }

    long hash = (table->hash_funct(item)) % (table->nlist);

    node_t* curr = table->list[hash]->head;

    // printf("hash: %ld, list length = %d\n", hash, table->list[hash]->nelem);

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
            // printf("FOUND AND REMOVED!\n");
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

