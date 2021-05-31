#include "server.h"

int read_file(int worker_no, long fd_client){
    int l;
    int path_len;
    char* pathname;

    // ----- READING FILE NAME FROM CLIENT ----- //
    if( (l = readn(fd_client, &path_len, sizeof(int))) == -1){
        return SA_ERROR;
    } if( l == 0 ) return SA_CLOSE;

    pathname = safe_calloc(path_len + 1, sizeof(char));
    if( (l = readn(fd_client, pathname, path_len + 1)) == -1) {
        free(pathname);
        return SA_ERROR;
    } if ( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }

    file_t* to_send;
    safe_pthread_mutex_lock(&files_mtx);
    if( hashmap_get_by_key(files, pathname, (void**)&to_send) == -1 ){
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        return SA_NO_FILE;
    }

    // locking file (in reader mode)
    file_reader_lock(to_send);

    if(!hashtbl_contains(to_send->fd_open, fd_client)){ // client didn't open this file
        file_reader_unlock(to_send);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        return SA_NO_OPEN;
    }

    // updating time of last use
    to_send->last_use = time(NULL);

    // unlocking general mutex (we don't need it anymore)
    safe_pthread_mutex_unlock(&files_mtx);

    // notify client of success (until now)
    int current_res = SA_SUCCESS;
    if( writen(fd_client, &current_res, sizeof(int)) == -1 ){
        return SA_ERROR;
    }

    // sending file to client
    if( send_single_file(worker_no, fd_client, to_send, false) == -1){
        return SA_ERROR;
    }

    // size_t size = to_send->size;
    file_reader_unlock(to_send);

    logger("[THREAD %d] [READ_FILE_SUCCESS] Successfully sent file \"%s\" to client.\n", worker_no, pathname);    
    // logger("[THREAD %d] [READ_FILE_SUCCESS][WB] %lu\n", worker_no, size);
    free(pathname);

    return SA_SUCCESS; 
}