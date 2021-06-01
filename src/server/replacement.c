#include "server.h"

/** 
 * Given two pointers to struct timespec structures, returns
 *  *  1 if the first is greater than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first is smaller than the second.
 */
static int timespec_cmp(struct timespec *a, struct timespec *b);

/** 
 * Given two files, orders them in LRU order. Returns
 *  *  1 if the first was used more recently than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first was used less recently than the second.
 */
static int LRU_policy(file_t* a, file_t* b);

/** 
 * Given two files, orders them in FIFO order. Returns
 *  *  1 if the first was created more recently than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first was created less recently than the second.
 */
static int FIFO_policy(file_t* a, file_t* b);

int expell_single_file(file_t** expelled_ptr){
    file_t* least_file = NULL;

    // setting up replacement policy
    int (*policy)(file_t*, file_t*) = ( server_config.policy == FIFO ? FIFO_policy : LRU_policy );
    char* policy_name = ( server_config.policy == FIFO ? "FIFO" : "LRU" );
    
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

        if(least_file == NULL || policy(curr_file, least_file) < 0){
            if(least_file != NULL) file_writer_unlock(least_file);

            // updating least_file
            least_file = curr_file;
        } else { // this file won't be expelled (for now)
            file_writer_unlock(curr_file);
        }

    } if( err == -1 ) // won't happen: both iter and files are != NULL
        return -1;

    // least_file is != NULL
    char* path = least_file->path_name;
    debug("\t[%s] File to be removed is: %s\n", policy_name, path);
    hashmap_remove(files, path, NULL, (void**)expelled_ptr);

    // no need for mutex, it has already been locked
    curr_state.files--;
    curr_state.space -= (*expelled_ptr)->size;
    curr_state.no_LRU++;
    free(iter);

    logger("[REPLACEMENT][%s] File \"%s\" was removed from the server by the replacement algorithm.\n", policy_name, (*expelled_ptr)->path_name);
    logger("[REPLACEMENT][BF] %lu bytes freed.\n", (*expelled_ptr)->size);
    return 0;
}

int expell_multiple_files(size_t size_to_free, list_t* expelled_list){
    if(expelled_list == NULL) return -1;

    size_t freed = 0;

    // no need for mutex, it has already been locked
    if(size_to_free > curr_state.space) // can't free more space than I currently have
        return -1;

    while(freed < size_to_free){
        file_t* expelled;
        if(expell_single_file(&expelled) == -1)
            break;

        if( list_push_back(expelled_list, NULL, expelled) == -1){
            perror("Error in adding element to list");
            fprintf(stderr, "Aborting server.\n");
            exit(EXIT_FAILURE);
        }
        freed = freed + expelled->size;
    }
    logger("[REPLACEMENT][STATS] In total %d files were removed from the server.\n", expelled_list->nelem);

    return 0;
}

static int timespec_cmp(struct timespec* a, struct timespec* b){
    if(a->tv_sec > b->tv_sec) return 1;
    if(a->tv_sec < b->tv_sec) return -1;
    if(a->tv_nsec > b->tv_nsec) return 1;
    if(a->tv_nsec < b->tv_nsec) return -1;
    return 0;
}

static int LRU_policy(file_t* a, file_t* b){
    return timespec_cmp(&(a->last_use), &(b->last_use));
}

static int FIFO_policy(file_t* a, file_t* b){
    return timespec_cmp(&(a->creation_time), &(b->creation_time));
}