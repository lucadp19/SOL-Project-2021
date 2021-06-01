
#include "server.h"
#include "server-api-protocol.h"

int remove_file(int worker_no, long fd_client){
    int l;
    char* pathname = NULL;
    int pathname_len;

    // reading pathname length
    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1 ){
        return SA_ERROR;
    } if (l == 0) return SA_CLOSE;

    pathname = safe_calloc((pathname_len + 1), sizeof(char));

    // reading pathname
    if( (l = readn(fd_client, (void*)pathname, pathname_len+1)) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }
    
    file_t* to_remove;

    safe_pthread_mutex_lock(&files_mtx);
    if(hashmap_get_by_key(files, pathname, (void**)&to_remove) == -1){ // file not present => success
        if(errno == ENOENT) {
            safe_pthread_mutex_unlock(&files_mtx);
            logger("[THREAD %d][REMOVE_FILE_SUCCESS] File %s is not currently present in server.\n", worker_no, pathname);
            free(pathname);
            return SA_SUCCESS;
        } else if (errno == EINVAL) { // files is NULL, abort
            fprintf(stderr, "Hashmap of files is NULL, aborting.\n");
            exit(EXIT_FAILURE);
        }
    }

    file_writer_lock(to_remove);
    
    // client didn't open this file
    if(!hashtbl_contains(to_remove->fd_open, fd_client)){ 
        file_writer_unlock(to_remove);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        return SA_NO_OPEN;
    }

    // file hasn't been locked by client
    if(to_remove->fd_lock != fd_client){
        file_writer_unlock(to_remove);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        return SA_NOT_LOCKED;
    }

    // updating current server state
    safe_pthread_mutex_lock(&curr_state_mtx);
    curr_state.files--;
    curr_state.space -= to_remove->size;
    logger("[STATS][CURRENT_FILES] %u files are currently stored.\n", curr_state.files);
    logger("[STATS][CURRENT_SPACE] %lu bytes are currently occupied.\n", curr_state.space);
    safe_pthread_mutex_unlock(&curr_state_mtx);

    hashmap_remove(files, pathname, NULL, (void**)&to_remove);
    file_delete(to_remove);

    // no need to destroy mutex

    safe_pthread_mutex_unlock(&files_mtx);

    logger("[THREAD %d] [REMOVE_FILE_SUCCESS] Successfully removed file \"%s\" from server.\n", worker_no, pathname);    
    free(pathname);

    return SA_SUCCESS;
}