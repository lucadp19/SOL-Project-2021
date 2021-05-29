#include "server.h"

int write_file(int worker_no, long fd_client){
    int l;
    int pathname_len;
    char* pathname;
    long size;
    void* buf;

    // ------- READING DATA FROM CLIENT ------- //
    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1){
        return SA_ERROR;
    } if( l == 0 ) return SA_CLOSE;

    pathname = safe_calloc(pathname_len + 1, sizeof(char));
    if( (l = readn(fd_client, pathname, pathname_len + 1)) == -1) {
        free(pathname);
        return SA_ERROR;
    } if ( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }

    if( (l = readn(fd_client, &size, sizeof(long))) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) return SA_CLOSE;

    buf = safe_malloc(size);
    if( (l = readn(fd_client, buf, size)) == -1) {
        free(pathname);
        free(buf);
        return SA_ERROR;
    } if ( l == 0 ) {
        free(pathname);
        free(buf);
        return SA_CLOSE;
    }

    debug("pathname_len = %d, pathname = %s, bufsize = %lu\n", pathname_len, pathname, size);

    // ----- WRITING DATA INTO FS ----- //
    file_t* file;

    safe_pthread_mutex_lock(&files_mtx);
    // file doesn't exist
    if(hashmap_get_by_key(files, pathname, (void**)&file) == -1 && errno == ENOENT){
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        free(buf);
        return SA_NO_FILE;
    }

    // don't need it anymore
    free(pathname);

    // file isn't locked
    if(file->fd_lock != fd_client){
        safe_pthread_mutex_unlock(&files_mtx);
        free(buf);
        return SA_NOT_LOCKED;
    }
    // file isn't empty
    if(file->size != 0){
        safe_pthread_mutex_unlock(&files_mtx);
        free(buf);
        return SA_NOT_EMPTY;
    }
    // file is too big
    if(size > server_config.max_space){
        safe_pthread_mutex_unlock(&files_mtx);
        free(buf);
        return SA_TOO_BIG;
    }


    list_t* to_expell;
    if( (to_expell = empty_list()) == NULL){
        safe_pthread_mutex_unlock(&files_mtx);
        free(buf);
        return SA_ERROR; 
    }

    file->size     = size;
    file->contents = buf;
    file->last_use = time(NULL);

    safe_pthread_mutex_lock(&curr_state_mtx);
    curr_state.space += file->size;
    long size_to_remove = curr_state.space - server_config.max_space;
    if(size_to_remove > 0){
        // maybe I should check the error
        expell_multiple_LRU(size_to_remove, to_expell);
    }
    safe_pthread_mutex_unlock(&curr_state_mtx);
    safe_pthread_mutex_unlock(&files_mtx);   

    // sending files to client
    if( send_list_of_files(worker_no, fd_client, to_expell) == -1){
        list_delete(&to_expell, files_node_cleaner);
        return SA_ERROR;
    }

    logger("[THREAD %d] [WRITE_FILE_SUCCESS] Successfully written file \"%s\" into server.\n", worker_no, file->path_name);
    logger("[THREAD %d] [WRITE_FILE_SUCCESS][WB] %lu\n", worker_no, file->size);
    
    list_delete(&to_expell, files_node_cleaner);
    return SA_SUCCESS;
}