#include "server.h"
#include "server-api-protocol.h"

int remove_file(int worker_no, long fd_client){
    int l;
    char* pathname = NULL;
    int pathname_len;

    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1 ){
        return SA_ERROR;
    } if (l == 0) return SA_CLOSE;

    pathname = safe_calloc((pathname_len + 1), sizeof(char));

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
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        return SA_SUCCESS;
    }

    file_writer_lock(to_remove);

    if(to_remove->fd_lock != fd_client){
        file_writer_unlock(to_remove);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        return SA_NOT_LOCKED;
    }

    hashmap_remove(files, pathname, NULL, (void**)&to_remove);
    file_delete(to_remove);

    // no need to destroy mutex

    safe_pthread_mutex_unlock(&files_mtx);

    logger("[THREAD %d] [REMOVE_FILE_SUCCESS] Successfully removed file \"%s\" from server.\n", worker_no, pathname);    
    free(pathname);

    return SA_SUCCESS;
}