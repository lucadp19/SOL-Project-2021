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

        // if current file is the file that called LRU continue
        if(!curr_file->can_be_expelled) continue;

        file_writer_lock(curr_file);

        if(least_time == -1 || curr_file->last_use < least_time){
            if(least_file != NULL) file_writer_unlock(least_file);

            // updating times
            least_time = curr_file->last_use;
            least_file = curr_file;
        } else { // this file won't be expelled (for now)
            file_writer_unlock(curr_file);
        }

        debug("\t[LRU] least: %lu, current: %lu, curr-name: %s\n",
            least_time, curr_file->last_use, curr_file->path_name);

    } if( err == -1 )
        return -1;

    // least_file is != NULL
    char* path = least_file->path_name;
    debug("\t[LRU] File to be removed is: %s\n", path);
    hashmap_remove(files, path, NULL, (void**)expelled_ptr);

    // no need for mutex, it has already been locked
    curr_state.files--;
    curr_state.space -= (*expelled_ptr)->size;
    free(iter);

    logger("[REPLACEMENT] File \"%s\" was removed from the server by the replacement algorithm.\n", (*expelled_ptr)->path_name);
    logger("[REPLACEMENT] %lu bytes freed.\n", (*expelled_ptr)->path_name);
    return 0;
}

int expell_multiple_LRU(size_t size_to_free, list_t* expelled_list){
    if(expelled_list == NULL) return -1;

    size_t freed = 0;

    // no need for mutex, it has already been locked
    if(size_to_free > curr_state.space) // can't free more space than I currently have
        return -1;

    while(freed < size_to_free){
        file_t* expelled;
        if(expell_LRU(&expelled) == -1)
            break;

        if( list_push_back(expelled_list, NULL, expelled) == -1){
            // no more memory
            // TODO
        }
        freed = freed + expelled->size;
    }

    return 0;
}