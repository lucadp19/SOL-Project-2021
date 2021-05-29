#include "server.h"

int read_n_files(int worker_no, long fd_client){
    int N, original_N;
    bool all_files = false;

    // ------ READING N ------ //
    int l;
    if( (l = readn(fd_client, &N, sizeof(int))) == -1 ){
        return SA_ERROR;
    } if( l == 0 ) return SA_CLOSE;

    original_N = N;

    list_t* file_list;
    if( (file_list = empty_list()) == NULL){
        // TODO: ?
        return SA_ERROR;
    }
    hash_iter_t* iter = safe_malloc(sizeof(hash_iter_t));
    hash_iter_init(iter);

    safe_pthread_mutex_lock(&files_mtx);

    safe_pthread_mutex_lock(&curr_state_mtx);
    if(N <= 0) {
        all_files = true;
        N = curr_state.files;
    }
    safe_pthread_mutex_unlock(&curr_state_mtx);

    int err;
    while(N > 0 && (err = hashmap_iter_get_next(iter, files)) == 0){
        if(list_push_back(file_list, NULL, iter->current_pos->data) == -1){
            safe_pthread_mutex_unlock(&files_mtx);
            list_delete(&file_list, free_only_node);
            // TODO ?
            return SA_ERROR;
        }
        N--;
    } if( err == -1 ) {
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&file_list, free_only_node);
        return SA_ERROR;
    }

    free(iter);

    // notify client of success up until now
    int current_res = SA_SUCCESS;
    if( writen(fd_client, &current_res, sizeof(int)) == -1){
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&file_list, free_only_node);
        return SA_ERROR;
    }

    // writing files to client
    if( send_list_of_files(worker_no, fd_client, file_list, true) == -1){
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&file_list, free_only_node);
        return SA_ERROR;
    }

    safe_pthread_mutex_unlock(&files_mtx);
    list_delete(&file_list, free_only_node);

    if(all_files)
        logger("[THREAD %d] [READ_N_FILES_SUCCESS] Successfully sent every file to client.\n");
    else 
        logger("[THREAD %d] [READ_N_FILES_SUCCESS] Successfully sent %d files to client.\n", worker_no, original_N);


    return SA_SUCCESS;
}