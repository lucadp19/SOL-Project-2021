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
        if(curr_file->last_use < least_time){
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

    return 0;
}

int expell_multiple_LRU(size_t size_to_free, list_t* expelled_list){
    size_t freed = 0;

    // no need for mutex, it has already been locked
    if(size_to_free > curr_state.space)
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

int send_expelled_files(int worker_no, long fd_client, list_t* expelled){
    if(expelled == NULL) return -1;

    debug("HELLO1\n");

    node_t* curr = expelled->head;
    debug("Hello from before the while loop\n");
    if(curr == NULL)
        debug("curr is null\n");
    while(curr != NULL){
        int pathname_len = strlen(curr->key);
        file_t* curr_file = (file_t*)curr->data;

        debug("hello from inside the while loop\n");
        debug("filename = %s\n", curr->key);

        if( writen(fd_client, &pathname_len, sizeof(int)) == -1)
            return -1;
        if( writen(fd_client, (void*)curr->key, (pathname_len+1) * sizeof(char)) == -1)
            return -1;
        
        if( writen(fd_client, &curr_file->size, sizeof(size_t)) == -1)
            return -1;
        
        if( curr_file->size != 0 && writen(fd_client, curr_file->contents, curr_file->size) == -1)
            return -1;

        curr = curr->next;
    }

    int terminating_zero = 0;
    debug("Finished list\n");
    int write_err;
    if( (write_err = writen(fd_client, &terminating_zero, sizeof(int))) == -1)
        return -1;
    if(write_err == 0) debug("write returned 0!\n");
    debug("about to return 0...\n");

    return 0;
}