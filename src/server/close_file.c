#include "server.h"
#include "server-api-protocol.h"

int close_file(int worker_no, long fd_client){
    int l;
    char* pathname = NULL;
    int pathname_len;

    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1 ){
        return SA_ERROR;
    } if (l == 0) return SA_CLOSE;
    debug("pathname_len = %d\n", pathname_len);

    pathname = safe_calloc((pathname_len + 1), sizeof(char));

    if( (l = readn(fd_client, (void*)pathname, pathname_len+1)) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }
    debug("pathname = <%s>\n", pathname);

    // removing client from table of clients who have opened the file
    file_t* file;
    safe_pthread_mutex_lock(&files_mtx);
    // if file doesn't exist, error
    if(hashmap_get_by_key(files, pathname, (void**)&file) == -1){
        if(errno == ENOENT){ // file doesn't exist
            free(pathname);
            safe_pthread_mutex_unlock(&files_mtx);
            return SA_NO_FILE;
        } else if (errno == EINVAL) { // files is NULL, abort
            fprintf(stderr, "Hashmap of files is NULL, aborting.\n");
            exit(EXIT_FAILURE);
        }
    }
    // otherwise remove client
    file_writer_lock(file);
    hashtbl_remove(file->fd_open, fd_client);
    // if file was locked, unlock it
    if(file->fd_lock == fd_client)
        file->fd_lock = -1;
    file_writer_unlock(file);
    safe_pthread_mutex_unlock(&files_mtx);

    logger("[THREAD %d] [CLOSE_FILE_SUCCESS] Successfully closed file \"%s\".\n", worker_no, pathname);
    free(pathname);
    return SA_SUCCESS;
}