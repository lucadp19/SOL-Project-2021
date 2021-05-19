#include "util/hash.h"

static inline long hash(long val, long nelem){
    return (val*258347 + 4) % nelem; // random function
}

int main(){
    hashtbl_t* table;

   
    if( hashtbl_init(&table, 32, hash) == -1){
        perror("hashtbl_init");
        return -1;
    }

    printf("table nelem = <%ld>, nlist = <%ld>\n", table->nelem, table->nlist);

    for (int i = 0; i < table->nlist; i++){
        if(table->list[i] == NULL){
            printf("List no. %d is NULL", i);
        }
    }

    for(int i = 0; i < 192; i++){
        printf("Inserting %d\n", i);
        if( hashtbl_insert(&table, (long)i) == -1){
            perror("hashtbl_insert");
            return -1;
        }
        printf("table nelem = <%ld>, nlist = <%ld>\n", table->nelem, table->nlist);
        fflush(stdout);
    }

    printf("nelem = <%ld>, nlist = <%ld>\n", table->nelem, table->nlist);
    
    if( hashtbl_remove(table, 8l) == 0)
        printf("Removed 8!\n");

    printf("Printing all elements of table.\n");
    hash_iter_t* iter;
    if( (iter = malloc(sizeof(hash_iter_t))) == NULL){
        perror("malloc iter");
        return -1;
    }
    hash_iter_init(iter);
    int err;
    while( (err = hashtbl_iter_get_next(iter, table)) == 0){
        printf("Current elem: %ld\n", (long)iter->current_pos->data);
    } if(err == -1){
        perror("iter");
        return -1;
    } else if(err == 1){
        printf("END OF TABLE\n");
    }

    printf("Deleting table!\n");
    hashtbl_free(&table);
    free(iter);
    printf("Yeee\n");
    
    return 0;
}