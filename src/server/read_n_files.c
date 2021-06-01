#include "server.h"

static void reader_unlock_all(list_t* list);

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
        file_t* curr_file = (file_t*)iter->current_pos->data;

        // locking current file
        file_reader_lock(curr_file);

        if(list_push_back(file_list, NULL, (void*)curr_file) == -1){
            file_reader_unlock(curr_file);
            reader_unlock_all(file_list);
            
            safe_pthread_mutex_unlock(&files_mtx);
            list_delete(&file_list, free_only_node);
            return SA_ERROR;
        }
        N--;
    } if( err == -1 ) { // cannot happen: both iter and files are != NULL
        reader_unlock_all(file_list);
            
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&file_list, free_only_node);
        return SA_ERROR;
    }

    free(iter);
    // unlocking general mutex
    safe_pthread_mutex_unlock(&files_mtx);

    // notify client of success up until now
    int current_res = SA_SUCCESS;
    if( writen(fd_client, &current_res, sizeof(int)) == -1){
        reader_unlock_all(file_list);
        list_delete(&file_list, free_only_node);
        return SA_ERROR;
    }

    // writing files to client
    if( send_list_of_files(worker_no, fd_client, file_list, true, "READ_N_FILES") == -1){
        reader_unlock_all(file_list);
        list_delete(&file_list, free_only_node);
        return SA_ERROR;
    }

    reader_unlock_all(file_list);
    list_delete(&file_list, free_only_node);

    if(all_files)
        logger("[THREAD %d] [READ_N_FILES_SUCCESS] Successfully sent every file to client.\n", worker_no);
    else 
        logger("[THREAD %d] [READ_N_FILES_SUCCESS] Successfully sent %d files to client.\n", worker_no, original_N);


    return SA_SUCCESS;
}

static void reader_unlock_all(list_t* list){
    node_t* curr = list->head;
    while(curr != NULL){
        file_reader_unlock((file_t*)curr->data);
        curr = curr->next;
    }
}