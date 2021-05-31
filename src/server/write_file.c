#include "server.h"

int write_file(int worker_no, long fd_client){
    int l;
    int pathname_len;
    char* pathname;
    long size;
    void* buf;

    // ------- READING DATA FROM CLIENT ------- //
    // reading pathname length
    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1){
        return SA_ERROR;
    } if( l == 0 ) return SA_CLOSE;

    // reading pathname
    pathname = safe_calloc(pathname_len + 1, sizeof(char));
    if( (l = readn(fd_client, pathname, pathname_len + 1)) == -1) {
        free(pathname);
        return SA_ERROR;
    } if ( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }

    // reading size of buffer
    if( (l = readn(fd_client, &size, sizeof(long))) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) return SA_CLOSE;

    buf = safe_malloc(size);
    // reading buffer
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
    if(hashmap_get_by_key(files, pathname, (void**)&file) == -1){
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        free(buf);
        return SA_NO_FILE;
    }

    // locking file
    file_writer_lock(file);

    // client hasn't openeid file
    if(!hashtbl_contains(file->fd_open, fd_client)){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        free(buf);
        return SA_NO_OPEN;
    }
    // file isn't locked
    if(file->fd_lock != fd_client){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        free(buf);
        return SA_NOT_LOCKED;
    }
    // file isn't empty
    if(file->size != 0){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        free(buf);
        return SA_NOT_EMPTY;
    }
    // file is too big
    if(size > server_config.max_space){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
        free(buf);
        return SA_TOO_BIG;
    }

    list_t* to_expell;
    if( (to_expell = empty_list()) == NULL){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        free(pathname);
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
        file->can_be_expelled = false;
        expell_multiple_LRU(size_to_remove, to_expell);
        file->can_be_expelled = true;
    }
    safe_pthread_mutex_unlock(&curr_state_mtx);

    // unlocking file and general mutex
    file_writer_unlock(file);
    safe_pthread_mutex_unlock(&files_mtx);   

    // notify client of success up until now
    int current_res = SA_SUCCESS;
    if( writen(fd_client, &current_res, sizeof(int)) == -1 ){
        free(pathname);
        list_delete(&to_expell, files_node_cleaner);
        return SA_ERROR;
    }

    // sending files to client
    if( send_list_of_files(worker_no, fd_client, to_expell, true) == -1){
        free(pathname);
        list_delete(&to_expell, files_node_cleaner);
        return SA_ERROR;
    }

    logger("[THREAD %d] [WRITE_FILE_SUCCESS] Successfully written file \"%s\" into server.\n", worker_no, pathname);
    logger("[THREAD %d] [WRITE_FILE_SUCCESS][WB] %lu\n", worker_no, size);

    free(pathname);    
    list_delete(&to_expell, files_node_cleaner);
    return SA_SUCCESS;
}