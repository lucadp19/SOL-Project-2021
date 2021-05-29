#include "server.h"

int expell_LRU(file_t** expelled_ptr){
    time_t least_time = -1;
    file_t* least_file = NULL;
    
    if(files->nelem == 0){
        return -1;
    }

    hash_iter_t* iter = safe_malloc(sizeof(hash_iter_t));
    hash_iter_init(iter);
    int err;

    while( (err = hashmap_iter_get_next(iter, files)) == 0){
        file_t* curr_file = (file_t*)iter->current_pos->data;
        if(least_time == -1 || curr_file->last_use < least_time){
            least_time = curr_file->last_use;
            least_file = curr_file;
        }
    } if( err == -1 )
        return -1;

    // least_file is != NULL
    char* path = least_file->path_name;
    hashmap_remove(files, path, NULL, (void**)expelled_ptr);

    // no need for mutex, it has already been locked
    curr_state.files--;
    curr_state.space -= (*expelled_ptr)->size;
    free(iter);

    logger("[REPLACEMENT] File \"%s\" was removed from the server by the replacement algorithm.\n", (*expelled_ptr)->path_name);
    return 0;
}

int expell_multiple_LRU(size_t size_to_free, list_t* expelled_list){
    size_t freed = 0;

    // no need for mutex, it has already been locked
    if(size_to_free > curr_state.space) // can't free more space than I currently have
        return -1;

    while(freed < size_to_free){
        file_t* expelled;
        if(expell_LRU(&expelled) == -1)
            break;

        list_push_back(expelled_list, NULL, expelled);
        freed = freed + expelled->size;
    }

    return 0;
}