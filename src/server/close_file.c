#include "server.h"
#include "server-api-protocol.h"

int close_file(long fd_client){
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
        free(pathname);
        safe_pthread_mutex_unlock(&files_mtx);
        return SA_NO_FILE;
    }
    // otherwise remove client
    safe_pthread_mutex_lock(&(file->file_mtx));
    hashtbl_remove(file->fd_open, fd_client);
    // if file was locked, unlock it
    if(file->fd_lock == fd_client)
        file->fd_lock = -1;
    safe_pthread_mutex_unlock(&(file->file_mtx));
    safe_pthread_mutex_unlock(&files_mtx);

    free(pathname);
    return SA_SUCCESS;
}