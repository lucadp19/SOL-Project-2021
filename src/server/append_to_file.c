#include "server.h"
#include "server-api-protocol.h"

int append_to_file(int worker_no, long fd_client){
    int l;
    char* pathname = NULL;
    int pathname_len;
    void* buf;
    size_t size;

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

    if( (l = readn(fd_client, &size, sizeof(size_t))) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }

    buf = safe_malloc(size);
    if( (l = readn(fd_client, buf, size)) == -1){
        free(pathname);
        free(buf);
        return SA_ERROR;
    } if( l == 0 ) {
        free(pathname);
        free(buf);
        return SA_CLOSE;
    }

    file_t* file;
    list_t* to_expell;
    if( (to_expell = empty_list()) == NULL){
        free(pathname);
        free(buf);
        return SA_ERROR;
    }

    safe_pthread_mutex_lock(&files_mtx);
    if(hashmap_get_by_key(files, pathname, (void**)&file) == -1){ // file not present => success
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&to_expell, files_node_cleaner);
        free(pathname);
        free(buf);
        return SA_NO_FILE;
    }

    // locking file
    file_writer_lock(file);

    // checking that client has previously opened the file
    if(!hashtbl_contains(file->fd_open, fd_client)){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&to_expell, files_node_cleaner);
        free(pathname);
        free(buf);
        return SA_NO_OPEN;
    }

    // checking that buf can be appended to file without going over space requirements
    if(file->size + size > server_config.max_space){
        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        list_delete(&to_expell, files_node_cleaner);
        free(pathname);
        free(buf);
        return SA_TOO_BIG;
    } 


    // updating file contents
    file->contents = safe_realloc(file->contents, file->size + size);
    memcpy((unsigned char*)(file->contents) + file->size, buf, size);
    file->size += size;
    free(buf);

    // updating last use time
    file->last_use = time(NULL);

    safe_pthread_mutex_lock(&curr_state_mtx);
    curr_state.space += size;
    long size_to_remove = curr_state.space - server_config.max_space;
    if(size_to_remove > 0){
        file->can_be_expelled = false;
        // maybe I should check the error
        expell_multiple_LRU(size_to_remove, to_expell);
        file->can_be_expelled = true;
    }
    safe_pthread_mutex_unlock(&curr_state_mtx);
    
    // unlocking locks
    file_writer_unlock(file);
    safe_pthread_mutex_unlock(&files_mtx);

    // notify client of success up until now
    int current_res = SA_SUCCESS;
    if( writen(fd_client, &current_res, sizeof(int)) == -1 ){
        list_delete(&to_expell, files_node_cleaner);
        free(pathname);
        return SA_ERROR;
    }

    // sending files to client
    if( send_list_of_files(worker_no, fd_client, to_expell, true) == -1){
        list_delete(&to_expell, files_node_cleaner);
        free(pathname);
        return SA_ERROR;
    }
    list_delete(&to_expell, files_node_cleaner);

    logger("[THREAD %d] [APPEND_TO_FILE_SUCCESS] Successfully appended contents to file \"%s\".\n", worker_no, pathname);    
    logger("[THREAD %d] [APPEND_TO_FILE_SUCCESS][WB] %lu\n", worker_no, size);
    free(pathname);

    return SA_SUCCESS;
}